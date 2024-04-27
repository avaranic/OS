#!/bin/bash

keywords=("corrupted" "dangerous" "risk" "attack" "malware" "malicious")

danger=0

for keyword in "${keywords[@]}"; do
    if grep -q "$keyword" "$1"; then
        echo "File $1 is dangerous"
        danger=1
        break
    fi
done

if [ $danger -eq 1 ]; then 
    exit 1
else 
    exit 0
fi

echo "File $1 is safe asta e scris din script"
