#!/bin/bash
set -eu

for FILE in "${@:2}"
do
    EXPECTED=$(openssl dgst -sha3-256 -r "$FILE" | awk '{print $1}')
    RESULT=$($1 "$FILE")

    if [ "$EXPECTED" = "$RESULT" ]; then
        echo "$FILE: OK"
    else
        echo "$FILE: FAILED"
        echo "Expected: $EXPECTED"
        echo "Got: $RESULT"
    fi
done
