# Ozark Menu-Base — 1:1-Port auf PS4 (InsulinGTAV)

**Datum:** 2026-08-09
**Status:** Design bestätigt (Ansatz A: source-treuer Port mit dünnem Platform-Layer)

## Ziel

Die Menu-Base des Ozark-Menüs (PC, V34-Source) wird 1:1 in das PS4-GHPLUGIN-Projekt
**InsulinGTAV** portiert (OpenOrbis-Toolchain + GoldHEN SDK, Ziel `build/InsulinGTAV.prx`,
GTA V PS4 **CUSA00411 v1.57**, Story Mode). „1:1" heißt: Ozarks Dateibaum, Namespaces,
Klassennamen und Logik bleiben identisch, sodass spätere Ozark-Feature-Dateien per
Copy + Minimal-Diff nachgezogen werden können. Features selbst sind **nicht** Teil dieses
Projekts — nur das Submenu-System mit einem Demo-Skelett.

## Referenzen

| Was | Pfad | Rolle |
|---|---|---|
| Ozark-Source (PC) | `C:\Users\BBC\Desktop\GTA\GTA5Menus-main\ozark` | Port-Vorlage (Ground Truth für Verhalten/Look) |
| InsulinGTA5 | `E:\Projects\PS4\InsulinGTA5` | **Nur** Invoker-Referenz: `src/rage/invoker/{invoker.h,invoker.cpp,natives.h,scaleform.h}`, `src/rage/types/base_types.h` |
| „Basic"-Invoker | `C:\Users\BBC\Desktop\Basic\Basic` | Sekundär-Referenz (siehe Invoker-Abschnitt) |

## Scope

**Portiert wird (1:1, Ozark-Pfade beibehalten):**

- `menu/base/base.{h,cpp}` — Open-State, Input-Gate, Control-Disables, Tick
- `menu/base/renderer.{h,cpp}` — Zeichnen via Game-Natives (draw_rect/draw_text/draw_sprite)
- `menu/base/submenu.{h,cpp}`, `menu/base/submenu_handler.{h,cpp}` — Submenu-System
- `menu/base/options/` — alle 9 Options-Typen: `option` (Basis), `button`, `toggle`,
  `number`, `scroll`, `radio`, `color_option`, `submenu_option`, `break`
- `menu/base/util/`: `menu_input` (Navigation), `control` (Control-Disables), `fonts`,
  `input` (On-Screen-Keyboard), `instructionals` (Button-Leiste), `notify` +
  `stacked_display` (Notifications), `timers`, `global.h`
- `menu/base/submenus/main.{h,cpp}` — als **Skelett**: Top-Level-Einträge als Stubs
  (Untermenüs mit Titel + Beispieloptionen), damit das System demonstrierbar ist
- `global/ui_vars.{h,cpp}` — UI-Konfiguration (Positionen, Farben, Max-Options, Open-Key)
- `global/vars.{h,cpp}` — minimales Subset (nur was die Base referenziert)
- `util/math.h`

**Nicht portiert (bewusst):**

- Alle Feature-Submenüs (player/vehicle/weapon/world/teleport/spawner/misc/settings-Inhalte)
- Kompletter `network/`-Baum, alle Netzwerk-/Session-Hooks, `auth`/`security`, curl/json
- `util/wic` (Windows-Texturloader), `menu/base/util/textures.cpp` — Sprites laufen über
  Sentinel-Quads bzw. Game-YTDs (`commonmenu`); Custom-YTD ist on-console ungelöst
- `util/fiber`, `util/fiber_pool`, `util/threads` — die Base läuft vollständig im
  Game-Script-Thread, eigene Threads braucht sie nicht
- `util/hooking` (MinHook), `util/memory` (PC-Sigscan), `util/xml`, `util/config`,
  `util/localization`, `wmi`, Konsolenfenster/SEH-Exception-Filter

## Architektur

```
InsulinGTAV/src/
  InsulinGTAV.cpp          module_start: Base-Resolve, Tick-Hook installieren
  rage/
    invoker/invoker.{h,cpp}   aus InsulinGTA5 übernommen + setVectors-Fixup (neu)
    invoker/natives.h         generierte v1.57-Natives (snake_case, RVA-basiert)
    invoker/scaleform.h       handgewrappte Scaleform-Natives
    types/base_types.h
  platform/
    stdafx.h               Shim statt Ozark-stdafx: PS4-Includes, XOR()->Passthrough,
                           POD-Typen (color_rgba, …), kein Windows
    log.{h,cpp}            LOG()-Makros -> GoldHEN-Notification + klog
    pad.{h,cpp}            scePad-Abfrage (nur Fallback, s. Input)
  menu/ …                  1:1 Ozark-Baum (s. Scope)
  global/ …                1:1 Ozark (ui_vars, vars-Subset)
  util/math.h              1:1 Ozark
```

- **Native-Callsites:** Ozark ruft `native::snake_case(…)` — exakt die Konvention unserer
  generierten `natives.h`. Callsites bleiben unverändert; fehlende Natives werden ergänzt
  (IDA-validiert, ggf. `MANUAL_RVA`, Technik wie bei `scaleform.h`).
- **GHPLUGIN-Regel:** `.init_array` läuft nicht → keine globalen Objekte mit Konstruktoren.
  Globals sind POD/`constexpr`; Singletons als function-local statics (Ozark macht das mit
  `get_base()`/`get_renderer()` bereits richtig).

## Invoker-Entscheidung

Es bleibt bei **unserem Invoker** aus InsulinGTA5 (on-console validiert für v1.57,
Laufzeit-Base-Resolve mit Guard, Stack-Kontext pro Aufruf/reentrant, variadisches Template).

