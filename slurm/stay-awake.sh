#!/usr/bin/env bash

while :
do
    scontrol update NodeName=gna,hel,loki,skaol,syn,zisa State=resume reason='whatever'
    sleep 600
done
