# STEP 08: “Hoops” (93–104) + выход из уровня (door) + завершение уровня (без HUD)

Цель шага: добавить **минимальный игровой прогресс** “собрать hoops → открыть выход → завершить уровень”, строго по оригиналу.

Почему **без HUD**: в оригинале HUD — это фиксированная нижняя полоса `176×29` с `setClip(0,179,176,29)` (h.java:617). Для PSP `480×272` этот фон нужно перепроектировать, поэтому HUD выносим в отдельный шаг (предлагаемый STEP 09).

Источники истины:
- `bounce_back/original_code/bounce_back_s60.jar.src/h.java` — координаты выхода + счётчик hoops `W` + логика “дверь открывается при W==0”
- `bounce_back/original_code/bounce_back_s60.jar.src/a.java` — прохождение hoops (смена тайлов + `W--`) + условие завершения уровня

---

## 1) Факты/правила из оригинала (обязательно)

### 1.1 Level header (/res/lf meta) содержит координаты выхода

Порядок чтения meta (первые 7 байт) в `h.b(int levelIndex)`:
- `themeId` = `readByte()` (h.java:195)
- `spawnY`  = `readByte()` (h.java:196)
- `spawnX`  = `readByte()` (h.java:197)
- `ballType`= `readByte()` (h.java:198)
- `exitY`   = `readByte()` (h.java:199) → поле `h.ar` (используется как Y тайла выхода в `h.c(Graphics)` — h.java:640)
- `exitX`   = `readByte()` (h.java:200) → поле `h.D`  (используется как X тайла выхода — h.java:641)
- `enemyCount` = `readByte()` (h.java:201) → поле `h.j` (используется для `b(Graphics)` — h.java:1206)

### 1.2 Hoop counter `W` считается при загрузке уровня

В `h.b(int)` после загрузки `Z.R[][]` (tileMap) выполняется скан всей карты:
- Если `tileId ∈ {93, 94, 97, 101}` → координаты кладутся в `h.h[][]`, а `W++` (h.java:322-338).

Важно: **`W` уменьшается при прохождении hoops** (а не при “соприкосновении” с любым тайлом):
- `a.b(tileY, tileX)` уменьшает `E.W--` для `tileId ∈ {93,94,97,98,101,102}` (a.java:316-362).

### 1.3 Door (выход) “открывается” только когда `W==0`

Отрисовка двери в `h.c(Graphics)`:
- позиция в мире: `P = exitX*16`, `R = exitY*16` (h.java:643-646)
- анимация:
  - если `o==true` (дверь “открыта”): `I++`, при `I==72` → `I=48` (h.java:664-667)
  - если `o==false` и `W==0`: `I++`, при `I==48` → `o=true` (h.java:668-672)

Для порта в этом шаге нам важны **значения и переходы** `I/o`, даже если дверь временно рисуется как debug-прямоугольник.

### 1.4 Условие завершения уровня (player enters door area)

В начале `a.d()`:
- если `E.o == true` и игрок внутри прямоугольника `32×48` в пикселях относительно `E.P/E.R`,
  то уровень завершается (a.java:607-612).

### 1.5 Hoops: смена tileId и точные условия (привязка к пиксельной сетке)

Факт: прохождение hoops делает 2 вещи:
1) `W--` (a.java:356-360)
2) меняет tileId (с сохранением флага `0x80`) (a.java:320-321 + ветки ниже)

Минимальные ветки, которые нужны для прогресса “собрать → выйти”:
- “горизонтальный hoop” `93` → становится `95` при точном X-выравнивании (a.java:1551-1558)
- “вертикальный hoop” `94` → становится `96` при точном Y-выравнивании (a.java:788-795)
- “двойной hoop” `97/98` → становится `99/100` (меняются 2 тайла) (a.java:1561-1579)
- “двойной hoop” `101/102` → становится `103/104` (меняются 2 тайла) (a.java:814-833)

Отдельный важный факт про `101..104` в вертикальном шаге: даже если коллизии “нет”, tile добавляется в список обработчиков как hit (`this.v.Y/P`) и дальнейшая логика выполняется (a.java:705-713).

---

## 2) Что реализуем в порте в этом шаге (минимум)

