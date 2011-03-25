set title "Environmental Radiation Measurement Result at Shinjuku"

set datafile separator ","

set xdata time
set timefmt "%m/%d %H:%M"
set format x "%d%b"
set xlabel "date"
set xrange ["3/13 00:00":*]

set yrange [0:1]
set ylabel "dose rate [uGy / h]"
set key autotitle columnhead
set bar 0

set bmargin 5
set label "Shinjuku: http://ftp.jaist.ac.jp/pub/emergency/monitoring.tokyo-eiken.go.jp/report/report_table.do.html\nand http://ftp.jaist.ac.jp/pub/emergency/monitoring.tokyo-eiken.go.jp/monitoring/past_data.html" \
    at screen 0.05, screen 0.05 font "/usr/share/fonts/truetype/ttf-liberation/LiberationSans-Regular.ttf,9"

set style line 1 pt 15 linetype rgb "red"
set style line 2 pt 14 linetype rgb "red"

set grid
set term png
set output "shinjuku.png"
plot "repo.csv" using 1:2:3:4 with yerrorbars linestyle 1 title "Shinjuku", \
    "past.csv" using 1:2 linestyle 2 notitle
