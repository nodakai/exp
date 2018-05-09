#!/usr/bin/env rdmd

template from(string mod) {
    mixin("import from = " ~ mod ~ ";");
}

immutable (string)[] enumerate(const(string)[] paths) {
    import std.file : dirEntries, SpanMode;
    import std.algorithm.iteration : filter;

    immutable(string)[] ret;
    foreach (immutable a; paths) {
        // writefln("[%s]", a);
        try {
            auto files = dirEntries(a, SpanMode.depth).filter!(f => f.isFile);
            foreach (immutable f; files) {
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

    foreach (immutable i; 0..5)
        stdout.writef("%-15d", 100 * i / 5);  // 5 * 15 + 5 == 80
    stdout.writeln("100 %");

    immutable allPaths = enumerate(paths),
              tot = double(allPaths.length),
              WIDTH = 80.;
    auto progress = 0,
         err_cnt = 0,
         buf = new ubyte[10 * 1024 * 1024];
    foreach (immutable i, path; allPaths) {
        for ( ; progress / WIDTH < (i + 1) / tot; ++progress) {
            stdout.write('-');
            stdout.flush();
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
    stdout.writeln();
    stdout.flush();
}

void main(string[] args) {
    immutable dt = from!"std.datetime.stopwatch".benchmark!({ main1(args[1..$]); })(1)[0];
    from!"std.stdio".writefln("Took %.1f sec.", 1e-3 * dt.total!"msecs");
}
