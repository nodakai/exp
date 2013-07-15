using System;
using System.Runtime.InteropServices;

class Prog {
    [DllImport("m", EntryPoint="sqrt")]
    static extern double MySqrt(double x);

    static void Main(string[] args) {
        double x;
        if (0 < args.Length && double.TryParse(args[0], out x)) {
            Console.Out.WriteLine("sqrt({0}) == {1}", x, MySqrt(x));
        } else {
            Console.WriteLine("Usage: PInvoke0 num");
            Environment.Exit(10);
        }
    }
}
