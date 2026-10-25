package org.mini.apploader;

import org.mini.gui.callback.GCallBack;

import java.io.File;
import java.security.Permission;
import java.util.HashSet;
import java.util.Set;

/**
 * 自定义安全管理器
 * 不允许跨Application的访问
 *
 * @author Gust
 */
public class GuiSecurityManager extends SecurityManager {
    static RuntimePermission PERMISSION_TOKEN = new RuntimePermission("accessPassword");
    // 允许访问的文件路径
    // 允许访问的线程名称
    private final Set<String> allowedThreadNames = new HashSet<>();

    // 允许访问的类名
    private final Set<String> allowedCaller = new HashSet<>();
    private final Set<String> declinedCaller = new HashSet<>();
    private final String runWork = "org.mini.gui.callback.GCmdHandler.runWork";

    public GuiSecurityManager() {
    }

    // 添加允许访问的线程名称
    public void addAllowedThread(String threadName) {
        allowedThreadNames.add(threadName);
    }

    // 添加允许访问的类名
    public void addAllowedCaller(String className, String methodName) {
        allowedCaller.add(className + "." + methodName);
    }

    public void addDeclinedCaller(String className, String methodName) {
        declinedCaller.add(className + "." + methodName);
    }

    @Override
    public void checkPermission(Permission perm) {
        // 默认允许所有权限
        switch (perm.getName()) {
            case "accessPassword":
                GApplication app = GCallBack.getInstance().getApplication();
                if (app != null && !isAppManager()) {
                    throw new SecurityException("Access declined: " + perm);
                }
                break;
            case "setSecurityManager":
                throw new SecurityException("Can't setSecurityManager");

        }
        super.checkPermission(perm);
    }

    @Override
    public void checkDelete(String filePath) {
        checkRestrictedFile(filePath);
        super.checkDelete(filePath);
    }

    @Override
    public void checkRead(String filePath) {
        checkRestrictedFile(filePath);
        super.checkRead(filePath);
    }

    @Override
    public void checkWrite(String filePath) {
        checkRestrictedFile(filePath);
        super.checkWrite(filePath);
    }

    private void checkRestrictedFile(String filePath) {
        // Most checks can be decided from the active application's path. Only
        // an otherwise denied access needs the framework's stack exceptions.
        // In particular RMS writes must not allocate a Java stack for every
        // exists/open/rename check.
        if (allowedThreadNames.contains(Thread.currentThread().getName())) return;
        GApplication app = GCallBack.getInstance().getApplication();
        if (app == null || isAppManager()) return;
        String appRoot = app.getSaveRoot();
        boolean allowed = false;
        if (appRoot != null) {
            String absolute = new File(filePath).getAbsolutePath();
            String allApps = new File(GCallBack.getInstance().getAppSaveRoot()).getAbsolutePath();
            String currentApp = new File(appRoot).getAbsolutePath();
            String temporary = new File(AppLoader.getTmpDirPath()).getAbsolutePath();
            // Preserve the existing path policy; this change only avoids stack
            // construction for paths that policy already permits.
            allowed = absolute.startsWith(temporary) || !absolute.startsWith(allApps) || absolute.startsWith(currentApp);
        }
        if (allowed) return;
        StackTraceElement[] stackTrace = Thread.currentThread().getStackTrace();
        if (isAccessAllowed(stackTrace) || isRunWork(stackTrace) &&
                Thread.currentThread().getContextClassLoader() == AppManager.class.getClassLoader()) return;
        throw new SecurityException("Access declined: " + filePath);
    }

    private boolean isAppManager() {
        return GCallBack.getInstance().getApplication() instanceof AppManager;
    }

    // 判断当前访问是否被允许
    private boolean isAccessAllowed(StackTraceElement[] stackTrace) {
        // 检查当前线程名称
        String currentThreadName = Thread.currentThread().getName();
        if (allowedThreadNames.contains(currentThreadName)) {
            return true;
        }

        // 检查调用栈中的类
        for (StackTraceElement element : stackTrace) {
            String className = element.getClassName();
            String methodName = element.getMethodName();
            if (allowedCaller.contains(className + "." + methodName)) {
                return true;
            }
        }

        return false;
    }

    private boolean isRunWork(StackTraceElement[] stackTrace) {
        for (StackTraceElement element : stackTrace) {
            String className = element.getClassName();
            String methodName = element.getMethodName();
            if (runWork.equals(className + "." + methodName)) {
                return true;
            }
        }
        return false;
    }

    private boolean isAccessDeclined(StackTraceElement[] stackTrace) {
        // 检查调用栈中的类
        for (StackTraceElement element : stackTrace) {
            String className = element.getClassName();
            String methodName = element.getMethodName();
            if (declinedCaller.contains(className + "." + methodName)) {
                return true;
            }
        }

        return false;
    }

    private void checkClassLoader() {
        ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
        if (classLoader != ClassLoader.getSystemClassLoader()) {
            throw new SecurityException("Access declined: " + classLoader);
        }
    }

}