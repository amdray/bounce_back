# Инженерный аудит — Bounce Back C/SDL2 PSP

Аудит C-реализации (`src/`) относительно оригинального Java J2ME кода (`original_code/bounce_back_s60.jar.src/`).

---

## 1. Приоритизированные находки

### CRITICAL


---

### HIGH

#### H-04 · `res/ic` загружается 4 раза независимо
| | |
|--|--|
| **Модули** | `foreground_pass.c:20`, `exit_door.c:57`, `enemy_renderer.c:42`, `hud.c:79` |
| **Проблема** | Каждый модуль делает `resource_load("res/ic")` → получает свою копию контейнера (**4× память** на один и тот же ресурс). Плюс каждый имеет свой cleanup. |
| **Java-референс** | `h.java:i()` — одна загрузка `ic[]` массива, общий доступ для door/enemies/hud/hoops. |
| **Исправление** | Ввести shared_ic модуль или передавать контейнер в init-функции. |
| **Риск** | Средний — нужно поменять init/shutdown сигнатуры у 4 модулей. |


#### H-06 · Per-frame `calloc` в `player_update()`
| | |
|--|--|
| **Файл** | `src/player.c:677–679` |
| **Проблема** | `obj_processed = calloc(level->objects.count, ...)` на **каждый тик** (50 раз/сек). На PSP malloc/free дороги. Макс. объектов на уровень: ~10. |
| **Java-референс** | `a.java` использует объектные флаги прямо в массиве (нет аллокации на тик). |
| **Исправление** | Статический/стековый массив `bool obj_processed[MAX_OBJECTS];` (MAX_OBJECTS ≤ 32) или поле в Player. |
| **Риск** | Минимальный. |

---

### MEDIUM


#### M-06 · `hoop_tiles[128]` — фиксированный массив без проверки переполнения
| | |
|--|--|
| **Файл** | `src/foreground_pass.c:91` (внутри `foreground_pass_build`) |
| **Проблема** | `out->hoop_count < 128` — при >128 hoop-тайлов на уровне лишние молча игнорируются. На текущих 21 уровнях макс. ~20, но это implicit limit. |
| **Исправление** | `#define MAX_HOOP_TILES 128` + assert или динамический массив. |

#### M-07 · `main.c` init — ручные cleanup-цепочки по 12+ строк
| | |
|--|--|
| **Файл** | `src/main.c:230–500` |
| **Проблема** | Каждый блок init-failure вручную перечисляет все предыдущие free в обратном порядке. 9 таких блоков, каждый на ~10 строк `free`/`shutdown`. Если добавить новый ресурс — нужно обновить все предыдущие блоки. |
| **Исправление** | `goto cleanup` паттерн с метками или массив destroyers. |
| **Риск** | Средний — рефакторинг чувствительной зоны. |


---

### LOW

#### L-01 · Demo replay (`d.java`) не реализован
| | |
|--|--|
| **Java-референс** | `d.java` — читает `res/r`, воспроизводит записанный ввод по кадрам. |
| **В C** | Нет аналога. Осознанный пропуск, но не задокументирован. |

#### L-02 · Меню без fade-переходов
| | |
|--|--|
| **Java-референс** | `i.java` — `fadeimg`, `fade = true`, alpha-blend переходы между экранами. |
| **В C** | Мгновенные переключения. Визуальная разница, не влияет на механику. |


---

## 3. Подозрительные fallback-пути

| Паттерн | Файлы | Замечание |
|---------|-------|-----------|
| `!s_controller` → silent no-input | input.c:31 | `input_update()` выходит без обновления полей → все кнопки = 0. Нет предупреждения. |
| `img == NULL` в player sprite loading | player.c:565–578 | `continue` при ошибке — пропускает спрайт. Позже `ball_sprites[i] == NULL` вызовет чёрный квадрат или пропуск рендера, но не краш. |

---

## 4. Кластеры дублирования

### Кластер A: Константы дисплея
**Файлы:** camera.h, level_renderer.c, exit_door.c, enemy_renderer.c, main.c  
**Дубль:** `#define SCREEN_WIDTH 480`, `#define SCREEN_HEIGHT 272`, `#define TILE_SIZE 16`  
**Решение:** Единый `display.h`

