#!/usr/bin/env bash

CalcPages(){
    tmpfile=$(mktemp)
    #echo $tmpfile
    domain=$1
    xzcat warc/$domain/httrack.warc.xz | grep WARC-Target-URI > $tmpfile
    pages=`cat $tmpfile | wc -l`
    properPages=`cat $tmpfile | grep -v "WARC-Target-URI: unknown" | wc -l`

    echo -e "$domain\t$pages\t$properPages" > warc/$domain/page-count

    rm $tmpfile
}

for domain in `ls warc` ; do
    CalcPages $domain
done

