# Reproduzierbare Screenshots

Die PNGs werden aus der echten JUCE-Oberfläche erzeugt, einschließlich der aus `VERSION`
kompilierten Versionsanzeige. Keine nachträglich ersetzten Versionsziffern.

```sh
cmake -S . -B build
cmake --build build --config Release --target RadiophoneScreenshot
# Windows:
build/RadiophoneScreenshot_artefacts/Release/RadiophoneScreenshot.exe <absoluter-Pfad-zum-docs-Ordner>
```

Unter Linux benötigt das Werkzeug eine X11-Sitzung (z. B. Xvfb); der ausführbare Name
hat dort kein `.exe`. Es nutzt die echte Message-Loop und schreibt Aufnahmen mit
zweifacher Pixelauflösung. Theme und Beispielparameter betreffen ausschließlich die
lokale Screenshot-Instanz, keine Plugin-Defaults oder gespeicherten Nutzerprojekte.

Vor einer Versionsveröffentlichung die Screenshots neu erzeugen und die sichtbare
Versionszeile mit `VERSION` abgleichen. Das Ziel ist vom normalen Build ausgenommen.

Das Werkzeug schreibt außerdem `docs/screenshots.json` mit der einkompilierten Version
und den erzeugten PNG-Dateien. Diesen Manifest-Eintrag nicht manuell hochsetzen.
Vor Veröffentlichung prüft der Workflow die Übereinstimmung mit `VERSION` und das
Vorhandensein aller aufgeführten Bilder. Das Manifest wird als Release-Asset angehängt,
damit die Homepage künftige Aufnahmen aus genau dem freigegebenen Release-Tag laden kann.
