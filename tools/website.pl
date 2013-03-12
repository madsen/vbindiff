#! /usr/bin/perl
#---------------------------------------------------------------------
# website.pl
# Copyright 2005 Christopher J. Madsen
#
# Generate VBinDiff documents for my website
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

chdir $Bin or die;

#---------------------------------------------------------------------
sub toWeb
{
  my $outfile = pop @_;

  $outfile =~ s!^~!$ENV{HOME}!;

  print "Writing $outfile...\n";

  system(qw(./genfile.pl -D HTML), @_, -o => "> $outfile");

} # end toWeb

#---------------------------------------------------------------------
toWeb(qw(vbindiff.pod.tt ~/web/docs/VBinDiff-curses.pod));

toWeb(qw(-D Win32 vbindiff.pod.tt ~/web/docs/VBinDiff-Win32.pod));
