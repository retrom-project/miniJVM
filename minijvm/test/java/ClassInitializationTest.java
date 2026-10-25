public final class ClassInitializationTest {
    static volatile boolean entered, release, secondStarted, secondDone;
    static volatile int observed;
    static volatile Throwable failure;
    static final class Value {
        static int value;
        static {
            if (read() != 0) throw new AssertionError("recursive initialization");
            entered = true;
            while (!release) {
                try { Thread.sleep(1); } catch (InterruptedException e) { throw new RuntimeException(e); }
            }
            value = 42;
        }
        static int read() { return value; }
    }
    static final class Broken {
        static int value = fail();
        static int fail() { throw new IllegalStateException("initialization failed"); }
    }
    public static void main(String[] args) throws Exception {
        Thread first = new Thread(new Runnable() {
            public void run() {
                try { Value.read(); } catch (Throwable e) { failure = e; }
            }
        });
        first.start();
        long deadline = System.currentTimeMillis() + 3000;
        while (!entered && System.currentTimeMillis() < deadline) Thread.sleep(1);
        if (!entered) throw new AssertionError("initializer did not start");
        Thread second = new Thread(new Runnable() {
            public void run() {
                secondStarted = true;
                try { observed = Value.read(); } catch (Throwable e) { failure = e; }
                secondDone = true;
            }
        });
        second.start();
        while (!secondStarted) Thread.sleep(1);
        Thread.sleep(100);
        boolean ranBeforeInitialized = secondDone;
        release = true;
        first.join(3000);
        second.join(3000);
        if (ranBeforeInitialized || !secondDone || observed != 42 || failure != null)
            throw new AssertionError("cross-thread initialization was not serialized: " + observed);
        boolean failed = false;
        try { int ignored = Broken.value; } catch (Throwable expected) { failed = true; }
        if (!failed) throw new AssertionError("initializer failure was swallowed");
        failed = false;
        try { int ignored = Broken.value; } catch (NoClassDefFoundError expected) { failed = true; }
        if (!failed) throw new AssertionError("failed class was initialized again");
        System.gc();
        System.out.println("CLASS_INITIALIZATION_VERIFIED");
    }
}
