package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
)

func DoTest(url string) {
	resp, err := http.Get(url)
	if err != nil {
		log.Fatal(err)
	}

	body, err := ioutil.ReadAll(resp.Body)
	resp.Body.Close()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("[%s]\n", body)
}

func main() {
	url0 := "http://example.net/"
	DoTest(url0)

	url1 := "http://example.nonexistent/"
	DoTest(url1)
}
