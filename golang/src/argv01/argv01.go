package main

import (
	"fmt"
	"os"
)

func main() {
	argc := len(os.Args)
	fmt.Printf("argc == %d\n", argc)
	for k, arg := range os.Args {
		fmt.Printf("%2d: [%s]\n", k, arg)
	}
}
