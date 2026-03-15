# Bounce Back PSP — Engineering Audit

**Model:** Claude Sonnet 4.6  
**Corpus:** all `src/*.c`, all `original_code/**/*.java` — read in full  
**Scope:** logic bugs, parity gaps vs Java, dead code, hacks, duplicates, architecture

---

## CRITICAL — Game-Breaking Bugs

---

### [C1] Breakable-tile destruction animation never completes

**Files:** `src/player.c` → `player_apply_tile_break()`, `src/player.c` HUD queue absent  
**What's wrong:** `player_apply_tile_break()` writes tile ID 105 to the map and stops there. The destruction sequence ends mid-animation: the crumbled tile stays as ID 105 forever.

**Java original (`a.java` → `b(int,int)` + `h.java` → `g()`):**
```java
// a.java b(int r, int c):
this.v.R[paramInt1][paramInt2] = 105;
for (byte b = 0; b < E.B.length; b++) {
    if (E.B[b] == 0) { E.B[b] = 15; E.v[b] = paramInt2; E.J[b] = paramInt1; break; }
}
// h.java g() — called every tick:
if (B[i] == 10) Z.R[J[i]][v[i]] = 105;
if (B[i] == 5)  Z.R[J[i]][v[i]] = 106;
if (B[i] == 0)  Z.R[J[i]][v[i]] = 0;
```
Three-phase animation: crumbling (105) → cracked (106) → empty (0), over 15 ticks.

**Why it matters:** Broken blocks render permanently as ID 105. Subsequent collision checks still test the tile; the "hole" never opens. Levels with popped-ball tile-breaking sequences are unsolvable.

**Fix:** Add a `TileBreakQueue` (slots with countdown + row/col) to the level state; tick the queue in the main game loop and update tile IDs at ticks 10, 5, 0.

**Regression risk:** Low — additive change.

---

### [C2] Gem (ID 34) collection does not update respawn checkpoint

**Files:** `src/player.c` → `player_collect_tile()` case 34, `src/player.c` → `player_respawn()`  
**What's wrong:** Collecting a coin updates score and changes the tile, but never moves `p->spawn_tile_x / spawn_tile_y / spawn_is_large` to the coin's position.

**Java original (`a.java` → `b(int paramInt1, int paramInt2)` case 34):**
```java
case 34:
    o.n += 200; E.C += 200;
    // clear old checkpoint tile-35 marker
    b2 = v.R[this.n][this.H];
    if ((b2 & 0x7F) == 35) v.R[this.n][this.H] = (byte)(0 | b2 & 0x80);
    // move checkpoint here
    this.n = paramInt1;   // checkpoint row
    this.H = paramInt2;   // checkpoint col
    this.t = this.I;      // checkpoint invert-state
    o.b(2);               // sound
    v.R[paramInt1][paramInt2] = (byte)(0x23 | j); // tile 35 = "last checkpoint marker"
```

**Why it matters:** After dying the player always respawns at the original level spawn, ignoring all collected coins. This removes the entire mid-level checkpoint mechanic.

**Fix:** In `player_collect_tile` case 34, after the tile update, also set `p->spawn_tile_x = tx; p->spawn_tile_y = ty; p->spawn_is_large = p->is_inverted;`. Optionally: clear old checkpoint tile-35 marker, set new checkpoint tile to 35.

**Regression risk:** Low — no existing code reads spawn state during mid-level.

---

### [C3] Collision-mask layout transposition in `level_loader.c`

**Files:** `src/level_loader.c` → `level_runtime_load_tf()` + `level_test_collision_collect()`  
**What's wrong:** `level_runtime_load_tf()` allocates `bool*[tile_w]` columns-first (`tile_mask[x][y]`), fills with y-outer / x-inner reads — producing **column-major** `[x][y]` storage. But `level_test_collision_collect()` accesses `tile_mask[mask_y][mask_x]`, treating the array as **row-major** `[y][x]`. The indices are transposed.

