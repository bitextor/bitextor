#!/usr/bin/perl -w 

# sntcooc.perl [-sort-buffer-size 200M] [-sort-batch-size 253] [-sort-compress gzip] output vcb1 vcb2 snt12 
#
# This file is part of mgiza++.  Its use is licensed under the GNU General
# Public License version 2 or, at your option, any later version.

use strict;
use File::Basename;
use FindBin qw($Bin);

sub systemCheck($);

my $sortArgs = "";
for (my $i = 0; $i < (@ARGV - 4); ++$i)
{
  my $arg = $ARGV[$i];
  if ($arg eq "-sort-buffer-size")
  {
		$sortArgs .= " -S " .$ARGV[++$i];
  }
  elsif ($arg eq "-sort-batch-size")
  {
	  $sortArgs .= " --batch-size " .$ARGV[++$i];
  }
  elsif ($arg eq "-sort-compress")
  {
	  $sortArgs .= " --compress-program " .$ARGV[++$i];
  }
}
					
my $out		= $ARGV[@ARGV - 4];
my $vcb1	= $ARGV[@ARGV - 3];
my $vcb2	= $ARGV[@ARGV - 2];
my $snt12	= $ARGV[@ARGV - 1];

my $SORT_EXEC = `gsort --help 2>/dev/null`; 
if($SORT_EXEC) {
  $SORT_EXEC = 'gsort';
}
else {
  $SORT_EXEC = 'sort';
}

my $TMPDIR=dirname($out);

my $cmd;
$cmd = "$Bin/snt2coocrmp $vcb1 $vcb2 $snt12 ";
$cmd .= "| $SORT_EXEC $sortArgs -T $TMPDIR -nk 1 -nk 2 | uniq > $out";
systemCheck($cmd);

#############################

sub systemCheck($)
{
  my $cmd = shift;
	print STDERR "Executing $cmd \n";
	
  my $retVal = system($cmd);
  if ($retVal != 0)
  {
    exit(1);
  }
}

