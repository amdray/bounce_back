# ИНЖЕНЕРНЫЙ АУДИТ КОДА BOUNCE BACK (C/SDL)

**Дата:** 2026-03-13  
**Аудитор:** Reverse Engineering & Code Quality Audit  
**Объект:** `src/*.c`, `src/*.h` (53 файла)  
**Эталон:** Java MIDP (`original_code/bounce_back_s60.jar.src/`)  
**Статус Java-кода:** Декомпилирован, основные классы проанализированы (a.java, h.java, g.java, i.java, c.java)

---

## FINDINGS ПО ПРИОРИТЕТУ

### 🔴 CRITICAL

---

#### **C1. Отсутствие обработки AIOOBE из collision_test**

**Где:** `src/collision.c:57-66`, `src/level_loader.c:315-450`

**Что не так:**
В Java `g.collisionTest()` любой `ArrayIndexOutOfBoundsException` ловится и интерпретируется как `true` (коллизия). Это **фича**, а не баг — выход за границы карты = стена.

В C-порте `level_test_collision_collect()` и `level_test_collision()` **не имеют try/catch** и возвращают `false` при выходе за границы, либо падают с segmentation fault.

**Влияние:**
- Игрок может выйти за пределы уровня через коллизии на границах
- Поведение не соответствует Java-оригиналу
- Потенциальный crash при отрицательных координатах

**Java-оригинал:** `g.java:326-343` — единый `try/catch(ArrayIndexOutOfBoundsException)` вокруг всего перебора тайлов.

**Минимально-инвазивное исправление:**
```c
// В начале level_test_collision_collect()
if (rect_x < -rect_w || rect_y < -rect_h || 
    rect_x >= level->width * 16 || rect_y >= level->height * 16) {
    if (collect_hits) hits->overflow = true;
    return true;  // AIOOBE mimic
}
```

**Риск регрессии:** Низкий — это восстановление паритета с Java.

**Уверенность:** **Высокая**

---

#### **C2. Неправильная ориентация collision masks при загрузке**

**Где:** `src/level_loader.c:220-240`

**Что не так:**
В Java inline mask загружается как `s[tileId][x][y] = readBoolean()` (y-outer, x-inner), но читается в коллизии как `tileMask[localY][localX]`. Это **транспонирование**.

C-порт загружает маску в `level->masks[tileId][x][y]` без транспонирования, что приводит к зеркально-перевернутым коллизиям для `collisionType=1`.

**Влияние:**
- Все тайлы с `collisionType=1` (3, 52, 54, 56, 59, 66, 69, 93, 95, 97, 99, 113) имеют **неправильную форму коллизии**
- Пиксель-перфект коллизии не работают как в оригинале

**Java-оригинал:** `g.java:225-226` (загрузка), `g.java:420-422` (чтение).

**Исправление:**
```c
// При загрузке inline mask (collisionType == 1)
// Java: s[tileId][x][y] = readBoolean() → читается как mask[localY][localX]
// C: нужно транспонировать при загрузке
for (uint8_t y = 0; y < level->tile_h; y++) {
    for (uint8_t x = 0; x < level->tile_w; x++) {
        mask[x][y] = read_boolean();  // ← сейчас так
        // Нужно:
        // mask[y][x] = read_boolean();
    }
}
```

**Риск регрессии:** Средний — затронет все коллизии с масками.

**Уверенность:** **Высокая** (подтверждено в `COLLISION_CONTRACT.md`)

---

### 🟠 HIGH

---

#### **H1. Захардкоженный цвет фона вместо чтения из /res/bg**

**Где:** `src/bg_layer.c:245-247`

**Что не так:**
```c
const uint8_t r = 0x00;
const uint8_t g = 0x71;
const uint8_t b = 0xEF;
```
Цвет читается из заголовка ресурса (`read_be32_i(p0+10)` на строке 94), но **отбрасывается**. Вместо него используется захардкоженное значение.

**Влияние:**
- Уровни с отличающимся цветом фона рендерятся неправильно
- Нарушение паритета с Java (там цвет из заголовка)

