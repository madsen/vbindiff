#! /usr/bin/perl -w
#---------------------------------------------------------------------
# $Id: genfile.pl 4650 2005-10-15 22:47:44Z cjm $
# Copyright 2005 Christopher J. Madsen
#
# Process files through Template Toolkit
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#--------------------------------------------------------------------

use strict;
use File::Spec::Functions 'rel2abs';
use FindBin '$Bin';
use Getopt::Long;
use Template;

#---------------------------------------------------------------------
my ($outfile, @vars);

Getopt::Long::config(qw(bundling no_getopt_compat));
GetOptions(
    'define|D=s' => \@vars,
    'output|o=s' => \$outfile,
);

# Filenames are relative to initial working directory:
foreach (@ARGV) { $_ = rel2abs($_) }

#---------------------------------------------------------------------
# Get version information from configure.ac:

our ($name, $major, $minor);
do "$Bin/getversion.pm";

#---------------------------------------------------------------------
my %data = (
   major   => $major,
   minor   => $minor,
   name    => $name,
   version => "$major.$minor",
   map { s/=(.*)// ? ($_ => $1) : ($_ => 1) } @vars
);

my $tt = Template->new({
  ABSOLUTE     => 1,
  INCLUDE_PATH => $Bin,
  EVAL_PERL    => 1,
  POST_CHOMP   => 1,
});

foreach my $file (@ARGV) {
  my $output;
  $tt->process($file, \%data, \$output);

##  print $output;
  # Get the output directive from the first non-blank line,
  # and skip the line after it:
  $output =~ s/^\s*([>|].+)\n.*\n// or die;
  # The user can override this with the --output option:
  $outfile = $1 unless defined $outfile;

  open(OUT, $outfile) or die;
  print OUT $output   or die;
  close OUT           or die;
} # end foreach $file