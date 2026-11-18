# RADIOPHONE – Audio Drama Radio & Phone Simulator

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/logo-dunkel.png">
  <img src="docs/logo-hell.png" alt="Stromschlag Audio" height="64">
</picture>

VST3-Insert-Effekt für Hörspiel-Produktion. Er lässt eine Stimme oder Musik klingen, als käme sie aus einem
Funkgerät, einem Radio oder einem Telefon, und bringt die passenden Geräusche gleich mit: das Tschk und Kschhh
der Sprechtaste, Rauschen und Knacken im Radio, Wählton, Wählscheibe, Freizeichen und das Klacken des Hörers.

**⬇ Download (Windows · VST3 · Stereo):** https://github.com/StromschlagProductions/Radiophone-vst/releases/latest

## Warum es dieses Plugin gibt

In fast jedem meiner Hörspiele kommt irgendwann eine Funkdurchsage, ein Radio oder ein Telefonat vor. Den Klang
habe ich bisher mit einer Lautsprecher-Simulation gemacht und die Squelch-Geräusche, Wähltöne und Störungen von
Hand in eine eigene Effektspur gelegt. Das kostet bei jeder Szene Zeit.

RADIOPHONE legt den Klang drauf und erzeugt die Geräusche selbst, an der richtigen Stelle. Der Klang ist fest
abgestimmt: Plugin auf die Spur, Modus wählen, fertig. Es gibt keinen Mix- und keinen Ausgangsregler, die Spur
läuft komplett durch den Effekt.

*RADIOPHONE soll niemandem die Erfahrung abnehmen, Funk und Radio selbst zu gestalten. Gerade am Anfang kann es viel Spaß machen und lehrreich sein, einen Funkspruch selbst aufzubauen und zu hören, wie Bandbreite, Rauschen und Verzerrung ineinandergreifen. Ich bin dankbar, dass ich diesen Weg selbst gegangen bin, auch wenn er manchmal umständlich und mühselig war. Gerade dadurch weiß ich, was ich mir von einem Werkzeug wünsche, das all das in einem vereint. RADIOPHONE soll das Gestalten nicht ersetzen, sondern die technische Umsetzung im Alltag einfacher machen.*

– Lars Eickstädt, Stromschlag Audio

## Oberfläche

| Funk (dunkel) | Radio (dunkel) | Telefon (hell) |
|---|---|---|
| ![Funk](docs/oberflaeche-funk.png) | ![Radio](docs/oberflaeche-radio.png) | ![Telefon](docs/oberflaeche-telefon.png) |

Oben wählt man den Modus, darunter zeigt eine Lampe den Zustand (SENDET, SENDERWECHSEL, GESPRÄCH). Der
**?**-Knopf öffnet die Hilfe, Look Dunkel/Hell und die Fenstergröße (50–300 %) werden mit dem Projekt gespeichert.

## Modi und Regler (alle automatisierbar)

### Funk

Das Squelch-Geräusch hängt an der Sprechtaste, nicht an Sprechpausen: Taste drücken = kurzes **Tschk**,
Taste loslassen = **Kschhh** mit hartem Ende. Solange die Taste gedrückt ist, hört man ein leises Kanalrauschen
unter der Stimme. Ist sie losgelassen, ist der Empfänger stumm.

| Regler | Wirkung |
|---|---|
| **Gerät** | Walkie-Talkie (schmal, rau, langes Kschhh), Autofunk (mittig, kurzes Kschhh), Tischfunk (voller). Beim Walkie-Talkie wechselt das Kschhh am Ende zufällig zwischen drei Aufnahmen, nie zweimal dasselbe hintereinander. |
| **Sprechtaste** | **Automatik:** öffnet beim ersten Wort (das Tschk liegt kurz davor), schließt erst nach der eingestellten Pause. Kurze Pausen zwischen Wörtern oder Sätzen lösen nichts aus. **Gedrückt / Losgelassen:** volle Kontrolle per Automation. |
| **Pause bis Ende** | nur Automatik: so lange Stille (0,2–3 s, Standard 0,6 s), bis die Taste als losgelassen gilt |
| **Abhacken** | aus = sauber. Je weiter aufgedreht, desto eher bricht die Verbindung bei leisen Stellen hart weg. |

### Radio

