#!/bin/bash

i=22
while ((i-->0)); do
    f=sokuteichiinryousui${i}.pdf
    curl -O "http://www.pref.fukushima.jp/j/$f"
    if grep -q 404 $f; then
        rm $f
        break
    fi
done
