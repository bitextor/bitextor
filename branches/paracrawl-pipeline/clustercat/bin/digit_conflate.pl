#!/usr/bin/env perl
## By Jon Dehdari 2013
## Conflates all digits to the same digit
## Usage: perl digit_conflate.pl [options] < in > out

use strict;
use Getopt::Long;

## Defaults
my $digit = 5;

my $usage     = <<"END_OF_USAGE";
digit_conflate.pl    (c) 2013 Jon Dehdari - LGPL v3

Usage:    perl $0 [options] < in > out

Function: Conflates all digits to the same digit
          For example, "12,629.24" -> "55,555.55"

Options:
 -h, --help        Print this usage
 -d, --digit <n>   Set output digit to <n> (default: $digit)

END_OF_USAGE

GetOptions(
    'h|help|?'		=> sub { print $usage; exit; },
    'd|digit=i'		=> \$digit,
) or die $usage;


while (<>) {
	s/\d/$digit/g;
	print;
}
