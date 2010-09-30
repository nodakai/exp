#! /bin/sh

echo 'CREATE TABLE syscalls(name text,linux_no integer,posix08_url text);' | \
    sqlite3 mine.sqlite3

wget http://www.opengroup.org/onlinepubs/9699919799/idx/functions.html
grep 'target="main"' functions.html | \
    while read line; do
        url=`sed<<<$line -re 's@.*href=".*/([^/"]+)".*@'$theUrl'/\1@'`
        fn=`sed<<<$line -re 's@.*<i>([^<]+)</i>.*@\1@'`
        sqlite3 mine.sqlite3 "insert into syscalls values(\"$fn\",null,\"$url\");"
    done

sqlite3 mine.sqlite3 'ALTER TABLE syscalls ADD COLUMN linux_url text;'

wget 'http://www.kernel.org/doc/man-pages/online/pages/man2/syscalls.2.html'
wget --max-redirect=10 -O syscalls.h 'http://git.kernel.org/?p=linux/kernel/git/stable/linux-2.6.35.y.git;a=blob_plain;f=include/linux/syscalls.h;hb=HEAD'
grep ^asmlinkage syscalls.h|wc -l

baseUrl="http://www.kernel.org/doc/man-pages/online/pages/man2/"
perl -ne 'print ($_) if(/------/../On many/);' syscalls.2.html| \
    grep -o 'man2/.*<'| \
    while read line; do
        url=$baseUrl`sed<<<$line -re 's@.*/(.*)".*@\1@'`
        fn=`sed<<<$line -re 's@.*>([^(]+)\(.*@\1@'`
        sqlite3 mine.sqlite3 "UPDATE syscalls SET linux_url=\"$url\" WHERE name=\"$fn\";"
    done
