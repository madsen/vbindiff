#! /usr/bin/perl
#---------------------------------------------------------------------
# $Id: website.pl 4651 2005-11-11 22:16:57Z cjm $
# Copyright 2005 Christopher J. Madsen
#
# Generate VBinDiff documents for my website
#---------------------------------------------------------------------

use strict;
use FindBin '$Bin';

chdir $Bin or die;

system(qw(./genfile.pl -D HTML vbindiff.pod.tt -o),
'| pod2html --noindex --outfile ~/comcast/out/vbindiff/VBinDiff-curses.html');

system(qw(./genfile.pl -D HTML -D Win32 vbindiff.pod.tt -o),
'| pod2html --noindex --outfile ~/comcast/out/vbindiff/VBinDiff-Win32.html');
