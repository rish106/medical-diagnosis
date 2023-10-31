#!/usr/bin/env bash

if [[ $# -ne 2 ]]; then
    echo "Bayesnet file and data file required as arguments"
    exit 1
fi

bayesnet_filename="$1"
data_filename="$2"

./main.out "$bayesnet_filename" "$data_filename"