**Java parity (`a.java` constructor):**
```java
this.f[b1] = new boolean[b2 /*width*/][b3 /*height*/]; // [x][y]
for (b4 = 0; b4 < b3; b4++)       // outer: y
    for (b5 = 0; b5 < b2; b5++)   // inner: x
        this.f[b1][b5][b4] = readBoolean(); // stored as [x][y]
```
Access: `arrayOfBoolean[i10][i11]` where i10=x-transformed, i11=y-transformed → reads `[x][y]`. Consistent.

`collision_masks.c` (which is dead — see C4) allocates **row-major `[y][x]`** for the same data, inconsistent with both Java and `level_loader.c`.

**Why it matters:** Any non-symmetric mask (which is all slope tiles, all irregular shapes) will have horizontally-mirrored collision. Player sticks into walls that look passable and falls through floors that look solid. Affects mask IDs 0..N wherever tile_w ≠ tile_h.

**Fix:** In `level_test_collision_collect`, change access from `tile_mask[mask_y][mask_x]` to `tile_mask[mask_x][mask_y]`, matching the `[x][y]` allocation.

**Regression risk:** Medium — existing broken behavior may have been "compensated" in level layout; requires playtesting all levels.

---

## HIGH — Significant Bugs and Parity Breaks

---

### [H1] Flat-surface bounce decay factor wrong

**Files:** `src/player.c` — `toward_surface` case 2 default branch  
**What's wrong:** C writes `p->bounce_state >>= 1` (50% decay per bounce), Java uses `G = 3 * G >> 2` (75% decay — slower attenuation, more bouncy feel).

**Java (`a.java` case 2):**
```java
j = this.G;
this.G = 3 * this.G >> 2;   // 75% per step
if (this.G > -10) { x = true; j = 30; G = 0; }
```
Slope cases (3,4,5,6) also use `3*G>>2`; the *only* place Java uses `G >>= 1` is in the helper `a(int, boolean)` for pass-through tiles (94/96, 101–104).

**Fix:** Replace `p->bounce_state >>= 1;` (flat case) with `p->bounce_state = (3 * p->bounce_state) >> 2;`

**Regression risk:** Low — the change makes bouncing more energetic (closer to original feel).

---

### [H2] Player respawn always uses +12 pixel offset for large-ball spawn points

**Files:** `src/player.c` → `player_respawn()`  
**What's wrong:**
```c
int offset = p->spawn_is_large ? 12 : 8;
```
Java `a.java e()` always uses +8 regardless of ball-type at spawn:
```java
this.D = this.H * 16 + 8;
this.i = this.n * 16 + 8;
```
The +12 offset is only applied at the *initial level load* (`h.java b() if b4 != 0`), not at respawn.

**Why it matters:** All large-ball spawn points place the player 4 pixels lower/righter than Java after each death.

**Fix:** In `player_respawn`, always use offset = 8.

**Regression risk:** Very low.

---

### [H3] Enemy render culling uses full screen height (272) instead of viewport (251)

**Files:** `src/enemy_renderer.c`  
**What's wrong:** `view_b = camera_y + SCREEN_HEIGHT` uses `SCREEN_HEIGHT=272`. The game viewport is 272 − 21 (HUD) = 251 px.

Enemies partially behind the HUD strip are still processed and rendered; their bottom rows draw under the HUD overlay — visible on slow platforms or when HUD alpha is non-opaque.

**Fix:** `view_b = camera_y + (SCREEN_HEIGHT - HUD_HEIGHT);`  
`HUD_HEIGHT` is already 21 in `camera.h`.

**Regression risk:** Very low — additive culling tightening only.

---

### [H4] /res/tf parsed three times at startup

**Files:** `src/tile_metadata.c`, `src/tile_animation.c`, `src/level_loader.c`  
**What's wrong:** Each module independently opens and fully reads `/res/tf`:
1. `tilemetadata_load("res/tf")` — startup
2. `animation_load("res/tf")` — startup
3. `level_runtime_load_tf()` inside `level_load()` — every level transition

The resource also contains game data that makes three passes O(tiles × file_size).

**Java:** Single `c("/res/tf")` reader in `h.java b()`:
```java
byte[] arrayOfByte3 = c2.a();   // obj[0]
byte[] arrayOfByte4 = c2.a();   // obj[1]
c2 = null;
```
Both arrays distributed to a single `g` constructor.

