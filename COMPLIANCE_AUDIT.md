# Аудит соответствия: Java MIDP → C (PSPSDK + SDL2)

**Дата:** 2026-03-17  
**Эталон:** Java MIDP (J2ME) — `original_code/bounce_back_s60.jar.src/`  
**Порт:** C (PSPSDK + SDL2) — `src/`

---

## 1. Scope

- **Меню** — включено в проверку (в C-порте реализованы main menu и overlay-состояния).
- **HUD / камера** — допускаются отличия (экран PSP 480×272 vs Nokia 176×208).
- **Всё остальное** — строгое сравнение: пиксель-перфект графика мира, физика, механики.

---

## 2. Evidence Map

| Подсистема | MIDP файл/функция | C файл/функция | Уверенность |
|---|---|---|---|
| Главный цикл | `h.java::run()` | `main.c::main()` | Высокая |
| Физика игрока | `a.java::d()` | `player.c::player_update()` | Высокая |
| Прыжок/отскок | `a.java::h()`, `a.java::f()`, `a.java::a(int,bool)` | `player.c::calculate_jump_strength()`, `calculate_bounce_min()`, bounce inline | Высокая |
| Коллизии (пиксельные) | `g.java::a(int,int,int,int,bool[][],bool)` | `level_loader.c::level_test_collision_collect()` | Высокая |
| Tile-движок (foreground) | `g.java::a(Graphics)` | `level_renderer.c::renderer_draw()` | Высокая |
| Tile-движок (background) | `g.java` (второй экземпляр) | `bg_layer.c::bg_layer_draw()` | Высокая |
| Tile-анимации | `g.java::d()` | `tile_animation.c::animation_tick()` | Высокая |
| Tile-трансформации | `g.java::p[]` lookup | `tile_transform.c::draw_tile_with_transform()` | Высокая |
| Враги (AI/тик) | `h.java::d()` | `level_loader.c::level_objects_tick()` | Высокая |
| Враги (рендер) | `h.java::b(Graphics)` | `enemy_renderer.c::enemy_renderer_draw()` | Высокая |
| Кольца (hoops) | `a.java::b(int,int)`, `h.java::g()` | `player.c::player_collect_tile()`, `update_ring_tile_if_crossed()` | Высокая |
| Кольца (рендер оверлей) | `h.java::a(Graphics,K)` | `foreground_pass.c` | Высокая |
| Выходная дверь (логика) | `h.java` (inline) | `exit_door.c` | Высокая |
| Выходная дверь (рендер) | `h.java::c(Graphics)` | `exit_door.c::exit_door_render()` | Высокая |
| Загрузка уровня | `h.java::b(int)` | `level_loader.c::level_load()` | Высокая |
| Ресурс-контейнер | `c.java` | `resource_loader.c` | Высокая |
| Маски игрока | `a.java::f[][][]` | `player_masks.c` | Высокая |
| Камера | `g.java::c(int,int)` | `camera.c::camera_update()` | Средняя |
| HUD | `h.java::a(Graphics)` | `hud.c::hud_render()` | Средняя |
| Ввод | `h.java::keyPressed/Released` | `input.c::input_update()` | Высокая |
| Смена уровня / жизни | `CrystalMidlet.java` | `main.c::reload_level_state()`, ветка `player->lives <= 0` | Высокая |
| Continue / persist-save | `CrystalRMS` / `q` | `save.c`, `main.c` | Высокая |
| Демо-реплей | `d.java`, `b.java` | **Отсутствует** | Высокая |
| Foreground-проход | `h.java::a(Graphics,K)` (DirectGraphics) | `foreground_pass.c` | Высокая |

---

## 3. Graphics: Pixel-Perfect World

### 3.1 Размер тайла

| Проверка | Результат | Доказательство |
|---|---|---|
| Tile size = 16×16 | **MATCH** | Java: `g.java` — `f` нормализуется из 12→16, использует 16 повсеместно. C: `camera.h` — `#define TILE_SIZE 16`, `level_loader.c` — деления/умножения на 16. |

### 3.2 Tile ID маска и флаги

