#! /bin/sh

CPP=${INCLGC_CPP:=cpp}
CPPFLAGS=${INCLGC_CPPFLAGS:=-w -x c++}

function printUsage {
    echo "usage: ${0%%*/} <path-to-dir>"
}

function processFile {
    hfile=$1
    basename=${hfile##*/}
    dir="${hfile%/*}"
    includes=`find "$dir" -name '.*' -prune -o -type d -name include -printf ' -I%p'`
    wc1="`echo -e '#include <'$basename'>\n' |
            $CPP $CPPFLAGS -I$dir $includes 2>/dev/null | wc -l`"
    wc2="`echo -e '#include <'$basename'>\n#include <'$basename'>\n' |
            $CPP $CPPFLAGS -I$dir $includes 2>/dev/null | wc -l`"
    if (( wc1 * 5 < wc2 * 3 )); then
        echo "$hfile: suspected lack of include guard (once=$wc1 vs. twice=$wc2)"
    fi
}

function processDir {
    dir="$1"
    includes=`find "$dir" -name '.*' -prune -o -type d -name include -printf ' -I%p'`
#    echo $includes
    find "$dir" -name '*.h' -o -name '*.hpp' -o -name '*.hxx' \
    | while read hfile; do
        #echo -I${hfile%/*} $includes
        basename=${hfile##*/}
        fdir=${hfile%/*}
        wc1="`echo -e '#include <'$basename'>\n' |
                $CPP $CPPFLAGS -I$fdir $includes 2>/dev/null | wc -l`"
        wc2="`echo -e '#include <'$basename'>\n#include <'$basename'>\n' |
                $CPP $CPPFLAGS -I$fdir $includes 2>/dev/null | wc -l`"
        if (( wc1 * 5 < wc2 * 3 )); then
            echo "$hfile: suspected lack of include guard (once=$wc1 vs. twice=$wc2)"
        fi
    done
}

function main {
    for a in "$@"; do
        if [ -f $a ]; then
            processFile "$a"
        elif [ -d $a ]; then
            processDir "$a"
        else
            echo $'Ayanami: "I\'m sorry. I don\'t know what to feel at times like' "$a" '..."'
        fi
    done
}

if [ $# -lt 1 ]; then
    printUsage
    exit 1
fi

main "$@"
