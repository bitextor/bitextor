#!/usr/bin/env bash

while :
do
    scontrol update NodeName=baldur,elli,gna,hel,loki,skaol,sigyn,syn,zisa State=resume reason='whatever'
    #scontrol update NodeName=hel,loki,skaol,syn State=resume reason='whatever'
    sleep 600
done
