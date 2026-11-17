# Veröffentlichung und Herstellerangaben

**Öffentlicher Hersteller:** Stromschlag Audio. **GitHub-Projekt:** StromschlagProductions/Radiophone-vst.

Änderungen dürfen mit beliebigen Entwicklungswerkzeugen (Claude, Codex, ChatGPT, IDE) erstellt werden. Diese Werkzeuge sind keine Plugin-Hersteller. In JUCE/CMake, VST3-Metadaten, Benutzeroberfläche, README und Release-Titeln wird Stromschlag Audio beziehungsweise Stromschlag Productions als Marke verwendet. Historische Git-Commit-Autoren und Co-Autoren bleiben unverändert; fremde Lizenz- und Urheberrechtshinweise dürfen nicht entfernt werden.

## Release-Ablauf

1. Code prüfen und nach `main` übernehmen.
2. Build, Regressionstests und pluginval Strictness 8 erfolgreich abschließen.
3. GitHub Actions → **Windows-VST3 bauen** → **Run workflow** auf `main` starten, Option **publish** aktivieren. Ohne ausdrückliche Freigabe entsteht kein öffentliches Release.
4. Prüfen, ob `/releases/latest` auf ein veröffentlichtes stabiles Release mit Windows-VST3-ZIP zeigt.
5. Die Stromschlag-Homepage liest dieses Release automatisch. Bei GitHub-API-Problemen führt ihr Fallback zur aktuellen Release-Seite.

Ein Git-Push oder erfolgreicher Build ist **nicht** automatisch eine Freigabe. Keine automatische öffentliche Veröffentlichung bei jedem Commit. So werden unfertige Builds und unnötige Windows-CI-Kosten vermieden.

Übergreifender Standard: https://github.com/StromschlagProductions/StromschlagProductions.github.io/blob/main/docs/RELEASE_STANDARD.md
