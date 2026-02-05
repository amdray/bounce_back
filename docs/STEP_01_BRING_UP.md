# Bounce Back — bring-up (первый запуск на PSP/SDL2)

Цель “первого запуска” здесь — **не gameplay**, а bring-up: доказать, что оригинальные бинарные ресурсы читаются 1-в-1, интерпретация данных корректна, и базовый рендер/тайминг на PSP живые.

Связанные документы:
- Deobfuscation / форматы: [`DEOBFUSCATION.md`](DEOBFUSCATION.md)
- Tick (50ms): [`GAME_LOOP_SPEC.md`](GAME_LOOP_SPEC.md)
- Коллизии: [`COLLISION_CONTRACT.md`](COLLISION_CONTRACT.md)

---

## Минимальная цель bring-up

1) Инициализация платформы (PSPSDK или SDL2): окно/буфер + цикл.

2) Тайминг: фиксированный tick **50 ms** (20 FPS) или хотя бы счётчик кадров.

3) Загрузка и разбор ресурсов (без игрока/камеры/меню):

- `/res/tf` полностью до структур:
  - `tileW/H`, `tileCount`, `clampX/clampY` (derived: true=clamp, false=wrap), `tileIdMask`, `tileFlagMask`, `bgColor`
  - таблица анимаций: `animCount`, `period`, `frames[]`
  - per-tile: `renderType(v)`, `collisionType(l)`, `transform(b)`, `aux(af)`, inline masks (для `collisionType=1`)

- `/res/if0` (+ `/res/if{theme}` опционально): декодирование хотя бы 1–2 изображений в surface/texture.

- `/res/lf` для одного уровня: заголовок + tileMap (масив байтов нужных размеров).

4) Самопроверки:

- Рендер статического “дампа”: отрисовать tileMap как сетку `width × height` тайлов 1:1, без скролла.
- Визуализация анимаций (`renderType=3`): смена кадров по `animPeriod`.
- Опционально: отрисовать диагностику коллизий (например, `collisionType` цветом поверх тайлов).

---

## Контракт декодирования изображений из контейнеров (FACT)

Формат контейнера `/res/*` — как класс `c.java`: `u16 chunkCount`, далее `u16 size[chunkCount]`, затем подряд `chunkData`.

Правило bring-up (FACT):
- chunk считается **PNG**, если начинается с PNG magic `89 50 4E 47 0D 0A 1A 0A`.
- иначе chunk трактуется как opaque/binary (не пытаться декодировать как картинку).

Подтверждено по артефакту `res_container_signatures.txt`:
- `/res/if0`, `/res/if1`, `/res/if2` — chunks PNG
- `/res/ib0` — PNG
- `/res/ic` — chunks PNG
- `/res/im` — chunks PNG

Важно:
- Некоторые контейнеры содержат “не‑PNG” метаданные как отдельный chunk — поэтому при декодировании **всегда** проверять magic (пример: `/res/b`, где `chunk #0` не PNG, а последующие chunks — PNG; см. `res_container_signatures.txt`).
- `/res/r` (demo script) — **не контейнер**, а raw поток записей (см. `bounce_back/original_code/bounce_back_s60.jar.src/d.java:17`).

Артефакт для проверки: `python3 dump_res_container_signatures.py > res_container_signatures.txt`.

---

## Политика декодирования контейнеров (FACT)

Основано на `res_container_signatures.txt` (поле `magic_counts`):

### PNG-only (можно декодировать “по умолчанию”, но всё равно magic-first)

- `/res/if0` — `PNG=104`
- `/res/if1` — `PNG=7`
- `/res/if2` — `PNG=7`
- `/res/ib0` — `PNG=1`
- `/res/ic` — `PNG=12`
- `/res/im` — `PNG=5`

### Mixed (декодировать только PNG-chunks)

- `/res/b` — `PNG=25 unknown=1` (chunk `#0` opaque, `#1..` — PNG)

### Binary-only (PNG-декодер применять нельзя)

- `/res/bg` — `unknown=3`
- `/res/s` — `unknown=11` (звук; на bring-up можно игнорировать)
- `/res/tf` — `unknown=2` (парсить по формату метаданных тайлов)
- `/res/lf` — `unknown=44` (парсить по формату уровней)

