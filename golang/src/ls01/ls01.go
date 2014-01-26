package main

import (
	"fmt"
	"log"
	"os"
	"path"
	"time"
)

func PrintFile(fPath string) {
	st, err := os.Stat(fPath)
	var detail string
	if err != nil {
		detail = err.Error()
	} else {
		detail = st.ModTime().Format(time.RubyDate)

		if st.IsDir() {
			fPath += "/"
		}
	}
	fmt.Printf("[%s]: %s\n", fPath, detail)
}

func DoLs(fPath string) {
	if len(fPath) == 0 {
		log.Print("invalid fPath: [%s]", fPath)
		return
	}

	var st os.FileInfo
	var err error
	if l := len(fPath); fPath[l-1] == '/' {
		st, err = os.Stat(fPath)
	} else {
		st, err = os.Lstat(fPath)
	}
	if err != nil {
		log.Fatal(err)
	}

	if st.IsDir() {
		f, err := os.Open(fPath)
		if err != nil {
			log.Fatal(err)
		}

		ents, err := f.Readdir(0)
		f.Close()

		for _, e := range ents {
			PrintFile(path.Join(fPath, e.Name()))
		}
	} else {
		PrintFile(fPath)
	}
}

func main() {
	if 1 < len(os.Args) {
		for _, arg := range os.Args[1:] {
			DoLs(arg)
		}
	} else {
		cwd, err := os.Getwd()
		if err != nil {
			log.Fatal(err)
		}
		DoLs(cwd)
	}
}