### Кластер B: Загрузка res/ic
**Файлы:** foreground_pass.c, exit_door.c, enemy_renderer.c, hud.c  
**Дубль:** `resource_load("res/ic")` + fallback + отдельный `resource_free()`  
**Решение:** shared_ic модуль с refcount или передача указателя

### Кластер C: Transform функция
**Файлы:** level_loader.c (`apply_transform_16`), collision.c (`apply_transform`)  
**Дубль:** Идентичны побитно  
**Решение:** общий хедер

### Кластер D: Init-failure cleanup chains
**Файл:** main.c:230–500  
**Дубль:** 9 блоков с возрастающим количеством `free()` вызовов  
**Решение:** `goto cleanup` с метками

---

## 5. Топ-10 Quick Wins

| # | Действие | Сложность | Эффект |
|---|----------|-----------|--------|
| 1 | Исправить оси маски в level_loader.c (C-01) | 5 мин | Правильная коллизия для всех масок |
| 2 | Вынести `apply_transform_16()` в общую utility-функцию (H-03) | 10 мин | Минус одна копия transform-логики |
| 3 | Удалить дубль `ITEM_COUNT` (M-02) | 1 мин | Нет compiler warnings |
| 4 | Удалить дубль `ITEM_COUNT` (M-02) | 1 мин | Нет compiler warnings |
| 5 | `static bool prev_cheat` → поле Player (M-03) | 2 мин | Корректный reset при смене уровня |
| 6 | `obj_processed` на стеке (H-06) | 3 мин | 0 heap alloc на тик |
| 7 | Удалить `apply_transform()` из collision.c (H-03) | 5 мин | Нет дубля |
| 8 | Единый `display.h` (H-05) | 10 мин | Одна точка для констант |
| 9 | Удалить `writeback_atlas()` + forward decl (M-04/M-05) | 2 мин | Нет мёртвого кода |
| 10 | Keyboard fallback в input.c (H-07) | 15 мин | PC-отладка без геймпада |

---

## 6. Топ-5 архитектурных улучшений

### A-01 · Shared resource cache для res/ic
Ввести `ic_cache.c` с `ic_cache_init(renderer)` / `ic_cache_get(index)` / `ic_cache_shutdown()`. Четыре модуля (foreground_pass, exit_door, enemy_renderer, hud) вызывают `ic_cache_get()` вместо собственной загрузки. Экономит 3× memory + 3× load time.

### A-02 · Единый collision API без транзитных параметров
Сейчас `player.c` → `collision.c` → `level_loader.c`, при этом `collision.c` — thin wrapper, который выбрасывает 2 из 3 параметров. Удалить collision.c как слой. Player напрямую вызывает `level_test_collision*()`. Один transform-хелпер в общем хедере.

### A-03 · `goto cleanup` в main.c init
Заменить 9 копипастных cleanup-блоков на единую cleanup-цепочку:
```c
if (!level) goto cleanup_menu;
...
if (!player) goto cleanup_renderer;
...
cleanup_renderer: renderer_free(level_renderer);
cleanup_bg:      bg_layer_free(bg_layer);
...
```
Сокращает ~90 строк до ~20.

### A-04 · Плоское 1D хранение масок вместо `bool**`/`bool***`
Java хранит маски как `boolean[][]` (row-major). C использует `bool***` — тройной указатель, ~100+ аллокаций при загрузке. Перейти на `bool*` (row-major `mask[y * w + x]`) — один `malloc` на маску, лучше cache locality, проще free.

### A-05 · Config header для game constants
Все Magic Numbers (SCREEN_WIDTH, TILE_SIZE, LEVEL_COUNT, MAX_FALL_SPEED, JUMP_NORMAL, ACCEL_NORMAL, SPLASH_PHASE_MS, HUD_HEIGHT, и т.д.) в одном `game_config.h`. Упрощает тюнинг и портирование на другие разрешения.

---

## 7. Что уже хорошо

