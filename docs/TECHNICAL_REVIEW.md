# Technische Prüfung – Radiophone 1.4

Ausgangspunkt: `46704018212ce57390cc14f0f77f55da42bf467e` (1.3), JUCE 8.0.9.
Plugin-/Herstellerkennung, Parameter-IDs, Reihenfolgen, Wertebereiche und XML-Wurzel
`RADIOPHONE` bleiben erhalten. Keine neue Audiofunktion und kein GUI-Redesign.

## Komponenten und Signalfluss

`PluginProcessor` übernimmt APVTS-Parameter, WAV-Dekodierung, Vorbereitung, Mono-Mischung,
Host-Latenz, Bypass und State. `RadiophoneDSP.h` ist eine eigenständig testbare C++-Engine.
`PluginEditor` verwendet Attachments und einen 15-Hz-Timer für Anzeige und Metadaten.

Eingang → Mono-Mischung → Vorlauf → Bandfilter → Kompressor → Density-Sättigung →
Sende-/Leitungspegel → Summe mit Geräuschen → identischer Ausgang auf beiden Kanälen.
Der Funkdetektor sieht den Eingang vor dem Vorlauf. Squelch-Aufnahmen umgehen ihren
Synthesefilter; Radio- und Telefongeräusche laufen durch den Leitungsfilter.
Ein separater, kontinuierlich gefüllter Stereo-Ringpuffer liefert den Bypass.

## Thread-Zuständigkeiten / Echtzeitregeln

- Vorbereitung: Vektoren reservieren, WAVs auf die Host-Abtastrate umrechnen, Latenz melden.
  `prepareToPlay`, `reset` und Zerstörung benötigen den üblichen Host-Lebenszyklus ohne
  gleichzeitig laufenden Callback. `reset` allokiert nicht.
- Audio: atomare Parameterwerte einmal je Hostblock lesen, feste Arbeitspuffer in
  Teilblöcken verwenden, ausschließlich eigene DSP-Zustände verändern. Keine Locks,
  Nachrichten, Dateizugriffe oder Containervergrößerung im eigenen Callback.
- GUI: Attachments für Parameter; Status über atomare Flags, Fensterdaten über atomare
  Integer. Kein direkter Zugriff mehr auf den austauschbaren APVTS-ValueTree.
- State: `copyState`/`replaceState` sind JUCE-synchronisiert und gehören **nicht** in den
  Audiocallback. Zusätzliche GUI-Metadaten werden in eine State-Kopie geschrieben.
  Mehrere atomare Parameter bilden keinen transaktionalen Snapshot; Automation und
  Restore werden wie bisher an Blockgrenzen wirksam.

Der Allokationstest instrumentiert globale C++-`new`/`new[]` auf dem geprüften Thread.
Er ersetzt keine umfassende Betriebssystem-Echtzeitanalyse und überwacht keine beliebigen
externen `malloc`-Aufrufe. Der eigene Audiopfad verwendet keine solchen Aufrufe.

## Architekturentscheidungen

### Konstante Latenz

Alle Modi verwenden denselben Vorlauf: `lround(0.03f * sampleRate)` Samples.
Das erhält die bisherige Funk-Rundung (bei 22.050 Hz: 661 Samples). Radio/Telefon
bekommen zusätzlich diesen Vorlauf. Die Änderung ist absichtlich dokumentiert: Ein
Timer würde die Hostmeldung vom Audiowechsel entkoppeln und vorübergehend falsche PDC
melden; `triggerAsyncUpdate` kann wiederum eine OS-Nachricht aus dem Callback erzeugen.

`setLatencySamples` ruft in JUCE 8.0.9 bei Änderung synchron `updateHostDisplay` mit
`latencyChanged` auf. Es steht hier ausschließlich in `prepareToPlay`, niemals in der
Verarbeitung. Ein Editor oder Message-Timer wird für die Latenz nicht benötigt.
Abtastratenwechsel bereiten alle Puffer neu vor. `processBlockBypassed` gibt das um exakt
diese Samplezahl verzögerte Originalsignal aus; ein kurzer Mixübergang vermeidet harte
Bypass-Kanten. Die Engine läuft dabei weiter, damit Wiedereinschalten nicht mit veralteten
Zuständen beginnt. Bypass ist somit bewusst kein CPU-Sparmodus.

### Blockgrößen