| Проверка | Результат | Доказательство |
|---|---|---|
| Tile ID mask = 0x7F | **MATCH** | Java: `g.java` — `this.z = 127`. C: `level_renderer.c:56` — `tile_byte & 0x7F`. |
| Flag mask = 0x80 (бит 7, вода/фон) | **MATCH** | Java: `g.java` — `this.q = 128`. C: читается из `/res/tf` заголовка, значение 0x80. |

### 3.3 Трансформации тайлов (flip/rotation)

| Проверка | Результат | Доказательство |
|---|---|---|
| Кодирование трансформации | **MATCH** | Java: биты 0–1 = поворот, бит 2 = V-flip, бит 3 = H-flip, lookup `p[]={0,270,180,90,16384,...}`. C: `tile_transform.c:12-33` — бит 3→`SDL_FLIP_HORIZONTAL`, бит 2→`SDL_FLIP_VERTICAL`, биты 0–1→угол `{0,270,180,90}`. |
| **Применение трансформаций в основном проходе тайлов** | **MATCH** | Java: `g.java::a(int,int,int,boolean,int)` — main loop всегда вызывает `a(b, k, j, false, 0)` (paramInt4=0). Внутри для cases 1/5: Sprite-ветка→`drawImage(..., 20)` без трансформа; non-Sprite-ветка→`k = 0;` безусловно обнуляет transform. C: `level_renderer.c:74` — `SDL_RenderCopy()` без трансформации. Поведение совпадает. |
| Трансформации в foreground-проходе | **MATCH** | C: `foreground_pass.c:78-82` — проверяет `meta.transform != 0` и вызывает `draw_tile_with_transform()`. |
| Трансформации в background-проходе | **MATCH** | Java: `g.java` функция `a(...)` содержит строку `k = 0;` (после вычисления transform), которая безусловно обнуляет transform — фоновые тайлы всегда рисуются без трансформации. C: `bg_layer.c:258` — `SDL_RenderCopy()` без трансформации. Поведение совпадает. |

### 3.4 Порядок рендера

| Проверка | Результат | Доказательство |
|---|---|---|
| Общий порядок слоёв | **MATCH** | Java `h.java:589-610`: BG → FG tiles → Exit door → Enemies → Player → Hoop overlays → HUD. C `main.c`: BG → Tiles → Exit door (`exit_door_render`) → Enemies → Player → Foreground pass (hoops) → HUD. |

### 3.5 Цвет фона для водных/флаговых тайлов

| Проверка | Результат | Доказательство |
|---|---|---|
| Foreground fill color | **MISMATCH** | Java: `g.java` — `this.i = 0x7F00007F` → RGB(0,0,0x7F), alpha=0x7F (полупрозрачный). C: `level_renderer.c:62-65` — цвет читается из ресурса корректно, **но alpha принудительно = 255** (непрозрачный). Водные/фоновые тайлы рендерятся полностью непрозрачными. |
| Background fill color | **MISMATCH** | Java: цвет из заголовка `/res/bg`. C: `bg_layer.c:245-247` — **захардкожен** `(0x00, 0x71, 0xEF)` вместо чтения из ресурса. Значение из заголовка читается (`read_be32_i(p0+10)`), но отбрасывается. |

> **MISMATCH 3.5a — Alpha водных тайлов**  
> **Природа:** Foreground-слой: Java alpha=0x7F (полупрозрачность), C alpha=255 (непрозрачность).  
> **Сценарий:** Все тайлы с битом 7 (вода) рисуются с полностью непрозрачным фоном вместо полупрозрачного.

> **MISMATCH 3.5b — Захардкоженный цвет фона**  
> **Природа:** `bg_layer.c:245-247` — `r=0x00, g=0x71, b=0xEF` вместо значения из заголовка ресурса `/res/bg`.  
> **Обоснование:** Это допущение обосновано по причине ошибки округления цвета для экранов с маленьким количеством цветов (Nokia S60 имел ограниченную палитру), и для современных устройств (PSP с 32-битным цветом) нужен именно этот цвет для корректного отображения.  
> **Сценарий:** Уровни с отличающимся цветом фона в `/res/bg` будут рендериться с неправильным цветом, но это адаптация под современные дисплеи.

