#!/bin/bash

sort $1 > /tmp/temp1.txt
sort $2 > /tmp/temp2.txt
comm -3 <(cat /tmp/temp1.txt) <(cat /tmp/temp2.txt) > $3