**Fix:** See [A3] in Architecture section. Short-term: remove pass 3 (dead code from C4).

**Regression risk:** Low once C4 is removed; full merge is medium-risk refactor.

---

### [H5] `resource_loader` prints to stdout on every load unconditionally

**Files:** `src/resource_loader.c`  
**What's wrong:**
```c
printf("resource_load: loaded '%s' - %u elements, %ld bytes\n", path, rc->count, total_bytes);
```
This fires for every asset at startup *and* on every level transition. No debug flag.

**Fix:** Wrap in `#ifdef DEBUG_RESOURCES` or remove entirely for release.

**Regression risk:** None.

---

### [H7] Debug level-skip shoulder-R uses wrong previous-state field

**Files:** `src/main.c`  
**What's wrong:**
```c
if (input.shoulder_r && !input._prev_right)
```
`_prev_right` is the **D-pad-right** previous state, not the right-shoulder previous state. The condition fires every frame that shoulder_R is held *and* D-pad-right is released — wrong trigger, wrong frequency.

**Java:** Cheat keys use key-press events (`case 51: if (H.j) ...`) with no separate "previous" check; the equivalent in C should use `input.shoulder_r_pressed`.

**Fix:** Use `input.shoulder_r && !input._prev_shoulder_r` (or the pre-computed `shoulder_r_pressed` edge from `input.c`).

**Regression risk:** Low.

---

### [H8] Front tiles (IDs 52–72) likely rendered twice

**Files:** `src/level_renderer.c`, `src/foreground_pass.c`  
**What's wrong:** `level_renderer.c` renders all tiles whose `render_type` is neither 0 nor 3. Unless tile metadata explicitly marks IDs 52–72 as render_type 0/3, they render in the main pass. `foreground_pass.c` then renders the same IDs again in its own scan, producing opaque overdraw.

**Java:** Front tiles are drawn *once* inside the single `g.a(Graphics)` tile-map render call; the separate `h.java a(Graphics, DirectGraphics)` only draws the hoop *sprite overlays* (ring animation), never the background tile again.

**Fix:** Either skip IDs 52–72 in `level_renderer` (check their render_type), or remove the duplicate tile-draw from `foreground_pass_render` and have it render only the hoop sprite overlays.

**Regression risk:** Low if tile metadata already has a flag to skip; medium if render_types must be audited.

---

## MEDIUM — Polish Bugs and Minor Parity Issues

---

### [M1] `sound_play()` permanently overwrites parsed OTT `loop` field

**Files:** `src/sound.c`  
**What's wrong:**
```c
g_sounds[index].loop = 0;   // modifies shared parsed state
ott_player_start(...);
```
If an OTT file was parsed with `loop=1`, the field is permanently zeroed after the first play. Subsequent plays always play once regardless of parsed intent.

**Fix:** Copy `ott_player_t` to a local struct and set `local.loop = 0` there; don't touch `g_sounds[index]`.

---

### [M2] Five modules each load `res/ic` independently

**Files:** `src/hud.c`, `src/foreground_pass.c`, `src/exit_door.c`, `src/enemy_renderer.c`, `src/player.c` (for `/res/b`, but similar pattern)
**What's wrong:** Each module calls `resource_load("res/ic")` with a fallback to `resource_load("release/res/ic")`. The container is opened and parsed up to 5 times.

Fallback `"release/res/ic"` also leaks a development path into the release binary.

**Fix:** Add `ic_loader.c` with a single `ic_load() / ic_free()` pair; modules receive `SDL_Texture**` pointers at init. Remove all `"release/res/ic"` fallback strings.

---

### [M3] `player_create()` loads `/res/b` twice — sprites and masks separately

**Files:** `src/player.c` → `player_create()`  
**What's wrong:** `resource_load("res/b")` is called once for sprite loading, then `player_masks_load("res/b")` opens it again for mask data. Two full file reads + mallocs for the same container.

**Fix:** Parse both sprite images and mask bool-arrays from a single in-memory `ResourceContainer*`.

---

### [M4] Inflate powerup (tile 22) plays sound unconditionally

