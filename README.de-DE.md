[![windows](https://github.com/TheAssemblyArmada/Thyme/workflows/windows/badge.svg)](https://github.com/TheAssemblyArmarda/Thyme/actions)  
[![linux](https://github.com/TheAssemblyArmada/Thyme/workflows/linux/badge.svg)](https://github.com/TheAssemblyArmarda/Thyme/actions)
[![codecov](https://codecov.io/gh/TheAssemblyArmada/Thyme/branch/develop/graph/badge.svg)](https://codecov.io/gh/TheAssemblyArmada/Thyme)

[![en-GB](https://img.shields.io/badge/lang-en--GB-green.svg)](https://github.com/TheAssemblyArmada/Thyme/README.md)
[![de-DE](https://img.shields.io/badge/lang-de--DE-green.svg)](https://github.com/TheAssemblyArmada/Thyme/README.de-DE.md)

**Thyme** ist eine Open-Source re-implementation des Spiels Command & Conquer Generals: Zero Hour.  
Zero Hour ist die Erweiterung des Spiels Command & Conquer Generals, welches 2003 veröffentlicht wurde. 

Dieses Projekt ist eine vollständige Neuprogrammierung von Command & Conquer Generals: Zero Hour, welche die original 
Dateien nutzt, um Funktionen zu unterstützen, welche noch nicht implementiert werden konnten.
Unsere Intention ist es das Beheben von Fehlern zu ermöglichen, neue Funktionen zu implementieren und Plattformen zu
unterstützen, welche vom Original nicht unterstützt werden.

Bitte beachten Sie, dass wir in keiner Weise für EA arbeiten.  
EA hat dieses Project in keiner Weise befürwortet und unterstützt dieses Projekt nicht.


## Chat

Sie können sich mit uns auf unserem [Discord Kanal](https://discord.gg/UnWK2Tw) über das Projekt mit uns unterhalten 
und diskutieren.


## Thyme ausführen

**Stand heute (Feb. 2023) läuft Thyme ausschließlich unter Windows und muss selbst kompiliert werden.**

Um Thyme ausführen zu können muss eine der folgenden Voraussetzungen erfüllt sein:

1. Sie sind im Besitz einer Installation von einer Original-CD des Spiels in englischer Fassung 
(**NICHT** "Die ersten 10 Jahre", "The Ultimate Collection" oder ähnliche Sammlungen / Bundles), welche auf die Version 
1.04 aktualisiert sein **muss**.
2. Sie kommen an die Datei `game.dat` von einer unter 1. genannten Installation heran und haben diese in Ihr 
Installationsverzeichnis von C&C Generals ZH kopiert

Wenn Sie eine der genannten Möglichkeiten bereit haben können Sie beginnen die `thyme.dll` und die `avifil32.dll` zu 
kompilieren, wie [hier](#thyme-selbst-kompilieren) beschrieben.

Wenn Sie die DLLs erfolgreich erstellt haben, kopieren Sie diese in das Installationsverzeichnis von C&C Generals ZH.

Nun benennen Sie die `game.dat` im Installationsverzeichnis zu `game.exe` um (der Name spielt keine Rolle, 
solange die Datei die Endung .exe hat).

Herzlichen Glückwunsch, Sie sollten nun Thyme durch das Ausführen der `game.exe` starten können!

Um zu überprüfen, ob Thyme korrekt geladen wurde können Sie das Spiel von der Konsole / PowerShell aus starten.  
Klicken Sie hierzu in dem Installationsverzeichnis mit der rechten Maustaste, während sie die Shift-Taste gedrückt halten  
und wählen Sie "PowerShell-Fenster hier öffnen".  
Geben Sie nun folgendes ein und bestätigen Sie mit Enter:

`.\game.exe -win`

Dies sollte das Spiel im Fenster-Modus starten, so dass Sie den Fenstertitel sehen können, welcher den Begriff "Thyme" 
enthalten sollte.

Wenn Sie irgendwelche Probleme haben sollten, wenn Sie dieser Anleitung folgen, können Sie uns jederzeit auf unserem 
[Discord Kanal](#chat) erreichen (in Englisch).


### Haftungsausschluss
Thyme wird stetig weiterentwickelt, sodass infolgedessen die Entwicklungsversionen sich möglicherweise nicht wie 
erwartet verhalten.
Wir bemühen uns stets nummerierte "Releases" zu veröffentlichen, welche so frei von Fehlern wie möglich sind, jedoch
können selbst diese sich unvorhergesehen verhalten.

### Virenscanner-Warnungen
Anti-Viren-Software wie der "Windows Defender" können Thyme fälschlicherweise als gefährlich einstufen.  
Hierbei handelt es sich um eine falsch-positive Meldung. Wir können versichern, dass es vollständig sicher ist Thyme zu 
nutzen. Wenn Sie immer noch unsicher bezüglich der Sicherheit sind, steht es Ihenen frei Thyme 
[selbst zu kompilieren](#thyme-selbst-kompilieren).

### Häufige Probleme

Ein schwarzer Bildschirm beim Start ist normalerweise ein zeichen das Sie ein "gentool" installiert haben.  
Aufgrund dessen wie Thyme aktuell funktioniert müssen Sie temporär die "gentool"-Datei `d3d8.dll` umbenennen oder entfernen.

## Thyme selbst kompilieren

Um eine funktionierende Datei zu erzeugen, müssen Sie das Projekt derzeit selbst mithilfe von Microsoft Visual Studio 
(2017 oder neuer) kompilieren.  
Das Kompilieren mit Clang gegen ein Windows SDK ist theoretisch für eine "Cross-compile" Lösung möglich aber zurzeit 
nicht getestet.

Es ist ebenfalls möglich eine "Standalone Binary" zu kompilieren, wenn Sie das Kompilieren für andere Platformen testen wollen.

Für [nähere Information](https://github.com/TheAssemblyArmada/Thyme/wiki/Compiling-Thyme) schauen Sie ins 
[Thyme Wiki](https://github.com/TheAssemblyArmada/Thyme/wiki). 

### Linux und MacOS

Ein nativer Support für Linux und MacOS ist für die Zukunft geplant, wird jedoch aufgrund dessen, wie das Projekt entwickelt 
wird, für einige Zeit nicht möglich sein. 
Währenddessen sollte es möglich sein "Wine" unter Linux und MacOS zu nutzen. Dies ist jedoch nicht getestet worden.

Wenn Sie an der Entwicklung des "Cross-Plattform" Aspekts des Projekts interessiert sind, ist es nun möglich ein 
"standalone" kompilat durch das Übergeben von ```-DSTANDALONE=TRUE``` an die CMake Konfiguration zu erzeugen.
Die erzeugte "Binary" hat aktuell jedoch eine sehr eingeschränkte Funktionalität.


## Etwas beitragen

Wenn Sie daran interessiert sind etwas zu Thyme beizutragen benötigen Sie mindestens C++ Kenntnisse.  
Treten Sie dem Entwickler-Chat, welcher [oben](#chat) aufgeführt ist, bei um mehr Informationen zu erhalten, was Sie 
sonst noch brauchen und um die idc-Dateien für die aktuelle "Binary" zu erhalten.

Für mehr Informationen können Sie ebenfalls einen Blick ins [Wiki](https://github.com/TheAssemblyArmada/Thyme/wiki) werfen.

## Lizenz

Der Quellcode, welcher in diesem "Repository" für Thyme zur Verfügung gestellt wird, ist unter der 
[GNU General Public License version 2](https://www.gnu.de/documents/gpl-2.0.de.html) oder neuer mit einer Ausnahme,
welche es erlaubt den resultierenden Quellcode gegen eine geschlossene ("closed source") "Binary" zu verlinken, bis Thyme
diese nicht länger benötigt, um lauffähig zu sein, lizenziert.

## Andere Open-Source Projekte

Unten ist eine Liste mit ähnlichen Projekten und den entsprechenden Originalspielen

 * [Alive](https://github.com/AliveTeam/alive_reversing) - Abe's Exodus
 * [dethrace](https://github.com/jeff-1amstudios/dethrace) - Carmageddon
 * [Devilution](https://github.com/diasurgical/devilution) - Diablo
 * [OpenDUNE](https://github.com/OpenDUNE/OpenDUNE) - Dune 2
 * [OpenFodder](https://github.com/OpenFodder/openfodder) - Cannon Fodder
 * [OpenLoco](https://github.com/OpenLoco/OpenLoco) - Locomotion 
 * [OpenMC2](https://github.com/LRFLEW/OpenMC2) - Midnight Club 2
 * [OpenRCT2](https://github.com/OpenRCT2/OpenRCT2) - RollerCoaster Tycoon 2
 * [OpenTTD](https://www.openttd.org) - Transport Tycoon Deluxe
 * [Vanilla Conquer](https://github.com/TheAssemblyArmada/Vanilla-Conquer) - Tiberian Dawn & Red Alert

Es gibt ebenfalls eine [Wikipedia Seite für Open-Source Spiele](https://de.wikipedia.org/wiki/Liste_quelloffener_Computerspiele).
