import java.io.ByteArrayOutputStream;
import java.io.InputStream;

/** A class-loader callback executes a virtual call before method-ref resolution completes. */
public final class ClassInitializationReentryTest {
    public static class Subject {
        public static Object initialized = new Dependency();
        public static String describe() {return new IllegalStateException("reentrant metadata").toString();}
    }
    public static class Dependency { }
    static class Loader extends ClassLoader {
        Class<?> subject;
        boolean callback;
        protected synchronized Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
            if (!name.equals("ClassInitializationReentryTest$Subject") && !name.equals("ClassInitializationReentryTest$Dependency")) return super.loadClass(name, resolve);
            Class<?> type = findLoadedClass(name);
            if (type != null) return type;
            if (name.endsWith("$Dependency") && subject != null) {
                try {
                    Object value = subject.getDeclaredMethod("describe").invoke(null);
                    if (!String.valueOf(value).contains("reentrant metadata")) throw new AssertionError("callback dispatch");
                    callback = true;
                } catch (Exception error) {throw new ClassNotFoundException(error.toString());}
            }
            try {
                InputStream input = ClassInitializationReentryTest.class.getResourceAsStream("/" + name + ".class");
                ByteArrayOutputStream bytes = new ByteArrayOutputStream();
                byte[] buffer = new byte[1024];
                int count;
                while ((count = input.read(buffer)) != -1) bytes.write(buffer, 0, count);
                input.close();
                byte[] data = bytes.toByteArray();
                type = defineClass(name, data, 0, data.length);
                if (name.endsWith("$Subject")) subject = type;
                return type;
            } catch (Exception error) {throw new ClassNotFoundException(error.toString());}
        }
    }
    public static void main(String[] args) throws Exception {
        Loader loader = new Loader();
        Class<?> type = Class.forName("ClassInitializationReentryTest$Subject", true, loader);
        if (type.getField("initialized").get(null) == null) throw new AssertionError("reentrant resolution lost its constructor");
        if (!loader.callback) throw new AssertionError("loader callback did not execute during initialization");
        System.out.println("CLASS_REENTRY_DISPATCH_VERIFIED");
    }
}
