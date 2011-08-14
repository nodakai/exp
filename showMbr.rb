#! /usr/bin/ruby
# -*- coding: utf-8 -*-
# ex: set fileencoding=utf-8 :

require 'tmpdir'
require 'tempfile'

def analyzeMasterBootstrapLoader(masterBootstrapLoader)
    return
    Tempfile.open('firstSector') do |tf|
        tf.write masterBootstrapLoader
        tf.flush
        disas = IO.popen("ndisasm #{tf.path}", 'r+') {|pipe| pipe.readlines}
        first = true
        skip = true
        disas.each do |line|
            if first
                print line
                puts '...'
                first = false
            else
                skip = false if skip and line.chomp =~ / cli$/
                print line unless skip
            end
        end
    end
end

def getChs(byteArr)
    h = byteArr[0]
    s = byteArr[1] & ~(~0 << 6)
    c = (byteArr[1] >> 6) | byteArr[2]
    [c, h, s]
end

def analyzePartitionTable(partitionTable, name=nil)
    puts "Analyzing the partition table ``#{name}''..." if name
    bootable = partitionTable[0]
    case bootable
    when 0x00
        puts 'not bootable'
    when 0x80
        puts 'bootable'
    else
        puts "bootable flag is #{bootable} rather than 0x00 nor 0x80"
    end

    chsBegin = partitionTable[1,3]
    cylindersB, headsB, sectorsB = getChs chsBegin
    chsEnd = partitionTable[1 + 3 + 1, 3]
    cylindersE, headsE, sectorsE = getChs chsEnd
    puts "C/H/S == #{cylindersB}/#{headsB}/#{sectorsB} .. #{cylindersE}/#{headsE}/#{sectorsE}"

    type = partitionTable[1 + 3]
    printf("type == 0x%x\n", type)

    lbaB = partitionTable[1 + 3 + 1 + 3, 4].pack('C*').unpack('V')[0]
    lbaS = partitionTable[1 + 3 + 1 + 3 + 4, 4].pack('C*').unpack('V')[0]
    puts "LBA: #{lbaB}, +#{lbaS}"
end

Ruby19Flag = RUBY_VERSION >= '1.9.0'
devPath = ARGV.length > 0 ? ARGV[0] : '/dev/sda'
openArg = 'r' + (Ruby19Flag ? ':ASCII-8BIT' : '')
File::open(devPath, openArg)  do |devFile|
    firstSector = devFile.read 512

    signature = firstSector[446 + 16 * 4..-1]
    refSig = "\x55\xAA"
    raise "#{signature.inspect} is different from #{refSig.inspect}" unless signature == refSig

    masterBootstrapLoader = firstSector[0...446]
    analyzeMasterBootstrapLoader masterBootstrapLoader

    partitionTables = []
    4.times do |i|
#   1.times do |i|
        pt = firstSector[446 + 16 * i, 16].unpack('C*')
        partitionTables << pt
        analyzePartitionTable(pt, "#{i}-th partition")
    end
end
