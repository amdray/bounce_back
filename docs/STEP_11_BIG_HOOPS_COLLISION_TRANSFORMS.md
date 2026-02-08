# STEP 11: “Большие” hoops (97–104) — состав, коллизия, повороты текстур (render transforms)

Цель шага: зафиксировать **точную структуру больших колец** (двойные hoops `97..104`) из оригинала и довести порт до поведения “как в jar” по двум осям:
1) **Коллизия**: где именно hoop попадает в hit-list, какие tileId участвуют, какие маски и трансформы используются.
2) **Рендер**: какие части рисуются tile-engine’ом (tileset из `/res/if*` + `/res/tf`), какие — overlay pass’ом (текстуры `x[0..3]` из `/res/ic`), и как применяются повороты/флипы.

Источники истины:
- Рендер пайплайн + overlay pass: `bounce_back/original_code/bounce_back_s60.jar.src/h.java:593-600`, `h.java:677-748`
- Hoops tileId switching (проход через кольцо): `bounce_back/original_code/bounce_back_s60.jar.src/a.java:788-867`, `a.java:1561-1579`
- Принудительное добавление big-hoops в hit-list: `bounce_back/original_code/bounce_back_s60.jar.src/a.java:705-713` (101..104), `a.java:1443` (97..100)
- Семантика transform byte (rotate/flip) + применения в collision: `bounce_back/original_code/bounce_back_s60.jar.src/g.java:315-430`, `g.java:401-419`
- Aliasing масок при `collisionType==3`: `bounce_back/original_code/bounce_back_s60.jar.src/g.java:226-234`
- Таблица DirectGraphics manip (для overlay и для tileset в оригинале): `bounce_back/original_code/bounce_back_s60.jar.src/g.java:134-136`

---

## 1) Что такое “большие hoops” (ID + геометрия)

В оригинале есть два “двойных” hoop’а (по 2 тайла):

### 1.1 Вертикальный двойной hoop: 97/98 → 99/100

Состав (в tileMap):
- `97` — верхняя половина
- `98` — нижняя половина

После прохождения:
- `97/98` превращаются в `99/100` (сохранением флага `0x80`) — см. `a.java:1567-1579`.

Как код подтверждает ориентацию “верх/низ”:
- Если текущий tile = `97`, то меняется он сам и **тайл ниже** (`tileY+1`) (a.java:1569-1574).
- Если текущий tile = `98`, то меняется **тайл выше** (`tileY-1`) и он сам (a.java:1575-1579).

### 1.2 Горизонтальный двойной hoop: 101/102 → 103/104

Состав:
- `101` — левая половина
- `102` — правая половина

После прохождения:
- `101/102` превращаются в `103/104` — см. `a.java:820-833`.

Ориентация “лево/право” подтверждается так:
- Если tile = `101`, меняется он сам и **тайл справа** (`tileX+1`) (a.java:828-833).
- Если tile = `102`, меняется **тайл слева** (`tileX-1`) и он сам (a.java:822-827).

---

## 2) Как они рисуются: base-pass vs overlay-pass

### 2.1 Base-pass (tile-engine g) — tileset из `/res/if0` + `/res/if{theme}`

Любые `tileId` (включая `97..104`) сначала рисуются tile-engine’ом `g.a(Graphics)` (в нашем порте это `LevelRenderer`), используя поля из `/res/tf`:
- `renderType` (рисовать/не рисовать/анимация)
- `imageIndex` (`T[tile]`)
- `transform` (`b[tile]`)
- `collisionType` (`l[tile]`)
- `aux` (`af[tile]`) (alias для масок при `collisionType==3`)

**Критично:** в оригинале при `transform != 0` рисование идёт через DirectGraphics manipulation (`g.p[transform]`) — см. `h.java:741-746` (front tiles) и `g.java:134-136`.

Следствие для порта: чтобы большие hoops выглядели как в оригинале, нужно рисовать tileset-тайлы с учётом `transform` (rotate/flip) для `tileId` вроде `98/100/101/102/103/104`, потому что их transform в `/res/tf` не нулевой.

### 2.2 Overlay-pass (hoops overlay) — `x[0..3]` из `/res/ic`

Overlay pass выполняется после игрока: `h.paint → ... → player → a(Graphics,DirectGraphics)` (h.java:593-600).

В overlay рисуются дополнительные куски hoops по текущему tileId (h.java:694-727):

- `97`/`99`: берётся `x[2]`/`x[3]` и рисуются **2 куска**:
  - `manip=8192` (flipX) в `(x, y)`
  - `manip=180` в `(x, y+16)`