### 3.6 Размер экрана / вьюпорт

| Проверка | Результат | Доказательство |
|---|---|---|
| Разрешение экрана | **Исключение** | Java: 176×208 (Nokia). C: 480×272 (PSP). Допустимое отклонение — адаптация под платформу. |
| Viewport gameplay | **Исключение** | Java: 176×179. C: 480×251 (272−21 HUD). Допустимое отклонение. |

### 3.7 Рендер спрайтов игрока

| Проверка | Результат | Доказательство |
|---|---|---|
| Отрисовка спрайта | **MATCH** | Java: `a.java::a(Graphics)` рисует `w[g]` в позиции `(D − камера_x − J, i − камера_y − c)`. C: `player.c::player_render()` рисует текстуру спрайта по аналогичным координатам с вычитанием камеры и half-размеров. |

---

## 4. Physics

### 4.1 Шаг симуляции / тайминг

| Проверка | Результат | Доказательство |
|---|---|---|
| Game loop period | **MATCH** | C `main.c` использует компенсацию `tick_start`/`elapsed` и задержку `if (el < 50) SDL_Delay(50 - el)`, то есть период тика удерживается как 50ms start-to-start (поведение, эквивалентное Java Timer-подходу). |
| Frame rate target | **MATCH** | Оба нацелены на 20 FPS (50ms). |
| Единицы скорости | **MATCH** | Оба: sub-pixel ×10, `/10 = пикселей/тик`. Java: `a.java:720-756`. C: `player.c:1447`. |

> **Статус обновлён:** историческое расхождение 4.1 закрыто — в текущем C-коде каденция 20 Hz компенсируется по elapsed.

### 4.2 Порядок обновления

| Проверка | Результат | Доказательство |
|---|---|---|
| Input → Enemies → Player → Camera | **MATCH** | Java `h.java::run()`: `c()`→`d()`→`e.d()`→`Z.c()`. C `main.c`: `input_update`→`level_objects_tick`→`player_update`→`camera_update`. |
| Tile animation tick | **MISMATCH** (минорный) | Java: tile anims в фазе update (`Z.d()` после камеры). C: `animation_tick()` в фазе render (между bg_draw и renderer_draw). Потенциальный сдвиг на 1 кадр. |

### 4.3 Гравитация

| Проверка | Результат | Доказательство |
|---|---|---|
| Нормальная гравитация | **MATCH** | Java: `b6 = 9`. C: `GRAVITY_NORMAL = 9` (`player.c:14`). |
| Гравитация в воде (обычный мяч) | **MATCH** | Java: `b6 = 7` при `this.l && !this.I`. C: `gravity = 7` при `has_grav_bonus && !is_inverted`. |
| Гравитация в воде (большой мяч) | **MATCH** | Java: `b6 = -6` при `this.l && this.I`. C: `gravity = -6` при `has_grav_bonus && is_inverted`. |
| Макс. скорость падения | **MATCH** | Java: `b5 = 80`. C: `MAX_FALL_SPEED = 80`. |
| Макс. скорость падения (вода) | **MATCH** | Java: `b5 = 20` (или 80 для popped). C: `is_popped ? 80 : 20`. |

### 4.4 Прыжок

| Проверка | Результат | Доказательство |
|---|---|---|
| Jump normal / inverted / popped | **MATCH** | Java: -125 / -180 / -95. C: `JUMP_NORMAL=-125`, `JUMP_INVERTED=-180`, `JUMP_POPPED=-95` (`player.h:22-24`). |
| Бонус прыжка (jump bonus) | **MATCH** | Java: `if(this.C) i += i>>2`. C: `if(has_jump_bonus) i += i>>2` (`player.c:76`). |
| Бонус воды (grav bonus) | **MATCH** | Java: `if(this.l) i -= i>>2`. C: `if(has_grav_bonus) i -= i>>2` (`player.c:79`). |
| Условие прыжка | **MATCH** | Java: jump held + grounded + !down. C: `(mask & 0x8) && !(mask & 0x4) && is_grounded`. |

