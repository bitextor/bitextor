#!/bin/bash

zcat $1 | while read line; do
	tempwarc=$(mktemp)
	tempprocess=$(mktemp -d)
	tempsplitsent=$(mktemp)
	deferredhash1=$(echo $line | cut -f 6)
	deferredhash2=$(echo $line | cut -f 7)
	
	wget "$(echo $line | cut -f 1)" --warc-file $tempwarc --output-document /dev/null
	python3 bitextor-warc2preprocess.py --output-dir $tempprocess --input $tempwarc
	
	zcat $tempprocess/*/plain_text.gz | python3 bitextor-split.py --langcode $(ls $tempprocess) > $tempsplitsent
	while read linesplit; do
		senthash=$(echo -n "$linesplit" | preprocess/bin/mmhsum)
		found=""
                for partdeferredhash1 in $(echo $deferredhash1 | tr "+" "\n"); do
			if [[ "$senthash" == "$partdeferredhash1" ]]; then
				echo -n "${linesplit} "
				found="yes"
				break
			fi
		done
		if [[ "$found" != "" ]]; then
			echo -n "	"
			break
		fi
	done < $tempsplitsent
	rm -rf $temprocess
	
	wget "$(echo $line | cut -f 2)" --warc-file $tempwarc --output-document /dev/null
	python3 bitextor-warc2preprocess.py --output-dir $tempprocess --input $tempwarc
	zcat $tempprocess/*/plain_text.gz | python3 bitextor-split.py --langcode $(ls $tempprocess) > $tempsplitsent
	while read linesplit; do
		senthash=$(echo -n "$linesplit" | preprocess/bin/mmhsum)
		found=""
		for partdeferredhash2 in $(echo $deferredhash2 | tr "+" "\n"); do
			if [[ "$senthash" == "$partdeferredhash2" ]]; then
				echo "${linesplit}"
				found="yes"
				break
			fi
		done
		if [[ "$found" != "" ]]; then
                        echo -n "	"
                        break
                fi
	done < $tempsplitsent
	rm -rf $temprocess
	rm -rf $tempwarc
	rm -rf $tempsplitsent
done
