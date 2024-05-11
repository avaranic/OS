#!/bin/bash

keywords=("corrupted" "dangerous" "risk" "attack" "malware" "malicious")

danger=0
file=$1

lines=$(wc -l < $file)
words=$(wc -w < $file)
chars=$(wc -m < $file)

for keyword in "${keywords[@]}"; do
    if grep -q "$keyword" "$file"; then
        echo "File $file have a keyword $keyword"
        danger=1
        break
    fi
done

# if ! file -bi "$file" | grep -q "charset=us-ascii"; then
#     echo "File $file contains non-ASCII characters"
#     danger=1
# fi

if LC_ALL=C grep -q '[^[:print:]]' "$file"; then
    echo "File $file contains non-ASCII characters"
    danger=1
fi

if( [ $lines -lt 3 ] || [ $words -gt 1000 ] || [ $chars -gt 2000 ] ); then
    echo "File $file is too small or too big"
    danger=1
fi

if [ $danger -eq 1 ]; then 
    echo "File $file is DANGEROUS!!!"
    exit 1
else 
    echo "File $file is SAFE!!!"
    exit 0
fi

echo "File $1 is safe asta e scris din script"