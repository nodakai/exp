(*
let show =
    open Parser in
    function
*)

let main argv = (
    Printf.printf "%s =>\n" argv.(1)
    (* let tree = Parser.parse (Lexer.analyze argv.(1)) in
    show tree *)
)

;; main Sys.argv
