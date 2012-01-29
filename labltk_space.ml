let main args =
    let top = Tk.openTk () in
    Wm.title_set top args.(1);
    Tk.mainLoop ()
;; main Sys.argv
