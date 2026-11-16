# Continuous Integration (GitHub Actions)

Stand: Radiophone 1.4. Ziel: wenig Actions-Verbrauch, aber vollständige Windows-Prüfung bei der Abnahme.

## Wann läuft was

| Workflow | Auslöser | Kosten |
|---|---|---|
| **Windows-VST3 bauen** (`build.yml`) | Push auf `main` (ohne reine Doku-Änderungen) oder manuell (`workflow_dispatch`) | hoch: Windows-Minuten zählen doppelt. Bau, Regressionstests (ctest) und pluginval Stufe 8. |
| **DSP sanitizers** (`sanitizers.yml`) | Push auf `main` nur bei Änderungen an DSP, Tests, CMake oder dem Workflow selbst, oder manuell | gering: Linux, zwei Läufe (Address/UB und Thread) |

Zwischencommits auf Branches und Pull Requests lösen **keinen** Lauf mehr aus. Frühere Läufe liefen bei jedem Push auf einen PR doppelt.

Keine Releases und keine automatischen Merges. Ein Release entsteht nur, wenn der Windows-Workflow manuell mit `publish` gestartet wird.

Laufende Läufe desselben Workflows auf demselben Branch werden durch `concurrency` abgebrochen, sobald ein neuerer startet.

## Empfohlener Ablauf

1. Entwicklung auf einem Branch, lokal bauen und testen:
   ```
   cmake -S . -B build -DRADIOPHONE_DSP_ONLY=ON && cmake --build build && ctest --test-dir build
   ```
   (Das Plugin selbst braucht JUCE, siehe `RADIOPHONE_JUCE_PATH` in `CMakeLists.txt`.)
2. Vor dem Merge: den Windows-Workflow einmal manuell starten (`workflow_dispatch`), wenn der Branch Plugin-Code verändert hat.
3. Nach dem Merge auf `main` läuft der Windows-Bau automatisch. Das ist die abschließende Abnahme.
4. Ein Release nur bewusst mit `publish` anstoßen.

## Was entfernt wurde

- `pull_request`-Trigger in beiden Workflows: jeder Push auf einen PR hat bisher einen vollen Windows-Bau ausgelöst.
- Die Sanitizer laufen nicht mehr bei jeder Doku-Änderung auf `main`.

## Was bleibt

Alle Prüfungen sind unverändert vorhanden: Windows-Bau, ctest, pluginval Stufe 8, ASan/UBSan und TSan. Verringert wurde nur, wann sie laufen.
