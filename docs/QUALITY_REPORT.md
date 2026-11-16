# Qualitätsbericht – Radiophone 1.4

Ausgangsstand: `46704018212ce57390cc14f0f77f55da42bf467e`.

## Bestätigt und korrigiert

- `mono.resize()` konnte bei übergroßen Hostblöcken im Audiothread allokieren.
- `triggerAsyncUpdate()` konnte aus dem Audiothread Nachrichten versenden; ersetzt durch
  ein atomares Postfach und einen Prozessor-Timer. Host-PDC bleibt bei laufenden
  Moduswechseln asynchron (siehe technische Dokumentation).
- `lastKeyShown` war zwischen allen Fenstern geteilt.
- Jeder Gerätewechsel leerte den Vorlauf; Moduswechsel stoppten Squelch/Senderwechsel.
- Die Senderwechsel-Synthese lief trotz Verwendung einer Aufnahme weiter.
- Reset ließ Resonator-/Modulationszustände zurück; Reproduktion war nicht vollständig.
- GUI-Metadaten griffen direkt auf den bei State-Restore ersetzten ValueTree zu.
- Standard-Bypass berücksichtigte die Plugin-Verzögerung nicht.

Gate-Hüllkurven wurden bei ausgeschaltetem Gate bereits übersprungen. Es wurde kein
pauschales Abschalten von Filtern oder Geräuschfahnen eingebaut.

## Klang und Kompatibilität

Grundklänge, Parameter und Kennungen bleiben erhalten. Wie bisher: Funk rund 30 ms,
Radio und Telefon 0 ms Plugin-Latenz. Den Modus vor der Wiedergabe wählen. Übergänge sind geglättet, ausgelöste Geräusche können über einen Moduswechsel
hinaus ausklingen. Zufällige Radio-Geräusche nach Senderwechseln sind nicht bitidentisch.

## Validierung

Die folgenden CI-Messungen beziehen sich auf `f3ae6a17f497ee383a8708eb20828d7c7034fd3e`
vor der Rückkehr zu modusabhängiger Latenz. Die aktualisierte Prüfung steht unten.

