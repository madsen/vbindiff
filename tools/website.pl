#! /usr/bin/perl
#---------------------------------------------------------------------
# $Id: website.pl 4687 2007-03-19 03:09:45Z cjm $
# Copyright 2005 Christopher J. Madsen
#
# Generate VBinDiff documents for my website
#---------------------------------------------------------------------

use strict;
use FindBin '$Bin';
use IO::All;

chdir $Bin or die;

#---------------------------------------------------------------------
sub toHTML
{
  my $outfile = pop @_;

  $outfile =~ s!^~!$ENV{HOME}!;

  system(qw(./genfile.pl -D HTML), @_,
         -o => "| pod2html --noindex --outfile $outfile");

  system(qw(tidy -q -m -ashtml), $outfile);

  my $line = io($outfile);

  $line->[0] = <<'';
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
      "http://www.w3.org/TR/html4/loose.dtd">

  foreach (@$line) {
    s/id="[^"_]*_[^"]*"//g; # Get rid of invalid id's inserted by tidy
  }
} # end toHTML

#---------------------------------------------------------------------
toHTML(qw(vbindiff.pod.tt ~/web/out/vbindiff/VBinDiff-curses.html));

toHTML(qw(-D Win32 vbindiff.pod.tt ~/web/out/vbindiff/VBinDiff-Win32.html));
