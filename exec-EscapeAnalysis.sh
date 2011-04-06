# /bin/sh
set -x
java -version
time java -verbose:gc -XX:+DoEscapeAnalysis EscapeAnasysis
time java -verbose:gc EscapeAnasysis
