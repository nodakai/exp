package main

import (
	"fmt"
	"os"
	"time"
)

func main() {
	var buf [10]byte

	readch := make(chan []byte)
	go func() {
		os.Stdin.Read(buf[:])
		readch <- buf[:]
	}()

	fmt.Println("Start")
	select {
	case <-time.After(1500 * time.Millisecond):
		fmt.Println("Timeout")
	case <-readch:
		fmt.Printf("%#v\n", buf)
	}
	fmt.Println("Stop")
}
