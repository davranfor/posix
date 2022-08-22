#!/bin/bash

lines=$(journalctl -q -t "ProgramName")

# File separator = line break
IFS=$'\n'

log=$(zenity \
    --list \
    --width=640 \
    --height=480 \
    --title="Logs" \
    --column="ProgramName" \
    $lines 2>/dev/null)

# If the dialog was not canceled
if [ $? -eq 0 ]; then
    echo "$log"
fi