Der Mono-Arbeitspuffer hat mindestens 4096 Samples. Beliebig größere Hostblöcke werden
vollständig in fortlaufenden Teilblöcken verarbeitet. Keine Zustände werden zwischen
Teilblöcken zurückgesetzt. Die Parameter bleiben innerhalb eines Hostblocks konstant.
Unterstützt: Mono→Mono, Mono→Stereo, Stereo→Stereo; Stereo wird gemittelt.

### Übergänge und Fahnen

Geräte-/Moduswechsel löschen den Vorlauf nicht mehr. Bandfilter interpolieren ihre
normalisierten Biquad-Koeffizienten über 5 ms ausgehend vom aktuellen Zwischenstand;
Filterzustände bleiben erhalten. Wiederholte Wechsel brauchen keine zweite Engine oder
Speicherallokation. Sendepegel springen bei Moduswechseln nicht mehr direkt zum Ziel.
Telefon-Tongeneratoren blenden beim Moduswechsel kurz aus. Bereits laufende Squelch-,
Hörer- und Senderwechselgeräusche werden weitergerechnet. Die gemeldete Nachlaufzeit
beträgt 2 s und deckt auch die längste eingebaute Aufnahme (Senderwechsel, ca. 1,602 s) ab.
Die absichtlich knackenden Telefon-/Squelch-Ereignisse bleiben Bestandteil des Effekts.

### Konservative Optimierungen

Das ausgeschaltete Voice-Gate war bereits bedingt ausgeführt; ein angeblicher ständig
laufender Gate-Hüllkurvenpfad wurde **nicht** bestätigt. Die übrige Klangkette bleibt aktiv,
um Filterzustände und Wiederöffnung zu erhalten. Neu: keine Kompressor-Logarithmen unter
der Schwelle, quadrierte Detektorschwellen statt Quadratwurzel, keine parallele
Synthese beim Abspielen der Senderwechselaufnahme. Das verändert den Verbrauch des
Zufallszahlengenerators beim Senderwechsel; späteres zufälliges Rauschen ist daher nicht
bitidentisch. Voicings, Schwellen, Sättigung und Geräuschaufnahmen bleiben erhalten.
`ScopedNoDenormals` schützt den JUCE-Callback; Standalone-DSP-Tests setzen FTZ/DAZ selbst.

## Reproduzierbare Prüfung

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
# Nur DSP, ohne JUCE/Desktop-Abhängigkeiten:
cmake -S . -B build-dsp -DRADIOPHONE_DSP_ONLY=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-dsp
ctest --test-dir build-dsp --output-on-failure
# Sanitizer (separate Buildverzeichnisse):
cmake -S . -B build-asan -DRADIOPHONE_DSP_ONLY=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
# ThreadSanitizer: entsprechend -fsanitize=thread
```

Linux-JUCE benötigt u. a. ALSA-, X11-, FreeType- und Fontconfig-Entwicklungsbibliotheken.
Der Prozessor-Test konstruiert Fenster ohne native Peers; er ist kein visueller GUI-Test.
Windows-CI baut VST3, führt CTest und `pluginval --strictness-level 8` aus.
Sanitizer-CI prüft die unabhängige Engine. Releases erfordern jetzt ausdrücklich
`workflow_dispatch` mit `publish=true` auf `main`; PRs erzeugen keine Releases.

`Tests/Benchmark.cpp` kann unverändert gegen den ursprünglichen und neuen DSP-Header
kompiliert werden (`g++ -O3 -std=c++17 -I<Source> Tests/Benchmark.cpp`). Vier Fälle:
geschlossener Funk bei Stille, offener Funk bei Stille, offener Funk mit Sinus und Radio
mit wiederholt gestarteter Testaufnahme. Je Fall 10.000 × 256 Samples bei 48 kHz.
Eingangserzeugung ist mitgemessen. Kein DAW- oder Vollplugin-CPU-Benchmark.

## Grenzen

Keine samplegenaue Automation innerhalb eines Hostblocks. Ein laufendes Ereignis kann
wie bisher durch erneutes Auslösen desselben Ereignisses ersetzt werden. Stetige
Geräuschgeneratoren sind keine ausklingenden Hallfahnen. Der Host darf das Plugin nicht
parallel zu `prepareToPlay`/Zerstörung aufrufen. Subjektive Klangfreigabe, native Fenster,
DAW-PDC und Monitoring müssen in der Ziel-DAW geprüft werden; automatisierte Tests
beweisen keine universelle Knackserfreiheit.
