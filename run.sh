#!/usr/bin/env bash

if [[ $# -ne 2 ]]; then
    echo "Bayesnet file and data file required as arguments"
    exit 1
fi

bayesnet_file="$1"
data_file="$2"

./main.out "$bayesnet_file" "$data_file"