- **Точный порт физики игрока**: `player.c` (2200 строк) — детальная реализация вертикального/горизонтального collision loop с pixel-stepping, bounce state machine, power-up таймерами, slope pass-through. Логика прямо следует `a.java` со ссылками на строки.
- **Корректный OTT синтезатор**: `sound.c` — полный парсер Nokia OTT ringtone format + wavetable synth @ 44.1kHz с ADSR envelope. Работает на PSP без внешних библиотек.
- **Big-endian resource loader**: `resource_loader.c` правильно обрабатывает Nokia big-endian контейнер.
- **Hoop overlay**: `foreground_pass.c` — аккуратный порт h.java overlay loop с корректным маппингом Nokia DirectGraphics transform → SDL2 flip/rotate.
- **Exit door animation**: `exit_door.c` — точная реализация clip-based анимации двери с dual-half scroll.
- **Level loading parity**: `level_loader.c` — hoops_remaining count точно повторяет Java h.java:356 (tiles 93/94/97/101). Object loading правильно обрабатывает координаты и velocity inversion.
- **Tile metadata + animation**: Корректный парсинг tf формата с split_index для двух потоков данных.
- **Чистые edge-detect в input**: `input.c` — proper previous-frame tracking для pressed-events без debounce артефактов.
- **Extensive debug logging**: main.c логирует время каждой фазы загрузки в debug.log — полезно для PSP profiling.

---

## 8. Как бы стоило спроектировать архитектуру — best practice 2026 для PSPSDK / embedded

Если бы я проектировал ту же игру (2D тайловый платформер, ~25 уровней, pixel-perfect коллизия,
custom ресурсный формат, OTT-синтезатор, контроллерный ввод, 480×272 при 50 FPS) с нуля
для PSP (MIPS R4000, 32 МБ RAM, нет MMU protection) или аналогичной embedded-платформы —
вот референсная архитектура.

### 8.1 · Принцип: zero-allocation game loop

На embedded главный враг — фрагментация хипа и непредсказуемая латентность `malloc`.
Правило: **после загрузки уровня ни один `malloc`/`calloc`/`realloc` не вызывается до конца уровня**.

Реализация:
- **Arena allocator** (bump allocator): один `malloc(ARENA_SIZE)` при старте, всё внутриуровневое
  выделение — `arena_alloc(&level_arena, size, align)` с O(1) cost. Reset при смене уровня:
  `arena_reset(&level_arena)`.
- **Frame scratch allocator**: маленькая арена (~4 КБ) для per-frame временных данных
  (аналог текущего `calloc(obj_count)` для obj_processed). Reset в начале каждого тика.
  Нулевой overhead, нулевая фрагментация.
- Два пула: `PERSISTENT` (живёт всё приложение: шрифты, звуки, контейнер ic) и
  `LEVEL` (живёт один уровень: тайлмап, маски, объекты, тайлсет). При смене уровня
  LEVEL-арена ресетится целиком — не нужны индивидуальные `free()`.

### 8.2 · Единый ресурсный кеш с refcount

Вместо четырёх независимых `resource_load("res/ic")`:

```c
typedef struct {
    const char* path;
    ResourceContainer* rc;
    int refcount;
} CacheEntry;

static CacheEntry g_cache[MAX_CACHED_RESOURCES];

ResourceContainer* res_cache_acquire(const char* path);  // ++refcount, load if first
void               res_cache_release(const char* path);  // --refcount, free if zero
```

Каждый модуль делает `acquire` в init и `release` в shutdown. Один экземпляр в памяти.
На PSP с 32 МБ это критично — ic-контейнер занимает ~200 КБ, 4 копии = 800 КБ впустую.

### 8.3 · Data-oriented tile engine

Текущий подход: `bool***` (triple pointer) для масок, `bool**` для конкретного тайла,
`bool*` для столбца — ~100 аллокаций на загрузку tf, плохая cache locality.

Лучше:
```c
typedef struct {
    uint8_t  render_type[MAX_TILES];   // SoA
    uint8_t  image_index[MAX_TILES];
    uint8_t  transform[MAX_TILES];
    uint8_t  collision_type[MAX_TILES];
    // Все маски в одном плоском буфере row-major
    uint8_t  mask_data[MAX_TILES * TILE_W * TILE_H];  // 127 * 16 * 16 = 32 КБ
    // Или bitpacked: 127 * 16 * 2 bytes = 4 КБ
} TileDB;
```

