#!/usr/bin/env rdmd

template from(string mod) {
    mixin("import from = " ~ mod ~ ";");
}

immutable (string)[] enumerate(const(string)[] paths) {
    import std.file : dirEntries, SpanMode;
    import std.algorithm.iteration : filter;

    immutable(string)[] ret;
    foreach (a; paths) {
        // writefln("[%s]", a);
        try {
            auto files = dirEntries(a, SpanMode.depth).filter!(f => f.isFile);
            foreach (f; files) {
                // writefln("[%s]", f);
                ret ~= f;
            }
        } catch (Exception _) { }
    }

    return ret;
}

void main1(const(string)[] paths) {
    // from!"core.thread".Thread.sleep(from!"core.time".dur!("msecs")(567));
    import std.stdio : stdout, File;

    foreach (i; 0..5)
        stdout.writef("%-15d", 100 * i / 5);
    stdout.write("100%\n");

    immutable allPaths = enumerate(paths),
              tot = cast(double)(allPaths.length),
              STEP = 1. / 80;
    double thresh = STEP;
    auto err_cnt = 0;
    auto buf = new ubyte[1 << 20];
    foreach(i, path; allPaths) {
        while (thresh < i / tot) {
            stdout.write('-');
            stdout.flush();
            thresh += STEP;
        }

        try {
            auto f = File(path);
            while (buf.length == f.rawRead(buf).length) {
            }
            err_cnt = 0;
        } catch (Exception ex) {
            err_cnt += 1;
            if (100 < err_cnt)
                throw ex;
        }
    }
    stdout.write("\n");
    stdout.flush();
}

void main(string[] args) {
    import std.stdio : writeln, writefln;

    /+
    import std.getopt : getopt;

    string[] paths;
    getopt(args, "paths|p", &paths);
    if (!paths) {
        writeln("yay");
        paths ~= ".";
    }
    +/

    const auto dt = from!"std.datetime.stopwatch".benchmark!({ main1(args[1..$]); })(1)[0];
    writefln("Took %.1f sec.", 1e-3 * dt.total!"msecs");
}
