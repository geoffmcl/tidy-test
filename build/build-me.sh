#!/bin/sh
#< build-me.sh - 20150520 - simple build script
BN=`basename $0`
TMPLOG="bldlog-1.txt"
TMPBIN="tidy-test"
VERBOSE="0"
RUNBIN=0
TMPDAT=`date +%Y/%m/%d`
TMPTIM=`date +%H:%M:%S`

TMPTOPS=
for arg in $*; do
    TMPOPTS="$TMPOPTS $arg"
done

if [ -z "$TMPOPTS" ]; then
    # if no command line option, at least build RELEASE
    TMPOPTS="-DCMAKE_BUILD_TYPE=Release"
fi

if [ -f "$TMPLOG" ]; then
    rm -f $TMPLOG
fi

echo "$BN: Build of tidy-test on $TMPDAT $TMPTIM - output to $TMPLOG"
echo "$BN: Build of tidy-test on $TMPDAT $TMPTIM" > $TMPLOG

echo "$BN: Doing 'cmake .. $TMPOPTS'"
echo "$BN: Doing 'cmake .. $TMPOPTS'" >> $TMPLOG
cmake ..  $TMPOPTS >> $TMPLOG 2>&1
if [ ! "$?" = "0" ]; then
    echo "$BN: cmake config/gen error - see $TMPLOG"
    exit 1
fi

if [ -f "$TMPBIN" ]; then
    rm -f $TMPBIN
fi

######################################################
if [ "$VERBOSE" = "1" ];then
    echo "$BN: Doing 'make VERBOSE=1'"
    echo "$BN: Doing 'make VERBOSE=1'" >> $TMPLOG
    make VERBOSE=1 >> $TMPLOG 2>&1
else
    echo "$BN: Doing 'make'"
    echo "$BN: Doing 'make'" >> $TMPLOG
    make >> $TMPLOG 2>&1
fi
if [ ! "$?" = "0" ]; then
    echo "$BN: make error - see $TMPLOG"
    exit 1
fi
######################################################
if [ -f "$TMPBIN" ]; then
    echo "$BN: appears a successful build... running $TMPBIN -v"
    ./$TMPBIN -v
    if [ ! "$?" = "0" ]; then
        echo "$BN: error running $TMPBIN"
        exit 1
    fi
    echo "$BN: Note tidy library version"
else
    echo "$BN: Appears binary '$TMPBIN' NOT built, but a successful build..."
fi

# eof

