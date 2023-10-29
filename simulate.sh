#!/usr/bin/env bash

rm -f *.out
rm -f solved_alarm.bif

./compile.sh &&

./run.sh alarm.bif records.dat &&

g++ format_checker.cpp -o checker.out &&
./checker.out
