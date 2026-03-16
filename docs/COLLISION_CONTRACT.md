# Bounce Back — collision contract (g.collisionTest)

Цель: зафиксировать **точную семантику коллизий тайлов** как контракт порта (PSP/PSPSDK/SDL2), чтобы дальнейшие подсистемы (физика, враги, триггеры) были доказуемо корректны.

Источники фактов:
- `bounce_back/original_code/bounce_back_s60.jar.src/g.java` — методы `a(...)` на строках ~315 и ~347 (pixel-perfect collision).
- `/res/tf` (`bounce_back/original_code/bounce_back_s60.jar.src/res/tf`) — метаданные тайлов: `renderType v[]`, `transform b[]`, `collisionType l[]`, `aux af[]`, а также inline-маски для `collisionType=1`.

Связанные документы:
- Game loop (50ms tick): [`GAME_LOOP_SPEC.md`](GAME_LOOP_SPEC.md)
- Общая деобфускация/ресурсы: [`DEOBFUSCATION.md`](DEOBFUSCATION.md)

Ниже все значения `tileW/tileH` для этой сборки фактически **16×16** (в `/res/tf` может быть 12×12, но `g` нормализует 12→16 при чтении).

---

## 1) Подпись и единицы измерения `collisionTest` (FACT)

Функция коллизии в `g.java`:

- `boolean a(int xPx, int yPx, int wPx, int hPx, boolean[][] mask, boolean collectHits)`

Единицы измерения и соглашения:
- `xPx`, `yPx` — **мировые пиксели**, **левый верх** тестируемого прямоугольника (см. логику overlap в `bounce_back/original_code/bounce_back_s60.jar.src/g.java:363` и `bounce_back/original_code/bounce_back_s60.jar.src/g.java:378`).
- `wPx`, `hPx` — размеры прямоугольника в пикселях (используются как ширина/высота `mask`; см. вычисления `i6/i7` в `bounce_back/original_code/bounce_back_s60.jar.src/g.java:365` и `bounce_back/original_code/bounce_back_s60.jar.src/g.java:380`).
- `mask` (`paramArrayOfboolean`) индексируется как `mask[u][v]`, где:
  - `u` — **локальная координата по X** (колонка) в объектной маске,
  - `v` — **локальная координата по Y** (строка) в объектной маске.

Доказательство ориентации `mask`:
- `i6` (“ширина overlap”) вычисляется из `wPx` (`paramInt5`) и X‑смещений `i2/i4` в `bounce_back/original_code/bounce_back_s60.jar.src/g.java:365`.
- `i7` (“высота overlap”) вычисляется из `hPx` (`paramInt6`) и Y‑смещений `i3/i5` в `bounce_back/original_code/bounce_back_s60.jar.src/g.java:380`.
- В двойном цикле проверка выглядит как `mask[i9 + i4][i8 + i5]`, где `i9` бежит `0..i6-1` (локальный X), а `i8` бежит `0..i7-1` (локальный Y) — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:393` и `bounce_back/original_code/bounce_back_s60.jar.src/g.java:395`.

Флаг карты (`tileByte & 0x80`) **не участвует в коллизии**:
- tileId берётся как `this.R[tileY][tileX] & this.z`, где `z=tileIdMask` (в этой версии `0x7F`) — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:329`.

---

## 2) Возвращаемое значение и “hit list” (FACT)

Что возвращает:
- Возвращает `true`, если есть **хотя бы один** пиксель, где `mask[u][v]==true` и тайл в этой точке считается коллизионным по своим правилам (`collisionType`).
- Любая `ArrayIndexOutOfBoundsException` во время выполнения `collisionTest` приводит к `true` (см. `catch` в `bounce_back/original_code/bounce_back_s60.jar.src/g.java:341`).
  - Следствие (частый практический эффект): выход за границы `R[][]` ведёт к “стене”, но это **не единственный** источник AIOOBE (см. ниже).

Что заполняется при `collectHits=true`:
- `g.Y[0..4]` и `g.P[0..4]` — списки попаданий по тайлам.
  - `Y[idx] = tileX`, `P[idx] = tileY`.
- Перед тестом массивы очищаются в `-1`.
- Попадания добавляются в порядке обхода: `tileX` слева→направо (внешний цикл), `tileY` сверху→вниз (внутренний цикл).
- Ограничение: массивы длины 5, а явной проверки `idx < 5` нет — если столкновений больше 5, происходит выход за границы и метод возвращает `true` по обработчику исключения. То есть при `collectHits=true` “много коллизий” корректно детектируется, но “список попаданий” может быть частично заполнен.

