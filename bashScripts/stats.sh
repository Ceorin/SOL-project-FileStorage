#!/bin/bash
echo "$0 not really implemented yet"
if [ $# -eq 0 ] ; then
    echo "No log file given"
else
    echo "\"Reading\" log file:"
    cat $1
fi