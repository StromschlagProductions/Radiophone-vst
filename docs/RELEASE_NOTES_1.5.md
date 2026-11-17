# Radiophone 1.5 – Radio gezielt abschalten und wechselnde Sendergeräusche

Markus' Vorschläge sind enthalten und geprüft:

- **Radio: Ein** lässt sich per Schalter und Automation abschalten. Nach dem kurzen Ausblenden und dem Abschaltklick bleiben Stimme, Rauschen und Knacken still, auch bei weiterlaufendem Eingang. Andere Modi werden nicht stummgeschaltet.
- **Senderwechsel** wählt aus drei Varianten, ohne dieselbe Variante zweimal hintereinander abzuspielen. Die beiden zusätzlichen Varianten sind aus der vorhandenen Aufnahme mit verändertem Drehtempo abgeleitet; es sind keine unabhängig neu aufgenommenen Senderwechsel.
- Der neue Radio-Ein/Aus-Zustand wird mit dem Projekt gespeichert. Ein zusätzlicher Prozessortest prüft seine Wiederherstellung.

Die bereits vorhandenen Echtzeitsicherheits-Verbesserungen aus 1.4 bleiben erhalten. DSP-Tests prüfen unter anderem 22.050–192.000 Hz, unterschiedliche Blockgrößen, Mode-Wechsel, Radio-Aus, Variantenwahl, Stille, ungültige Werte und Speicherallokationen. Die Veröffentlichung erfolgt nach bestandenem Windows-Build, Regressionstests und pluginval Level 8.

Installation: DAW schließen, ZIP entpacken und den ganzen Ordner `Radiophone.vst3` nach `C:\Program Files\Common Files\VST3` kopieren. Alte Releases bleiben erhalten.
