# Einstieg und Bedienung

## Installation und Deinstallation

Installiere mit `EasyKnack-Setup-<Version>-x64.exe`; `EasyKnack.exe` startet das Programm. Benötigt werden Windows x64 und ein externes x64-JDK 25 einschließlich `javac`. Zum Entfernen schließe EasyKnack und wähle **Einstellungen → Apps → Installierte Apps → EasyKnack → Deinstallieren**, oder starte das Setup erneut und wähle das Entfernen. Eine eigene Uninstall-EXE ist nicht nötig. Deine Dokumente und das separat installierte JDK bleiben erhalten.

Das Setup prüft Java vor dem Kopieren der Programmdateien. Fehlt ein kompatibles JDK, kannst du seinen Ordner auswählen oder ausdrücklich eine externe JDK-Installation starten. Abbrechen beendet das Setup; Java bleibt separat installiert.

Linux ist derzeit ein reiner Entwickler-Port und keine offizielle Veröffentlichung. Es gibt kein Linux-Setup und kein Paket; der Quellbau ist in der Entwicklerdokumentation beschrieben. System-/Anwendungsaufnahme, ASIO, Routing, Gerätelautstärke, Hardware-MIDI-Eingang, MP3-Aufnahme und Plugin-Builds fehlen dort noch.

Wähle **Datei → Neu**, stelle am **Audio Output** dein Ausgabegerät und eine niedrige Lautstärke ein und starte Audio. Der Anfangsgraph verbindet einen Oscillator mit dem Ausgang. Für Mikrofon oder externe Signale verwende Audio Input.

Füge Nodes über das Add-Menü hinzu. Ziehe vom Ausgang zum passenden Eingang; Warnungen erklären abgelehnte Verbindungen. Der Inspector zeigt die Einstellungen der Auswahl. **N** schaltet ihn um, **Strg+Umschalt+L** ordnet den aktuellen Graphen an. Menüs zeigen weitere Tastenkürzel. Ausblenden spart Platz, stoppt aber keine Berechnung.

Speichere als `.knack`. Beispiele öffnen ein neues Dokument; speichere eigene Varianten unter einem neuen Namen. Undo/Redo gilt für den geöffneten Graphen, Speichern für das gesamte Dokument. Kamerabewegungen allein machen ein Dokument nicht geändert. Nach einem Absturz bietet der nächste Start vorhandene Wiederherstellungskopien an; sie ersetzen kein regelmäßiges Speichern.

## Audio und Dateien

WASAPI verwendet Windows-Geräte; ASIO benötigt einen installierten passenden Treiber. Gerätestärke am WASAPI-Ausgang verändert den Windows-Endpunkt; Gain verändert das Signal im Graphen. Copy-Aufnahme von System-/Anwendungsaudio lässt den ursprünglichen Wiedergabeweg bestehen. Für ausschließlich bearbeiteten Ton leite die Quellanwendung über ein separat installiertes virtuelles Kabel zu EasyKnack und von dort zum physischen Ausgang.

Audiodateien und Plugins werden nicht in `.knack` eingebettet. Prüfe sie nach dem Weitergeben oder Verschieben. Bei Aussetzern prüfe Gerätestatus und Renderdiagnose. Für Fehlerberichte helfen **Hilfe → Über**, die Meldung und ein kleines Beispieldokument.

## Einstellungen und Hilfe

Programmeinstellungen betreffen Sprache und Darstellung. Dokumenteneinstellungen betreffen unter anderem Steuerungsglättung, unbenutzte Gruppen und experimentelle Funktionen. Alte Dokumente mit experimentellen Nodes aktivieren die Freigabe automatisch, sofern kein ausdrückliches Aus gespeichert ist. Ausschalten bei vorhandenen experimentellen Nodes verlangt eine Warnungsbestätigung; vorhandene Nodes bleiben erhalten.

Die Dokuansicht speichert **Wie Anwendung / Deutsch / English** unabhängig von der Programmsprache. Node-Kontextmenü und Hilfe öffnen die passende Node-Referenz; Zurück/Vorwärts folgt lokalen Links. **Suchen / Strg+F** durchsucht die aktuelle Seite mit denselben Optionen wie der Text-/Codeeditor; **F3 / Umschalt+F3** wählt den nächsten/vorherigen markierten Treffer.

[Handbuch](index.md) · [Node-Verzeichnis](../../nodes/de/index.md) · [English](../en/start.md)

Standardordner für Projekte ist `Dokumente/EasyKnack/Projects`, für Aufnahmen `Dokumente/EasyKnack/Recordings`. Mitgelieferte Beispiele liegen bei der Installation. Eigene Varianten speicherst du unter Projects. Früher im Dokumenteordner angelegte Beispielkopien bleiben unangetastet.

Anordnen (**Strg+Umschalt+L**) berücksichtigt die grobe Anordnung und Zweigreihenfolge, rückt verbundene Nodes kompakt zusammen und bereinigt Überschneidungen. Platziere zusammengehörige Nodes vorher neben- oder übereinander; Signalfluss und Gruppenränder haben Vorrang. Undo stellt die vorherigen Positionen wieder her.
