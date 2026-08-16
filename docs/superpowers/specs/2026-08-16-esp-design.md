# ESP: draw players and world entities through the world, the way Ozark does

Ozark's ESP is not a feature but a shared facility: an `esp_context` describing
what to draw and in which colours, a reusable settings editor that edits
whichever context is currently selected, and a handful of drawing functions that
any consumer can call for any entity. This ports that facility, fills the one
part Ozark left unimplemented, and adds the per-frame budget a 30 fps console
needs and a PC menu never did.

## What exists today

Everything the drawing needs is already in the port and verified:

| Need | Where | Note |
|---|---|---|
| world → screen | `natives.h:722` `get_screen_coord_from_world_coord` | direct RVA, returns false when off-screen |
| entity position | `missing_natives.h:112` `get_entity_coords` | Vector3 return, hand-written wrapper |
| offset in world coords | `missing_natives.h:115` `get_offset_from_entity_in_world_coords` | the 3D box is built from these |
| model bounds | `natives.h:578` `get_model_dimensions` | two Vector3 out-params |
| bone position | `natives_hash.h:798` `get_ped_bone_coords` | **hash native** - inert until `hash_natives::usable()` |
| bone index | `natives.h:632` `get_ped_bone_index` | direct RVA |
| markers | `natives.h:364` `draw_marker` | joints are marker type 28 |
| draw origin | `natives.h:1986` / `:172` | `set_draw_origin` / `clear_draw_origin` |
| health / armour | `natives.h:494`, `:629`, `:500` | entity health, ped armour, max health |
| ped tests | `natives.h:967`, `:1050` | `is_entity_a_ped`, `is_ped_a_player` |
| 2D and 3D primitives | `menu/base/renderer.h` | `draw_rect`, `draw_line`, `draw_line_2d`, `draw_text`, `calculate_string_width` |
| player list | `game/player_list.h` | `valid()`, `get()`, `local_id()`, 32 slots |
| per-frame window | `menu::tick` → `feature_update()` | runs with the menu closed, gated on `player_valid()` |

Two gaps, both small: the `SKEL_*` bone constants do not exist here and need a
small header of standard GTA V bone ids, and Ozark's `g_draw_origin_index`
global has no PS4 equivalent — `name_esp` will position text from screen
coordinates directly rather than through the draw-origin correction.

## What the reference implementations actually do

Read before designing, because the obvious guess was wrong twice.

- **Ozark** (`helper/helper_esp.cpp`) implements name, snapline, 3D box, 3D axis,
  skeleton bones, skeleton joints and weapon. Its `_2d_esp` is an **empty stub**
  and the matching menu entries are commented out. "ESP box" in Ozark means a
  *3D wireframe box in world space*, built from `get_model_dimensions` and eight
  `get_offset_from_entity_in_world_coords` corners joined by 3D lines.
- **Impulse** (`ESPMenu.cpp:249`) draws the same 3D wireframe, but with hardcoded
  ±0.5 / ±0.75 extents instead of model dimensions. It gates on
  `HasEntityClearLosToEntity`.
- **2take1** (`script.cpp:690-791`) is the only one of the four with a real
  screen-space box, and it is the source for the part Ozark left open:

      world_to_screen(pos + (0,0,-1.0)) -> feet
      world_to_screen(pos + (0,0,+0.8)) -> head
      height = |feet.y - head.y|
      width  = height / 4

  Four thin `draw_rect` edges make the frame; a health bar sits to its left,
  filled by `(health + armour) / (maxHealth + 50)`; a text column sits to its
  right; text scales with distance and the whole box is skipped past 500 m,
  where it would be too small to read.

## Scope

In: the full `esp_context` model, all drawing functions including the 2D box and
corners Ozark left unimplemented, the health bar from 2take1, the three-level
settings menu, and three consumers.

Out: Ozark's spawner manage-edit contexts, which have no equivalent submenu in
this port; line-of-sight occlusion (Impulse has it, and it costs a shape test per
entity per frame — revisit only if asked for).

## The context

`menu/base/util/esp.h`:

