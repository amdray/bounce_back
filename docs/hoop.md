# Hoops (tileId 93–104) — textures, transforms, transitions (FACT)

Источник истины:
- Отрисовка hoops: `bounce_back/original_code/bounce_back_s60.jar.src/h.java:677-727`
- Загрузка `/res/ic` (где лежат hoop-изображения): `bounce_back/original_code/bounce_back_s60.jar.src/h.java:351-373`
- Смена tileId при “прохождении” hoops + `W--`: `bounce_back/original_code/bounce_back_s60.jar.src/a.java:316-362`, `bounce_back/original_code/bounce_back_s60.jar.src/a.java:788-833`, `bounce_back/original_code/bounce_back_s60.jar.src/a.java:1551-1579`

---
## 0) Render Layers (порядок отрисовки)

Из `h.java:584-609` (метод `paint`):

```java
public void paint(Graphics paramGraphics) {
    // ...
    this.A.a(paramGraphics);      // (1) Level tiles: g.a() - base pass
    this.Z.a(paramGraphics);      // (2) Player render: e.a()
    c(paramGraphics);             // (3) Exit door animation
    b(paramGraphics);             // (4) ???
    this.e.a(paramGraphics);      // (5) ???
    // ...
    a(paramGraphics, this.K);     // (6) Foreground pass: hoops overlay + front tiles
    // ...
}
```

**Ключевой факт:** Hoops overlay (`a(Graphics, DirectGraphics)`) рисуется **ПОСЛЕ** игрока.  
Это позволяет создать эффект "мяч проходит через кольцо":
- Задняя часть кольца (tileId 98, 100, 102, 104) рисуется в base pass (до игрока)
- Передняя часть кольца (tileId 97, 99, 101, 103) рисуется в foreground pass (после игрока)

---

## 0.1) DirectGraphics Manipulation Codes → C Transform Mapping

**DirectGraphics (J2ME Nokia):**
- `270` = rotate 270° CW (= 90° CCW)
- `180` = rotate 180°
- `8192` = flip horizontal (FLIP_HORIZONTAL_MASK = 0x2000)
- `8462` = 8192 + 270 = flipX + rot270

**C encoding (`ic_loader.c:17-19`):**
```c
const uint8_t rot = (uint8_t)(transform & 0x3);   // 0=0°, 1=270°CW, 2=180°, 3=90°CW
const bool flip_x = (transform & 0x8) != 0;       // bit 3
const bool flip_y = (transform & 0x4) != 0;       // bit 2
```

**Mapping table:**

| DirectGraphics | Meaning         | C transform t |
|----------------|-----------------|---------------|
| 0              | none            | 0             |
| 270            | rot270 CW       | 1             |
| 180            | rot180          | 2             |
| 90             | rot90 CW        | 3             |
| 8192           | flipX           | 8             |
| 8462           | flipX + rot270  | 9             |
| 8192+180       | flipX + rot180  | 10            |
| 8192+90        | flipX + rot90   | 11            |

---

## 0.2) Сводная таблица Hoops: tileId → texture → transforms

Из `h.java:697-726`:

| tileId | x[] idx | Parts | DirectGraphics | C transform | Notes |
|--------|---------|-------|----------------|-------------|-------|
| 93     | 0       | 1     | 0 (none)       | 0           | small horiz, y-=1 |
| 95     | 1       | 1     | 0 (none)       | 0           | small horiz passed, y-=1 |
| 94     | 0       | 1     | 270            | 1           | small vert, x-=1 |
| 96     | 1       | 1     | 270            | 1           | small vert passed, x-=1 |
| 97     | 2       | 2(v)  | 8192, 180      | 8, 2        | big vert: flipX, then rot180 at y+16 |
| 99     | 3       | 2(v)  | 8192, 180      | 8, 2        | big vert passed |
| 101    | 2       | 2(h)  | 8462, 270      | 9, 1        | big horiz: flipX+rot270, then rot270 at x+16 |
| 103    | 3       | 2(h)  | 8462, 270      | 9, 1        | big horiz passed |

**Base texture x[2] (ic_hoops_5.png):** "/" shape (top-left to bottom-right diagonal, 16x16)

---

## 0.3) Порядок применения трансформаций

**В `ic_loader.c:surface_transform_bake()` (строки 42-69):**
```c
/* Step 1: flip in SOURCE space */
int fx = flip_x ? (src_w - 1 - sx) : sx;
int fy = flip_y ? (src_h - 1 - sy) : sy;

/* Step 2: rotate the flipped coordinates into DEST space */
```

То есть порядок: **flip → rotate**.

**DirectGraphics также применяет flip → rotate** (это стандартное поведение Nokia DirectGraphics API).

---
## 1) Набор текстур hoops и откуда берутся

Hoop-спрайты загружаются из контейнера `/res/ic` в массив `h.x` размером **4**:
- `this.x = new Image[4];` (h.java:368)
- `this.x[b] = Image.createImage(c.a(), 0, arrayOfByte.length);` в цикле `b=0..3` (h.java:369-372)