**Обоснование в README:** Адаптация под современные дисплеи из-за ошибки округления на Nokia S60.

**Java-оригинал:** `g.java` — цвет из заголовка `/res/bg`.

**Исправление:** Сохранить значение из заголовка в `BgLayer` и использовать.

**Риск регрессии:** Низкий — визуальное изменение.

**Уверенность:** **Высокая**

---

#### **H2. Alpha водных тайлов = 255 вместо 0x7F**

**Где:** `src/level_renderer.c:62-65`

**Что не так:**
```c
if ((tile_byte & flag_mask) != 0) {
    SDL_SetRenderDrawColor(r, bg_r, bg_g, bg_b, 255);  // ← alpha=255
    SDL_RenderFillRect(r, &dest);
}
```

**Влияние:**
- Водные/фоновые тайлы полностью непрозрачные
- В Java alpha=0x7F (полупрозрачный фиолетовый `0x7F00007F`)

**Java-оригинал:** `g.java` — `this.i = 0x7F00007F` → RGB(0,0,0x7F), alpha=0x7F.

**Исправление:**
```c
SDL_SetRenderDrawColor(r, bg_r, bg_g, bg_b, 0x7F);
```

**Риск регрессии:** Низкий — визуальное изменение.

**Уверенность:** **Высокая**

---

#### **H4. Tileset/bg_layer загружаются при каждой смене уровня без кэширования**

**Где:** `src/main.c:95-125` (`reload_level_state`)

**Что не так:**
При смене уровня **всегда** грузится весь тайлсет и bg_layer заново, даже если `theme_id` не изменился.

**Влияние:**
- +0.067s на каждый reload (из `debug.log`)
- Избыточная нагрузка на I/O и VRAM

**Исправление:**
```c
// Проверка перед загрузкой
static int cached_theme_id = -1;
static Tileset* cached_tileset = NULL;
if (new_level->theme_id != cached_theme_id) {
    // Загружать только при смене темы
}
```

**Риск регрессии:** Низкий.

**Уверенность:** **Высокая**

---

#### **H5. Фризы при загрузке шрифтов (hud_init 0.571s)**

**Где:** `src/hud_font.c:95-115`

**Что не так:**
3 отдельных атласа (9px, 12px, 23px) строятся независимо:
- Каждый аллоцирует RGBA буфер
- Распаковка 4bpp → RGBA8888 пиксельным циклом
- `SDL_CreateTexture` + `SDL_UpdateTexture` на GPU

**Влияние:**
- 0.571s на загрузку (из `debug.log`)
- Много мелких GPU upload'ов

**Исправление:**
- Объединить все 3 размера в одну текстуру-атлас
- Или кэшировать распакованные данные

**Риск регрессии:** Средний — изменение рендеринга текста.

**Уверенность:** **Высокая**

---

**Исправление:**
```c
case APP_STATE_MENU:
    menu_render_main(renderer, &menu_state, &input);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);  // 60 FPS для UI
    continue;
```

**Риск регрессии:** Низкий.

**Уверенность:** **Высокая**

---

### 🟡 MEDIUM

---

#### **M1. Отсутствие демо-реплея и звука**

**Где:** Не реализовано

**Что не так:**
- Java: `d.java` + `b.java` — демо-реплей из `/res/r`
- Java: 11 звуковых эффектов из `/res/s`
- C: **отсутствует**

**Влияние:**
- MISMATCH 5.11, 5.12 из COMPLIANCE_AUDIT.md
- Не критично для gameplay, но нарушение паритета

**Уверенность:** **Высокая**

---

#### **M2. Tile animation tick в render-фазе, а не update-фазе**

**Где:** `src/main.c:420-430`

**Что не так:**
```c
animation_tick(tile_anim, tile_meta);  // Между bg_draw и renderer_draw
```

В Java: `Z.d()` (tile anims) в фазе **update**, до рендера.

**Влияние:**
- Сдвиг на 1 кадр анимации
- MISMATCH 4.2 из COMPLIANCE_AUDIT.md

**Исправление:** Переместить `animation_tick()` в фазу update (перед `player_update`).

