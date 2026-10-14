#!/bin/bash
#Generate timing data for mdu running with different number of threads
#Author: Aleksander Zdunek
#Date: 2026-10-14

DIR=~
if [[ $HOSTNAME == itchy ]]; then
    DIR=/pkg
fi
echo "$HOSTNAME $DIR"

LC_NUMERIC=C
TIMEFORMAT=%R #Real time in seconds

printf "number of threads | real time (seconds)\n"
for i in {1..100}; do
    REAL_TIME=$({ time ./mdu $DIR -j $i 1>/dev/null 2>&1; } 2>&1)
    printf "%d\t%s\n" $i "$REAL_TIME"
done