То есть hoops используют ровно **4 текстуры**: `x[0]`, `x[1]`, `x[2]`, `x[3]`.

Привязка к порядку чтения `c.a()` в `h.i()` (важно для порта):
1) Сначала читаются `S[0..2]` (3 PNG) (h.java:357-362),
2) затем читаются `x[0..3]` (4 PNG) (h.java:368-373),
3) затем читается `Q` (1 PNG) (h.java:375-382),
4) затем читаются `ac[0..3]` (4 PNG) (h.java:383-392).

---

## 2) Какой tileId рисуется какими `x[]`, сколько частей и какие transforms

Отрисовка hoops выполняется в `h.a(Graphics, DirectGraphics)` и запускается только если `tileId` в диапазоне `93..104` (h.java:694).

Обозначения:
- `tileId = tileByte & 0x7F` (h.java:691-693)
- базовая позиция на экране: `i4 = Z.a(tileX*16)` (h.java:688-689), `i7 = Z.b(tileY*16)` (h.java:701 / 709 / 716 / 723)
- `b1=b2=16` (размер тайла) (h.java:678-679)

### 2.1 tileId 93 / 95 — 1 часть, без DirectGraphics transforms

- `93` → `x[0]`, `95` → `x[1]` (h.java:698-704)
- Рисуется через `Graphics.drawImage(...)` (без манипуляции): (h.java:703)
- Смещение по Y: `i7 = Z.b(tileY*16) - 1` (h.java:701-702)

### 2.2 tileId 94 / 96 — 1 часть, DirectGraphics manipulation = `270`

- `94` → `x[0]`, `96` → `x[1]` (h.java:705-712)
- Рисуется через `DirectGraphics.drawImage(..., manipulation=270)` (h.java:711)
- Смещение по X: `i4--` (h.java:708)

### 2.3 tileId 97 / 99 — 2 части (вертикально), manipulations = `8192` и `180`

- `97` → `x[2]`, `99` → `x[3]` (h.java:713-719)
- Рисуется 2 раза:
  - первая часть: `DirectGraphics.drawImage(..., manipulation=8192)` (h.java:717)
  - вторая часть: `DirectGraphics.drawImage(..., y + 16, manipulation=180)` (h.java:718)

### 2.4 tileId 101 / 103 — 2 части (горизонтально), manipulations = `8462` и `270`

- `101` → `x[2]`, `103` → `x[3]` (h.java:720-726)
- Рисуется 2 раза:
  - первая часть: `DirectGraphics.drawImage(..., manipulation=8462)` (h.java:724)
  - вторая часть: `DirectGraphics.drawImage(..., x + 16, manipulation=270)` (h.java:725)

### 2.5 Важно: 98 / 100 / 102 / 104

В `h.a(Graphics, DirectGraphics)` **нет** отдельных `case` для `98`, `100`, `102`, `104` (h.java:697-727).

**Следствие для “слоёв” (то, что видно на экране):**
- В `h.paint(Graphics)` тайлмапа/уровень рисуются через `this.Z.a(paramGraphics)` **до** игрока `this.e.a(paramGraphics)` (h.java:593-598).
- Hoops-overlay из `x[]` рисуется отдельным проходом `a(paramGraphics, this.K)` **после** игрока (h.java:598-600).

Поэтому эффект “мяч проходит между частями кольца” может быть реализован так:
- “задняя/нижняя” часть кольца рисуется как обычные тайлы в `Z.a(...)` (до игрока),
- “передняя/верхняя” часть кольца рисуется в `h.a(Graphics,DirectGraphics)` через `x[]` (после игрока),
и смена состояния при прохождении меняет обе половины через разные tileId (например, пары `97/98 → 99/100` в a.java:1561-1579).

### 2.5.1 Факт: большие hoops строго двухтайловые (без исключений)

По оригинальному коду и по данным уровней `/res/lf` “большие” hoops всегда состоят из **двух соседних тайлов**:
- горизонтальное большое кольцо: строго `[101][102]` (слева направо) (переход в `103/104` меняет 2 тайла: `a.java:814-833`);
- вертикальное большое кольцо: строго
  ```
  [97]
  [98]
  ```
  (сверху вниз) (переход в `99/100` меняет 2 тайла: `a.java:1561-1579`).

Проверка по бинарникам уровней `/res/lf` (tileMap): все вхождения `97` имеют `98` непосредственно снизу, а все вхождения `101` имеют `102` непосредственно справа; других сочетаний нет.

### 2.6 Подтверждение: tileId 98/100 действительно рисуются как обычные тайлы через `Z.a(...)`

1) `g` (tile engine) читает из `/res/tf` для каждого `tileId`:
- `v[tileId]` (renderType) (g.java:220)
- `T[tileId]` (imageIndex) (g.java:221)
- `b[tileId]` (transform) (g.java:222)
- `l[tileId]` (collisionType) (g.java:223)

