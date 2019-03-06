#!/usr/bin/env bash

while :
do
    scontrol update NodeName=baldur,gna,hel,loki,skaol,zisa State=resume reason='whatever'
    sleep 600
done
