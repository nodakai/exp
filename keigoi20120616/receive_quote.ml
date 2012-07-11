(*
save関数を、画面にプリントするようにしてあるので、次のような出力が得られる：

A 1100;1098;1081;1120;
B 910;940;
A 1100;1098;1081;1120;
*)

let main argv = (
    Printf.printf "Ready...";
    let a="A","1","4" and b = "B","1","2" in
    let resps = [
     {req=a; body={price=1100}; total=4; seq=0};
     {req=a; body={price=1098}; total=4; seq=1};
     {req=a; body={price=1100}; total=4; seq=0};
     {req=b; body={price=910}; total=2; seq=0};
     {req=a; body={price=1098}; total=4; seq=1};
     {req=a; body={price=1081}; total=4; seq=2};
     {req=a; body={price=1081}; total=4; seq=2};
     {req=a; body={price=1120}; total=4; seq=3};
     {req=b; body={price=940}; total=2; seq=1};
     {req=a; body={price=1120}; total=4; seq=3};
    ] in (
      Printf.printf "\n";
      Printf.printf "Go! ... ";
      List.iter receive_quote resps
      Printf.printf "Done\n"
    )
)

;;
main Sys.argv