**Files:** `src/player.c` → `player_special_tile()` case 22  
**What's wrong:** C plays `SND_INFLATE` always. Java plays sound 5 only when not already popped:
```java
case 22:
    if (!this.F) { this.o.b(5); } else { this.b = 550; }
```
When already popped, Java just resets the popped timer (silent).

**Fix:**
```c
case 22:
    if (!p->is_popped) sound_play(SND_INFLATE);
    if (p->is_popped) p->timer_c = 550;
    ...
```

---

### [M5] `ITEM_COUNT` defined twice in `menu.c`

**Files:** `src/menu.c`  
**What's wrong:** `#define ITEM_COUNT 6` appears on two separate lines; the second line would generate a `-Wmacro-redefined` warning (or error with `-Werror`).

**Fix:** Remove the duplicate.

---

### [M6] Camera dead zone not in original Java

**Files:** `src/camera.c`  
**What's wrong:** C implements a dead-zone scroll (player must reach CAMERA_DEADZONE_PERCENT of the viewport before the camera moves). Java `g.java` uses direct clamp + follow — camera tracks the player tile exactly, clamped to level bounds.

The dead zone makes the game feel different on tall, narrow levels where the player sits in the "float zone" for extended periods.

**Severity:** Acceptable PSP adaptation, but worth documenting as intentional divergence.

---

### [M7] HUD ring counter uses individual icon draws; Java uses a clip-based strip

**Files:** `src/hud.c`  
**What's wrong:** C calls `SDL_RenderCopy` for each ring individually, spacing them 6 px apart. Java uses `g.setClip(157, 185, 12, 16)` + `drawImage(S[1], ...)` to reveal a vertical strip of 16-pixel ring frames — a filmstrip approach. The visual layout and spacing may differ.

**Severity:** Cosmetic; acceptable adaptation.

---

## LOW — Nits and Polish

---

### [L1] `tile_metadata.c` hardcodes 128-element array

**File:** `src/tile_metadata.c`  
```c
g_tile_metadata = calloc(128, sizeof(TileMetadata));
```
If a future theme or level pack uses tile IDs > 127 the array is silently out-of-bounds. Java reads the actual count from the stream (`b` = `images_total`).

**Fix:** Read the count from the resource header and allocate dynamically, as Java does.

---

### [L2] `exit_door.c` completion test uses strict inequality

**File:** `src/exit_door.c`  
```c
(door_px_x < p->x_pos && p->x_pos < door_px_x + 32 && ...)
```
Player standing exactly at `door_px_x` or `door_px_x+32−1` is excluded from the door trigger. Java uses `>` / `<` in the same style for the bounding box. Matches Java, but the semantics mean the edge pixels of the door frame are excluded. Low likelihood of player being exactly on the boundary.

---

### [L3] `player_apply_tile_break` does not clean up old checkpoint tile-35 marker

**File:** `src/player.c`, by extension of C2's gem-checkpoint gap  
When a gem checkpoint is eventually implemented (fix C2), the old tile-35 marker must also be cleared. Currently no such cleanup exists anywhere in C.

---

### [L4] Flat-surface escape: Java uses `G >>= 1` only for helper `a()`, not for case-2

This was already captured in H1; this entry is here only to note that the `a(int, boolean)` helper (used for hoop tiles 94/96 and arrow tiles 84, 79, 83) *correctly* uses `>>= 1` — only the case-2 flat surface path is wrong. Do not change the helper path.

---

### [L5] `clamp_u8()` in `menu.c` unused

**File:** `src/menu.c`  
`static uint8_t clamp_u8(int v)` is defined but never called. `-Wunused-function` warning.

**Fix:** Remove the function.

---

## Dead Code Inventory

| Symbol | File | Status |
|---|---|---|
| `clamp_u8()` | `src/menu.c` | Defined, never called. |
| Duplicate `#define ITEM_COUNT 6` | `src/menu.c` | Preprocessor artefact. |
| `tileIdEcho`, `images_total`, `images_base`, `clampX`, `clampY`, `tileIdMask` reads in `tile_metadata.c` | `src/tile_metadata.c` | Read from stream then `(void)`-cast — silently discarded. Keep reads (maintain stream-position), remove `(void)` casts only if values are consumed. |

