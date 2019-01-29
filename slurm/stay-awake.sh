#!/usr/bin/env bash

while :
do
    scontrol update NodeName=elli,gna,hel,loki,skaol,syn,vali,zisa State=resume reason='whatever'
    sleep 600
done
