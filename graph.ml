(* module type GRAPH =
    sig
        type adj_graph
        val create  : int                               -> adj_graph
        val get     : adj_graph -> int -> int           -> bool
        val set     : adj_graph -> int -> int -> bool   -> unit
        val print   : adj_graph -> out_channel          -> unit
    end *)

module AdjacencyMatrix (* : GRAPH *) =
    struct
        type adj_graph = bool array * int

        let create sz = (Array.create (sz * (sz - 1) / 2) false, sz)

        let get_idx sz i j =
            let (i, j) = if i < j then (i, j) else (j, i) in
            let ret = (2 * sz - i - 1) * i / 2 + j - i - 1 in
                (* Printf.printf "[%d]" ret; *)
                ret

        let get (ar, sz) i j =
            let ii = get_idx sz i j in ar.(ii)

        let set (ar, sz) i j v =
            let ii = get_idx sz i j in ar.(ii) <- v

        let print ((ar, sz) as am) ch =
            for i = 0 to sz - 1 do
                for j = i + 1 to sz - 1 do
                    Printf.fprintf ch "(%d,%d:%s) " i j (if get am i j then "O" else "x");
                done;
                if i < sz - 1 then output_string ch "\n"
            done
    end

module type RAND_GRAPH_GEN =
    sig
        type graph
    end

module RandGraphGenErdosRenyi : RAND_GRAPH_GEN =
    struct
        type graph = AdjacencyMatrix
    end

module RandGraphGenWattsStrogatz : RAND_GRAPH_GEN =
    struct
        type graph = AdjacencyMatrix
    end

module RandGraphGenBarabasiAlbert : RAND_GRAPH_GEN =
    struct
        type graph = AdjacencyMatrix
    end

;;
let main args = (
    Printf.printf "Hello %d\n" (Array.length args);
    let am = AdjacencyMatrix.create (int_of_string args.(1)) in (
        AdjacencyMatrix.set am 2 3 true;
        AdjacencyMatrix.print am stdout
    )
) in main Sys.argv
