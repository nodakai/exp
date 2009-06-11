#!/bin/sh

if [ $# -lt 1 -o ! -f "$1" ]; then
    echo "usage: ${0##*/} <filename>  (note: the filename must contain the problem id)"
    exit 1
fi
sourceCode=$1
problemId=`echo $sourceCode \
    | sed -r -e 's/[^[:digit:]]*([[:digit:]]+).*/\1/' -e 's/^0*//'`
case ${sourceCode##*.} in
   c ) langId=0 ;;
   cpp ) langId=1 ;;
   cxx ) langId=1 ;;
   java ) langId=2 ;;
   * ) echo Unknown language.  Exitting ...; exit 1;;
esac

idFile=id.txt # name=<name>&pswd=<password> (without EOL; set binary noeol if you are a vimmer)
loginResponceSaveFile=login0.txt # TODO: mktemp?
submitContentFile=submitted0.txt
submitResponceSaveFile=submit0.txt
mjudge=http://m-judge.maximum.vc
loginCgi=$mjudge/login.cgi
submitCgi=$mjudge/submit.cgi

# wgetOpt='-d' # debug
wgetOpt='-q' # quiet

lastLogin=`ls -l --time-style=+%s $loginResponceSaveFile 2>/dev/null \
    |cut -d' ' -f6` # UNIX time
now=`date +%s`
hour=3600
if (( $now > $lastLogin + 12*$hour )) ; then
    echo -n Logging in ...
    wget $wgetOpt --post-data=`sed -r -e 's/[\r\n]//' $idFile` \
        --save-headers -O $loginResponceSaveFile $loginCgi
else
    echo -n Reading cookie ...
fi
success=`grep -i -m 1 'success' $loginResponceSaveFile`
if [ x"$success" = x ]; then
    echo failed.
    echo Exitting ...
    exit 1
fi
echo done.

sessionCookie=`grep -m 1 Set-Cookie login0.txt \
    | sed -r -e 's/^.*(_session_id=[^;]+);.*$/\1/'`
boundary=boundary`od -t x8 -N 8 /dev/urandom \
    |head -n 1|cut -d' ' -f2`boundary
if [ -f $submitContentFile ]; then
    mv $submitContentFile $submitContentFile.bak
fi
cat >> $submitContentFile <<EOS
--$boundary
Content-Disposition: form-data; name="m"

1
--$boundary
Content-Disposition: form-data; name="pid"

$problemId
--$boundary
Content-Disposition: form-data; name="lang"

$langId
--$boundary
Content-Disposition: form-data; name="code"

EOS
cat $sourceCode >> $submitContentFile
echo --$boundary-- >> $submitContentFile
nkf --in-place -Lw $submitContentFile
echo -n Submitting your code...
wget $wgetOpt --header="Cookie: $sessionCookie" \
    --header="Content-Type: multipart/form-data; boundary=$boundary" \
    --post-file=$submitContentFile -O $submitResponceSaveFile \
    $submitCgi?pid=$problemId
success=`grep -i -m 1 'success' $submitResponceSaveFile`
if [ x"$success" == x ]; then
    echo failed.
    echo Exitting ...
    exit 1
fi
echo done.
touch $loginResponceSaveFile
