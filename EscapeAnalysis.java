public class EscapeAnalysis {

    private static class Foo {
        private int x;
        private static int counter;

        public Foo() {
            synchronized (this) {
                x = (++counter);
            }
        }
    }

    public static void main(String[] args) {
        System.out.println("start");
        for (int i = 0; i < 1000 * 1000 * 1000; ++i) {
                Foo foo = new Foo();
        }

        System.out.println(Foo.counter);
    }
}