1) Парсинг `exitX/exitY` в `Level`.
2) Подсчёт `W` (hoops remaining) из tileMap (тайлы `93/94/97/101`) (h.java:322-338).
3) Логика:
   - `W` уменьшается при прохождении hoops: минимум обработать смену тайлов для `93`, `94`, `97/98`, `101/102` (a.java:788-833 и a.java:1551-1579) и сам декремент (a.java:356-360).
   - `door` начинает открываться при `W==0` (h.java:668-672).
   - условие “вышли из уровня” (a.java:607-612): для bring-up достаточно `debug.log: "LEVEL_COMPLETE"` и `running=0`.

Визуализация в этом шаге допускается “debug-only”:
- дверь = прямоугольник по `(exitX*16, exitY*16)` размером `32×48` (a.java:607-612)
- `W` печатать в `debug.log`

---

## 3) Файлы

### Создать
- [ ] `src/hoops.h`
- [ ] `src/hoops.c` — подсчёт W + применение “hoops” tile transitions (a.java:788-833, 1551-1579)
- [ ] `src/exit_door.h`
- [ ] `src/exit_door.c` — state `I/o` + проверка зоны выхода (a.java:607-612, h.java:664-672)

### Изменить
- [ ] `src/level_loader.h` — добавить `exit_x/exit_y`
- [ ] `src/level_loader.c` — распарсить `meta[4]`/`meta[5]` как `exit_y/exit_x` (h.java:199-200)
- [ ] `src/level_loader.h` — добавить `level_set_tile()` (нужно для смены tileId при прохождении hoops)
- [ ] `src/level_loader.c` — реализовать `level_set_tile()` (запись в `tile_map[]`)
- [ ] `src/collision.h` — добавить `CollisionHits` + новую функцию `collision_test_collect(...)` (аналог `g.Y/P[5]`) (g.java:315-344)
- [ ] `src/collision.c` — реализовать `collision_test_collect(...)` (заполнение hit-list, лимит 5) (g.java:318-336, COLLISION_CONTRACT.md § 2)
- [ ] `src/player.c` — добавить обработку hoops/door в update (только то, что нужно для W/exit)
- [ ] `src/main.c` — создать `HoopsState` + `ExitDoorState`, вызывать update, логировать завершение

Примечание про HUD:
- отдельный шаг (предлагаемый STEP 09) займётся `/res/ic` и переразметкой UI под `480×272` (h.java:612-633, h.java:617).

---

## 4) Структуры данных (минимальные)

### 4.1 Level (добавить поля выхода)

- `exit_x_tiles` = meta[5] (h.java:200)
- `exit_y_tiles` = meta[4] (h.java:199)

### 4.2 HoopsState

Храним:
- `int remaining` = `W`

### 4.3 ExitDoorState

Минимально:
- `bool open` (аналог `h.o`, h.java:664-672)
- `int I` (аналог `h.I`, h.java:664-672)

## 5) Ключевые функции (без “лишнего”)

1) `hoops_build(Level* level, HoopsState* out)`
   - Скан `level->tile_map`:
     - если `tileId ∈ {93, 94, 97, 101}` → `remaining++` (h.java:322-338)

2) `hoops_try_collect_at(Player* p, Level* level, HoopsState* hs, int tile_x, int tile_y)`
   - `tile_byte = level_get_tile(level, tile_x, tile_y)`, `tileId = tile_byte & 0x7F`, `flag = tile_byte & 0x80` (a.java:320-321)
   - Проверять **только** условия выравнивания из оригинала:
     - `93`: `p->y_pos - p->half_height == tile_y*16` и `p->x_pos == tile_x*16 + 8` → `W--`, `tile=95|flag` (a.java:1551-1558, a.java:356-360)
     - `94`: `p->x_pos - p->half_width == tile_x*16` и `p->y_pos == tile_y*16 + 8` → `W--`, `tile=96|flag` (a.java:788-795, a.java:356-360)
     - `97/98`: `p->x_pos == tile_x*16 + 8` → `W--`, сменить 2 тайла `99/100` (a.java:1561-1579, a.java:356-360)
     - `101/102`: `p->y_pos == tile_y*16 + 8` → `W--`, сменить 2 тайла `103/104` (a.java:814-833, a.java:356-360)

