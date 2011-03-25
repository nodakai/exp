#! /usr/bin/ruby
# -*- coding: utf-8 -*-

ARGV.each {|fn|
    times = []
    arrs = [[], [], []] # maxs, mins, avgs
    cnt = 0
    IO.foreach(fn) {|line|
        line.chomp!
        case line
        when %r[^ *<td>\d+-(\d+)-(\d+).*(\d+):(\d+).(\d+):(\d+)</td>$]
            month, day, hb, mb, he, me = Regexp.last_match[1..6].map{|s| s.to_i}
            tb = 60 * hb + mb
            te = 60 * he + me
            tm = (tb + te) / 2
            hm = tm / 24
            mm = tm - hm * 24
            times << "%d/%d %02d:%02d" % [month, day, hm, mm]
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