- Windows-x64-VST3-Build erfolgreich; beide CTest-Suites und `pluginval` Stufe 8 bestanden.
  [Windows-CI](https://github.com/StromschlagProductions/Radiophone-vst/actions/runs/43821308238).
  Release-Schritt wurde übersprungen.
- [Sanitizer-CI](https://github.com/StromschlagProductions/Radiophone-vst/actions/runs/43821308303):
  AddressSanitizer/UndefinedBehaviorSanitizer sowie ThreadSanitizer bestanden.
- Lokale DSP- und Prozessor-Regressionstests bestanden: fünf Abtastraten (22.050–192.000 Hz),
  Mono/Mono→Stereo/Stereo, Blöcke 0–65.537 Samples, wechselnde Größen, schnelle Modi,
  Extremwerte, C++-Allokationskontrolle, State-Restore, Bypass-Latenz und unabhängige Instanzen.
  GUI-Lebensdauerprüfung mit mehrfacher Konstruktion/Zerstörung ohne native Fenster-Peers.
- Unveränderter Referenzstand erfolgreich als Linux-VST3 gebaut; Linux-pluginval Stufe 8 bestanden.
- `Tests/Render.cpp`: selbst erzeugtes Zweitonsignal, Stimme ohne Geräusche, 48 kHz.
  Nach 5.000 Einschwing-Samples und Ausgleich des zusätzlichen Vorlaufs maximale
  Sampleabweichung **0** in allen drei Modi. Keine Aussage über sämtliche Signale,
  Übergänge oder zufällige Geräusche und kein subjektiver Hörtest.

### Performance

Debian 13 x86_64, GCC 14.2, `-O3`, Intel Xeon Platinum 8573C, Container mit zwei
CPU-Kernen Kontingent. Fünf Läufe je Version in wechselnder Reihenfolge, Median der
Prozess-CPU-Zeit (`std::clock`), je Fall 2.560.000 Samples bei 48 kHz, FTZ/DAZ aktiv.
Ein- und Ausgangserzeugung sind mitgemessen; keine DAW-CPU-Prozentangaben.

| Fall | vorher (ms CPU) | nachher (ms CPU) | Änderung |
|---|---:|---:|---:|
| Funk geschlossen / Stille | 366.06 | 279.35 | -23.7 % |
| Funk offen / Stille | 368.17 | 279.01 | -24.2 % |
| Funk offen / Sinus | 402.38 | 392.55 | -2.4 % |
| Radio / Senderwechselaufnahme | 664.27 | 513.59 | -22.7 % |
| Moduswechsel alle 256 Samples | 473.92 | 524.26 | +10.6 % |

Stille und aufgenommene Senderwechsel benötigen im Test deutlich weniger CPU.
Sehr häufige Moduswechsel kosten durch die neue Filterglättung etwa 11 % mehr CPU;
das ist der bewusst dokumentierte Preis der verbesserten Übergänge. Die Zahlen sind
Engine-Messungen und kein Nachweis geringerer maximaler Callback-Zeiten im Vollplugin.
Die vollständigen Rohmesswerte stehen in `docs/benchmark-results.json`.

Testumfang und Befehle: [Technische Prüfung](TECHNICAL_REVIEW.md).

### Aktualisierte Prüfung nach Rücknahme der zusätzlichen Latenz

Code: `da7784ebe57aec7ed17ed38f12e60e640758debf`.

- Linux-VST3-Build erfolgreich; DSP- und Prozessor-CTest bestanden.
- Linux `pluginval --strictness-level 8 --validate-in-process` bestanden.
- Sanitizer-CI (ASan/UBSan und TSan) bestanden:
  [Lauf](https://github.com/StromschlagProductions/Radiophone-vst/actions/runs/43921516343).
- Zusätzliche Tests: Prepare mit jedem Modus; Bypass-Impulse bei allen fünf Abtastraten
  in allen drei Modi; Hostmeldung per Message-Timer ohne Editor und mit unabhängiger
  zweiter Instanz. Funk hat Vorlauf, Radio/Telefon geben den Bypass-Impuls bei Sample 0 aus.
- Zweitonsignal erneut mit 1.3 verglichen: maximale Sampleabweichung nach 5.000
  Einschwing-Samples **0** in allen Modi, diesmal ohne zusätzlichen Zeitversatz.
- Die neue Windows-CI läuft separat:
  [Lauf](https://github.com/StromschlagProductions/Radiophone-vst/actions/runs/43921516316).
  Der vorherige Windows-Erfolg ist kein Testergebnis des neuen Codes.
- Die Performance-Tabelle oben gehört zum vorherigen Stand; nach der Latenzrücknahme
  wurde sie nicht erneut gemessen. Die entfernte redundante Synthese und die weiteren
  Optimierungen bleiben im Code enthalten.


## Selbst in Fender Studio Pro prüfen

1. Altes Projekt/Preset laden: Parameter, Fenstergröße, Theme und Automation vergleichen.
2. Stimme und kurze Impulse mit aktivem Plugin-Latenzausgleich rendern; Radio/Telefon
   und Bypass auf zeitliche Ausrichtung zu einer Referenzspur prüfen.
3. Funk-Geräte, Sprechtaste und Modi während durchgehender Sprache schnell wechseln;
   auf verlorene Silben, ungewollte Knackser und erhaltene Geräuschfahnen achten.
4. Telefonabläufe, Senderwechsel und Gate in Extremstellungen anhören.
5. Mehrere Instanzen/Fenster öffnen, verschieden einstellen, schließen und erneut öffnen.
6. Kleine/große Audiopuffer, Offline-Export, Projekt-Neuladen und Abtastratenwechsel testen.
7. Live-Monitoring: Nur Funk hat Vorlauf; Radio/Telefon und deren Bypass müssen unverzögert sein.

Kein Merge und keine Veröffentlichung im Rahmen dieser Überarbeitung.
