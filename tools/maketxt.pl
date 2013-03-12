#! /usr/bin/perl
#---------------------------------------------------------------------
# maketxt.pl
# Copyright 2005 Christopher J. Madsen
#
# Generate FFV.txt from FFV.pod
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
use Pod::Text;

my ($outfile) = @ARGV;

#---------------------------------------------------------------------
# Format FFV.pod using Pod::Text:

my $p = Pod::Text->new(loose => 1, indent => 4, width => 74, sentence => 1);

my $text;

open(OUT, '>', \$text) or die;

$p->parse_from_filehandle(\*STDIN, \*OUT);

close OUT;

#---------------------------------------------------------------------
# Now perform fixups:

# Fixup NAME section:
$text =~ s/^NAME\s+//;

# Format section headings:
#$text =~ s/^(?:  )?([A-Z ]+)$/ sprintf "\n%s %s", $1, ('-' x (69 - length $1))/meg;
$text =~ s/^(?:  )?([A-Z].*)$/ sprintf "\n%s %s", $1, ('-' x (69 - length $1))/meg;

# Only one blank line before SYNOPSIS
$text =~ s/\n(?=SYNOPSIS)//;

# Shift text back to left margin:
$text =~ s/^    //mg;
$text =~ s/^ (?=\S| *--)//mg;

# Remove any leftover trailing spaces:
$text =~ s/ +$//mg;

#---------------------------------------------------------------------
open(OUT, '>', $outfile) or die;

print OUT $text;

close OUT;
