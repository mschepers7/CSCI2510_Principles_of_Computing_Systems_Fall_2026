#!/bin/bash

FILE="$1"

echo "LINES: $(wc -l < "$FILE")"
echo
echo "STATUS_COUNTS:"
awk '{print $2}' "$FILE" | sort | uniq -c | sort -k1,1nr | awk '{print $2, $1}'