**Риск регрессии:** Низкий — визуальное изменение.

**Уверенность:** **Высокая**

---

#### **M3. God_mode cheat без паритета с Java**

**Где:** `src/player.c` (L+R → god_mode)

**Что не так:**
В Java нет god_mode. Это PSP-специфичное дополнение.

**Влияние:**
- Нарушение паритета (допустимое)
- Может маскировать баги коллизий/смерти

**Уверенность:** **Высокая**

---

#### **M4. Дублирование логики трансформаций**

**Где:** `src/collision.c:33-57`, `src/tile_transform.c:7-31`

**Что не так:**
Функция `apply_transform()` в `collision.c` дублирует логику из `tile_transform.c`, но с небольшими отличиями в порядке операций.

**Влияние:**
- Риск рассинхронизации
- Сложнее поддерживать

**Исправление:** Выделить в `tile_transform_apply()` и использовать везде.

**Риск регрессии:** Средний.

**Уверенность:** **Средняя**

---

### 🟢 LOW

---

#### **L1. fprintf(log, ...) в горячем пути**

**Где:** `src/main.c:410-420`

**Что не так:**
```c
if (log) {
    fprintf(log, "xSpeed=%d\n", ACCEL_NORMAL);  // Каждый тик!
}
```

**Влияние:**
- I/O каждый 50ms тик
- 80+ строк в логе (из `debug.log`)

**Исправление:** `#ifdef DEBUG_VERBOSE` или убрать.

**Уверенность:** **Высокая**

---

#### **L2. SDL_QueryTexture каждый кадр при смене спрайта**

**Где:** `src/player.c:56-67` (`player_refresh_sprite_dims`)

**Что не так:**
```c
SDL_QueryTexture(tex, NULL, NULL, &w, &h);
```
Вызывается при каждой смене спрайта (инфляция/дефляция).

**Влияние:**
- GPU query в горячем пути
- Можно кэшировать размеры в `Player`

**Исправление:** Кэшировать `sprite_width/height` при загрузке `ball_sprites`.

**Уверенность:** **Высокая**

---

#### **L3. Индивидуальные SDL_RenderCopy на каждый тайл**

**Где:** `src/level_renderer.c:50-80`

**Что не так:**
~510 draw calls на кадр (30×17 видимых тайлов).

**Влияние:**
- На PSP SDL → GU, каждый RenderCopy = потенциальный flush
- CPU bottleneck

**Исправление:** Batch рендеринг через `SDL_RenderGeometry` или render target.

**Риск регрессии:** Высокий — сложная оптимизация.

**Уверенность:** **Высокая**

---

## DEAD CODE CANDIDATES

| Файл | Строки | Описание | Рекомендация |
|------|--------|----------|--------------|
| `player.c` | `god_mode` | Без паритета с Java | Удалить или скрыть за `#ifdef CHEATS` |
| `main.c` | `fprintf(log, ...)` | Отладочный лог в production | `#ifdef DEBUG_VERBOSE` |
| `hud.c` | Fallback ring icon | Дублирование с `/res/ic` | Упростить |

---

## SUSPICIOUS FALLBACK/HACK CANDIDATES

| Файл | Строки | Описание | Проблема |
|------|--------|----------|----------|
| `bg_layer.c` | 245-247 | Захардкоженный цвет фона | MISMATCH 3.5b |
| `level_renderer.c` | 62-65 | Alpha=255 для воды | MISMATCH 3.5a |
| `main.c` | 400-450 | SDL_Delay без компенсации | MISMATCH 4.1 |
| `collision.c` | 57-66 | Нет AIOOBE handling | Нарушение паритета |

---

## DUPLICATE LOGIC CLUSTERS

### 1. Трансформации тайлов

**Где:** `collision.c:33-57`, `tile_transform.c:7-31`, `foreground_pass.c:78-82`

**Проблема:** Логика `transform` битов дублируется с минорными отличиями.

**Рекомендация:** Выделить `tile_transform_apply(uint8_t transform, int* x, int* y)` в `tile_transform.h`.

---

### 2. Загрузка PNG из памяти

