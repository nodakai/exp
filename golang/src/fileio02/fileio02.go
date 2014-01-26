package main

import (
	"bufio"
	"fmt"
	"io"
	"log"
	"os"
)

func main() {
	infile, err := os.Open("/proc/cpuinfo")
	if err != nil {
		log.Fatal(err)
	}
	defer infile.Close()

	outfile, err := os.Create("/tmp/tmp.txt")
	if err != nil {
		log.Fatal(err)
	}
	defer outfile.Close()

	i := 0
	for rd := bufio.NewReader(infile); ; i++ {
		ln, er := rd.ReadString('\n')
		_, ew := outfile.WriteString(ln)
		if ew != nil {
			log.Fatal(ew)
		}

		if er != nil {
			if er != io.EOF {
				log.Fatal(er)
			}
			break
		}
	}

	fmt.Printf("%d lines.\n", i)
}
