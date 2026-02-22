#!/bin/bash

FILE="$1"
EVENT="$2"
EMAIL="$3"

HOST=$(hostname)
TIME=$(date)

MESSAGE="WatchDog Alert

Host: $HOST
Time: $TIME
Event: $EVENT
File: $FILE
"

logger "WatchDog alert: $EVENT on $FILE"

# If email provided and mail exists, send it
if [ -n "$EMAIL" ] && command -v mail >/dev/null 2>&1; then
    echo "$MESSAGE" | mail -s "[WATCHDOG] Canary tripped on $HOST" "$EMAIL"
fi
