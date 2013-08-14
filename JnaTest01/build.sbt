name := "JnaTest01"

version := "0.1"

organization := "org.flaboratory"

libraryDependencies ++= Seq(
    "net.java.dev.jna" % "jna" % "4.0.0",
    "com.novocode" % "junit-interface" % "0.10" % "test"
)

autoScalaLibrary := false

crossPaths := false

javacOptions += "-Xlint:unchecked"

exportJars := true

mainClass in (Compile, run) := Some("org.flaboratory.JnaTest01")

mainClass in (Compile, packageBin) := Some("org.flaboratory.JnaTest01")

seq(com.github.retronym.SbtOneJar.oneJarSettings: _*)