2) При отрисовке тайлмапы `g` вызывает внутренний `a(tileByte, screenX, screenY, ...)`, который:
- берёт `tileId = tileByte & tileIdMask` (g.java:599-601),
- берёт `renderType = v[tileId]` (g.java:600),
- при `renderType==1`/`5` рисует тайл как изображение `V[ T[tileId] ]` (g.java:613-633).

3) Для `tileId 98` и `100` в этой сборке `/res/tf` задаёт `renderType==1` и реальные `imageIndex` (то есть они не “пустые”):
- `98: v=1, T(img)=98, b(transform)=0x04, l(coll)=3, aux=97` (артефакт `artifacts/tf_tiles_dump.txt:103`)
- `100: v=1, T(img)=99, b(transform)=0x04, l(coll)=3, aux=99` (артефакт `artifacts/tf_tiles_dump.txt:105`)

Итог: `98/100` обязаны рисоваться в `Z.a(...)` как часть тайлмапы (через `g.a(...)`), при этом hoops-overlay (`h.a(Graphics,DirectGraphics)`) их не рисует напрямую, т.к. нет `case 98/100` (h.java:697-727).

---

## 3) Когда “меняются текстуры” (то есть tileId переключается)

В оригинале “картинка hoop” меняется не заменой `x[]`, а сменой `tileId` в `v.R[][]`.

Общий факт: при сборе hoops `E.W--` делается в `a.b(tileY, tileX)` для `tileId ∈ {93,94,97,98,101,102}` (a.java:350-360).
В ветках ниже перед записью новых tileId вызывается `b(...)`, что и уменьшает `W` (a.java:792-795, a.java:821-822, a.java:1556-1558, a.java:1568-1569).

Флаг `0x80` сохраняется при смене:
- `j = tileByte & 0x80` (a.java:320-321)
- новые tileBytes пишутся как `newId | j` или через “сохранить 0x80” и добавить новый id (a.java:823-832, a.java:1570-1579).

### 3.1 94 → 96 (одиночный вертикальный hoop)

Условие перехода и запись:
- в ветке `case 94/96`:
  - если `i5 == 94` и `this.i == tileY*16 + 8`, то
    - `b(tileY, tileX)` (уменьшает `W`) (a.java:792-794)
    - `v.R[tileY][tileX] = 0x60 | flag` (то есть `96 | 0x80`) (a.java:794-795)

См. `a.java:788-795`.

### 3.2 101/102 → 103/104 (двойной вертикальный hoop, меняются 2 тайла)

Условие:
- если `(i5 == 102 || i5 == 101)` и `this.i == tileY*16 + 8` (a.java:820)
- затем `b(tileY, tileX)` (уменьшает `W`) (a.java:821-822)

Запись tileId (с сохранением 0x80):
- если `i5 == 102`:
  - соседний тайл `v.R[tileY][tileX - 1]` сохраняет `0x80`, затем `| 0x67` (103) (a.java:823-825)
  - текущий `v.R[tileY][tileX] = 0x68 | flag` (104) (a.java:825-826)
- если `i5 == 101`:
  - текущий `v.R[tileY][tileX] = 0x67 | flag` (103) (a.java:828-830)
  - соседний `v.R[tileY][tileX + 1]` сохраняет `0x80`, затем `| 0x68` (104) (a.java:830-832)

См. `a.java:814-833` (и повторяется в `a.java:836-871` при вариантах бокового обхода).

### 3.3 93 → 95 (одиночный горизонтальный hoop)

Условие перехода и запись:
- в ветке `case 93/95`:
  - если `i4 == 93` и `this.D == tileX*16 + 8`, то
    - `b(tileY, tileX)` (уменьшает `W`) (a.java:1555-1557)
    - `v.R[tileY][tileX] = 0x5F | flag` (95) (a.java:1557-1558)

См. `a.java:1551-1558`.

### 3.4 97/98 → 99/100 (двойной горизонтальный hoop, меняются 2 тайла)

Условие:
- если `(i4 == 97 || i4 == 98)` и `this.D == tileX*16 + 8` (a.java:1567)
- затем `b(tileY, tileX)` (уменьшает `W`) (a.java:1568-1569)

Запись tileId (с сохранением 0x80):
- если `i4 == 97`:
  - текущий `v.R[tileY][tileX] = 0x63 | flag` (99) (a.java:1569-1571)
  - соседний `v.R[tileY][tileX + 1]` сохраняет `0x80`, затем `| 0x64` (100) (a.java:1571-1572)
- если `i4 == 98`:
  - соседний `v.R[tileY][tileX - 1]` сохраняет `0x80`, затем `| 0x63` (99) (a.java:1575-1578)
  - текущий `v.R[tileY][tileX] = 0x64 | flag` (100) (a.java:1578-1579)

См. `a.java:1561-1579`.
