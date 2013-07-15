using System;
using System.Runtime.InteropServices;

class Prog {
    [DllImport("libm.so")]
    static extern double sqrt(double x);

    static int Main(string[] args) {
        double x;
        if (0 < args.Length && double.TryParse(args[0], out x)) {
            Console.WriteLine("sqrt({0}) == {1}", x, sqrt(x));
            return 0;
        } else {
            Console.WriteLine("Usage: PInvoke0 num");
            return 10;
        }
    }
}