**Где:** `bg_layer.c:25-35`, `hud.c:35-45`, `enemy_renderer.c:17-27`, `exit_door.c:52-62`

**Проблема:** 4 идентичные реализации `load_png_texture_from_mem()`.

**Рекомендация:** Выделить в `resource_utils.c`.

---

### 3. Wrap/Clamp логика

**Где:** `bg_layer.c:37-45`, `camera.c:40-60`

**Проблема:** Дублирование `wrap_mod()` и clamp-логики.

**Рекомендация:** Общий `world_wrap_coordinate()` в `level_loader.h`.

---

## ТОП-10 БЫСТРЫХ УЛУЧШЕНИЙ С ВЫСОКИМ ROI

| # | Улучшение | Сложность | Эффект |
|---|-----------|-----------|--------|
| 1 | Убрать `fprintf` из game loop | 5 мин | -I/O per frame |
| 2 | SDL_Delay(16) в меню | 10 мин | 60 FPS UI |
| 3 | Компенсация тайминга | 30 мин | 20 FPS стабильно |
| 4 | Alpha=0x7F для воды | 5 мин | Визуальный паритет |
| 5 | Кэш tileset по theme_id | 1 ч | -0.067s на reload |
| 6 | AIOOBE handling в collision | 30 мин | Паритет с Java |
| 7 | Транспонирование collision masks | 1 ч | Корректные коллизии |
| 8 | Кэш sprite dimensions | 30 мин | -GPU queries |
| 9 | Объединить font atlases | 2 ч | -0.3s startup |
| 10 | Выделить load_png_from_mem | 1 ч | -дублирование |

---

## ТОП-5 АРХИТЕКТУРНЫХ УЛУЧШЕНИЙ

### 1. **Batch рендеринг тайлов**

**Проблема:** ~510 `SDL_RenderCopy` на кадр.

**Решение:** 
- Статический `SDL_Texture` как render target для неподвижных тайлов
- Перерисовывать только при изменении (анимации, хоops)
- Использовать `SDL_RenderGeometry` для batch

**ROI:** Высокий — CPU/GPU bottleneck.

---

### 2. **Централизованная система ресурсов**

**Проблема:** 4 реализации `load_png_from_mem`, дублирование загрузки.

**Решение:**
- `resource_cache.c` с кэшированием по пути
- Автоматическая выгрузка при смене уровня
- Референс-каунтинг для общих ресурсов

**ROI:** Средний — сопровождаемость.

---

### 3. **Разделение update/render фаз**

**Проблема:** `animation_tick()` в render-фазе.

**Решение:**
- Четкое разделение: `game_update()` → `game_render()`
- Фиксированный 20 Hz update, переменный render
- Интерполяция для 60 FPS рендера

**ROI:** Высокий — чистота архитектуры.

---

### 4. **Конечный автомат состояний игры**

**Проблема:** `switch(app_state)` в `main.c` разрастается.

**Решение:**
- `GameState` struct с `init/update/render/shutdown`
- Массив состояний: `states[APP_STATE_COUNT]`
- Диспетчер в `main.c`

**ROI:** Средний — масштабируемость.

---

### 5. **Система событий вместо прямого вызова**

**Проблема:** `player_collect_tile()` вызывает изменение уровня напрямую.

**Решение:**
- `GameEvent` queue: `EVENT_COLLECT`, `EVENT_LEVEL_COMPLETE`
- Обработчики в `main.c`
- Чище для добавления звуков/эффектов

**ROI:** Средний — расширяемость.

---

## ЧТО ХОРОШО СДЕЛАНО

### ✅ Сохранено без изменений

1. **Паритет физических констант:** Все 12 констант из Java (гравитация, скорости, отскок) точно перенесены.

2. **Pixel-perfect коллизии:** Маски 16×16 загружаются корректно (кроме транспонирования).

3. **Трансформации тайлов:** Битовая логика flip/rotate соответствует Java.

4. **Загрузка ресурсов:** Big-endian парсинг `c.java` контейнеров работает корректно.

5. **Camera deadzone:** Логика из `bounce_zero` адаптирована правильно.

