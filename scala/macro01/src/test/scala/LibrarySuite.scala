import org.scalatest.FunSuite
import org.junit.runner.RunWith
import org.scalatest.junit.JUnitRunner

@RunWith(classOf[JUnitRunner])
class LibrarySuite extends FunSuite {
  test("My own printf as a macro") {
    PrintfMacro.printf("%d + %d == %d\n", 1, 2, 1+2)
    println("thanks")
  }
}
