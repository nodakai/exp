set title "Radio-isotopes detected in tap water in Fukushima\nhttp://www.pref.fukushima.jp/j/index.htm"

set datafile separator ","

set xdata time
set timefmt "%m/%d %H:%M"
set format x "%d%b"
set xlabel "date"

set yrange [0:200]
set ylabel "radioactivity [Bq / L]"
set key autotitle columnhead

set grid
set style data linespoints
set term png
set output "fksmWater.png"
plot "fksmWater.csv" using 1:2, "" using 1:3, "" using 1:4

