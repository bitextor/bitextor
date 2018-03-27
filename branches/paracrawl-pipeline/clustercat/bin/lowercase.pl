#!/usr/bin/env perl
## Lowercase text
## Usage: perl lowercase.pl < input > output

binmode(STDIN, ":utf8");
binmode(STDOUT, ":utf8");

print lc while <STDIN>;