```cpp
struct esp_context {
    bool m_ped;                 // gates skeleton and weapon
    bool m_name, m_snapline;
    bool m_2d_box, m_2d_corners, m_healthbar;
    bool m_3d_box, m_3d_axis;
    bool m_skeleton_bones, m_skeleton_joints, m_weapon;
    int  m_name_type;           // 0 = name, 1 = name + distance
    int  m_max_distance;        // per-context cull, default 500 m
    int  m_skeleton_distance;   // bones and joints only, default 75 m

    color_rgba m_name_text_color, m_name_bg_color, m_snapline_color,
               m_2d_box_color, m_2d_corners_color, m_healthbar_color,
               m_3d_box_color, m_skeleton_bones_color,
               m_skeleton_joints_color, m_weapon_color;

    // One per colour above, same order, same names.
    bool m_name_text_rainbow, m_name_bg_rainbow, m_snapline_rainbow,
         m_2d_box_rainbow, m_2d_corners_rainbow, m_healthbar_rainbow,
         m_3d_box_rainbow, m_skeleton_bones_rainbow,
         m_skeleton_joints_rainbow, m_weapon_rainbow;
};
```

**One deliberate divergence from Ozark.** Ozark gives every element its own
`menu::rainbow` object. This port already has a *shared* animator with a
registry — `get_rainbow()->add(&colour)` registers a colour and one pass per
frame cycles all of them, and `menu::tick` already calls it. So each element
carries a `bool` that registers or unregisters its colour instead of owning an
animator. Same behaviour, nine fewer objects per context, and it uses the
registry for exactly what it was built for.

`m_max_distance` is new. Ozark had a single global draw distance; making it
per-context lets world peds be culled hard while session players stay visible.

**Corrected against what was built.** Both distances are `int` metres, not
`float`: `number_option` and `scroll_option` bind `int&`, and a cull radius in
whole metres loses nothing. And the skeleton radius named under "The per-frame
budget" below is per-context (`m_skeleton_distance`, default 75 m and editable
from the menu) rather than the single shared constant this section originally
implied.

## Drawing

`menu/base/util/esp.cpp`, named after Ozark's functions so the lineage stays
readable: `name_esp`, `snapline_esp`, `box_2d_esp`, `box_3d_esp`,
`skeleton_esp`, `weapon_esp`. Each takes `const esp_context&` and an `Entity`,
and each re-checks the entity itself rather than trusting its caller: an entity
can die between the loop that selected it and the call that draws it, and
`get_entity_coords` returning null is the abort.

`box_2d_esp` follows 2take1 exactly (above). `box_3d_esp` follows Ozark: model
dimensions to a radius, eight corners through
`get_offset_from_entity_in_world_coords`, twelve edge lines plus Ozark's inner
spokes from the centre; type 1 draws the three coloured axes instead.
`skeleton_esp` draws Ozark's fourteen bones as world-space 3D lines
(`menu::renderer::draw_line`) directly between the two bone endpoints' world
positions, or joint markers of type 28.

**Bones are 3D, not Ozark's screen-space `draw_line_2d`.** Ozark's skeleton
lines are 2D, consumed by a PC render hook (`render_script_texture.cpp`) this
port has no equivalent for; on PS4, `draw_line_2d`'s backing buffer
(`global::ui::m_line_2d`) is never allocated and nothing reads it. Rather than
build a hook this port lacks, the skeleton draws through the 3D `draw_line`
that `box_3d_esp` and the snapline already use, with the two bones' world
positions and no projection.

## The per-frame budget

This is design, not tuning. Ozark ran on PC; this runs at 30 fps with up to 32
players plus world entities, and the skeleton alone is a hash native per bone
position per ped per frame. (As built, that is **15** — the fifteen unique
joints, fetched once into a local array that the bones then index. Drawing the
fourteen pairs from their endpoints would have been 28, plus 15 more when
joints are on as well.)

- Every context culls by `m_max_distance` before any other work.
- A hard cap of **48 entities drawn per frame** across all contexts, nearest
  first; entities beyond it are skipped rather than the frame rate sacrificed.
  48 covers a full 32-player session with headroom for world entities. When the
  cap bites, a throttled log line says so — silently dropping half the session
  reads as a bug in the ESP.
- Skeleton and joints obey a second radius, `m_skeleton_distance`, defaulting to
  **75 m** and per-context like the cull above: they are the most expensive
  elements by an order of magnitude and illegible past that anyway.

