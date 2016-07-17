package main

import (
	"fmt"
)

func main() {
	m := make(map[int]string)
	m[1] = "one"
	m[2] = "two"
	m[3] = "three"

	fmt.Println("range loop")
	for k, v := range m {
		fmt.Println(k, v)
	}
	fmt.Println("Once again")
	for k, v := range m {
		fmt.Println(k, v)
	}
	fmt.Println("Bonus!")
	for k, v := range m {
		fmt.Println(k, v)
	}
}
