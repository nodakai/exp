open System.Runtime.InteropServices;

[<DllImport("m", EntryPoint="sqrt")>]
extern double MySqrt(double x);

[<EntryPoint>]
let main args =
  try
    let input = double args.[0]
    printfn "sqrt(%A) == %A" input (MySqrt input)
    0
  with | _ ->
    System.Console.Error.WriteLine("Usage: PInvoke0 num")
    10
