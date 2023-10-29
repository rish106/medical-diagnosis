#!/usr/bin/env bash

./compile.sh &&

./run.sh alarm.bif records.dat &&

./checker.sh