- `101`/`103`: берётся `x[2]`/`x[3]` и рисуются **2 куска**:
  - `manip=8462` (flipX + rot270) в `(x, y)`
  - `manip=270` в `(x+16, y)`

Где `x = Z.a(tileX*16)`, `y = Z.b(tileY*16)` (h.java:689-726).

---

## 3) Где и как срабатывает коллизия больших hoops

### 3.1 Общая схема оригинала

В оригинале `Player` обновляет движение по пикселям и на каждом шаге:
1) вызывает `Level.collisionTest(..., storeHits=true)` (g.java:315-345) — это заполняет `Y[]/P[]` (hit-list) тайлами пересечения,
2) затем выполняет `switch(tileId)` по каждому hit-тайлу и применяет логику (включая hoops) — см. большой switch в `a.java`.

### 3.2 Принудительное попадание в hit-list (важное отличие от “малого вертикального” 94/96)

Чтобы big-hoops работали даже когда “чистая” pixel-perfect коллизия не дала hit, в оригинале добавлены форсы:

- При вертикальном шаге (движение по Y): если tile под центром ∈ `101..104`, координаты **принудительно пишутся** в `v.Y/v.P` и `bool=true` (a.java:705-713).
- При горизонтальном шаге (движение по X): если tile под центром ∈ `97..100`, координаты тоже форсятся (a.java:1443).

Именно поэтому большие hoops обычно “реагируют” стабильнее, чем `94/96`, который зависит от истинной mask-коллизии.

### 3.3 Какие маски используются (collisionType + aux alias + transform)

Факты из загрузчика `/res/tf` в `g.java`:
- если `collisionType == 1`: читается `boolean[16][16]` маска (g.java:226-230),
- если `collisionType == 3`: маска **алиасится**: `s[tileId] = s[aux]` (g.java:234),
- а затем при проверке коллизии, если `collisionType == 3`, к координатам пикселя применяется `transform` (flip + rotate) (g.java:401-419).

Практически для `97..104` это означает:
- часть tileId — “базовая маска” (`collisionType==1`),
- часть — “тот же контур, но с transform” (`collisionType==3` + `aux` указывает base tileId).

---

## 4) Transform byte: перевод правил в SDL2

В оригинале один и тот же `transform` байт используется:
- в collision как битовое поле (g.java:401-419),
- в render как индекс в `g.p[]` (g.java:134-136).

Правило для порта (SDL2 `SDL_RenderCopyEx`):
- `rot = transform & 0x3` → угол `{0,270,180,90}[rot]` (соответствует `g.p[0..3]`)
- `flipX` если `(transform & 0x8)`
- `flipY` если `(transform & 0x4)`

Важно: для квадратных тайлов `16×16` anchor/смещение не требует дополнительной коррекции (bounding box не меняется).

---

## 5) Что именно делаем в порте в этом шаге (минимум)

### 5.1 Рендер transforms для tileset (base pass)

Изменить рендер тайлов в `LevelRenderer` так, чтобы при рисовании tileset-тайлов использовался `transform` из `/res/tf` через helper `tile_draw_ex(...)`:
- `display_tile = animation_get_tile(...)`
- `meta = tile_meta[display_tile]`
- `tile_draw_ex(renderer, tex, screen_x, screen_y, meta.transform)`

Это должно “включить” повороты/флипы для `98/100/101/102/103/104` и визуально собрать большое кольцо как в оригинале.

### 5.2 Визуальная валидация именно больших hoops

Критерии успеха (визуальные):
- Двойной вертикальный hoop `97/98` корректно выглядит как единое кольцо (base + overlay), и после прохождения меняется на `99/100` и overlay начинает использовать `x[3]` вместо `x[2]` (h.java:713-719).
- Двойной горизонтальный hoop `101/102` корректно выглядит и после прохождения меняется на `103/104` и overlay переключается аналогично (h.java:720-726).

---

## 6) Файлы

### Изменить
- [ ] `src/level_renderer.c` — рисовать tileset через `tile_draw_ex(...)` (с transform), а не через `SDL_RenderCopy`.

### (Опционально, если потребуется точное совпадение DirectGraphics manip для не-квадратных картинок)
- [ ] `src/level_renderer.c` — добавить helper уровня DirectGraphics manip (`0/90/180/270` + flipX/flipY) с корректной привязкой к top-left.

---

## 7) Порядок работ

1) Подтвердить по оригиналу, что tileset-рендер учитывает transform (`g.p[]`) (g.java:134-136).
2) Внести минимальную правку в `src/level_renderer.c`: заменить `SDL_RenderCopy` на `tile_draw_ex` с `meta.transform`.
3) Проверить только big-hoops уровни: `97..104` выглядят/переключаются правильно.

