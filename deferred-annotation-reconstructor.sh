#!/bin/bash

tempcache=$(mktemp -d)

zcat -f $1 | while read line; do
	tempwarc1=$(mktemp)
	tempwarc2=$(mktemp)
	tempprocess1=$(mktemp -d)
	tempprocess2=$(mktemp -d)
	deferredhash1=$(echo "$line" | cut -f 6)
	deferredhash2=$(echo "$line" | cut -f 7)
	url1=$(echo "$line" | cut -f 1)
	url2=$(echo "$line" | cut -f 2)
	tempsplitsent1="$tempcache/$(echo $url1 | preprocess/bin/mmhsum)"
	tempsplitsent2="$tempcache/$(echo $url2 | preprocess/bin/mmhsum)"
	echo -n "$url1	$url2"
	
	
	(if [ ! -f $tempsplitsent1 ]; then
		if [ "$4" != "" ]; then
			> ${tempwarc1}.warc.gz
			for id in $(warcio index $4 | grep -i "$url1" | cut -f 1 -d ',' | cut -f 2 -d ':' | cut -f 2 -d '"'); do warcio extract $4 $id | gzip >> ${tempwarc1}.warc.gz; done
	
		else
			wget "$url1" --quiet --warc-file $tempwarc1 --output-document /dev/null
		fi
	
		warc2text/bin/warc2text -o $tempprocess1 ${tempwarc1}.warc.gz 2> /dev/null
		
		zcat $tempprocess1/*/text.gz | python3 bitextor-split.py --langcode $2 > $tempsplitsent1
	fi) &
	
	(if [ ! -f $tempsplitsent2 ]; then
		if [ "$5" != "" ]; then
			> ${tempwarc2}.warc.gz
			for id in $(warcio index $5 | grep -i "$url2" | cut -f 1 -d ',' | cut -f 2 -d ':' | cut -f 2 -d '"'); do warcio extract $5 $id | gzip >> ${tempwarc2}.warc.gz; done
		else
			wget "$url2" --quiet --warc-file $tempwarc2 --output-document /dev/null
		fi
	
		warc2text/bin/warc2text -o $tempprocess2 ${tempwarc2}.warc.gz 2> /dev/null
		
		zcat $tempprocess2/*/text.gz | python3 bitextor-split.py --langcode $3 > $tempsplitsent2
	fi) &

	wait

	rm -rf $tempprocess1 $tempprocess2 $tempwarc1 $tempwarc2 ${tempwarc1}.warc.gz ${tempwarc2}.warc.gz

	echo -n "	"
	while read linesplit; do
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
	done < <(cat $tempsplitsent1 | base64 -d)


	echo -n "	"
	while read linesplit; do
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
	done < <(cat $tempsplitsent2 | base64 -d)
	echo -n "	"	
	echo -n "$line" | cut -f 5-

done

rm -rf $tempcache
