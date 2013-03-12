#! /usr/bin/perl
#---------------------------------------------------------------------
# zipdist.pl
# Copyright 2005 Christopher J. Madsen
#
# Generate the Win32 distribution ZIP file
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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#---------------------------------------------------------------------

use strict;
use FindBin '$Bin';
use IO::All;

$| = 1;                         # Disable buffering

my @unlink;

#---------------------------------------------------------------------
sub getFrom
{
  my ($from, $to) = @_;

  return if -e $to and not -e $from;

  Win32::CopyFile($from, $to, 0) or die "Can't copy $from to $to\n";
  push @unlink, $to;
} # end getFrom

#---------------------------------------------------------------------
sub makeZip
{
  my $fn   = shift @_;
  my $opts = shift @_;

  die "$fn exists" if -e $fn;

  print "Creating $fn...\n";

  open(ZIP, "| zip -9X\@$opts $fn") or die;

  foreach (@_) {
    die "$_ not found" unless -e $_;
    print ZIP "$_\n"
  }

  close ZIP or die;
} # end makeZip

#---------------------------------------------------------------------
# Get version from configure.ac:

our ($name, $major, $minor);
do "$Bin/getversion.pm";

my $zip = sprintf "VBinDiff-%s.%s.zip", $major, $minor;

# Read Makefile.am:

chdir "$Bin/.." or die "Can't cd to $Bin/..: $!";

die "$zip exists" if -e $zip;

my $conf = io('Makefile.am')->scalar;

$conf =~ s/[ \t]+\\\n[ \t]*/ /g; # Merge up continued lines

# Parse out the source files:

my @files = qw(Makefile.am aclocal.m4 config.h.in configure.ac);
while ($conf =~ /^\w+_(?:SOURCES|DIST) +=(.+)/mg) {
  push @files, split(' ', $1);
}

# Zip up the source files:

makeZip('Source.zip', '', @files);
push @unlink, 'Source.zip';

# Generate documentation:

system qw(perl tools/genfile.pl -D Win32 tools/vbindiff.pod.tt tools/ReadMe.tt);

getFrom('AUTHORS', 'AUTHORS.txt');
getFrom('\emacs\21.3\etc\COPYING', 'COPYING.txt');

# Create ZIP file:

makeZip($zip, 'j', qw(AUTHORS.txt COPYING.txt
                      win32/Release/VBinDiff.exe VBinDiff.txt
                      ReadMe.txt Source.zip));

# Clean up:

unlink @unlink;
