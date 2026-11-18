package com.vin.easyknack;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

/** Locates installed resources without depending on the launcher's working directory. */
public final class InstallationPaths {
    private InstallationPaths() { }

    public static Path applicationDirectory() {
        try {
            return Path.of(InstallationPaths.class.getProtectionDomain().getCodeSource()
                    .getLocation().toURI()).toAbsolutePath().normalize().getParent();
        } catch (Exception e) {
            throw new IllegalStateException("Cannot locate the EasyKnack installation", e);
        }
    }

    public static Path audioHost() {
        return applicationDirectory().resolve("native").resolve(com.vin.easyknack.app.Platform.audioHostName());
    }

    public static List<Path> compilerRoots() {
        Path base = applicationDirectory();
        // Packaged applications never select a compiler from the current directory.
        if (Files.isRegularFile(base.resolve("installation.properties")))
            return List.of(base.resolve("tools"));
        Path checkout = base.getParent();
        if (base.getFileName().toString().equals("target") && checkout != null
                && Files.isRegularFile(checkout.resolve("pom.xml"))
                && Files.isRegularFile(checkout.resolve("src/main/java/com/vin/easyknack/InstallationPaths.java")))
            return List.of(checkout.resolve(".local/tools"), base.resolve("tools"));
        return List.of(base.resolve("tools"), Path.of(".local/tools"), Path.of("tools"), Path.of("target/tools"));
    }
}