### Raw stream (не контейнер)

- `/res/r` — raw поток записей демо/реплея (см. `bounce_back/original_code/bounce_back_s60.jar.src/d.java:17`)

---

## Контракт рендера тайла (без камеры) (FACT)

Базовая функция рисования тайла (внутри `g.java`) делает:

1) `tileId = tileByte & tileIdMask` (`tileIdMask = this.z`, обычно `0x7F`).

2) `if (tileByte & tileFlagMask) != 0` (`tileFlagMask = this.q`, обычно `0x80`): заливка под тайлом `bgColor` (`this.i`) прямоугольником `16×16` — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:601`.

3) Дальше зависит от `renderType` (`v[tileId]`):
- `0`: ничего не рисовать (кроме заливки bgColor по флагу).
- `1` и `5`: в `g.java` оба значения `renderType==1` и `renderType==5` обрабатываются как “draw image” (один обработчик), где `imageIndex = T[tileId] & 0xFF` — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:612` и `bounce_back/original_code/bounce_back_s60.jar.src/g.java:623`.
- `3`: анимированный тайл — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:634`:
  - `group = aux[tileId]` (`j = af[i]`) — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:635`
  - текущий tileByte берётся из `frames[group][frameIdx]` (`this.m[group][this.ai[group]]`) и **рекурсивно** рисуется через тот же рендер‑метод — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:638`
  - следствие (FACT): если в `frames[]` закодирован `0x80` флаг, он будет обработан точно так же, как и у статических тайлов (потому что `tileByte` для рендера в этой ветке — именно `frames[]`).

TO CONFIRM (не требуется для минимального bring-up):
- точная схема выбора `imageIndex` при анимации (в оригинале используется отрицательный `paramInt4` как “override image index”, см. `bounce_back/original_code/bounce_back_s60.jar.src/g.java:626`–`bounce_back/original_code/bounce_back_s60.jar.src/g.java:638`). Для bring-up достаточно, что кадры переключаются и “похоже на оригинал”.

Важно про `transform` в рендере:
- В ветке “не-pretransform” (`this.ab == false`) rotate/flip фактически отключены (внутри рисования принудительно ставится `k = 0`) — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:622`–`bounce_back/original_code/bounce_back_s60.jar.src/g.java:625`.
- Для bring-up допускается рисовать **без transform**; проверку “видимое совпадает с коллизионным” делать позже (см. `COLLISION_CONTRACT.md`).

---

## Контракт tileMap байта (FACT)

- `tileId = tileByte & tileIdMask` (`0x7F`).
- `tileByte & tileFlagMask` (`0x80`) в рендере вызывает заливку `bgColor` под тайлом (удобный визуальный маркер корректной интерпретации).

---

## Семантика clamp/wrap (FACT)

В `/res/tf` два флага читаются как boolean:
- `this.d` (X) и `this.X` (Y) — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:153` и `bounce_back/original_code/bounce_back_s60.jar.src/g.java:154`.

Семантика определяется использованием в `setCamera(pxX, pxY)`:
- если флаг `true` → используется `a(value, max)` (clamp) — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:284` и `bounce_back/original_code/bounce_back_s60.jar.src/g.java:289`
- если флаг `false` → используется `b(value, max)` (wrap) — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:286` и `bounce_back/original_code/bounce_back_s60.jar.src/g.java:291`

То есть корректные нейтральные имена:
- `clampX` / `clampY` (true=clamp, false=wrap), либо
- `wrapX` / `wrapY` (true=wrap, false=clamp) — но тогда придётся инвертировать при чтении.

---

## Быстрые инструменты/артефакты

- `/res/tf` dump: `python3 dump_tf_tiles.py > tf_tiles_dump.txt`
- Inline masks (runtime): `python3 dump_tf_tiles.py --dump-mask 3 --dump-mask 52 --dump-mask 97 > tf_inline_masks_runtime.txt`
- `/res/lf` tile positions: `python3 dump_lf_tile_positions.py --tile 4 --tile 53 --tile 102 > lf_tile_positions_collision_cases.txt`
- `/res/*` signatures: `python3 dump_res_container_signatures.py > res_container_signatures.txt`
