# STEP 09: Слои (foreground pass) — hoops “перед мячом” + front-tiles 52..61/66..72

Цель шага: реализовать **двухпроходный рендер** (base → player → foreground), как в оригинале, чтобы:
- “маленькие” hoops визуально давали эффект **мяч между частями**,
- некоторые tileId рисовались **поверх** игрока (как в `h.a(Graphics,DirectGraphics)`).

Источники истины:
- Порядок слоёв (render pipeline): `bounce_back/original_code/bounce_back_s60.jar.src/h.java:593-600`
- Hoops overlay (tileId 93..104, `x[]`): `bounce_back/original_code/bounce_back_s60.jar.src/h.java:677-727`
- Список координат для hoops overlay (`h.h[][]`): `bounce_back/original_code/bounce_back_s60.jar.src/h.java:322-331`
- “Front tiles” список координат (`h.c[][]`, id ranges): `bounce_back/original_code/bounce_back_s60.jar.src/h.java:332-335`
- Отрисовка “front tiles” поверх игрока: `bounce_back/original_code/bounce_back_s60.jar.src/h.java:731-748`
- Transform byte и таблица DirectGraphics manip (`g.p[]`): `bounce_back/original_code/bounce_back_s60.jar.src/g.java:134-136`
- Семантика битов transform byte (flip/rotate) используется в коллизиях: `bounce_back/original_code/bounce_back_s60.jar.src/g.java:402-419`
- `/res/tf` поля `v/T/b` (renderType/imageIndex/transform): `bounce_back/original_code/bounce_back_s60.jar.src/g.java:219-223`

---

## 1) Факты: как устроены слои в оригинале (FACT)

`h.paint(Graphics)` рисует в таком порядке:
1) `this.A.a(...)` (background map) (h.java:593)
2) `this.Z.a(...)` (foreground tilemap base pass) (h.java:594)
3) `c(paramGraphics)` (door) (h.java:595)
4) `b(paramGraphics)` (enemies) (h.java:596)
5) `this.e.a(paramGraphics)` (player) (h.java:597)
6) `a(paramGraphics, this.K)` (overlay pass: hoops + front tiles) (h.java:598-600)

**Следствие:** часть графики может быть “за мячом” (попадает в `Z.a(...)`), а часть — “перед мячом” (попадает в overlay pass).

---

## 2) Что именно является “overlay pass” и что там рисуется (FACT)

`h.a(Graphics, DirectGraphics)` содержит 2 независимых прохода:

### 2.1 Hoops overlay (tileId 93..104) через `x[]` из `/res/ic`

Hoops overlay рисуется только по списку координат `h.h[][]`, который собирается при загрузке уровня:
- в `h.b(levelIndex)` при скане всей карты: если `tileId ∈ {93,94,97,101}` → координаты пишутся в `h.h[][]`, счётчик `X++` (h.java:322-331).

Рисование по текущему tileId в карте:
- если `tileId` попадает в `93..104` → выбирается `x[]` и DirectGraphics manipulation (h.java:694-727).

Параметры для hoops overlay (точно по коду):
- `93/95`: `x[0]/x[1]`, `drawImage` без manipulation, `y = Z.b(tileY*16) - 1`, высота 14 (h.java:698-704)
- `94/96`: `x[0]/x[1]`, manipulation `270`, `x = Z.a(tileX*16) - 1`, ширина 14 (h.java:705-712)
- `97/99`: `x[2]/x[3]`, 2 draw’а: manipulation `8192` и `180` на `y+16` (h.java:713-719)
- `101/103`: `x[2]/x[3]`, 2 draw’а: manipulation `8462` и `270` на `x+16` (h.java:720-726)

### 2.2 Front tiles overlay (tileId ranges 52..61 и 66..72) поверх игрока

Список координат `h.c[][]` собирается при загрузке уровня:
- если `(tileId >= 52 && tileId <= 61) || (tileId >= 66 && tileId <= 72)` → координаты пишутся в `h.c[][]`, счётчик `z++` (h.java:332-335).

В overlay pass эти тайлы рисуются поверх игрока из tileset `Z`:
- читается `tileId = Z.R[tileY][tileX] & 0x7F` (h.java:737)
- если `Z.ab` (pretransformed mode) — `drawImage(Z.E[tileId], ...)` (h.java:738-740)
- иначе берётся `transformByte = Z.b[tileId]` и используется `Z.p[transformByte]` в `DirectGraphics.drawImage(...)` (h.java:741-746).

---

## 3) Рендер-трансформации: как перевести в SDL (FACT)

В `/res/tf` для каждого `tileId` хранится байт `transform` (`b[]`), который:
- используется в коллизиях как битовое поле: `0x8` flipX, `0x4` flipY, `rot = b & 0x3` (g.java:402-419),
- используется в рендере как индекс в таблицу `g.p[]` (g.java:134-136).

Таблица `g.p[]` (DirectGraphics manipulation) задана константно:
`{0, 270, 180, 90, 16384, 16654, 16564, 16474, 8192, 8462, 8372, 8282}` (g.java:134-136).

