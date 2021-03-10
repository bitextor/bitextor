#!/bin/bash

zcat $1 | while read line; do
	tempwarc=$(mktemp)
	tempprocess=$(mktemp -d)
	tempsplitsent=$(mktemp)
	deferredhash1=$(echo "$line" | cut -f 6)
	deferredhash2=$(echo "$line" | cut -f 7)
	url1=$(echo "$line" | cut -f 1)
	url2=$(echo "$line" | cut -f 2)
	echo -n "$url1	$url2"
	
	
	
	wget "$url1" --quiet --warc-file $tempwarc --output-document /dev/null
	warc2text/bin/warc2text -o $tempprocess ${tempwarc}.warc.gz 2> /dev/null
	
	zcat $tempprocess/*/text.gz | python3 bitextor-split.py --langcode $2 > $tempsplitsent

	echo -n "	"
	cat $tempsplitsent | base64 -d | while read linesplit; do
		senthash=$(echo -n "$linesplit" | preprocess/bin/mmhsum)
		found1=""
                for partdeferredhash1 in $(echo "$deferredhash1" | tr "+" "\n"); do
			if [[ "$senthash" == "$partdeferredhash1" ]]; then
				echo -n "${linesplit}"
				found1="yes"
				break
			fi
		done
		if [[ "$found1" != "" ]]; then
			break
		fi
	done
	rm -rf $tempprocess



	wget "$url2" --quiet --warc-file $tempwarc --output-document /dev/null
	warc2text/bin/warc2text -o $tempprocess ${tempwarc}.warc.gz 2> /dev/null
	
	zcat $tempprocess/*/text.gz | python3 bitextor-split.py --langcode $3 > $tempsplitsent
	
	echo -n "	"
	cat $tempsplitsent | base64 -d | while read linesplit; do
		senthash=$(echo -n "$linesplit" | preprocess/bin/mmhsum)
		found2=""
		for partdeferredhash2 in $(echo "$deferredhash2" | tr "+" "\n"); do
			if [[ "$senthash" == "$partdeferredhash2" ]]; then
				echo -n "${linesplit}"
				found2="yes"
				break
			fi
		done
		if [[ "$found2" != "" ]]; then
                        break
                fi
	done
	rm -rf $tempprocess
	echo -n "	"	
	echo -n "$line" | cut -f 5-

	rm -rf $tempwarc
	rm -rf ${tempwarc}.warc.gz
	rm -rf $tempsplitsent
done