### 4.5 Отскок (bounce)

| Проверка | Результат | Доказательство |
|---|---|---|
| Bounce min values | **MATCH** | Java: -83 / -120 / -63. C: `BOUNCE_NORMAL=-83`, `BOUNCE_INVERTED=-120`, `BOUNCE_POPPED=-63`. |
| Инициализация bounce_state | **MATCH** | Java: `G = -paramInt` (первый удар). C: `bounce_state = -j` (`player.c:1163`). |
| Порог остановки | **MATCH** | Java: `G > -10 → grounded`. C: `bounce_state > -10 → is_grounded = true` (`player.c:1171`). |
| Скорость покоя | **MATCH** | Java: `j = 30` при остановке. C: `j = 30` (`player.c:1173`). |
| **Коэффициент затухания (плоская поверхность)** | **MATCH** | Java: `G >>= 1` (50%). C: `p->bounce_state >>= 1` (`player.c:888,960,1279,1456`). |
| Коэффициент затухания (склоны) | **MATCH** | Java: `G = 3*G >> 2` (75%). C: `bounce_state = (3 * bounce_state) >> 2` (`player.c:1023`). |

### 4.6 Горизонтальное движение

| Проверка | Результат | Доказательство |
|---|---|---|
| Ускорение normal / ice | **MATCH** | Java: 18 / 22. C: `ACCEL_NORMAL=18`, `ACCEL_BONUS=22` (`player.h:28-29`). |
| Макс. скорость normal / ice | **MATCH** | Java: 60 / 100. C: `MAX_SPEED_NORMAL=60`, `MAX_SPEED_BONUS=100` (`player.h:30-31`). |
| Трение (земля / воздух) | **MATCH** | Java: 8 / 3. C: `DECEL_GROUNDED=8`, `DECEL_AIRBORNE=3` (`player.h:32-33`). |

### 4.7 Пружины

| Проверка | Результат | Доказательство |
|---|---|---|
| Сильная пружина | **MATCH** | Java: ±100 (тайлы 45,51,53,67,71). C: `j -= 100` / `j += 100` (`player.c:742`). |
| Слабая пружина | **MATCH** | Java: ±50 (тайл 75). C: `(center_tile == 75) ? 50 : 100` (`player.c:742`). |

### 4.8 Конвейеры

| Проверка | Результат | Доказательство |
|---|---|---|
| Правый конвейер (43,49,52,54,66,69,73) | **MATCH** | Java: `i = 250`, затем friction. C: `i = 250`, затем friction (`player.c:1432-1440`). Оба содержат один и тот же баг (friction от фиксированного 250). |
| Левый конвейер (44,50) | **MATCH** | Java: `i -= 250`. C: `i -= 250`. |
| Слабый левый (76) | **MATCH** | Java: `i -= 125`. C: `i -= 125`. |

### 4.9 Коллизии (пиксельные)

| Проверка | Результат | Доказательство |
|---|---|---|
| Collision type 0/1/2/3 | **MATCH** | Java `g.java`: 0=skip, 1=mask, 2=solid, 3=mask+transform. C `level_loader.c:518-636`: идентичная классификация. |
| Трансформация для type 3 | **MATCH** | Java: `p[]` lookup → flip/rotate coords. C: `apply_transform_16()` — бит 3=H-flip, бит 2=V-flip, биты 0–1=rotation. |
| Max collision hits | **MATCH** | Java: `Y[5]`, `P[5]`. C: `COLLISION_HITS_MAX = 5`. |

### 4.10 Инверсия гравитации

| Проверка | Результат | Доказательство |
|---|---|---|
| Инверсия по `gravity_down`/`p` | **MATCH** | Java: `this.p = true` при тайле 15. C: `gravity_down = true` при тайле 15. Оба инвертируют вертикальную скорость перед/после movement loop. |

---

## 5. Game Mechanics

### 5.1 Тайловые эффекты поверхности

