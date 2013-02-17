name := "SimplestSwingSample"

version := "1.0"

// scalaVersion := "2.9.2"

libraryDependencies <+= scalaVersion { "org.scala-lang" % "scala-swing" % _ }