3) `collision_test_collect(..., CollisionHits* hits)`
   - Функция нужна только для Step 08: чтобы `player_update` мог получить **координаты тайлов**, которые “зацепили” игрока (аналог `g.Y/P`).
   - Заполняет максимум 5 попаданий (`hits->x[i]=tileX`, `hits->y[i]=tileY`) (g.java:333-335).
   - Порядок обхода должен совпадать с оригиналом: внешний цикл `tileX` слева→направо, внутренний `tileY` сверху→вниз (g.java:327-335).
   - Если попаданий больше 5: в оригинале это приводит к AIOOBE и `return true` (COLLISION_CONTRACT.md § 2). Для порта: фиксировать `hits->overflow=true` и возвращать `true`.

4) `collision_hits_clear(CollisionHits* hits)` / `collision_hits_add(CollisionHits* hits, int tile_x, int tile_y)`
   - `clear`: выставляет `x/y = -1`, `overflow=false` (g.java:318-321).
   - `add`: ищет первый слот `x==-1` и пишет координаты; если слотов нет → `overflow=true` (a.java:706-713 использует тот же паттерн “первый -1”).

5) `exit_door_tick(ExitDoorState* door, int hoops_remaining)`
   - Если `hoops_remaining==0` и `door->open==false`: `I++`, при `I==48` → `open=true` (h.java:668-672).
   - Если `door->open==true`: `I++`, при `I==72` → `I=48` (h.java:664-667).

6) `exit_door_test_complete(ExitDoorState* door, Level* level, Player* p)`
   - Возвращает true, если `door->open==true` и игрок внутри зоны `32×48` от `(exit_x*16, exit_y*16)` (a.java:607-612).

---

## 6) Пошаговая интеграция

1) **Создать новые файлы:**
   - [ ] `src/hoops.h/c`
   - [ ] `src/exit_door.h/c`

2) **Изменить `Level`:**
   - [ ] `src/level_loader.h`: добавить `uint8_t exit_x, exit_y;`
   - [ ] `src/level_loader.c`: `exit_y = meta[4]`, `exit_x = meta[5]` (h.java:199-200)
   - [ ] `src/level_loader.h`: добавить `level_set_tile(Level*, int x, int y, uint8_t tile_byte)`
   - [ ] `src/level_loader.c`: реализовать запись в `tile_map[]` (нужно для a.java:788-833, 1551-1579)

3) **main.c: загрузка и state**
   - [ ] После `level_load`: `hoops_build(level, &hoops);`
   - [ ] Инициализировать door state: `door.I=0`, `door.open=false` (h.java:288-291, 664-672)

4) **main.c: игровой тик**
   - [ ] Внутри `player_update`: на каждом пиксель-степе движения:
     - `collision_hits_clear(&hits)` (g.java:318-321)
     - получить hit-list через `collision_test_collect(...)`:
     - вертикаль: после проверки вычислить `m = p->x_pos/16`, `n = (test_y)/16`, и если `tileId ∈ {101,102,103,104}` — вызвать `collision_hits_add(&hits, m, n)` и форсировать `collision=true` (a.java:702-713)
     - горизонталь: после проверки вычислить `m = (test_x)/16`, `n = p->y_pos/16`, и если `tileId ∈ {97,98,99,100}` — вызвать `collision_hits_add(&hits, m, n)` и форсировать `collision=true` (a.java:1440-1451)
   - [ ] При `collision==true`: пройтись по `hits` и вызвать `hoops_try_collect_at(...)` для каждого `(tile_x,tile_y)` (a.java:788-833, 1551-1579).
   - [ ] `exit_door_tick(&door, hoops.remaining)` (h.java:664-672)
   - [ ] Если `exit_door_test_complete(...) == true`: `debug.log: LEVEL_COMPLETE` и выход из цикла (a.java:607-612)

5) **Makefile**
   - [ ] Добавить `src/hoops.c` и `src/exit_door.c` в `OBJS`.

---

## 7) Тесты / критерии успеха

- [ ] На старте `debug.log`: `hoops_remaining=W` совпадает с количеством тайлов `93/94/97/101` в уровне (h.java:322-338).
- [ ] При прохождении hoops:
  - [ ] `remaining--` ровно 1 раз на hoop и соответствующий tileId меняется на “пройденный” (a.java:788-795, 814-833, 1551-1579).
- [ ] Когда `remaining==0`: дверь начинает “открываться” и через 48 тиков становится `open=true` (h.java:668-672).
- [ ] Когда `open==true` и игрок входит в `32×48` зону двери: `debug.log: LEVEL_COMPLETE` и игра завершает цикл (a.java:607-612).
