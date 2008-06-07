#! /usr/bin/perl
#---------------------------------------------------------------------
# $Id: website.pl 4739 2008-06-07 20:37:26Z cjm $
# Copyright 2005 Christopher J. Madsen
#
# Generate VBinDiff documents for my website
#---------------------------------------------------------------------

use strict;
use FindBin '$Bin';

chdir $Bin or die;

#---------------------------------------------------------------------
sub toWeb
{
  my $outfile = pop @_;

  $outfile =~ s!^~!$ENV{HOME}!;

  system(qw(./genfile.pl -D HTML), @_, -o => "> $outfile");

} # end toWeb

#---------------------------------------------------------------------
toWeb(qw(vbindiff.pod.tt ~/web/docs/VBinDiff-curses.pod));

toWeb(qw(-D Win32 vbindiff.pod.tt ~/web/docs/VBinDiff-Win32.pod));
