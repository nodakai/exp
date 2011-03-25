set title "Environmental Radiation Measurement Result at Shinjuku\nhttp://ftp.jaist.ac.jp/pub/emergency/monitoring.tokyo-eiken.go.jp/report/report_table.do.html"

set datafile separator ","

set xdata time
set timefmt "%m/%d %H:%M"
set format x "%d%b"
set xlabel "date"
set xrange ["3/13 00:00":*]

set yrange [0.0:0.6]
set ylabel "dose rate [uGy / h]"
set key autotitle columnhead
set bar 0

set style line 1 pt 15

set grid
set nokey
# set term png
# set output "repo.png"
plot "repo.csv" using 1:2:3:4 with yerrorbars linestyle 1, \
    "past.csv" using 1:2 linestyle 1
