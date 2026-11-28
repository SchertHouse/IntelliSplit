# IntelliSplit

IntelliSplit è un plug-in MIDI scritto in C++ e basato su iPlug2. Analizza l'attività delle note in ingresso, assegna dinamicamente le note alla mano sinistra o destra e le instrada verso canali MIDI e transposizioni indipendenti.

I formati compilabili su Windows sono VST3, VST2, AAX e standalone; il repository contiene anche file di progetto per macOS, iOS e web.

## Requisiti

### Windows

- Windows 10/11
- Visual Studio 2022
- Workload **Desktop development with C++**
- MSVC v143
- Windows 10 SDK
- Python 3.12
- iPlug2 and its SDK/dependencies

Il progetto usa `projects\IntelliSplit-vst3.vcxproj` con toolset MSVC v143.

## Ottenere iPlug2

iPlug2 non e' incluso nel repository. Il workflow GitHub usa la revisione:

```text
61c042e936d832b6c7d93c5dd8b1e09cf2518bb9
```

Posizionare iPlug2 accanto alla cartella del repository o configurare `IPLUG2_ROOT` nel progetto. Esempio con cartelle separate:

```text
C:\Users\domen\Desktop\Domenico\Progetti\
  IntelliSplit\
    Repository\
  iPlug2\
```

Scaricare le dipendenze iPlug2 dalla relativa cartella `Dependencies\IPlug` da Git Bash o da WSL:

```bash
chmod +x download-iplug-sdks.sh
./download-iplug-sdks.sh
```

Il progetto deve trovare almeno il VST3 SDK in:

```text
<iPlug2>\Dependencies\IPlug\VST3_SDK
```

## Compilazione su Windows

Aprire **Developer Command Prompt for VS 2022** o **Developer PowerShell for VS 2022**, quindi posizionarsi nella radice del repository:

```bat
cd /d C:\Users\domen\Desktop\Domenico\Progetti\IntelliSplit\Repository
```

### Build Release x64

```bat
msbuild IntelliSplit.sln /t:IntelliSplit-vst3 /p:Configuration=Release /p:Platform=x64 /m:1 /v:minimal
```

### Build Debug x64

```bat
msbuild IntelliSplit.sln /t:IntelliSplit-vst3 /p:Configuration=Debug /p:Platform=x64 /m:1 /v:minimal
```

In entrambi i casi il progetto compila una configurazione x64. La soluzione supporta anche `Win32` e `Tracer`, ma per la maggior parte dei DAW moderni e per il debug con Cantabile usare `x64`.

## Installare o distribuire il VST3

Durante la post-build, il progetto crea il bundle:

```text
build-win\IntelliSplit.vst3
```

Per copiare automaticamente il bundle in una cartella VST3 personalizzata, passare `VST3_64_PATH` a MSBuild:

```bat
msbuild IntelliSplit.sln /t:IntelliSplit-vst3 /p:Configuration=Release /p:Platform=x64 /p:VST3_64_PATH="C:\Users\domen\Desktop\Programmi\Audio\VST" /m:1 /v:minimal
```

Con il bundle installato in una cartella personalizzata, eseguire una nuova scansione dei plugin nel DAW o riavviare il DAW.

Il bundle compilato si trova anche in:

```text
build-win\vst3\x64\<Configuration>\
build-win\IntelliSplit.vst3\
```

Se si usa il percorso standard di sistema, in genere:

```text
C:\Program Files\Common Files\VST3
```

Se il DAW usa una cartella personalizzata, usare quella cartella sia per `VST3_64_PATH` sia nelle impostazioni di scansione del DAW.

## Compilazione e debug con un DAW

### 1. Compilare in Debug e installare il bundle

```bat
msbuild IntelliSplit.sln /t:IntelliSplit-vst3 /p:Configuration=Debug /p:Platform=x64 /p:VST3_64_PATH="C:\Users\domen\Desktop\Programmi\Audio\VST" /m:1 /v:minimal
```

Sostituire il valore di `VST3_64_PATH` con una delle cartelle che il DAW effettivamente scansiona.