6. **HUD и шрифты:** 3 размера шрифтов, бонус-бары — визуально соответствуют.

7. **Exit door анимация:** Clip-based двухполовинная анимация — точная копия Java.

8. **Foreground pass:** Отдельный проход для front tiles + hoop overlay — архитектурно чисто.

9. **Enemy renderer:** 3 типа врагов, спрайты из `/res/ic` — паритет.

10. **Tile animation:** RenderType=3, 50ms tick — работает.

---

## ЗАКЛЮЧЕНИЕ

**Критичные проблемы:** 2 (AIOOBE, транспонирование масок)  
**Высокий приоритет:** 6 (цвет фона, alpha, тайминг, кэширование, шрифты, меню)  
**Средний приоритет:** 4 (демо, звук, анимации, дублирование)  
**Низкий приоритет:** 3 (логирование, кэширование, batch рендеринг)

**Общая оценка:** Код работоспособен, но требует исправлений для достижения pixel-perfect паритета с Java. Основные проблемы — в обработке граничных случаев (AIOOBE) и ориентации масок.

**Рекомендуемый порядок исправлений:**
1. AIOOBE handling (C1)
2. Транспонирование collision masks (C2)
3. Alpha водных тайлов (H2)
4. Компенсация тайминга (H3)
5. Кэширование tileset (H4)

---

## АРХИТЕКТУРА С НУЛЯ: BEST PRACTICES 2026 ДЛЯ PSP/EMBEDDED

Если бы эта игра создавалась с нуля в 2026 году под PSPSDK (или portable/embedded платформы вроде Switch, Steam Deck, Retro handheld), вот оптимальная архитектура:

### 1. **Язык и сборка**

- **Ядро на C99/C11** — предсказуемая память, детерминизм, минимальный runtime overhead.
- **Опционально: Rust для подсистем** — физика, коллизии, state machine (безопасность памяти без GC).
- **CMake + кросс-компиляция** — единый билд под PSP, Linux, Windows, Switch.
- **Статическая линковка** — никаких runtime зависимостей, один бинарник.

### 2. **Память и аллокации**

- **Арена/регион аллокатор** — один `malloc()` при старте на 2-4 MB, дальше pointer bump.
- **Никаких malloc/free в game loop** — всё преаллоцируется при загрузке уровня.
- **Object pools** — враги, частицы, звуки: фиксированные массивы, no fragmentation.
- **VRAM явно** — текстуры грузятся прямо в VRAM через `sceGuTexImage()`, без промежуточных копий.

### 3. **Рендеринг**

- **Прямой GU/Vulkan/Metal** — никакой SDL обёртки, прямой доступ к display list.
- **Batching + instancing** — один draw call на тайлсет, вершины в VBO, инстансы для тайлов.
- **Tilemap в VRAM** — статичный слой как текстура, перерисовка только при изменении.
- **Sprite atlas** — все спрайты (игрок, враги, бонусы) в одной текстуре 512×512.
- **Double/triple buffering** — vsync без tearing, 60 FPS target.

### 4. **Game Loop**

- **Fixed timestep 50 Hz** — физика всегда 20ms, независимо от FPS.
- **Variable render** — рендеринг на макс. FPS (60+), интерполяция между tick.
- **Delta-accumulator**:
  ```c
  accumulator += frame_time;
  while (accumulator >= TICK_MS) {
      game_update(TICK_MS);
      accumulator -= TICK_MS;
  }
  game_render(interpolation);
  ```

### 5. **Компонентная архитектура (ECS)**

- **Data-oriented ECS** — сущности = ID, компоненты = плотные массивы, системы = итераторы.
- **Cache-friendly** — компоненты сортируются по типу, contiguous memory.
- **No virtual calls** — системы обрабатывают массивы напрямую.
- **Пример**: `PhysicsSystem` iterates `Position[] + Velocity[] + CollisionMask[]`.

### 6. **Ресурсы**

- **Атласы при сборке** — текстуры упакованы в `.pam` (PSP atlas format) с метаданными.
- **Асинхронная загрузка** — I/O в отдельном потоке, загрузка без фризов.
- **Референс-каунтинг** — ресурсы выгружаются, когда `refcount == 0`.
- **Hot reload** — перезагрузка текстур по хоткею для dev-итераций.

