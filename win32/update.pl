#! /usr/bin/perl
#---------------------------------------------------------------------
# $Id: update.pl 4612 2005-03-22 16:14:44Z cjm $
# Copyright 2005 Christopher J. Madsen
#
# Copy the version number from configure.ac to Win32 config.h
#---------------------------------------------------------------------

use strict;
use FindBin;
use IO::All;

chdir $FindBin::Bin or die;

# Find the version number in configure.ac:

my $configure = io('../configure.ac')->tie;

my $version;

while (<$configure>) {
  if (/AC_INIT\(\[\[.+?\]\], \[\[(.+?)\]\]/) {
    $version = $1;
    last;
  }
}

die unless $version;


# Copy it to win32/config.h:

print "Found version $version, updating config.h...\n";
my $config = io('config.h');

foreach (@{$config}) {
  if (s/(#define PACKAGE_VERSION\s+)"(.*?)"/$1"$version"/) {
    if ($2 eq $version) { print "  No change needed.\n"               }
    else                { print "  Version number changed from $2!\n" }
    last;
  }
}
