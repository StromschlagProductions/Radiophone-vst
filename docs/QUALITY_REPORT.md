# Qualitätsbericht – Radiophone 1.4

Ausgangsstand: `46704018212ce57390cc14f0f77f55da42bf467e`.

## Bestätigt und korrigiert

- `mono.resize()` konnte bei übergroßen Hostblöcken im Audiothread allokieren.
- `triggerAsyncUpdate()` konnte aus dem Audiothread Nachrichten versenden; Latenzmeldung
  und tatsächlicher Moduswechsel waren zeitlich nicht synchron.
- `lastKeyShown` war zwischen allen Fenstern geteilt.
- Jeder Gerätewechsel leerte den Vorlauf; Moduswechsel stoppten Squelch/Senderwechsel.
- Die Senderwechsel-Synthese lief trotz Verwendung einer Aufnahme weiter.
- Reset ließ Resonator-/Modulationszustände zurück; Reproduktion war nicht vollständig.
- GUI-Metadaten griffen direkt auf den bei State-Restore ersetzten ValueTree zu.
- Standard-Bypass berücksichtigte die Plugin-Verzögerung nicht.

Gate-Hüllkurven wurden bei ausgeschaltetem Gate bereits übersprungen. Es wurde kein
pauschales Abschalten von Filtern oder Geräuschfahnen eingebaut.

## Klang und Kompatibilität

Grundklänge, Parameter und Kennungen bleiben erhalten. Radio/Telefon erhalten jetzt
rund 30 ms deklarierte Latenz; das ist beim Live-Monitoring und in alten Projekten zu
beachten. Übergänge sind geglättet, ausgelöste Geräusche können über einen Moduswechsel
hinaus ausklingen. Zufällige Radio-Geräusche nach Senderwechseln sind nicht bitidentisch.

## Validierung

Messwerte und endgültige Build-/Testergebnisse werden nach Abschluss der Prüfungen
in diesem Bericht ergänzt. Testumfang und Befehle: [Technische Prüfung](TECHNICAL_REVIEW.md).

## Selbst in Fender Studio Pro prüfen

1. Altes Projekt/Preset laden: Parameter, Fenstergröße, Theme und Automation vergleichen.
2. Stimme und kurze Impulse mit aktivem Plugin-Latenzausgleich rendern; Radio/Telefon
   und Bypass auf zeitliche Ausrichtung zu einer Referenzspur prüfen.
3. Funk-Geräte, Sprechtaste und Modi während durchgehender Sprache schnell wechseln;
   auf verlorene Silben, ungewollte Knackser und erhaltene Geräuschfahnen achten.
4. Telefonabläufe, Senderwechsel und Gate in Extremstellungen anhören.
5. Mehrere Instanzen/Fenster öffnen, verschieden einstellen, schließen und erneut öffnen.
6. Kleine/große Audiopuffer, Offline-Export, Projekt-Neuladen und Abtastratenwechsel testen.
7. Live-Monitoring beurteilen: Der konstante Vorlauf gilt jetzt in allen Modi.

Kein Merge und keine Veröffentlichung im Rahmen dieser Überarbeitung.
