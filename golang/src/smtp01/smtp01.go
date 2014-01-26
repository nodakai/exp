package main

import (
	"io/ioutil"
	"log"
	"net/smtp"
	"os"
)

func main() {
	var subj []byte
	if 1 < len(os.Args) {
		subj = []byte("Subject: " + os.Args[1] + "\n")
	} else {
		subj = make([]byte, 0)
	}

	dat, err := ioutil.ReadAll(os.Stdin)
	if err != nil {
		log.Fatal(err)
	}
	dat = append(subj, dat...)

	smtpSvr := "TachiColo2:25"
	auth := smtp.Auth(nil)
	sender := "noda@dcm-limited.com"
	recp := []string{"nodakai@gmail.com"}
	if err = smtp.SendMail(smtpSvr, auth, sender, recp, dat); err != nil {
		log.Fatal(err)
	}
}
