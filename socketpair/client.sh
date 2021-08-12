#!/bin/bash

count=0

while read line
do
    echo "$count) - $line"
    count=$((count+1))
done