---

## Suspicious Fallback / Hack Catalogue

| Location | Fallback | Verdict |
|---|---|---|
| `enemy_renderer.c` | `if (!g_ic) g_ic = resource_load("release/res/ic")` | Dev-path leak. Remove; use shared ic_loader. |
| `exit_door.c` | same `"release/res/ic"` fallback | Same fix. |
| `foreground_pass.c` | same | Same fix. |
| `hud.c` | same + ring-icon fallback to tileset tile 17 | Tileset fallback is a reasonable defence; release path is dev leak. |
| `hud.c` ring fallback | `ring_index = 1 + 4*4 = 17` | Magic number. At minimum add a comment explaining the tileset layout. |
| `resource_loader.c` | unconditional `printf` on every load | Debug artefact. Guard with `#ifdef`. |
| `player.c` | `static bool prev_cheat` inside `player_update` | File-static inside a called function = invisible global state. Move to `Player` struct. |

---

## Duplicate Logic Clusters

### D1 — `/res/tf` parsed in three independent modules

`tile_metadata.c::tilemetadata_load` + `tile_animation.c::animation_load` + `level_loader.c::level_runtime_load_tf` all independently open `/res/tf`, walk its binary layout, and allocate their own output structures. Zero code reuse.

### D2 — `/res/ic` loaded in five independent modules

`hud.c`, `foreground_pass.c`, `exit_door.c`, `enemy_renderer.c`, `player.c` (for `/res/b` via `player_masks_load`) all replicate the pattern:
```c
static ResourceContainer* g_xx;
if (!g_xx) g_xx = resource_load("res/…");
if (!g_xx) g_xx = resource_load("release/res/…");  // dev fallback
```

### D3 — Bounce-state management copy-pasted in vertical and horizontal collision loops

`player_update` has nearly identical bounce-state blocks for:
- `toward_surface` case 3/6/52 (vertical, left slope)
- `toward_surface` case 4/5/53 (vertical, right slope)
- `toward_surface` case 2 (vertical, flat)
- `toward_surface` cases 94/96, 101–104
- Horizontal equivalents of the same

Java centralised these in `a.java::a(int, boolean)` (generic bounce helper). A C equivalent `static int apply_bounce(Player*, int j, bool jump, BounceType)` would halve the block count.

### D4 — Object `top`/`left` calculation repeated verbatim 8+ times in `player.c`

```c
int top  = level->objects.f[obj][0] * 16 + level->objects.ag[obj][1];
int left = level->objects.f[obj][1] * 16 + level->objects.ag[obj][0];
```
Appears in the vertical collision loop (both `toward_surface` and away), and again in the horizontal loop. Should be a `level_object_world_rect(level, obj, &left, &top, &w, &h)` helper.

---

## Top-10 Quick Wins (High ROI, Low Risk)

1. **[H1] Fix flat bounce decay**: `p->bounce_state >>= 1` → `p->bounce_state = (3 * p->bounce_state) >> 2` — one-line, changes feel to match Java.
2. **[H5] Guard `resource_loader` printf**: wrap in `#ifdef DEBUG` — silent release builds.
3. **[H2] Fix respawn offset** — change `spawn_is_large ? 12 : 8` to always `8` — one-line correctness fix.
4. **[H3] Fix enemy culling height**: `camera_y + SCREEN_HEIGHT` → `camera_y + SCREEN_HEIGHT - HUD_HEIGHT` — one-line.
5. **[H7] Fix debug level-skip edge detection**: replace `!input._prev_right` with `!input._prev_shoulder_r` — one-line.
6. **[M5] Remove duplicate `#define ITEM_COUNT 6`** in `menu.c` — single line.
7. **[M4] Fix inflate-powerup sound condition** — add `if (!p->is_popped)` guard around `sound_play(SND_INFLATE)`.
8. **[L5] Remove unused `clamp_u8`** in `menu.c`.

---

## Top-5 Architectural Improvements

### [A1] Unified `/res/tf` loader

Create `tf_loader.c` that parses `/res/tf` once at startup, keeping shared tile metadata and animation groups in a single `TileDatabase` handle. All current callers receive pointers into it. `level_runtime_load_tf` becomes a thin view (using a cached pointer), eliminating the per-level re-parse.

