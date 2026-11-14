# RADIOPHONE – Funk, Radio und Telefon für Hörspiele

<sub>Stromschlag Audio · in Entwicklung</sub>

VST3-Insert-Effekt für Hörspiel-Produktion. Er legt den Klang eines Funkgeräts, eines Radios oder eines
Telefons über eine Spur. Das Signal läuft zu 100 % durch den Effekt, es gibt keinen Mix-Regler und keinen
Ausgangspegel. Die Geräusche sind fest eingebaut.

**Status:** Platzhalter. Das Plugin befindet sich in der Entwicklung, eine Version zum Download gibt es noch nicht.

## Modi

Ein Dropdown wählt den Modus: **Funk**, **Radio** oder **Telefon**. Die Oberfläche folgt dem Look von LOCATIONS
(Dunkel oder Hell).

### Funk

- **Sprechtaste** (automatisierbar, 0 oder 1): Der Wechsel von 0 auf 1 löst das Öffnungsgeräusch aus, der Wechsel
  von 1 auf 0 das Schließgeräusch. Pausen innerhalb einer Durchsage bleiben bei 1 und lösen nichts aus.
- **Gerätetyp:** Walkie-Talkie, Autofunk, Tischfunk
- **Versatz Öffnungsgeräusch** (ms)
- **Länge Rauschschwanz** (ms)
- **Lautstärke Geräusche**
- **Gate:** Schwelle, Haltezeit, Abknack-Stärke. Das Gate gibt es nur im Funkmodus.

### Radio

- Durchgehendes Grundrauschen, fest eingestellt
- Störimpulse mit fester, zufälliger Verteilung, fest eingestellt
- **Senderwechsel** (automatisierbarer Impuls)

### Telefon

- **Wählton** an oder aus
- **Abheben**, **Wählen** und **Auflegen** als automatisierbare Auslöser
- Leitungsrauschen, klein, fest eingestellt

## Klangbild (alle Modi)

Bandbreite (Tief- und Hochschnitt), Verzerrung, Lo-Fi und Lautsprecher-Charakter sind fest auf den Klang des
jeweiligen Modus abgestimmt. Es gibt dafür keine Regler, damit das Plugin gleich klingt.

## Entwicklung

Umgesetzt mit JUCE als VST3-Plugin für Windows.

---

– Lars Eickstädt, Stromschlag Audio