Struct-of-Arrays (SoA) вместо Array-of-Structs: при итерации коллизий нужен только
`collision_type[]` + `mask_data[]` — два линейных массива, попадающих в D-cache PSP целиком.
Triple-pointer chase на каждый пиксель — cache miss гарантирован.

Маски в bitpacked формате (`uint16_t mask_rows[TILE_H]`): 16 бит на строку × 16 строк =
32 байта на тайл. Вся таблица масок: 127 × 32 = 4 КБ. Коллизия одного пикселя:
`(mask_rows[y] >> (15 - x)) & 1` — одна операция вместо pointer chase.

### 8.4 · Модульная init/shutdown через registry

Вместо 9 ручных cleanup-цепочек в main.c:

```c
typedef struct { const char* name; int (*init)(SDL_Renderer*); void (*shutdown)(void); } Module;

static Module g_modules[] = {
    { "sound",      sound_init_wrap,     sound_shutdown     },
    { "menu",       menu_init,           menu_shutdown      },
    { "hud",        hud_init,            hud_shutdown        },
    { "enemy",      enemy_renderer_init, enemy_renderer_shutdown },
    { "exit_door",  exit_door_init_wrap, exit_door_renderer_shutdown },
    { "foreground", fg_init_wrap,        foreground_pass_shutdown },
};
#define MODULE_COUNT (sizeof(g_modules) / sizeof(g_modules[0]))

static int g_init_count = 0;

int modules_init_all(SDL_Renderer* r) {
    for (int i = 0; i < MODULE_COUNT; i++) {
        if (g_modules[i].init(r) != 0) {
            modules_shutdown_all();  // cleanup всех предыдущих
            return -1;
        }
        g_init_count = i + 1;
    }
    return 0;
}

void modules_shutdown_all(void) {
    for (int i = g_init_count - 1; i >= 0; i--)
        g_modules[i].shutdown();
    g_init_count = 0;
}
```

Добавление нового модуля — одна строка в массиве. Невозможно забыть cleanup.

### 8.5 · Fixed-point physics вместо int/10

Текущий код: скорости хранятся как `int`, делятся на 10 для получения пикселей
(`y_pixels = abs(j) / 10`). Это теряет точность и создаёт нелинейное поведение.

На embedded стандарт — Q8.8 или Q12.4 fixed-point:

```c
typedef int32_t fixed_t;  // Q12.4: 12 бит целая + 4 бит дробная
#define FP_SHIFT 4
#define FP_ONE   (1 << FP_SHIFT)
#define FP_HALF  (FP_ONE >> 1)
#define FP_TO_INT(x) ((x) >> FP_SHIFT)
#define INT_TO_FP(x) ((x) << FP_SHIFT)
```

Но: если цель — pixel-perfect parity с Java, то оригинальные int-арифметики правильнее
сохранить as-is. Fixed-point — для новых проектов, не для реверс-портов.

### 8.6 · Collision: spatial hash вместо brute-force

Текущий подход: `level_test_collision_collect()` итерирует все тайлы в bounding rect
игрока (OK для 16px тайлов, макс 6 тайлов), но объектный check делает линейный проход
по **всем** объектам каждый тик.

Для игры с ≤10 объектами на уровень это не проблема. Но best practice:
- Spatial hash grid (cell = 32×32 px) для объектов. Insert O(1), query O(1).
  На PSP с маленькими уровнями даже не нужен — простой массив + AABB test достаточен.
- Важнее: **вынести объектный collision из вложенного pixel-step loop**.
  Сейчас объектное пересечение проверяется внутри `level_test_collision_collect()`
  на каждый пиксельный шаг (до 14 вызовов/тик × 2 оси). Объекты не двигаются
  между шагами — достаточно одной проверки до начала stepping, с кешированием результата.

### 8.7 · State machine через таблицу переходов

Текущий main loop — цепочка `if (app_state == ...)`, копипаста render block в каждом
state (bg_layer_draw + renderer_draw + overlay). Лучше:

