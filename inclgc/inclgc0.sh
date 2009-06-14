#! /bin/bash

CPP=${INCLGC_CPP:=cpp}
CPPFLAGS=${INCLGC_CPPFLAGS:=-w -x c++}
EXTS=${INCLGC_EXTS:=h hpp hxx}

MYNAME=${0%%*/}

function printUsage {
    echo "usage: $MYNAME <path>"
}

function process {
    hfileFull=$1
    pdir=$2
    hfile=${hfileFull##*/}
    dir=${hfileFull%/*}

    includes=-I$dir `find "$pdir" -name '.*' -prune -o -type d -name include -printf ' -I%p'`
    wc1=`echo -e "#include <$hfile>" | \
            $CPP $CPPFLAGS $includes 2>/dev/null | wc -l`
    wc2=`echo -e "#include <$hfile>\\n#include <$hfile>" | \
            $CPP $CPPFLAGS $includes 2>/dev/null | wc -l`
    wc3=`echo -e "#include <$hfile>\\n#include <$hfile>\\n#include <$hfile>" | \
            $CPP $CPPFLAGS $includes 2>/dev/null | wc -l`
    if (( (wc2-wc1) * 5 < (wc3-wc1) * 3 )); then
        echo "$hfileFull: suspected lack of include guard (x2-x1=$wc2-$wc1=$((wc2-wc1)) vs. x3-x1=$wc3-$wc1=$((wc3-wc1)))"
    fi
}

function processFile {
    hfile=$1
    pdir=${hfile%/*}
    process "$hfile" "$pdir"
}

function processDir {
    pdir=$1
    pattern='-name *.'`echo $EXTS|sed -e 's/ \+/ -o -name *./g'`
    find "$pdir" $pattern \
    | while read hfile; do
        process "$pdir" "$hfile"
    done
}


if [ $# -lt 1 ]; then
    printUsage
    exit 1
fi

for a in "$@"; do
    if [ -f "$a" ]; then
        processFile "$a"
    elif [ -d "$a" ]; then
        processDir "$a"
    else
        echo "$a: \"I'm sorry. I don't know what to feel at times like this...\""
    fi
done
