package com.vin.easyknack.app;

import java.util.Locale;

/** Platform names used by executable discovery and Windows-only UI features. */
public final class Platform {
    private Platform() { }

    public static boolean isWindows() {
        return System.getProperty("os.name", "").startsWith("Windows");
    }

    public static boolean isLinux() {
        return System.getProperty("os.name", "").toLowerCase(Locale.ROOT).contains("linux");
    }

    public static String audioHostName() {
        return isWindows() ? "easyknack-audio-host.exe" : "easyknack-audio-host";
    }

    public static String sharedLibraryExtension() {
        return isWindows() ? ".dll" : ".so";
    }
}