### 7. **Аудио**

- **Mixer на 32 канала** — софт-микшер с ресемплингом, приоритеты, fade-in/out.
- **VAG/ADPCM** — сжатие 4:1 для экономии памяти.
- **3D позиционирование** — панорамирование по X для врагов/эффектов.

### 8. **Ввод**

- **Input abstraction layer** — единый API для PSP, keyboard, gamepad, touch.
- **Input buffering** — очередь на 3-5 кадров для responsive controls.
- **Rebindable keys** — конфигурируемые управления в меню.

### 9. **Состояния и скрипты**

- **State machine с стеком** — `push(Menu)`, `pop()` → возврат в игру.
- **Lua/JSON для уровней** — данные уровней в Lua-скриптах, hot reload.
- **Scriptable events** — `on_level_complete`, `on_player_death` вызывают Lua-хуки.

### 10. **Тестирование и отладка**

- **Deterministic replay** — лог семян RNG + ввода, воспроизведение багов.
- **Frame profiler** — встроенный profiler (CPU/GPU/ms) по хоткею.
- **Assert + санитайзеры** — UB sanitizer в debug, assert в release.
- **Headless mode** — запуск без рендера для автотестов физики/коллизий.

### 11. **Платформенная абстракция**

- **PAL (Platform Abstraction Layer)** — единый API для FS, threading, time, audio.
- **Условная компиляция** — `#ifdef PLATFORM_PSP`, `#ifdef PLATFORM_SWITCH`.
- **Единая кодовая база** — 95% кода shared между платформами.

### 12. **Инструменты**

- **Tiled для уровней** — экспорт в JSON/Lua, парсер в движке.
- **TexturePacker** — атласы + metadata (pivot, anchor, nine-patch).
- **FMOD/Wwise** — аудио-микшер и события, если бюджет позволяет.
- **CI/CD** — автосборка под все платформы при push.

### 13. **Оптимизации под PSP**

- **Выравнивание 64 байта** — текстуры и вершины выровнены под cache line.
- **DMA transfer** — загрузка текстур через DMA, не блокируя CPU.
- **Fixed-point математика** — опционально для CPU-физики (быстрее на MIPS без FPU).
- **Prefetch cache** — данные предзагружаются в кэш перед использованием.

### 14. **Структура проекта**

```
/game
  /core        — ECS, memory, math, utils
  /gameplay    — player, enemies, tiles, physics
  /render      — GU/Vulkan backend, batching, atlases
  /audio       — mixer, VAG decoder, 3D sound
  /input       — controller abstraction
  /scripting   — Lua VM, bindings
  /platform    — PSP, Switch, Linux backends
/tools
  /atlas       — texture packer
  /level       — JSON→binary compiler
  /audio       — VAG encoder
```

### 15. **Итоговая архитектура**

```
┌─────────────────────────────────────────────────┐
│                  Game Loop                      │
│  (fixed 50Hz update + variable render @ 60Hz)   │
├─────────────────────────────────────────────────┤
│              ECS Systems (C/Rust)               │
│  Physics │ Collision │ AI │ Particles │ Audio  │
├─────────────────────────────────────────────────┤
│           Platform Abstraction Layer            │
│  (memory, FS, threading, time, audio, input)    │
├─────────────────────────────────────────────────┤
│            Direct GPU (GU/Vulkan)               │
│  (batching, instancing, VRAM management)        │
└─────────────────────────────────────────────────┘
```

**Ключевые принципы:**
1. **Data-oriented design** — память contiguous, cache-friendly.
2. **Zero allocations in loop** — всё преаллоцировано.
3. **Direct hardware access** — минимум слоёв абстракции.
4. **Deterministic simulation** — воспроизводимые баги, replay.
5. **Shared codebase** — 95% кода portable между платформами.

**Ожидаемый результат:**
- 60 FPS стабильно на PSP
- <1s загрузка уровня
- <50 MB память
- 1-2 дня на порт на новую платформу
