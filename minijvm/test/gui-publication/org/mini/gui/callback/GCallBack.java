package org.mini.gui.callback;
import org.mini.apploader.GApplication;
/** Recording callback used only by the constructor-publication regression. */
public final class GCallBack {
    private static final GCallBack INSTANCE = new GCallBack();
    private GApplication application;
    public int publications;
    public boolean incompleteProperties;
    public static GCallBack getInstance() {return INSTANCE;}
    public GApplication getApplication() {return application;}
    public void setApplication(GApplication value) {
        publications++;
        try {value.getProperty("probe", "ready");}
        catch (NullPointerException unconstructed) {incompleteProperties = true;}
        application = value;
    }
}
