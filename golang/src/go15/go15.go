package main

import "fmt"

type Point struct {
  x, y float64
}

func main() {
  m := map[Point]string{
    {29.935523, 52.891566}:   "Persepolis",
    {-25.352594, 131.034361}: "Uluru",
    {37.422455, -122.084306}: "Googleplex",
  }

  fmt.Println("go 1.5 feature")
  fmt.Println(m)
}
