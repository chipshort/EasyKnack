package com.vin.easyknack.audio;

import java.nio.charset.StandardCharsets;
import java.util.HexFormat;

/** Persistent endpoint identity, independent of runtime process IDs and backend objects. */
public record AudioEndpoint(String id, String name, Type type, boolean input, boolean available, boolean canRoute) {
    public enum Type { SYSTEM_MIX, APPLICATION, HARDWARE_INPUT, HARDWARE_OUTPUT, VIRTUAL_INPUT, VIRTUAL_OUTPUT, ASIO_INPUT, ASIO_OUTPUT }
    public static final AudioEndpoint DEFAULT = new AudioEndpoint("default", "System Default", Type.HARDWARE_OUTPUT, false, true, false);
    public static final AudioEndpoint SYSTEM = new AudioEndpoint("system", "System Audio", Type.SYSTEM_MIX, true, false, false);
    public AudioEndpoint {
        if (id == null || id.isBlank() || id.length() > 4096 || name == null || name.length() > 4096 || type == null)
            throw new IllegalArgumentException(com.vin.easyknack.i18n.AppText.literal("Invalid audio endpoint"));
        boolean source = isInput(type);
        if (source != input) throw new IllegalArgumentException(com.vin.easyknack.i18n.AppText.literal("Endpoint direction/type mismatch"));
    }
    /** Store type, stable ID, and fallback name; never persist availability or PID. */
    public String reference() { return type.name() + ":" + hex(id) + ":" + hex(name); }
    public static AudioEndpoint reference(String text) {
        String[] fields = text.split(":", -1);
        if (fields.length != 3) throw new IllegalArgumentException(com.vin.easyknack.i18n.AppText.literal("Invalid endpoint reference"));
        var type = Type.valueOf(fields[0]);
        boolean input = isInput(type);
        return new AudioEndpoint(unhex(fields[1]), unhex(fields[2]), type, input, false, false);
    }
    public static String hex(String text) { return text.isEmpty() ? "-" : HexFormat.of().formatHex(text.getBytes(StandardCharsets.UTF_8)); }
    public static String unhex(String text) { return text.equals("-") ? "" : new String(HexFormat.of().parseHex(text), StandardCharsets.UTF_8); }
    private static boolean isInput(Type type) {
        return type != Type.HARDWARE_OUTPUT && type != Type.VIRTUAL_OUTPUT && type != Type.ASIO_OUTPUT;
    }
    public String interfaceLabel() {
        return switch (type) {
            case ASIO_INPUT, ASIO_OUTPUT -> "ASIO";
            case SYSTEM_MIX, APPLICATION -> "Capture";
            default -> com.vin.easyknack.app.Platform.isWindows() ? "WASAPI" : "ALSA";
        };
    }
    @Override public String toString() { return "[" + interfaceLabel() + "] " + name + (available ? "" : " (unavailable)"); }
}

