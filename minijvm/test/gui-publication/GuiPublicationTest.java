import org.mini.apploader.GApplication;
import org.mini.gui.callback.GCallBack;
public final class GuiPublicationTest {
    static final class Application extends GApplication {
        boolean complete;
        Application() {complete = true;}
        public void onInit() { }
    }
    public static void main(String[] args) {
        GCallBack callback = GCallBack.getInstance();
        Application app = new Application();
        if (callback.publications != 0 || callback.incompleteProperties)
            throw new AssertionError("GUI observed an application before its constructor completed");
        callback.setApplication(app); // The loader activates only after construction and metadata setup.
        if (!app.complete || callback.publications != 1 || callback.getApplication() != app || callback.incompleteProperties)
            throw new AssertionError("Completed application could not be activated");
        System.out.println("GUI_CONSTRUCTION_PUBLICATION_VERIFIED");
    }
}