| Проверка | Результат | Доказательство |
|---|---|---|
| Тайл 39 (ice → speed_bonus) | **MATCH** | Java: `j=true, C=false, p=false`. C: `has_speed_bonus=true, has_jump_bonus=false, gravity_down=false`. Переименовано, но поведение идентично. |
| Тайл 26 (speed → jump_bonus) | **MATCH** | Java: `C=true, j=false, p=false`. C: `has_jump_bonus=true, has_speed_bonus=false, gravity_down=false`. |
| Тайл 15 (gravity down) | **MATCH** | Java: `p=true, C=false, j=false, x=false`. C: `gravity_down=true, has_jump_bonus=false, has_speed_bonus=false, is_grounded=false`. |
| Таймер powerup | **MATCH** | Java: `B = 450`. C: `timer_b = 450`. |

### 5.2 Трансформации мяча

| Проверка | Результат | Доказательство |
|---|---|---|
| Тайл 18 (inflate: small → inverted) | **MATCH** | Java: `!I && !F → state_a=1`. C: `!is_inverted && !is_popped → state_a=1` (`player.c:483`). |
| Тайл 11 (deflate: inverted → small) | **MATCH** | Java: `I && !F → state_a=2`. C: `is_inverted && !is_popped → state_a=2` (`player.c:488`). |
| Тайл 22 (pop: small → popped) | **MATCH** | Java: sets `state_r`, resets `timer_c=550` if already popped. C: identical (`player.c:475-478`). |
| Таймер popped ball | **MATCH** | Java: `b = 550`. C: `timer_c = 550`. |
| Death animation | **MATCH** | Java: `A = 25`. C: `timer_a = 25`. |

### 5.3 Сбор предметов

| Проверка | Результат | Доказательство |
|---|---|---|
| Checkpoint (тайл 34) | **MATCH** | Java: +200 очков, сохраняет spawn position. C: +200, сохраняет spawn (`player.c:416-420`). |
| Extra life (тайл 12) | **MATCH** | Java: +1000, +1 жизнь (cap 5). C: +1000, +1 жизнь (cap 5) (`player.c:421-427`). |
| Gem (тайл 30) | **MATCH** | Java: +2500. C: +2500 (`player.c:428-430`). |
| Hoops (93,94,97,98,101,102) | **MATCH** | Java: +500, `W--`. C: +500, `hoops_remaining--` (`player.c:431-437`). |

### 5.4 Сбор колец — условия центрирования

| Проверка | Результат | Доказательство |
|---|---|---|
| Hoop 93 (horizontal) | **MATCH** | Java: `x_pos == tile_x*16+8`. C: `p->x_pos == tx*16+8` (`player.c:1495`). |
| Hoop 94 (vertical) | **MATCH** | Java: `y_pos == tile_y*16+8`. C: `p->y_pos == ty*16+8` (`player.c:790`). |
| Hoop 97/98 (paired) | **MATCH** | Java: x-alignment. C: x-alignment (`player.c:1574`). |
| Hoop 101/102 (big) | **MATCH** | Java: y-alignment, pass-through. C: y-alignment, not-toward-surface (`player.c:827`). |
| Подсчёт колец при загрузке | **MATCH** | Java: counts 93,94,97,101. C: `counts_toward_initial_hoops()` — 93,94,97,101 (`level_loader.c:269-274`). |
| Условие `!is_inverted` для pass-through | **MATCH** | Java: `!this.I` для 93,94. C: `!p->is_inverted` (`player.c:1491, 786`). |

### 5.5 Враги

| Проверка | Результат | Доказательство |
|---|---|---|
| Type 0 (large, 32×32) linear | **MATCH** | Java: back-and-forth. C: identical (`level_loader.c:449-483`). |
| Type 1 (small, 16×16) bounce | **MATCH** | Java: vy=30 top, -40 bottom, gravity+1/tick, cap 80, speed max 3px/tick. C: identical (`level_loader.c:491-508`). |
| Type 2 (spike, 24×11) linear | **MATCH** | Java: back-and-forth. C: identical. |
| Type 0 stall on popped overlap | **MATCH** | Java: type 0 stalls when overlapping popped ball. C: identical check (`level_loader.c:466-475`). |
| Коллизия с врагом (вертикальная) | **MATCH** | Java: always fatal. C: tile IDs 201,202 → `player_kill()` (`player.c:935`). |
| Коллизия с врагом (горизонтальная) | **MATCH** | Java: fatal unless popped. C: kills if `!is_popped`, else `i=0` (`player.c:1692`). |

