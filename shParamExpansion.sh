#! /bin/sh

echo 'No. of args: $#='$#
echo -n 'Bare args: $*='
for i in $*; do
    echo -n \"$i\" ' '
done
echo
echo -n 'Quotes-aware args (can contain IFS): "$@"='
for i in "$@"; do
    echo -n \"$i\" ' '
done
echo
echo

a=/var/www/html/index.html
echo '$a='$a
echo 'basename: ${a##*/}='${a##*/}
echo 'extension: ${a##*.}='${a##*.}
