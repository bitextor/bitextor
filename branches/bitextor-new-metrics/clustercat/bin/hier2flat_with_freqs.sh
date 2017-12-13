#!/bin/sh
## By Jon Dehdari, 2015, public domain
## Converts Percy Liang's brown-cluster hierarchical cluster labels to flat labels, starting with 0

awk '
	BEGIN {
		clus=2
		nclus=-1
	}
	{
		if (clus != $1) {
			clus=$1
			nclus++
		}
		print $2 "\t" nclus "\t" $3
	}
	
'
