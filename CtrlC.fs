type ProgOpts = { mutable running : bool }
let g_opts = { ProgOpts.running = true }

[<EntryPoint>]
let main args =
    System.Console.CancelKeyPress.Add( fun cxl ->
        printfn "SIGINT caught;  g_opts.running <- false"
        g_opts.running <- false
        cxl.Cancel <- true
    )

    while g_opts.running do
        stdout.WriteLine( System.DateTime.Now.ToString("HH:mm:ss.ffffff") )
        System.Threading.Thread.Sleep 300

    printfn "Quitting..."
    0