Fester Radioklang (kleiner Lautsprecher, etwas Sendekompression) mit leisem Grundrauschen und gelegentlichem
Knacken. Passt auf Musik und auf Durchsagen.

| Regler | Wirkung |
|---|---|
| **Störungen** | Wie oft und wie stark der Empfang schlecht wird: Die Stimme sackt ab und geht ins Gruscheln, das Rauschen kommt hoch, dann ist sie wieder da. aus = keine Störungen, voll = häufig und stark (Standard 50 %). |
| **Senderwechsel** | Knopf oder Automation. Jede Änderung dreht den Sender weiter: Das Signal ist etwa eine Sekunde weg, es pfeift und rauscht, dann rastet der Sender ein. Neun verschiedene Senderwechsel-Geräusche (echte Aufnahmen, gleich laut), zufällig gewählt und nie zweimal dasselbe hintereinander. So klingt kein Senderwechsel wie der vorige. |
| **Radio: Ein** | Knopf oder Automation. Schaltet das Radio aus, mit einem Klick: Rauschen, Knacken und Stimme sind sofort still, auch wenn die Spur weiterläuft. Ein zweiter Klick schaltet es wieder an. So endet das Rauschen genau dort, wo das Radio abgeschaltet wird. |

### Telefon

Fester Telefonklang (schmale Leitung, Kohlemikrofon). Der Ablauf eines Anrufs wird über den Zustand der Leitung
gesteuert. Die Stimme ist nur bei **Gespräch** zu hören.

| Regler | Wirkung |
|---|---|
| **Leitung** | Gespräch · Aufgelegt · Wählton (425 Hz) · Wählen · Freizeichen (1 s Ton, 4 s Pause) · Besetzt. Weg von *Aufgelegt* = Hörer abnehmen (Klacken), hin zu *Aufgelegt* = die Leitung wird aufgelegt (eine echte Aufnahme, die zu jedem Telefon passt: Wählscheibe, Tastentelefon, Handy). Andere Wechsel knacken kurz in der Leitung. |
| **Wählart** | Wählscheibe (Impulse, typisch für die 80er) oder Tasten (Töne). Gewählt wird, bis man auf Freizeichen umschaltet. **Jedes Wählen ist eine neue, zufällige Nummer**, mit leicht unterschiedlichen Pausen zwischen den Ziffern, wie bei einem Menschen. Wer mehrmals wählen lässt, bekommt nie zweimal dieselbe Tonfolge und muss keine Wähltöne mehr von Hand variieren. Der Zufall beginnt nach dem Laden des Plugins immer gleich, darum klingt ein Export jedes Mal identisch. |

### Für alle Modi

| Regler | Wirkung |
|---|---|
| **Geräusche – Lautstärke** | Pegel aller eingebauten Geräusche (Squelch, Rauschen, Knacken, Senderwechsel, Telefontöne), −24 bis +6 dB. Die Stimme bleibt davon unberührt. |

## Anwendung

- **Funkspruch, ohne etwas zu automatisieren:** Plugin auf die Sprecherspur, Modus Funk, Gerät wählen, Sprechtaste
  auf Automatik. Jeder Take bekommt sein Tschk am Anfang und sein Kschhh am Ende.
- **Funkspruch mit genauer Kontrolle:** Sprechtaste auf Losgelassen, dann in der Automationsspur „Funk: Sprechtaste“
  vor jeder Durchsage auf Gedrückt und danach wieder auf Losgelassen stellen.
- **Radio in der Ecke:** Musik- oder Sprecherspur auf Radio. Für den Raum (Radio steht hinten rechts) danach
  LOCATIONS oder den Pan am Kanal benutzen. Für „Musik hört auf, Sprecher setzt ein“ an der Stelle einen
  Senderwechsel setzen.
- **Anruf:** Spur auf Telefon, „Telefon: Leitung“ automatisieren, z. B. Aufgelegt → Wählton → Wählen →
  Freizeichen → Gespräch → Besetzt → Aufgelegt.

## Hinweise

- **Latenz:** Nur Funk hat rund 30 ms Vorlauf, damit das Squelch-Geräusch vor dem ersten Wort
  liegt. Radio und Telefon haben 0 ms Plugin-Latenz. Die DAW gleicht die Funk-Latenz aus.
  Nach einem Moduswechsel wird die geänderte Latenz über den Message-Thread gemeldet;
  die DAW kann ihren Latenzausgleich kurz darauf neu berechnen. Für verlässliche zeitliche
  Ausrichtung den Modus vor der Wiedergabe wählen.
