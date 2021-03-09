#!/usr/bin/env bash

CalcPages(){
    domain=$1
    echo Processing $domain

    tmpfile=$(mktemp)
    #echo $tmpfile
    xzcat $domain/httrack.warc.xz | grep WARC-Target-URI > $tmpfile
    pages=`cat $tmpfile | wc -l`
    properPages=`cat $tmpfile | grep -v "WARC-Target-URI: unknown" | wc -l`

    echo -e "$domain\t$pages\t$properPages" >> page-count

    rm $tmpfile
}

for domain in `find warc -type d | grep -ve "warc$"` ; do
    CalcPages $domain
done

