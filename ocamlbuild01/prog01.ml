let main argv =
    Printf.printf "Hello, %s! My name is %s.\n"
        (if Array.length argv > 1 then argv.(1) else "stranger")
        argv.(0)

;; main Sys.argv
