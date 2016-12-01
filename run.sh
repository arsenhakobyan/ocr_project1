#!/bin/bash

for i in tests/input*
do
    rm -f output.json
    echo $i
    ./run.py $i 0
done