Правило для порта (без догадок, по структуре значений):
- `flipX` соответствует биту `0x8` в `transform` (g.java:402-403) → в `g.p[]` это группа значений с базой `8192` (g.java:135).
- `flipY` соответствует биту `0x4` (g.java:404-405) → в `g.p[]` это группа значений с базой `16384` (g.java:135).
- `rot = transform & 0x3` выбирает градусы `{0,270,180,90}` (g.p[0..3], g.java:134-136).

В SDL2 это реализуем через `SDL_RenderCopyEx`:
- `angle = {0,270,180,90}[rot]`
- `flip`:
  - если `(transform & 0x8)` → `SDL_FLIP_HORIZONTAL`
  - если `(transform & 0x4)` → `SDL_FLIP_VERTICAL`

---

## 4) Файлы

### Создать
- [ ] `src/ic_loader.h`
- [ ] `src/ic_loader.c` — загрузка **только** `x[0..3]` из `res/ic` (h.java:352-373)
- [ ] `src/foreground_pass.h`
- [ ] `src/foreground_pass.c` — хранение списков координат (hoops anchors + front tiles) и отрисовка overlay pass

### Изменить
- [ ] `src/level_renderer.h` — добавить вызов `foreground_pass_draw(...)` после `player_render`
- [ ] `src/level_renderer.c` — добавить helper для `SDL_RenderCopyEx` с transform byte
- [ ] `src/main.c` — загрузить `IcHoopsTextures` и `ForegroundPass`, передать в рендер
- [ ] `Makefile` — добавить новые `.c` в `OBJS`

---

## 5) Структуры данных

### 5.1 IcHoopsTextures

Хранит 4 текстуры из `/res/ic`:
- `SDL_Texture* x[4]` соответствует `h.x[0..3]` (h.java:368-373).

Контракт чтения из контейнера `/res/ic` (по порядку `c.a()` в `h.i()`):
- chunks `0..2` → `S[0..2]` (HUD) (h.java:357-362) — в этом шаге НЕ нужны
- chunks `3..6` → `x[0..3]` (hoops) (h.java:368-373)

### 5.2 ForegroundPass

Два списка координат в тайлах:
- `HoopAnchors[]`: позиции, где стартовый tileId был `93/94/97/101` (h.java:322-331)
- `FrontTiles[]`: позиции, где tileId в диапазонах `52..61` или `66..72` (h.java:332-335)

Важно: как и в оригинале, списки **строятся при загрузке уровня** и в рендере читается текущий tileId по этим координатам.

---

## 6) Функции (минимально нужные)

1) `ic_hoops_load(SDL_Renderer*, const char* path, IcHoopsTextures* out)`
   - загружает `res/ic` и декодирует chunks `3..6` как PNG → `SDL_Texture* x[0..3]` (h.java:357-373).

2) `foreground_pass_build(Level* level, ForegroundPass* out)`
   - скан `level->tile_map`:
     - если `tileId ∈ {93,94,97,101}` → добавить в `HoopAnchors` (h.java:327-330)
     - если `tileId in [52..61] || [66..72]` → добавить в `FrontTiles` (h.java:332-335)

3) `foreground_pass_draw(...)`
   - вызывается **после** `player_render` (h.java:597-600).
   - часть A: hoops overlay
     - для каждого anchor: прочитать `tileId` из карты и выполнить `switch(tileId)` как в `h.a(...)` (h.java:697-727)
   - часть B: front tiles
     - для каждого координата: прочитать `tileId`, `transformByte = tile_meta[tileId].transform` и нарисовать tileset texture через `SDL_RenderCopyEx` (h.java:741-746, g.java:134-136)

4) `tile_draw_ex(SDL_Renderer*, SDL_Texture*, int x, int y, uint8_t transformByte)`
   - helper: применяет rotation/flip из раздела 3 (g.java:402-419, g.java:134-136).

---

## 7) Пошаговая интеграция

1) **Создать новые файлы:**
   - [ ] `src/ic_loader.h/c`
   - [ ] `src/foreground_pass.h/c`

2) **Изменить существующие файлы:**
   - [ ] `src/main.c`: после загрузки уровня создать:
     - `IcHoopsTextures hoops_tex; ic_hoops_load(renderer, "res/ic", &hoops_tex);`
     - `ForegroundPass fg; foreground_pass_build(level, &fg);`
   - [ ] `src/level_renderer.c`: добавить вызов `foreground_pass_draw(...)` **после** `player_render(...)`.
   - [ ] `Makefile`: добавить `src/ic_loader.c` и `src/foreground_pass.c` в `OBJS`.

3) **Порядок изменений:**
   a) `ic_loader.*` (загрузка `x[0..3]`)
   b) `foreground_pass.*` (build + draw)
   c) `level_renderer.*` (интеграция overlay pass после игрока)
   d) `main.c` (инициализация структур и проброс указателей)
   e) `Makefile`

---

## 8) Критерии успеха

- [ ] На уровне с “маленьким” hoop (пара `97/98`): визуально часть кольца остаётся “за мячом”, часть — “перед мячом” (слои как в `h.paint`, h.java:593-600).
- [ ] После прохождения такого hoop (смена `97/98 → 99/100` по `a.java:1561-1579`) overlay начинает использовать `x[3]` вместо `x[2]` (h.java:713-719).
- [ ] TileId из диапазонов `52..61` и `66..72` видны поверх мяча (h.java:332-335, h.java:731-748).