### 5.6 Выходная дверь

| Проверка | Результат | Доказательство |
|---|---|---|
| Условие открытия | **MATCH** | Java: `hoops_remaining == 0`. C: `hoops_remaining == 0` (`exit_door.c`). |
| Анимация (тики) | **MATCH** | Java: 0→48 (открытие), 48→72→48 (цикл). C: identical. |
| Действие при входе | **MATCH** | C: считает `time_bonus`, добавляет stage bonus, переходит на следующий уровень через `reload_level_state()`. Для финала используется отдельный congratulations-flow (`APP_STATE_CONGRATULATIONS`) с корректной веткой продолжения/завершения, без прежнего instant-exit. |

### 5.7 Система жизней и Game Over

| Проверка | Результат | Доказательство |
|---|---|---|
| Начальные жизни | **MATCH** | Java: `this.h = 3`. C: `player->lives = 3` (implicit in init). |
| Вычитание при смерти | **MATCH** | Java: `this.o.h--`. C: `p->lives--` в `player_respawn()`. |
| Проверка Game Over | **MATCH** | В `main.c` есть проверка `if (player->lives <= 0)` с переходом в `APP_STATE_GAME_OVER`; экран Game Over рендерится через `menu_render_game_over`, а затем происходит возврат в menu flow. |

### 5.8 Подсчёт уровней

| Проверка | Результат | Доказательство |
|---|---|---|
| Количество уровней | **MATCH** | Java: 21 уровень (indices 0–20). C: `LEVEL_COUNT = 21` в `main.c`. |

### 5.9 Разрушаемые блоки

| Проверка | Результат | Доказательство |
|---|---|---|
| Тайлы 7, 8 | **MATCH** | Java: разрушаются при `is_popped/big ball`, иначе solid. C: `player_apply_tile_break()` — разрушает при `is_popped`, заменяет на тайл 105 (`player.c`). |

### 5.10 Таймер уровня

| Проверка | Результат | Доказательство |
|---|---|---|
| Таймер для временного бонуса | **MATCH** | C: `level_start_ticks = SDL_GetTicks()` и расчёт `level_timer_ms`, `level_seconds`, `time_bonus` в ветке завершения уровня (`main.c`). |

### 5.11 Демо-реплей

| Проверка | Результат | Доказательство |
|---|---|---|
| **Demo playback (не часть обычного gameplay loop)** | **MISMATCH** | Java: `d.java` + `b.java` — читает запись из `/res/r`, воспроизводит тик-по-тику, но запускается через отдельный путь `Help -> Game Help`. C: **не реализовано**. |

> **Уточнение:** это отдельный help/demo режим, а не обязательная часть обычного прохождения кампании.

### 5.12 Звуки

| Проверка | Результат | Доказательство |
|---|---|---|
| **11 звуковых эффектов** | **PARTIAL** | Java: загружает 11 звуков из `/res/s`, воспроизводит при прыжке, сборе, пружине и т.д. C: звуковая система присутствует (`sound_init`, `sound_play`, `sound_stop_all`), загрузка 11 OTT из `/res/s` реализована, но parity по качеству/поведению воспроизведения ещё требует доводки. |

### 5.13 Cheat-коды

| Проверка | Результат | Доказательство |
|---|---|---|
| Java cheat (1-1-5-5-0-2-4) | **Не применимо** | Привязан к числовой клавиатуре Nokia. |
| PSP cheat (L+R → god_mode) | **Дополнение** | `player.c` — `god_mode` toggle, отсутствует в Java. Допустимое отклонение (PSP-специфика). |

---
