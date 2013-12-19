package main

import (
    "log"
    "io/ioutil"
)

func main() {
    dat, err := ioutil.ReadFile("/proc/cpuinfo")
    if err != nil {
        log.Fatal(err)
    }

    err = ioutil.WriteFile("/tmp/tmp.txt", dat, 0777)
    if err != nil {
        log.Fatal(err)
    }
}