- Funk, Radio und Telefon sind mono: Eine Stereospur wird zusammengemischt und auf beide Seiten gelegt.
- Die Stimme kommt etwa so laut heraus, wie sie hineingeht (ca. 1 dB leiser, durch die schmalere Bandbreite).

## Installation (Windows)

1. Über den Download-Link oben `Radiophone-VST3-Windows.zip` laden.
2. ZIP entpacken und den **ganzen Ordner** `Radiophone.vst3` nach `C:\Program Files\Common Files\VST3` kopieren.
3. DAW neu starten bzw. Plugins neu scannen.

## Versionen

Die Versionsnummer steht in der Datei `VERSION`, im Plugin-Kopf und im Namen der Release. Sie wird bei jeder
Änderung hochgezählt.

- **1.7** – Telefon: neues Auflegegeräusch. Statt eines eingehängten Hörers ist jetzt das Auflegen der Leitung zu hören, passend für jedes Telefon. Radio: neun Senderwechsel-Aufnahmen statt drei, gleich laut, zufällig. Funk: drei Schluss-Squelches beim Walkie-Talkie, zufällig. Anregungen aus dem Beta-Test. [Release-Hinweise](docs/releases/1.7.md).
- **1.6** – Einheitliche öffentliche Versionsnummer ohne Build-Suffix; Audioverarbeitung und Projektkompatibilität unverändert. [Release-Hinweise](docs/releases/1.6.md).
- **1.5** – Radio: neuer Schalter „Radio: Ein“ (Klick, dann Stille, kein Rauschen mehr nach dem Abschalten) und drei Senderwechsel-Geräusche, zufällig gewählt, nie zweimal dieselbe hintereinander. Die zwei zusätzlichen Senderwechsel sind aus der vorhandenen Aufnahme abgeleitet (anderes Drehtempo). Anregung aus dem Beta-Test.
- **1.4** – Echtzeitsichere Blockverarbeitung, echtzeitsichere Latenzmeldung, verzögerungskompensierter Bypass, geglättete Filterwechsel, erhaltene Geräuschfahnen und unabhängige Fenster. Details: [Technische Prüfung](docs/TECHNICAL_REVIEW.md), [Qualitätsbericht](docs/QUALITY_REPORT.md).
- **1.3** – Hilfe und README allgemein für jede DAW formuliert (kein Bezug mehr auf eine bestimmte DAW).
- **1.2** – Latenz von 80 auf 30 ms gesenkt (gemessen, nicht geschätzt), im Radio- und Telefonmodus 0.
- **1.1** – Radio: neuer Regler „Störungen“ (Empfangsschwund mit Gruscheln, einstellbar); Squelch und Senderwechsel
  nutzen jetzt eingebaute Aufnahmen aus der HörTalk-Community statt reiner Synthese.
- **1.0** – Untertitel „Audio Drama Radio & Phone Simulator“, Versionsnummer auf 1.0 gesetzt (die 0.x-Versionen waren Testläufe).
- **0.1.0** – Erste Testversion: Modi Funk (Walkie-Talkie, Autofunk, Tischfunk; Sprechtaste mit Automatik,
  Abhacken), Radio (Rauschen, Knacken, Schwund, Senderwechsel) und Telefon (Leitungszustände, Wählscheibe und
  Tasten, Hörergeräusche); Oberfläche im Stil von LOCATIONS mit Hilfe, Hell/Dunkel und skalierbarem Fenster.
- **0.0.2** – Parameterliste festgelegt.
- **0.0.1** – Platzhalter.

## Aufbau

- `Source/RadiophoneDSP.h`: Klang-Engine (reines C++). Die Klangwerte je Gerät stehen oben in `voicing()`.
- `Source/PluginProcessor.*`, `Source/PluginEditor.*`: JUCE-Plugin und Oberfläche
- `.github/workflows/build.yml`: Windows-Bau, Regressionstests und pluginval; Releases nur nach explizitem Workflow-Start mit `publish`, siehe [docs/CI.md](docs/CI.md)

