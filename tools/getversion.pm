#---------------------------------------------------------------------
# getversion.pm
# Copyright 2005 Christopher J. Madsen
#
# Get version information from configure.ac
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

# Expects "use FindBin '$Bin'":

chdir $Bin                       or die "Can't cd to $Bin: $!";
open(IN, '<', '../configure.ac') or die "Can't open configure.ac: $!";

our ($name, $major, $minor);

while (<IN>) {
  next unless /^AC_INIT\(\[\[(.+?)\]\], \[\[(\d+).(\d+(?:_\w+)?)\]\],/;
  $name = $1; $major = $2; $minor = $3;
  last;
}

close IN;

die "No version" unless defined $major;