```c
typedef struct {
    AppState    (*update)(void* ctx, Input* input);
    void        (*render)(void* ctx, SDL_Renderer* r);
} StateVTable;

static StateVTable g_states[APP_STATE_COUNT] = {
    [APP_STATE_MENU]           = { menu_update,          menu_render          },
    [APP_STATE_GAME]           = { game_update,          game_render          },
    [APP_STATE_LEVEL_COMPLETE] = { level_complete_update, level_complete_render },
    [APP_STATE_GAME_OVER]      = { game_over_update,     game_over_render     },
    [APP_STATE_CONGRATULATIONS]= { congrats_update,      congrats_render      },
};

// Main loop body:
AppState next = g_states[app_state].update(ctx, &input);
if (next != app_state) app_state = next;
g_states[app_state].render(ctx, renderer);
```

Добавление нового состояния — строка в таблице + две функции. Нулевой overhead (indirect call).

### 8.8 · Render pipeline: command buffer

Текущий подход: каждый модуль (`bg_layer_draw`, `renderer_draw`, `enemy_renderer_draw`,
`player_render`, `foreground_pass_draw`, `hud_render`) напрямую вызывает
`SDL_RenderCopy()` по 50–200 раз за кадр. На PSP каждый `SDL_RenderCopy` — это
sceGuDrawArray() с state change overhead.

Best practice: **собрать все draw commands в массив, отсортировать по текстуре,
отправить батчами**:

```c
typedef struct {
    SDL_Texture* tex;
    SDL_Rect     src, dst;
    uint8_t      transform;
    int16_t      z_order;       // bg=0, tiles=10, enemies=20, player=30, fg=40, hud=50
} DrawCmd;

static DrawCmd g_draw_buf[MAX_DRAW_CMDS];  // ~2048 достаточно
static int g_draw_count;

void draw_submit(SDL_Texture* tex, SDL_Rect* src, SDL_Rect* dst, uint8_t tf, int z);
void draw_flush(SDL_Renderer* r);  // sort by z then by tex pointer, batch RenderCopy
```

Это минимизирует texture state switches на GPU. На PSP GE (Graphics Engine)
texture switch — дорогая операция (~50 циклов), при 400 тайлах на экране экономия
заметна.

### 8.9 · Конфигурация через единый header

```c
// game_config.h
#define SCREEN_W          480
#define SCREEN_H          272
#define TILE_W            16
#define TILE_H            16
#define MAX_TILES         128
#define MAX_LEVELS        21
#define MAX_OBJECTS        32
#define MAX_HOOP_TILES    128
#define TICK_MS            50
#define HUD_HEIGHT         21
#define ARENA_LEVEL_SIZE  (256 * 1024)   // 256 КБ per level
#define ARENA_FRAME_SIZE  (4 * 1024)     // 4 КБ per frame scratch

// Physics (original Java values, do not change for parity)
#define PHYS_GRAVITY       9
#define PHYS_MAX_FALL     80
#define PHYS_JUMP_NORMAL -125
#define PHYS_ACCEL        18
```

Одна точка правды. Портирование на другое разрешение = изменить 2 строки.

### 8.10 · Резюме: идеальная файловая структура

```
src/
  game_config.h          — все константы
  arena.c/h              — bump allocator (persistent + level + frame)
  res_cache.c/h          — ресурсный кеш с refcount
  tile_db.c/h            — SoA tile database + bitpacked masks
  level.c/h              — загрузка/free уровня (через arena)
  player.c/h             — физика + collision response (без аллокаций)
  objects.c/h            — enemy tick + collision (выделен из level_loader)
  collision.c/h          — pixel-perfect test (один transform helper)
  camera.c/h             — deadzone camera
  input.c/h              — gamepad + keyboard fallback
  render_cmd.c/h         — draw command buffer + batch flush
  bg_layer.c/h           — фон
  tile_renderer.c/h      — main + foreground tile pass
  sprite_renderer.c/h    — enemies + door + player + hoops (общий)
  hud.c/h                — UI overlay
  menu.c/h               — state machine для меню
  sound.c/h              — OTT synth
  font.c/h               — шрифты (merged atlas + hud_font)
  modules.c/h            — init/shutdown registry
  main.c                 — 50 строк: init modules → run loop → shutdown
```

Итого: ~20 файлов вместо ~28, main.c сокращён с 900 до ~50 строк,
ноль heap-аллокаций в game loop, единая точка для констант,
батч-рендеринг для GPU PSP, модульный init без копипасты cleanup.