Уточнение (важно отделить источники AIOOBE):
- В `collisionTest` есть единый `try/catch(ArrayIndexOutOfBoundsException)` вокруг перебора тайлов — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:326`.
- Поэтому **любая** AIOOBE внутри (в т.ч. из-за:
  - выхода за границы `R[tileY][tileX]`,
  - переполнения hit‑list `Y/P`,
  - несоответствия размеров `mask` заявленным `wPx/hPx`,
  - иных ошибок индексации)
  приводит к `return true`.

---

## 3) Связь с clamp/wrap мира (FACT)

В `Level (g)` есть два флага из `/res/tf`:
- `clampX` (`this.d`) и `clampY` (`this.X`) читаются в `bounce_back/original_code/bounce_back_s60.jar.src/g.java:153`.

Они **не применяются внутри collisionTest**:
- `collisionTest` вычисляет `tileX/tileY` напрямую делением и делает прямой доступ `R[tileY][tileX]` без `wrap()`/`clamp()` — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:322` и `bounce_back/original_code/bounce_back_s60.jar.src/g.java:329`.
- Нормализация координат мира (clamp или wrap) применяется в других местах (например, `setCamera`/`a()`), но это отдельный контракт. Для порта важно: если вызывающая сторона подаст координаты, которые выводят `tileX/tileY` за пределы карты, collisionTest вернёт `true` по AIOOBE.

## 4) Математика выбора тайлов для теста (FACT)

Пусть `tileW = this.f`, `tileH = this.A` (в этой версии 16).

Диапазоны тайлов:
- `tileX0 = xPx / tileW`
- `tileX1 = (xPx + wPx) / tileW`
- `tileY0 = yPx / tileH`
- `tileY1 = (yPx + hPx) / tileH`

Далее выполняется полный перебор `tileX ∈ [tileX0..tileX1]`, `tileY ∈ [tileY0..tileY1]`.

Привязки к коду:
- Диапазоны тайлов вычисляются ровно так в `bounce_back/original_code/bounce_back_s60.jar.src/g.java:322`–`bounce_back/original_code/bounce_back_s60.jar.src/g.java:325`.
- В Java целочисленное деление для отрицательных значений **усекает к нулю** (например, `-1/16 == 0`, а `-16/16 == -1`). Это влияет на то, когда возникает AIOOBE по “вне карты”.

Overlap‑прямоугольник внутри тайла (используется при pixel‑тесте):
- `tileOriginX = tileX * tileW`, `tileOriginY = tileY * tileH` — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:348`–`bounce_back/original_code/bounce_back_s60.jar.src/g.java:349`.
- Для X:
  - если `xPx < tileOriginX`: `skipLeft = tileOriginX - xPx`, `overlapW = min(tileW, wPx - skipLeft)`
  - если `xPx > tileOriginX`: `localX0 = xPx - tileOriginX`, `overlapW = min(wPx, tileW - localX0)`
  - если `xPx == tileOriginX`: `localX0=0`, `overlapW = min(wPx, tileW)`
  - это реализовано в `bounce_back/original_code/bounce_back_s60.jar.src/g.java:363`–`bounce_back/original_code/bounce_back_s60.jar.src/g.java:377`.
- Для Y аналогично (смещения `skipTop/localY0/overlapH`) — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:378`–`bounce_back/original_code/bounce_back_s60.jar.src/g.java:392`.
- Если `overlapW==0` или `overlapH==0`, внутренние циклы по пикселям не выполняются (потому что они идут `for (i8 = overlapH-1; i8>=0; ...)` и `for (i9 = overlapW-1; i9>=0; ...)`) — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:393`–`bounce_back/original_code/bounce_back_s60.jar.src/g.java:394`.

---

## 5) Таблица `collisionType` (0/1/2/3) (FACT)

`collisionType` хранится в `l[tileId]` и влияет на то, как проверяется попадание:

| collisionType | Поведение | Источник геометрии |
|---:|---|---|
| 0 | Нет коллизии: тайл пропускается до pixel-test | — |
| 2 | **Solid full-tile**: если есть хотя бы 1 пиксель overlap с `mask==true`, считается коллизией | целый тайл 16×16 |
| 1 | **Pixel mask без transform**: проверяется `tileMask[localY][localX]` | inline маска в `/res/tf` (256 boolean) |
| 3 | **Pixel mask с transform + alias**: берётся mask от “base tile” (`aux`), но координаты трансформируются по `transform` | alias-маска из `/res/tf` + `transform` |

Формат inline mask (`collisionType=1`) в `/res/tf`:
- 16×16 значений `readBoolean()` (то есть **256 байт**, не битпак).
- Порядок в файле: `y=0..15`, внутри `x=0..15`.

Ориентация tile‑mask в памяти (критичный момент):
- При чтении inline mask (`collisionType=1`) создаётся `new boolean[tileW][tileH]` и заполняется как `s[tileId][x][y] = readBoolean()` при обходе `y` затем `x` — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:225` и `bounce_back/original_code/bounce_back_s60.jar.src/g.java:226`.
- При проверке коллизии значение извлекается как `tileMask[localY][localX]` (и в ветке `collisionType=3` как `tileMask[transY][transX]`) — см. `bounce_back/original_code/bounce_back_s60.jar.src/g.java:420` и `bounce_back/original_code/bounce_back_s60.jar.src/g.java:422`.
- Следствие (FACT): runtime‑интерпретация inline mask является **транспонированной относительно порядка хранения**. То есть boolean, считанный из файла в позиции `(fileY, fileX)`, используется в коллизии как значение в `(runtimeY=fileX, runtimeX=fileY)`.