## Qualität und Prüfung

Wenige Regler heißen nicht, dass hier etwas schnell zusammengeschustert wurde. Die Einfachheit liegt in der Bedienung, nicht in der Technik dahinter. Jede Version durchläuft Prüfungen, wie sie auch bei kommerziellen Plugins üblich sind, und alles ist im Quellcode nachvollziehbar:

- **pluginval, Stufe 8:** Vor jeder Veröffentlichung läuft das Prüfprogramm pluginval von Tracktion auf strenger Stufe. Es lädt das Plugin wieder und wieder, wechselt Abtastraten und Puffergrößen, speichert und lädt Einstellungen und fährt alle Regler mit Zufallswerten durch.
- **Automatische Tests der Klangberechnung**, zusätzlich mit Speicher- und Fehlerprüfern (AddressSanitizer, UndefinedBehaviorSanitizer), die auch seltene Programmierfehler aufspüren.
- **Echtzeitsicher:** Während der Wiedergabe wird kein Speicher angefordert und nirgends gewartet. Das verhindert Aussetzer und Knackser, auch bei kleinen Puffern und vielen Instanzen.
- **Bewährte Bausteine:** Die Sättigung folgt dem Algorithmus Airwindows Density von Chris Johnson, dessen Plugins in vielen Studios im Einsatz sind.
- **Gemessene Rechenlast:** Die Klangberechnung ist mit Benchmarks vermessen, die Rohwerte liegen im Repository.
- Die Verzögerung wird an die DAW gemeldet und beim Bypass ausgeglichen, die Spur bleibt also immer synchron.

Ob es gut klingt, entscheidet am Ende das Ohr. Deshalb geht jede Version auch durch den Studioalltag von Markus Hildebrandt (siehe unten).

## Beta-Test und Danke

**SOUNDJOB / Markus Hildebrandt** – Beta-Testing

Ein großes Dankeschön an **Markus Hildebrandt** von SOUNDJOB. Markus betreibt ein eigenes Tonstudio und hat weit mehr Hörspiele produziert als ich. Er hat RADIOPHONE im Studioalltag getestet und mir ehrliches, fachkundiges Feedback gegeben. Auf seine Anregung hin gibt es seit Version 1.5 den Schalter „Radio: Ein“ mit Abschaltklick und echter Stille danach, dazu zufällig wechselnde Senderwechsel-Geräusche, seit 1.7 das Auflegen der Leitung, das zu jedem Telefon passt, und wechselnde Squelches beim Walkie-Talkie. Dafür danke ich ihm sehr: Seine Erfahrung hat das Plugin spürbar besser gemacht.

Gebaut mit [JUCE](https://juce.com) 8. Die Sättigung (übersteuerter Funk- und Lautsprecherklang) folgt dem
Algorithmus von **Airwindows Density** von Chris Johnson ([github.com/airwindows](https://github.com/airwindows/airwindows), MIT-Lizenz).
Squelch und Senderwechsel nutzen Aufnahmen aus der HörTalk-Community, die dort frei nutzbar sind; die übrigen Geräusche werden im Plugin erzeugt.

---

## Hinweis / Disclaimer

**Open Source · Work in Progress**

Dieses Plugin wurde in erster Linie für meinen persönlichen Workflow bei der Hörspielproduktion entwickelt. Es ist auf meine eigenen Anforderungen zugeschnitten und muss daher nicht für jeden Anwendungsfall geeignet sein.

Das Projekt wird laufend weiterentwickelt. Trotz sorgfältiger Tests können Fehler oder Kompatibilitätsprobleme auftreten. Die Software wird ohne Gewährleistung bereitgestellt; die Nutzung erfolgt auf eigene Verantwortung. Eine Haftung für mögliche Schäden oder Datenverluste wird, soweit gesetzlich zulässig, ausgeschlossen. Es gelten die jeweiligen Lizenzbedingungen.

**Feedback, Fehlermeldungen und Verbesserungsvorschläge sind herzlich willkommen!**

Wenn dieses kleine Werkzeug auch anderen die Hörspielarbeit erleichtert und mehr Raum für Geschichten und menschliche Kreativität schafft, freut mich das ganz besonders.

Öffentliches Versionsschema und Release-Ablauf: [Versionsnummern](docs/VERSIONING.md).
