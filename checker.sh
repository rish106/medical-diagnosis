#!/usr/bin/env bash

g++-13 -ld_classic format_checker.cpp -o checker.out &&
./checker.out