`collisionType=3` (alias) (FACT):
- `aux` читается как `int` (`readInt`) в `bounce_back/original_code/bounce_back_s60.jar.src/g.java:231`.
- Если `collisionType==3`, маска назначается через alias: `s[tileId] = s[aux]` — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:232`.
- Это означает: `aux` для `collisionType=3` должен быть **валидным tileId** (иначе NPE/AIOOBE на доступе).

Практическая проверка по данным этой сборки:
- Base inline masks (`collisionType=1`): `tileId ∈ {3, 52, 54, 56, 59, 66, 69, 93, 95, 97, 99, 113}`.
- Варианты (`collisionType=3`) — это повёрнутые/флипнутые версии этих масок (например `4/5/6` — варианты `3`).

Источник артефакта: `artifacts/tf_tiles_dump.txt:1` (сгенерирован `python3 scripts/dump_tf_tiles.py`).

---

## 6) Контракт `transform` для коллизии (FACT)

`transform` — байт `b[tileId]` из `/res/tf`.

Разложение битов:
- `rot = transform & 0x03`:
  - `0` = 0°
  - `1` = 90° CW (по фактической формуле)
  - `2` = 180°
  - `3` = 270° CW
- `transform & 0x08` — flip по X (инверсия `x → 15-x`)
- `transform & 0x04` — flip по Y (инверсия `y → 15-y`)

Порядок применения (важно):
1) flips (`0x08`, `0x04`)
2) rotation (`rot`)

Точные формулы, как они реализованы в `g.java` для `collisionType=3`:

- Инициализация: `x = localX`, `y = localY`
- flips:
  - если `0x08`: `x = 15 - x`
  - если `0x04`: `y = 15 - y`
- rotation:
  - `rot==0`: `(x,y)` без изменений
  - `rot==1`: `(x,y) = (y, 15 - x)`
  - `rot==2`: `(x,y) = (15 - x, 15 - y)`
  - `rot==3`: `(x,y) = (15 - y, x)`

Опора на код:
- flips выполняются до rot‑ветвления — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:402`.
- rot‑ветвления `rot==3/1/2` — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:406`–`bounce_back/original_code/bounce_back_s60.jar.src/g.java:419`.

Важно: `transform` влияет на коллизию **только** для `collisionType=3` (ветка `b1==3`). Для `collisionType=1` трансформации **не применяются** — `bounce_back/original_code/bounce_back_s60.jar.src/g.java:396` и `bounce_back/original_code/bounce_back_s60.jar.src/g.java:422`.

---

## 7) Мини-набор “фактов наблюдения” (из реальных уровней) (FACT)

Ниже — не юнит-тесты, а конкретные проверяемые факты: “в уровне есть тайл X в позиции (tileX,tileY), у него collisionType/transform/aux такие-то; для заданной точки в тайле ожидается collide / no-collide”.

Источники для этих кейсов:
- Позиции тайлов в уровнях: `artifacts/lf_tile_positions_collision_cases.txt:1` (сгенерирован `python3 scripts/dump_lf_tile_positions.py --tile 4 --tile 53 --tile 102`).
- Метаданные тайлов (`collisionType/transform/aux`): `artifacts/tf_tiles_dump.txt:1`.
- Базовые inline‑маски (runtime‑ориентация): `artifacts/tf_inline_masks_runtime.txt:1`.

Для привязки к уровням:
- `tile origin worldPx = (tileX*16, tileY*16)`.
- “Точка в тайле” задаётся как `local (lx, ly)`; мировая точка: `(xPx, yPx) = origin + (lx, ly)`.
- В качестве `mask` достаточно мысленно использовать 1×1 маску с единственным `true` пикселем.

### Case A — tileId 4 (collisionType=3, aux=3, transform=0x01) в уровне 04

- Позиция: `level 04`, `tile (14,42)` (из `/res/lf`).
- Метаданные (`/res/tf`): `tile 4: collisionType=3, aux=3, transform=0x01`.
- Base mask: `tile 3` — диагональная маска (см. Appendix A ниже).

Ожидания (1×1 mask):
- `local (0,0)` **collide** (transform 0x01 → base `(0,15)`, в `tile 3` это `#`).
- `local (1,0)` **no-collide** (transform 0x01 → base `(0,14)`, в `tile 3` это `.`).