Il PDB generato per Visual Studio e':

```text
build-win\pdbs\IntelliSplit_x64.pdb
```

### 2. Avviare il DAW dal debugger

Aprire `IntelliSplit.sln` in Visual Studio 2022. Il progetto da compilare e':

```text
IntelliSplit-vst3
```

Configurare:

- **Configuration:** `Debug`
- **Platform:** `x64`

Il DAW puo' essere avviato in Visual Studio tramite **Debug > Start Without Debugging**, quindi collegare il debugger con **Debug > Attach to Process** e selezionare il processo del DAW.

In alternativa, avviare il DAW con:

```bat
devenv /DebugExe "C:\Percorso\Versione-x64\Cantabile.exe"
```

Chiudere quindi il DAW, compilare di nuovo in Debug e riavviarlo con il debugger. Sostituire il percorso con quello reale dell'eseguibile del proprio DAW.

### 3. Mettere un breakpoint e caricare il plugin

1. Aprire un punto di interesse in `IntelliSplit.cpp`.
2. Inserire un breakpoint.
3. Avviare il DAW o collegare Visual Studio al processo gia' avviato.
4. Riavviare la scansione dei plugin nel DAW.
5. Inserire IntelliSplit in una route MIDI.
6. Riprodurre o suonare una nota per raggiungere il breakpoint.

Per distinguere i simboli della build:

- usare il bundler compilato in `Debug`;
- non alternare tra bundle `Debug` e `Release` nello stesso percorso;
- avviare una nuova sessione del DAW dopo ogni ricompilazione.

## Visual Studio

Per usare la soluzione interattivamente:

1. Aprire `IntelliSplit.sln`.
2. Selezionare `IntelliSplit-vst3` come progetto di build.
3. Selezionare `Debug|x64` o `Release|x64`.
4. Impostare `VST3_64_PATH` nelle proprieta' utente del progetto se necessario.
5. Usare **Build > Build Solution** oppure `Ctrl+Shift+B`.

## macOS

Il repository contiene un workspace Xcode e workflow GitHub per VST3 e AU. Comandi tipici:

```bash
xcodebuild -project projects/IntelliSplit-macOS.xcodeproj \
  -target VST3 \
  -configuration Release \
  -sdk macosx \
  ARCHS="arm64 x86_64" \
  ONLY_ACTIVE_ARCH=NO \
  build
```

```bash
xcodebuild -project projects/IntelliSplit-macOS.xcodeproj \
  -target AU \
  -configuration Release \
  -sdk macosx \
  ARCHS="arm64 x86_64" \
  ONLY_ACTIVE_ARCH=NO \
  build
```

## Risultati di build

| Tipo | Percorso |
|---|---|
| DLL temporaneo VST3 | `build-win\vst3\x64\<Configuration>\IntelliSplit.vst3` |
| Bundle VST3 | `build-win\IntelliSplit.vst3` |
| PDB Debug x64 | `build-win\pdbs\IntelliSplit_x64.pdb` |

## Test manuale consigliato

1. Compilare in Debug.
2. Ricaricare o riavviare il DAW.
3. Inserire IntelliSplit su una route MIDI.
4. Abilitare input MIDI e due destinazioni.
5. Verificare:
   - assegnazione left/right;
   - canali MIDI destro e sinistro;
   - transposizioni;
   - tutte le modalita' SX/Mix/DX;
   - Smooth, Range, Min/Max e Reset;
   - note duplicate e rilascio;
   - cambiamenti dei parametri mentre le note sono tenute;
   - All Notes Off;
   - salvataggio e ripristino dello stato del rack/progetto.

## Documentazione

- [Panoramica del progetto](docs/PROJECT_OVERVIEW.md)
- [Architettura](docs/ARCHITECTURE.md)
- [Algoritmo](docs/ALGORITHM.md)
- [Routing MIDI](docs/MIDI_ROUTING.md)
- [Parametri UI](docs/UI_PARAMETERS.md)
- [Compilazione e test](docs/BUILD_AND_TEST.md)
- [Review e problemi noti](docs/CODE_REVIEW.md)
