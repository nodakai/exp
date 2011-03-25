#! /usr/bin/ruby
# -*- coding: utf-8 -*-

ARGV.each {|fn|
    times = []
    arrs = [[], [], []] # maxs, mins, avgs
    cnt = 0
    IO.foreach(fn, :external_encoding => Encoding.find("CP932")) {|line|
        line.chomp!
        case line
        when %r[^ *<td [^>]+>\d+/(\d+)/(\d+)</td>$]
            month = $1.to_i
            day = $2.to_i
            times << "%d/%d 12:00" % [month, day]
        when %r[^ *<td [^>]+>([^<]+)</td>$]
            val = $1.to_f
            arrs[cnt] << val
            cnt = (cnt + 1) % 3
        end
    }
    arrs = arrs.reverse.transpose
    puts "date,avg,min,max"
    times.zip(arrs).each {|r| puts r.flatten.join(',')}
}