Both numbers are first estimates to be corrected against step 4 of the test
plan, not measurements. They are written down so that what gets adjusted is a
named constant rather than a guess rediscovered later.
- Skeleton work is skipped entirely unless `rage::hash_natives::usable()`.
- Everything hangs off `feature_update()`, which is already gated on
  `game::player_valid()`.

## Menu surface

Ozark's three levels, unchanged in shape:

```
helper_esp                 which elements are on, for m_current
└ Settings                 one entry per element
  └ Settings > Edit        colour + rainbow for the chosen element
```

The edit level needs two pointers rather than Ozark's three (`m_color`,
`m_rainbow_toggle`), the animator pointer having become unnecessary. Its colour
option delegates to the existing `helper_color_menu`.

**One targeted change to a shared component.** `helper_color_menu::target()`
currently takes an index into `menu::theme`'s `COLORS[]`, which is the registry
of *menu chrome* colours that themes persist. Putting ~50 ESP colours in there
would both bloat that registry and wrongly bind ESP to themes. So a second
target form is added — `target(color_rgba*, const char* title, color_rgba
revert_to)` — supplying by argument exactly what the index was there to provide
(a title and a default to revert to). No second colour editor.

Click handlers capture nothing; they act on file-scope state. `stl::function`
caps captures at 64 bytes and this is the pattern that stays under it.

## Persistence

**Not implemented in this version.** ESP settings do not survive a restart:
`helper_esp_menu` builds fresh from `esp_context`'s compiled defaults every
boot, and toggling an element writes nothing to disk.

This was attempted and reverted. The obvious approach -
`add_savable(get_submenu_name_stack())` on each toggle and number option,
the same call every other savable option in this codebase uses - does not
work here, and cannot be made to work without a different key. Its save key
is `(get_submenu_name_stack(), option name)`: the menu's position in the
tree, plus the option's own name. `helper_esp_menu` is not one menu with one
state, though - it is a single shared editor, opened via `open_for()` for
Session ESP, for each of up to 32 per-player `esp_context`s, and for Vehicle
ESP in turn, editing whichever one is current. None of that - which
consumer, which player slot - reaches the menu path or the option name, so
the key cannot tell "Snapline" for the session apart from "Snapline" for
player 7 or for vehicles: all of them would read and write the same one
stored value. (This used to be a no-op as well as wrong: `helper_esp_menu`
had no parent, so its name stack was empty and `add_savable`'s own guard
rejected it before any of the above applied. It now takes a parent per
opening — `open_for<T>()`, so that "back" returns to whichever consumer
opened it — and that second line of defence is gone. The key is still the
wrong key.)

The real fix is its own task: serialise each `esp_context` directly through
`util::config`, keyed by consumer identity rather than menu path - e.g.
`"session"`, `"player-<slot>"`, `"vehicle"` - read at the point each
consumer constructs its context and written wherever it changes, bypassing
`add_savable` entirely. That needs its own design (where the per-slot key
comes from, when it's safe to read/write relative to `menu::build()`) and is
out of scope here.

The one thing that does still hold: whatever the eventual key, ESP settings
belong in `config.json`, not in themes - a theme describes how the menu
looks, and ESP is a setting about the world.

## Failure modes

| Case | Behaviour |
|---|---|
| entity dies mid-frame | each function re-checks; null coords abort that entity, not the pass |
| bone natives not up yet | skeleton and joints skipped while `!hash_natives::usable()` |
| entity off-screen | `get_screen_coord_from_world_coord` returns false; nothing drawn |
| beyond `m_max_distance` | culled before any native is called |
| more entities than the cap | drawn up to the cap, then a throttled log line naming what was dropped |
| model has no dimensions | 3D box skipped, 2D box unaffected (it needs no model) |

## Testing, on console, in this order

Each step protects the next.

1. Story Mode, one ped: geometry only — is the box where the ped is, does it
   scale with distance, does the 3D box sit on the model.
2. Every element toggled one at a time, to catch an element that is individually
   broken while the others hide it.
3. Multiplayer with a full session: the case the budget exists for.
4. A long session watching the heartbeat. It reports once a minute with a
   monotonic frame count, so a frame rate collapse or a death shows up as a
   changed cadence or a stopped counter — this is the instrument the image crash
   was eventually solved with, and it is already running.

## Out of scope

Line-of-sight occlusion; ESP for spawner-managed entities (no submenu to hang it
on); any aim assistance whatsoever.