Der „Basic"-Invoker (`C:\Users\BBC\Desktop\Basic\Basic`) wurde geprüft:

- Seine `natives.h` zielt auf eine **andere Spielversion**: 0 von 2839 gemeinsamen Natives
  haben denselben RVA, Abweichungen regional −0x19A0…−0x5400 → **nicht** als RVA-Quelle
  nutzbar.
- **Übernommen wird:** der `setVectors()`-Mechanismus (Vector3-Out-Param-Fixup: Natives
  schreiben Vector4-Slots, die nach dem Call in die Vector3-Pointer der Argumente
  zurückkopiert werden). Unser Invoker hat die Kontext-Felder bereits, führt den Fixup aber
  nie aus — ohne ihn liefern Natives wie `GET_MODEL_DIMENSIONS` keine Ergebnisse.
- **Als Referenz genutzt:** 6492 benannte Natives mit PC-Hash-Kommentaren (unsere: 2840)
  für Namens-Cross-Check und als Suchhilfe (Basic-RVA + Regional-Delta) beim
  IDA-Verifizieren von Drift-Verdachtsfällen.
- Basic belegt außerdem, dass Control-Natives-Input on-console funktioniert (s. Input).

## Tick & Entry

Wie Ozark (Script-Hook in `main_persistent` ruft `menu::base::update()`) und wie
InsulinGTA5: unser **Native-Hook** ruft `menu::base::update()` **1×/Frame im
Game-Script-Thread**. `module_start` macht ausschließlich: Base-Resolve → Hook
installieren → Notify „loaded". Kein eigener Thread, keine Fibers.

## Input

`menu_input` wird **1:1 mit Control-Natives** portiert (`is_control_pressed`,
`is_control_just_pressed`, `disable_control_action`, `set_input_exclusive`, …).
Das frühere „disable_control_action driftet"-Problem war Alignment-Drift, kein
prinzipielles Problem — Basic beweist, dass diese Natives on-console funktionieren.

- **Vorab (Teil von M1):** Die ~6 Input-Natives werden in IDA gegen Drift verifiziert und
  bei Bedarf per `MANUAL_RVA` festgenagelt.
- **Fallback (dokumentiert, nicht Default):** scePad-Polling über `platform/pad`, falls
  einzelne Control-Natives doch nicht sauber arbeiten.
- **Öffnen-Kombo:** **L1 + ○** (wie Basic: `INPUT_FRONTEND_LB` + `INPUT_FRONTEND_CANCEL`),
  konfigurierbar über `ui_vars` (ersetzt Ozarks `VK_F4`).

## On-Screen-Keyboard

Ozarks `menu/base/util/input.cpp` (Texteingabe für number/text-Optionen) wird auf das
Game-Keyboard-Native (`display_onscreen_keyboard`-Familie) umgestellt — dieselbe Technik
ist in InsulinGTA5 Phase 2a bereits on-console gelaufen. API von `menu::input` bleibt
unverändert.

## STL-Risiko & Milestone 0

Ozark nutzt `std::string`, `std::vector`, `std::shared_ptr` durchgehend (u. a. in
Options-Listen und Renderer-Signaturen). Ob libc++ im GHPLUGIN-Kontext (ohne
`.init_array`) vollständig funktioniert, ist unbewiesen → **M0 gate-t den ganzen Port:**

- M0-Smoke-Test im .prx on-console: `std::string`-Ops (append/format), `std::vector`
  push/grow, `std::shared_ptr`, function-local static mit Konstruktor (`__cxa_guard`),
  Heap-Allokation im Game-Prozess.
- Schlägt M0 fehl → Fallback: API-gleiche Eigen-Typen (`fixed_string`,
  Mini-`vector`/`shared_ptr`) im `platform/`-Layer; die Ozark-Dateien bleiben trotzdem
  unverändert (Typen kommen über das `stdafx.h`-Shim herein).

## Logging & Fehlerbehandlung

- Ozark `LOG*`-Makros → `platform/log`: GoldHEN-Notification (sparsam) + klog.
- `XOR("…")` → Passthrough-Makro (keine String-Verschlüsselung auf PS4 nötig).
- Kein SEH/Exception-Filter (existiert auf PS4 nicht); Invoker-Guard (Base==0 → no-op)
  bleibt als Schutz.

## Milestones & Validierung (jeweils on-console)

| # | Inhalt | Akzeptanz |
|---|---|---|
| M0 | STL-Smoke-Test im .prx | alle Tests grün via Notify/klog |
| M1 | Invoker + setVectors, Tick-Hook, Input-Natives verifiziert, `base` + `renderer` + leeres Main-Submenü | Menü öffnet mit L1+○, Header/Background rendern im Ozark-Look |
| M2 | Submenu-System + alle 9 Options-Typen im Demo-Skelett | Navigation (rein/raus/scrollen/wrap), jeder Options-Typ bedienbar |
| M3 | Instructionals, Notify/Stacked-Display, On-Screen-Keyboard | Button-Leiste korrekt, Test-Notification, Texteingabe in number-Option |

## Risiken

- **STL im GHPLUGIN** — durch M0 vorgezogen; Fallback definiert.
- **Native-Drift** in von der Base genutzten Natives (Input, HUD, Scaleform) — Verifikation
  per IDA + Basic-Referenz; Korrektur per `MANUAL_RVA`.
- **Sprites/Texturen:** Ozark-Default rendert ohne PNGs (Sentinel-Quads); sollte `notify`
  Icons aus Custom-Dicts erwarten, werden sie durch Game-YTD-Assets (`commonmenu`) ersetzt.
- **Vector-Fixup:** neue Invoker-Funktionalität → in M1 mit einem bekannten
  Vector3-Out-Native gegentesten.
