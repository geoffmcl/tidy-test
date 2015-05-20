#!/bin/sh
#< build-me.sh - 20150520 - simple build script
BN=`basename $0`
TMPLOG="bldlog-1.txt"
TMPBIN="tidy-test"

if [ -f "$TMPLOG" ]; then
    rm -f $TMPLOG
fi

echo "$BN: Build of tidy-test... output to $TMPLOG"
echo "$BN: Build of tidy-test..." > $TMPLOG

echo "$BN: Doing 'cmake ..'"
echo "$BN: Doing 'cmake ..'" >> $TMPLOG
cmake .. >> $TMPLOG 2>&1
if [ ! "$?" = "0" ]; then
    echo "$BN: cmake config/gen error - see $TMPLOG"
    exit 1
fi

if [ -f "$TMPBIN" ]; then
    rm -f $TMPBIN
fi

echo "$BN: Doing 'make VERBOSE=1'
echo "$BN: Doing 'make VERBOSE=1' >> $TMPLOG
make VERBOSE=1 >> $TMPLOG 2>&1

if [ ! "$?" = "0" ]; then
    echo "$BN: make error - see $TMPLOG"
    exit 1
fi
if [ ! -f "$TMPBIN" ]; then
    echo "$BN: problem - apeears binary NOT built!"
fi

echo "$BN: appears a successful build... running $TMPBIN -v"
./$TMPBIN -v
if [ ! "$?" = "0" ]; then
    echo "$BN: error running $TMPBIN"
    exit 1
fi

echo "$BN: Note tidy library version"

# eof

