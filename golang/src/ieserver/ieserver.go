package main

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"strings"
	"time"
)

const (
	ieServerUserEnv   = "IESERVER_USER"
	ieServerDomainEnv = "IESERVER_DOMAIN"
	ieServerPassEnv   = "IESERVER_PASS"
	myDNSUserEnv      = "MYDNS_USER"
	myDNSPassEnv      = "MYDNS_PASS"
	lastFile          = "last.txt"
	interval          = 12 * time.Hour
	oneWeek           = 7 * 24 * time.Hour
)

func get(u string) (ret string, e error) {
	r, e := http.Get(u)
	if e != nil {
		return
	}
	defer r.Body.Close()
	b, e := ioutil.ReadAll(r.Body)
	if e != nil {
		return
	}
	ret = strings.TrimSpace(string(b))
	return
}

func myAddr() (string, error) {
	const checkip = "https://checkip.amazonaws.com/"
	return get(checkip)
}

func registerToIeServer(_ string) (string, error) {
	const endpoint = "https://ieserver.net/cgi-bin/dip.cgi"
	user := os.Getenv(ieServerUserEnv)
	domain := os.Getenv(ieServerDomainEnv)
	pass := os.Getenv(ieServerPassEnv)
	if len(user) == 0 || len(domain) == 0 || len(pass) == 0 {
		e := fmt.Errorf("%s=%q, %s=%q, %s=%q",
			ieServerUserEnv, user, ieServerDomainEnv, domain, ieServerPassEnv, pass)
		return "", e
	}

	v := url.Values{
		"username":   {user},
		"domain":     {domain},
		"password":   {pass},
		"updatehost": {"1"},
	}
	return get(endpoint + "?" + v.Encode())
}

func registerToMyDNS(a string) (string, error) {
	const endpoint = "https://www.mydns.jp/directip.html"
	user := os.Getenv(myDNSUserEnv)
	pass := os.Getenv(myDNSPassEnv)
	if len(user) == 0 || len(pass) == 0 {
		return "", fmt.Errorf("%s=%q, %s=%q", myDNSUserEnv, user, myDNSPassEnv, pass)
	}

	v := url.Values{
		"MID":      {user},
		"PWD":      {pass},
		"IPV4ADDR": {a},
	}
	return get(endpoint + "?" + v.Encode())
}

func checkAddrChange(outDir, newAddr string) (write, same bool) {
	write = true
	fn := filepath.Join(outDir, lastFile)
	f, e := os.Open(fn)
	if e != nil {
		log.Printf("no last addr to compare with my address %q\n", newAddr)
		return
	}
	defer f.Close()
	r := bufio.NewScanner(f)
	r.Scan()
	if e = r.Err(); e != nil {
		log.Printf("Scan: %s", e)
		return
	}
	la := r.Text()
	if len(la) == 0 {
		log.Printf("Scan returned an empty line")
		return
	}
	write = false

	log.Printf("my address %q vs last address %q\n", newAddr, la)
	if newAddr != la {
		return
	}

	fi, e := f.Stat()
	if e != nil {
		log.Printf("Stat %q: %s", fn, e)
		return
	}
	if now, mt := time.Now(), fi.ModTime(); mt.Add(oneWeek).Before(now) {
		dt := now.Sub(mt)
		log.Printf("last refresh at %v is too old (%v)\n", mt, dt)
		return
	}

	log.Printf("nothing to do\n")
	same = true
	return
}

func storeMyAddr(d, a string) error {
	fn := filepath.Join(d, lastFile)
	tmp := fn + ".tmp"
	f, e := os.Create(tmp)
	if e != nil {
		return e
	}
	_, e = f.WriteString(a)
	if e != nil {
		f.Close()
		return e
	}
	f.Close()
	e = os.Rename(tmp, fn)
	return e
}

func checkAndRegister(siteName string, register func(string) (string, error)) {
	jobID := strings.Split(siteName, ".")[0]
	now := time.Now()
	d := filepath.Join(os.TempDir(), jobID)
	e := os.MkdirAll(d, os.ModePerm)
	if e != nil {
		log.Panicf("MkdirAll %s: %s\n", d, e)
		return
	}
	fn := filepath.Join(d, fmt.Sprintf("%s.%s.log", jobID, now.Format("2006-01-02-15-04-05")))
	f, e := os.Create(fn)
	if e != nil {
		log.Panicf("Create %s: %s\n", fn, e)
		return
	}
	defer f.Close()
	log.SetOutput(f)
	defer log.SetOutput(ioutil.Discard)

	a, e := myAddr()
	if e != nil {
		log.Printf("chekcip: %s\n", e)
		return
	}
	w, s := checkAddrChange(d, a)
	if w || !s {
		e = storeMyAddr(d, a)
		if e != nil {
			log.Printf("cannot record the current addr to %q: %s\n", fn, e)
		}
	}
	if s {
		return
	}

	log.Printf("registering to %s...\n", siteName)
	reg, e := register(a)
	if e != nil {
		log.Printf("http.Get: %s\n", e)
		return
	}
	if strings.Contains(reg, a) {
		log.Printf("OK\n")
	} else {
		log.Printf("%s: %q\n", siteName, reg)
	}
}

func main() {
	checkAndRegister("ieserver.net", registerToIeServer)
	checkAndRegister("mydns.jp", registerToMyDNS)
}
