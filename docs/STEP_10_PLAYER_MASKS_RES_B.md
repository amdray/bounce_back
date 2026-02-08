# STEP 10: Player mask из `/res/b` (pixel-perfect object mask) для `collision_test_collect`

Цель шага: сделать коллизии игрока **pixel-perfect как в оригинале**, используя `player_mask` (boolean-матрицу) из `/res/b` chunk `0`, и передавать выбранную маску в `collision_test_collect(..., player_mask, ...)` вместо `NULL`.

Это критично для корректной обработки объектов, которые зависят от точного пересечения “маска мяча × маска тайла”, включая hoops (например, `94→96`) — см. `a.java` и `g.java` ниже.

Источники истины:
- Формат `/res/b` chunk `0` (маски): `bounce_back/original_code/bounce_back_s60.jar.src/a.java:83-99`
- Выбор маски по `spriteIndex` и вызов tile-engine collision: `bounce_back/original_code/bounce_back_s60.jar.src/a.java:243-255`
- Подпись/семантика `collisionTest` и ориентация `mask[x][y]`: `bounce_back/original_code/bounce_back_s60.jar.src/g.java:315-345`, `g.java:393-396`

---

## 1) Факты из оригинала (FACT)

### 1.1 `/res/b` chunk `0` содержит boolean-матрицы масок

`a` открывает `/res/b` контейнером `c("/res/b")` и первый `c.a()` трактует как бинарный блок масок:
- `b = readByte()` = количество масок (a.java:89-90)
- для каждой маски:
  - `w = readByte()`, `h = readByte()` (a.java:92-94)
  - `mask = new boolean[w][h]` (a.java:94)
  - заполнение `readBoolean()` в двойном цикле (a.java:95-98)

После этого тот же контейнер читает **25 PNG** для спрайтов мяча (a.java:104-108).

### 1.2 Выбор маски зависит от `spriteIndex` (this.g)

Перед вызовом `g.collisionTest` оригинал выбирает индекс маски `i` так:
- если `g ∈ [0..8]` или `g ∈ [20..22]` → `i=0` (a.java:245-246)
- если `g ∈ [9..19]` → `i=1` (a.java:247-248)

### 1.3 Размеры collision rect берутся из маски, а не из PNG-спрайта

Оригинал вычисляет:
- `j = this.f[i].length` (ширина маски) (a.java:250)
- `k = this.f[i][0].length` (высота маски) (a.java:251)
- `m = x - (j >> 1)`, `n = y - (k >> 1)` (a.java:252-253)

И вызывает tile-engine collision:
`this.v.a(m, n, j, k, this.f[i], paramBoolean)` (a.java:254-255),
где `this.v` — это `g` (tile engine) (a.java:116-117).

### 1.4 Ориентация `player_mask` = `mask[x][y]`

В `g.collisionTest` проверка пикселя выглядит так:
`if (paramArrayOfboolean[i9 + i4][i8 + i5])` (g.java:395),
где `i9` — локальный X в overlap, `i8` — локальный Y (см. вложенные циклы g.java:393-396).

Следствие для порта: при хранении маски в 1D массиве индекс должен соответствовать `mask[x + y*width]`.

---

## 2) Что реализуем в порте в этом шаге (минимум)

1) Парсер `/res/b` chunk `0` → загрузить все маски в память как `bool*` (flattened) + их `w/h`.
2) Выбор активной маски по `sprite_index` (a.java:245-248).
3) В `player_update` использовать `mask_w/mask_h` как `rect_w/rect_h` при вызовах `collision_test_collect` (и/или `collision_test`), и передавать `player_mask != NULL`.

---

## 3) Файлы

### Создать
- [ ] `src/player_masks.h`
- [ ] `src/player_masks.c` — загрузка `/res/b` chunk `0` и API выбора маски по `sprite_index`

### Изменить
- [ ] `src/player.h` — добавить ссылку на `PlayerMasks` и поля `mask_w/mask_h` (см. §4)
- [ ] `src/player.c` — загрузить маски, выбрать маску по `sprite_index`, передавать `player_mask` в коллизии (a.java:243-255)
- [ ] `src/collision.h` — расширить `collision_test_collect` параметром `const bool* player_mask` (если сейчас нет — добавить)
- [ ] `src/collision.c` — использовать `player_mask` при pixel-perfect тесте (аналог g.java:395-396 + g.java:424-423)
- [ ] `src/main.c` — только если нужно пробросить `PlayerMasks*` извне (в зависимости от дизайна)
- [ ] `Makefile` — добавить `src/player_masks.c` в `OBJS`

---

## 4) Структуры данных (минимально)

### 4.1 PlayerMasks

Хранить массив масок:
- `count` (byte `b` из a.java:89-90)
- для каждой маски:
  - `w`, `h` (a.java:92-94)
  - `bool* data` размером `w*h` с индексом `x + y*w` (g.java:395)

### 4.2 Player изменения

Добавить в `Player`:
- `PlayerMasks* masks`
- `int mask_w, mask_h`
- `int mask_half_w, mask_half_h`
- `const bool* active_mask` (указатель на data выбранной маски)

---

## 5) Функции

1) `PlayerMasks* player_masks_load(const char* path)`
   - `ResourceContainer* b = resource_load("res/b")`
   - `chunk0 = resource_get_element(b, 0, ...)`
   - парсинг формата (a.java:89-99)

2) `const bool* player_masks_select(PlayerMasks* pm, int sprite_index, int* out_w, int* out_h)`
   - выбор mask index по правилам (a.java:245-248)
   - возвращает `pm->masks[idx].data` и размеры `w/h`

3) `player_update(...)` (изменения)
   - перед каждым вызовом `collision_test_collect` обновить `active_mask` и размеры (если `sprite_index` меняется)
   - `rect_x = x_pos - (mask_w >> 1)`, `rect_y = y_pos - (mask_h >> 1)` (a.java:252-253)
   - `collision_test_collect(..., rect_x, rect_y, mask_w, mask_h, active_mask, ...)`

---

## 6) Пошаговая интеграция

1) **Создать новые файлы:**
   - [ ] `src/player_masks.h`
   - [ ] `src/player_masks.c`

2) **Изменить существующие файлы:**
   - [ ] `src/player.h`: добавить `PlayerMasks*` в `Player`
   - [ ] `src/player.c`:
     - загрузить `PlayerMasks` из `res/b` (a.java:83-99)
     - выбрать активную маску по `sprite_index` (a.java:243-249)
     - использовать `mask_w/mask_h` и `active_mask` в коллизиях (a.java:250-255)
   - [ ] `src/collision.h` / `src/collision.c`: `player_mask` обязателен в `collision_test_collect` и участвует в pixel-perfect проверке как в `g.java` (g.java:395-396)
   - [ ] `Makefile`: добавить `src/player_masks.c`

3) **Порядок изменений:**
   a) `player_masks.h/c`
   b) `collision.h` (подписи)
   c) `collision.c` (использование `player_mask`)
   d) `player.h` (поля)
   e) `player.c` (подключение масок)
   f) `main.c` (если требуется)
   g) `Makefile`

---

## 7) Критерии успеха

- [ ] `debug.log`: при разных `sprite_index` маска выбирается как `0` или `1` по правилам (a.java:245-248).
- [ ] Hoops `94→96` срабатывает **только** при реальном pixel-perfect пересечении, а не “по прямоугольнику” (сравнить поведение до/после, используя один и тот же уровень).
- [ ] Коллизии с наклонными масками (collisionType=3) не регрессируют (g.java:402-419, COLLISION_CONTRACT.md).

