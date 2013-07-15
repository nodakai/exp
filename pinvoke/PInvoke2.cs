using System;
using System.Runtime.InteropServices;

class Prog {
    [DllImport("m", EntryPoint="sqrt")]
    static extern double MySqrt(double x);

    static int Main(string[] args) {
        double x;
        if (0 < args.Length && double.TryParse(args[0], out x)) {
            Console.WriteLine("sqrt({0}) == {1}", x, MySqrt(x));
            return 0;
        } else {
            Console.WriteLine("Usage: PInvoke0 num");
            return 10;
        }
    }
}