### [A2] Shared IC resource module

`ic_loader.c` exposes `ic_init(renderer)` / `ic_get(index)` / `ic_free()`. All five current loaders become `ic_get(N)` calls. Removes 5 independent `resource_load` sites, all fallback strings, all module-local `g_ic` statics.

### [A3] Bounce helper to eliminate collision-loop duplication

See D3. A `calc_bounce(Player*, int j, bool jump_held, int G_init, int G_decay_num, int G_decay_den)` static function reduces the current ~300 lines of repeated bounce-state blocks to ~50.

### [A4] Tile-break animation queue as proper Level state

C1 fix has architectural implications: a `TileBreakEntry break_queue[8]` array inside `Level` (matching Java's `h.B[5]`) keeps the destruction state alongside the rest of level mutable state. This generalises naturally and makes save/restore trivial if save-state is ever added.

---

## What's Already Done Well

- **Transform mapping is pixel-perfect**: `tile_transform.c` maps Nokia's 4-bit transform encoding to SDL_RenderCopyEx angles and flips correctly. bit3=H-flip, bit2=V-flip, 01→270°, 10→180°, 11→90° matches the original DirectGraphics rotation convention.
- **Resource container format is faithfully implemented**: `resource_loader.c` correctly reads the BE uint16 count + BE uint16[] sizes format from `c.java`, with random-access offset tables instead of sequential streams — a sensible improvement for PSP memory access patterns.
- **Sound synthesis is complete**: OTT ringtone parser + sine-wavetable mixer in `sound.c` is a clean, full-fidelity implementation; the mixing strategy (divide by active count) matches Nokia's SDL mixer semantics.
- **Sprite-change collision resolution (player_change_sprite) is thorough**: The expand-H, expand-V, diagonal scan mirrors `a.java::a(int)` exactly, including the parameter ordering of horizontal-before-vertical-before-diagonal.
- **Hoop tile collection and tile mutation logic is correct**: The 93→95/96, 94→96, 97→99/100, 101→103/104 mutual-tile updates in `apply_tile_97_98` and `apply_tile_101_102` match Java exactly, and the pass-through conditions (center-pixel-aligned) are faithfully reproduced.
- **Level count and score formula match Java exactly**: congratulations at `level_index == 19` maps to Java `t == 20` (1-based); time bonus `(1200 - seconds) * (level_index + 1)` is identical to Java `(1200 - B.g/1000) * this.t`.
- **Animation timer tick direction matches**: `anim->timer[i]--; if (timer[i]==0) { timer[i]=period[i]; frame_index[i]++; }` matches Java `g.java::d()` exactly including initial-value-equals-period behaviour.
- **Object coordinate storage is consistent**: `f[n][0]` = top-row, `f[n][1]` = left-col, `ag[n][0]` = x-pixel-offset, `ag[n][1]` = y-pixel-offset matches `h.java`'s load code and drawing code throughout.
- **Power-up timer and ball-inflation state machines are complete**: `player_apply_a_state`, `player_apply_r_state`, `player_apply_j_state` faithfully cover all three inflation-state chains (a=1/2, r=1/2/3, and the j squash animation) including the chained r=3→r=1 transition for inverted-to-popped.
- **Menu structure is faithful**: 6 items (Continue/New Game/Options/Records/Help/Exit) in correct order; level-select pagination and wrapping; level-complete/game-over/congratulations panels with correct Java string content.

---

## Идеальная архитектура с нуля — PSP SDK 2026

> Контекст: 153 KB ресурсов, единственная платформа PSP (или PSP-класс: 32 MB RAM, MIPS R4000
> @ 333 MHz, аппаратный 3D-ускоритель PSP GU, 60 fps экран 480×272), весь геймплей — одиночная
> 2D аркада без сети. Ниже — как это стоило бы построить, руководствуясь принципами embedded-
> разработки 2026 года.

---

### 1. Одна загрузка, нулевые аллокации после старта

153 KB полностью умещаются в RAM. При старте:
1. Открываем EBOOT.PBP / архив один раз, копируем в единый сырой буфер (назовём его `g_res_blob`, 192 KB с запасом).
2. Поверх него поднимаем два arena-аллокатора:
   - **`arena_perm`** — всё постоянное: декодированные текстуры в VRAM, метаданные тайлов, маски коллизий. Живёт до выхода из игры.
   - **`arena_frame`** — scratch-память на один кадр (display list, временные sorted-draw-calls). Сбрасывается в начале каждого кадра.
3. После декодирования ресурсов `heap` игре не нужен вообще — `malloc/free` не вызываются нигде в игровом цикле.

```c
typedef struct { uint8_t *base, *cur, *end; } Arena;
static inline void* arena_push(Arena* a, size_t n)  { ... }  // bump pointer
static inline void  arena_reset(Arena* a)            { a->cur = a->base; }
```

`sceKernelAllocPartitionMemory` один раз на 512 KB — весь heap игры. Никаких `malloc` в hot path.

---

### 2. Двухбуферный display list вместо SDL_RenderCopy

PSP GU принимает command buffer (display list). Правильная схема:

```c
uint32_t __attribute__((aligned(64))) dl_buf[2][DL_SIZE];
int dl_active = 0;

void frame_begin(void) {
    sceGuStart(GU_DIRECT, dl_buf[dl_active]);
}
void frame_end(void) {
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
    dl_active ^= 1;
}
```

Все draw-вызовы внутри frame_begin/frame_end склеиваются в один DL и исполняются GPU параллельно с CPU следующего кадра. Это и есть native API PSP — никакого SDL-overhead.

---

### 3. Текстурный атлас вместо отдельных текстур

Каждый вызов `sceGuTexImage` меняет текстуру — это flush GU pipeline. При 153 KB ресурсов всё влезает в **один атлас 512×512 в VRAM** (1 MB при 16bpp, укладывается рядом с фреймбуффером).

Упаковка при старте:
- Тайлсет `if0` (32 KB) + `if1` (1 KB) + `if2` (2 KB) + `ic` (5 KB) + `b` (7 KB) + `im` (9 KB) → один `sceGuTexImage` на весь кадр тайлового рендера.
- Спрайты игрока и врагов — второй субатлас, один `sceGuTexImage` на entity-pass.
- Итого: 2-3 смены текстуры на кадр против 15-20 в текущей реализации.

UV-координаты хранятся как `uint16_t u, v, w, h` в структуре `Sprite`, вычисляются один раз при загрузке.

---

### 4. Data-Oriented Design для объектов уровня

Java хранила объекты как `f[][]` (row/col) + `ag[][]` (offset) + `s[][]` (velocity) — три параллельных массива. Это уже SoA. В C 2026 стоит оформить явно:

```c
#define MAX_OBJECTS 32
typedef struct {
    int16_t  world_x[MAX_OBJECTS];   // пиксели
    int16_t  world_y[MAX_OBJECTS];
    int16_t  vel_x[MAX_OBJECTS];
    int16_t  vel_y[MAX_OBJECTS];
    uint8_t  type[MAX_OBJECTS];
    uint8_t  active[MAX_OBJECTS];    // bitfield экономит кэш
    uint8_t  count;
} ObjectPool;
```

Tick всех объектов — линейный проход по массивам, без pointer chasing. MIPS R4000 (in-order) критически выигрывает от кэш-дружественного layout по сравнению с массивом указателей.

---

### 5. Фиксированный шаг 20 Hz + рендер 60 Hz с интерполяцией

```c
#define UPDATE_MS 50
uint32_t accumulator = 0, prev_ticks = sceKernelGetSystemTimeLow() / 1000;

while (running) {
    uint32_t now = sceKernelGetSystemTimeLow() / 1000;
    accumulator += now - prev_ticks;
    prev_ticks = now;

    while (accumulator >= UPDATE_MS) {
        game_tick();         // детерминированная логика 20 Hz
        accumulator -= UPDATE_MS;
    }

    float alpha = (float)accumulator / UPDATE_MS;  // 0.0 … 1.0
    game_render(alpha);      // интерполяция позиций: pos_prev + (pos_cur - pos_prev)*alpha
}
```

Мяч движется плавно при 60 fps, физика остаётся 20 Hz как в оригинале. `alpha` нужна только для позиций спрайтов — две `int16_t prev_x/prev_y` в `Player` держат предыдущую позицию.

---

### 6. Единый модуль ресурсов `res.c`

Весь `/res/` распаковывается при старте в `ResDB`:

```c
typedef struct {
    SDL_Texture* textures[RES_MAX];  // или GU texture handles
    uint8_t*     raw[RES_MAX];       // для tf/lf — сырые байты
    uint32_t     size[RES_MAX];
} ResDB;

static ResDB g_res;

void res_init(void);           // один проход по всем файлам
SDL_Texture* res_tex(ResId);   // O(1) lookup
const uint8_t* res_raw(ResId); // для tf, lf
```

`ResId` — enum: `RES_LF, RES_IF0, RES_IF1, RES_IF2, RES_IC, RES_B, RES_IM, RES_TF, ...`

`tf` парсится ровно один раз в `TileDB` (metadata + collision masks + animation groups). Все потребители получают `const TileDB*` — никаких повторных открытий файла.

---

### 7. Карта уровня как flat bytemap + отдельный volatile-слой

```c
typedef struct {
    uint8_t  base[LEVEL_MAX_ROWS][LEVEL_MAX_COLS];     // из lf, ro, не меняется
    uint8_t  mutable[LEVEL_MAX_ROWS][LEVEL_MAX_COLS];  // runtime: сломанные блоки, собранные гемы
} LevelMap;
```

Рендер читает `mutable`. Spawn/reset копирует `base → mutable` за один `memcpy`. Нет нужды в `level_set_tile` с поиском — прямая индексация `map.mutable[row][col]`.

Очередь разрушения блоков — встроена:
```c
typedef struct { uint8_t row, col, timer; } BreakEntry;
BreakEntry break_queue[8];  // ровно столько нужно (Java: 5)
```

---

### 8. Коллизии: один источник истины

Маски коллизий — `uint16_t mask[TILE_COUNT]` (1 бит на пиксель, 16×16 → 256 бит → 4×uint64_t или 16×uint16_t на тайл). Храним row-major `[y][x]` явно задокументировано:

```c
// tile_collision.h
// mask[tile_id][y] — 16-bit row, бит x = (mask[tile_id][y] >> x) & 1
typedef uint16_t TileMask[16];
extern const TileMask g_tile_masks[TILE_COUNT]; // ro, из tf
```

Один заголовок, один источник — конец путаницы между тремя модулями.

---

### 9. Аудио: ring-buffer + ISR без malloc

```c
#define AUDIO_BUF_SAMPLES 512
int16_t audio_buf[2][AUDIO_BUF_SAMPLES * 2];  // stereo, double-buffer
int     audio_active = 0;

// PSP sceAudio callback:
void audio_callback(void* buf, unsigned int samples, void* userdata) {
    mix_samples(buf, samples, &g_synth);  // чистый mixer без alloc
    // g_synth: 4 active OTT voices, все состояния — plain structs
}
```

OTT голоса — массив `OttVoice active_voices[4]`; `sound_play` находит свободный слот или вытесняет наименее приоритетный. Нет `loop = 0` мутаций — `OttVoice` хранит `bool one_shot` независимо от источника.

---

### 10. Итог: структура проекта

```
src/
  main.c          — PSP entry, две arena, game loop с alpha
  res.c / res.h   — единая загрузка всех 153 KB
  tile_db.c       — TileDB из tf: metadata + masks + anim (один проход)
  level.c         — LevelMap base/mutable + break_queue + ObjectPool
  player.c        — Player + collision против TileDB
  render.c        — atlas pass: bg → tiles → entities → hud (2-3 tex switch)
  audio.c         — OTT synth + voice pool
  input.c         — edge detection (pressed/released bitfields)
  menu.c          — overlay screens
```

**Размер кодовой базы** при таком подходе: ~3 000–3 500 строк C (против ~5 800 в текущей). Нулевые аллокации в hot path, один проход по каждому ресурсу, фиксированная память, портируемость — замена PSP GU на SDL2 GL backend требует правки только `render.c`.