### Case B — tileId 53 (collisionType=3, aux=52, transform=0x03) в уровне 05

- Позиция: `level 05`, `tile (9,31)`.
- Метаданные: `tile 53: collisionType=3, aux=52, transform=0x03`.
- Base mask: `tile 52` — “левая клиновидная” маска (см. Appendix A).

Ожидания:
- `local (4,0)` **collide** (transform 0x03 → base `(15,4)`, в `tile 52` это `#`).
- `local (0,0)` **no-collide** (transform 0x03 → base `(15,0)`, в `tile 52` это `.`).

### Case C — tileId 102 (collisionType=3, aux=97, transform=0x05) в уровне 00

- Позиция: `level 00`, `tile (43,17)`.
- Метаданные: `tile 102: collisionType=3, aux=97, transform=0x05` (`0x04` flipY + `rot==1`).
- Base mask: `tile 97` имеет `#` в верхней строке на `x=7..8` (см. Appendix A; это **runtime‑ориентация**, с учётом транспонирования при хранении).

Ожидания:
- `local (15,7)` **collide** (mapping 0x05 → base `(8,0)`).
- `local (0,0)` **no-collide** (mapping 0x05 → base `(15,15)`).

### Case D — “вне карты = коллизия” (граница уровня)

Факт из реализации: любая AIOOBE внутри `collisionTest` приводит к `true` (см. раздел 2).

Проверяемое ожидание:
- Если в переборе тайлов получится `tileX<0` или `tileY<0` (то есть, например, `xPx <= -16` или `yPx <= -16` при tileSize=16), доступ к `R[tileY][tileX]` вызовет AIOOBE и результат будет `true`.
- Для “малых” отрицательных координат (`-15..-1`) `xPx / 16` может дать 0 из-за усечения к нулю, и AIOOBE может **не** возникнуть (поведение зависит от маски и overlap).

---

## Appendix A — inline masks (collisionType=1) для базовых тайлов

Формат: 16×16, `#` = solid, `.` = empty, оси: `x` слева→направо, `y` сверху→вниз.

Важно: это **runtime‑ориентация**, то есть значения приведены так, как они реально читаются в коллизии (`tileMask[localY][localX]`). Из‑за того, что загрузка пишет `s[x][y]`, а коллизия читает `s[y][x]`, runtime‑маска является транспонированной относительно “как записано в /res/tf” (см. раздел 5).

Источник артефакта: `artifacts/tf_inline_masks_runtime.txt:1` (сгенерирован через `python3 scripts/dump_tf_tiles.py --dump-mask 3 --dump-mask 52 --dump-mask 97`).

### tileId 3 (base для 4/5/6)

```
...............#
..............##
.............###
............####
...........#####
..........######
.........#######
........########
.......#########
......##########
.....###########
....############
...#############
..##############
.###############
################
```

### tileId 52 (base для 53)

```
#...............
##..............
###.............
####............
#####.......####
################
################
################
................
................
................
................
................
................
................
................
```

### tileId 97 (base для 98/101/102)

```
.......##.......
................
................
................
................
................
................
................
................
................
................
................
................
................
................
................
```

---

## Быстрые инструменты для извлечения фактов

- Список тайлов с `collisionType=3`/`aux`/`transform`: `python3 scripts/dump_tf_tiles.py --only-collision3`
- Общий дамп всех тайлов `/res/tf`: `python3 scripts/dump_tf_tiles.py`
- Позиции тайлов по уровням можно извлечь из `/res/lf` (tileMap chunk) простым сканированием `tileByte & 0x7F`.
