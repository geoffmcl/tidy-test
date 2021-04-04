#!/bin/sh
#< build-me.sh - 20150520 - simple build script
BN=`basename $0`
TMPLOG="bldlog-1.txt"
TMPBIN="tidy-test"
VERBOSE="0"
RUNBIN=0
TMPDAT=`date +%Y/%m/%d`
TMPTIM=`date +%H:%M:%S`

wait_for_input()
{
    if [ "$#" -gt "0" ] ; then
        echo "$1"
    fi
    echo -n "Enter y to continue : "
    read char
    if [ "$char" = "y" -o "$char" = "Y" ]
    then
        echo "Got $char ... continuing ..."
    else
        if [ "$char" = "" ] ; then
            echo "Aborting ... no input!"
        else
            echo "Aborting ... got $char!"
        fi
        exit 1
    fi
    # exit 0
}


BLDDBG=0
TMPSRC=".."
TMPOPTS=""
##############################################
### ***** NOTE THIS INSTALL LOCATION ***** ###
### Change to suit your taste, environment ###
TMPINST="/usr/local"
TMPOPTS="$TMPOPTS -DCMAKE_INSTALL_PREFIX=$TMPINST"
##############################################

### Accept user argument
for arg in $@; do
      case $arg in
         VERBOSE) TMPOPTS="$TMPOPTS -DCMAKE_VERBOSE_MAKEFILE=ON" ;;
         DEBUG) BLDDBG=1 ;;
         SHARED) TMPOPTS="$TMPOPTS -DBUILD_SHARED_LIB:BOOL=TRUE" ;;
         *) TMPOPTS="$TMPOPTS $arg" ;;
      esac
done

if [ "$BLDDBG" = "1" ]; then
    TMPOPTS="$TMPOPTS -DCMAKE_BUILD_TYPE=Debug -DENABLE_DEBUG_SYMBOLS:BOOL=TRUE"
else
    TMPOPTS="$TMPOPTS -DCMAKE_BUILD_TYPE=Release"
fi

echo "$BN: Will do: 'cmake $TMPSRC $TMPOPTS' to $BLDLOG"
wait_for_input


if [ -f "$TMPLOG" ]; then
    rm -f $TMPLOG
fi

echo "$BN: Build of tidy-test on $TMPDAT $TMPTIM - output to $TMPLOG"
echo "$BN: Build of tidy-test on $TMPDAT $TMPTIM" > $TMPLOG

echo "$BN: Doing 'cmake $TMPSRC $TMPOPTS'"
echo "$BN: Doing 'cmake $TMPSRC $TMPOPTS'" >> $TMPLOG
cmake $TMPSRC  $TMPOPTS >> $TMPLOG 2>&1
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

