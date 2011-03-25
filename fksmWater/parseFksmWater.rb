#! /usr/bin/ruby
# -*- coding: utf-8 -*-

data = {}

ARGV.each { |f|
    times = []
    days = []
    kaimes = []
    isotopes = [[]]

    open(f) { |f|
        f.each { |line|
            line.chomp!
            case line
            when /^(\d+):(\d+)/
                hour = $1.to_i
                min = $2.to_i
                minofday = hour * 24 + min
                times << line# minofday
            when /^(\d+)月(\d+)日/
                month = $1.to_i
                day = $2.to_i
                days << "#{month}/#{day}"
            when /^(\d+)回目/
                kaimes << $1.to_i
            when /^(\d+|ND)$/
                if isotopes[-1].size == kaimes.size
                    isotopes << []
                end
            isotopes[-1] << line
            else
#               STDERR.puts "line is <#{line}>" if $DEBUG
            end
        }
    }

    if $DEBUG
        STDERR.puts times.inspect
        STDERR.puts isotopes.inspect
        STDERR.puts days.inspect
        STDERR.puts isotopes.inspect
    end

    isotopes = isotopes.transpose
    d = nil
    times.zip(kaimes) {|t,k|
        if k == 1
            d = days.shift
        end
        isos = isotopes.shift
        dt = "#{d} #{t}"
        STDERR.puts ([dt] + isos).join(',') if $DEBUG
        if data[dt] and data[dt] != isos
            puts "WARNING: data[#{dt}]==#{data[dt]} := #{isos}"
        end
        data[dt] = isos
    }
}

puts 'time,I-131,Cs-134,Cs-137'
data.each {|k,v|
    puts "#{k},#{v.join(',')}"
}
