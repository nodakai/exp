package org.flaboratory;

interface LibC extends com.sun.jna.Library {
    final LibC INSTANCE = (LibC)com.sun.jna.Native.loadLibrary("c", LibC.class);
    void printf(String fmt, Object... args);
    int getpid();
}

interface LibM extends com.sun.jna.Library {
    final LibM INSTANCE = (LibM)com.sun.jna.Native.loadLibrary("m", LibM.class);
    double sqrt(double x);
}

public class JnaTest01 {
    public static void main(String[] args) {
        LibC.INSTANCE.printf("Hello, world!\n");

        for (int i = 0; i < args.length; ++i)
            LibC.INSTANCE.printf("%d-th arg: %s\n", i, args[i]);

        LibC.INSTANCE.printf("getpid() == %d\n", LibC.INSTANCE.getpid());

        final double x = args.length;
        LibC.INSTANCE.printf("sqrt(%f) == %f\n", x, LibM.INSTANCE.sqrt(x));
    }
}
