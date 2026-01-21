#!/bin/bash

FILE="$1"
EVENT="$2"
HOST=$(hostname)
TIME=$(date)

#Not yet functional

echo -e "Watchdog Alert\n\nHost: $HOST\nTime: $TIME\nEvent: $EVENT\nFile: $FILE" \
| mail -s "[WATCHDOG] Canary tripped on $HOST" 
