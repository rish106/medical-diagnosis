#!/usr/bin/env zsh

rm -f *.out
rm -f solved_alarm.bif

./compile.sh &&
g++ format_checker.cpp -o checker.out &&

cnt=0
THRESHOLD=17
i=0
MAX_ITERATIONS=100
while [[ $i -lt $MAX_ITERATIONS ]]; do
    ./run.sh alarm.bif records.dat > /dev/null
    score=$(./checker.out | grep "Score" | awk '{print $3}')
    if [[ $((score)) -gt $THRESHOLD ]]; then
        cnt=$((cnt + 1))
    fi
    sleep 1
    i=$((i + 1))
    echo -n "$score "
done

echo ""
echo "$cnt scores out of $i are greater than $THRESHOLD"
