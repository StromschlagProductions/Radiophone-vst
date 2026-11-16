# CI-Nutzung (GitHub Actions)

Stand: 2026-11-16. Das GitHub-Konto hat nur noch wenige der enthaltenen Actions-Minuten. Windows-Läufe zählen doppelt.
Alle Prüfungen bleiben erhalten (Windows-Bau, Regressionstests, pluginval Strictness 8, ASan/UBSan, TSan). Nur das Wann ändert sich.

| Workflow | Läuft bei | Läuft nicht bei |
|---|---|---|
| `build.yml` Windows-VST3 (Build, Regressionstests, pluginval 8, ZIP) | **Run workflow** von Hand | Push, Pull Request, Doku |
| `sanitizers.yml` DSP-Sanitizer (Linux, ohne JUCE) | Push auf `main` und PR (nicht Draft), nur bei Änderungen an `Source/`, `Tests/`, `CMakeLists.txt`, `VERSION`; von Hand | Doku, README |

- Release nur mit Haken **publish** beim manuellen Start, nur auf `main`. Es wird nie automatisch veröffentlicht oder gemergt.
- `concurrency` bricht überholte Läufe ab (nicht den Release-Lauf). Windows-Timeout 30 statt 45 Minuten. Artefakt 7 Tage.

## Ablauf

1. Lokal bauen und testen.
2. Zur Abnahme: Actions > „Windows-VST3 bauen“ > „Run workflow“ auf `main`, zunächst ohne Haken.
3. Wenn alles passt und gehört wurde: dieselbe Aktion mit Haken **publish**.
