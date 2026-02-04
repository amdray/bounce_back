# Шаг 2: Тайловый движок и рендер уровня

## Статус: Не начат

**Предыдущий шаг:** ✅ Шаг 1 - SDL2 инициализация + загрузка ball sprite из `/res/b`

## Цель шага

Загрузить и отрисовать статичную тайловую карту первого уровня (level 0) на экране PSP без физики, коллизий и камеры.

## Что нужно реализовать

### 2.1. Парсер уровня (`level_loader.c/h`)

**Входные данные:** `/res/lf` (контейнер из 44 chunk'ов, по 2 на уровень)

**Формат данных уровня** (из DEOBFUSCATION.md, строки 483-527):

```c
// Metadata chunk: chunk[2*levelIndex]
typedef struct {
    uint8_t theme_id;      // Индекс темы (0, 1, 2)
    uint8_t spawn_y;       // Y координата спавна в тайлах
    uint8_t spawn_x;       // X координата спавна в тайлах
    uint8_t ball_type;     // 0=маленький, другое=большой
    uint8_t ar;            // Дополнительный параметр
    uint8_t D;             // Дополнительный параметр
    uint8_t enemy_count;   // Количество врагов
    // Далее: enemy_count записей по 9 байт (пропустить на этом шаге)
} LevelMetadata;

// TileMap chunk: chunk[2*levelIndex + 1]
// u8 height
// u8 width
// height * width байт тайлов (построчно)
typedef struct {
    uint8_t height;        // Высота карты в тайлах
    uint8_t width;         // Ширина карты в тайлах
    uint8_t* tiles;        // Массив [height][width] байт
} TileMap;
```

**Семантика байта тайла:**
```c
tileId = tileByte & 0x7F;      // ID тайла (0-127)
bgFill = (tileByte & 0x80);    // Флаг заливки фона перед рисованием
```

**Функции для реализации:**
```c
typedef struct {
    uint8_t theme_id;
    uint8_t spawn_x;
    uint8_t spawn_y;
    uint8_t ball_type;
    uint8_t width;
    uint8_t height;
    uint8_t* tile_map;     // [height * width] байт
} Level;

Level* level_load(const char* lf_path, int level_index);
void level_free(Level* level);
uint8_t level_get_tile(Level* level, int tile_x, int tile_y);
```

### 2.2. Парсер метаданных тайлов (`tile_metadata.c/h`)

**Входные данные:** `/res/tf` (контейнер из 2 chunk'ов с метаданными)

**Формат данных** (из DEOBFUSCATION.md, строки 264-291):

```c
// Chunk 0: основные свойства тайлов (127 тайлов)
// Для каждого tileId:
//   u8 renderType (v)      // 0=пусто, 1=картинка, 3=анимация
//   u8 imageIndex (T)      // Индекс спрайта в if0/if1/if2
//   u8 transform (b)       // Битовое поле rotate/flip
//   u8 collisionType (l)   // 0=нет, 1=mask, 2=solid, 3=mask+transform
//   u8 aux (af)            // Для renderType=3: индекс анимации

// Chunk 1: анимации (пропустить на этом шаге)

typedef struct {
    uint8_t render_type;      // 0=skip, 1=static, 3=animated
    uint8_t image_index;      // Индекс в tileset
    uint8_t transform;        // rotate(0-3) + flip(0x4/0x8)
    uint8_t collision_type;   // Пропустить на этом шаге
    uint8_t aux;              // Пропустить на этом шаге
} TileMetadata;

TileMetadata* tilemetadata_load(const char* tf_path);
void tilemetadata_free(TileMetadata* meta);
```

### 2.3. Загрузчик тайлсета (`tileset_loader.c/h`)

**Входные данные:** 
- `/res/if0` - базовый тайлсет (104 PNG изображения)
- `/res/if1` или `/res/if2` - тематический тайлсет (7 PNG, дополняет/заменяет if0)

**Формат:** Бинарный контейнер PNG изображений (проверено `dump_res_container_signatures.py`)

```c
typedef struct {
    SDL_Texture** textures;   // Массив SDL_Texture*
    int count;                // Количество текстур
} Tileset;

Tileset* tileset_load(SDL_Renderer* renderer, 
                      const char* if0_path,
                      const char* theme_path);  // if1 или if2
void tileset_free(Tileset* tileset);
SDL_Texture* tileset_get(Tileset* tileset, int image_index);
```

**Логика слияния** (из DEOBFUSCATION.md, таблица /res/):
- Сначала грузим все 104 изображения из `/res/if0`
- Затем перезаписываем последние 7 изображениями из `/res/if{theme}`
- Если theme файл отсутствует - используем только if0

### 2.4. Рендер тайловой карты (`level_renderer.c/h`)

**Задача:** Отрисовать видимую область уровня на экране PSP (480×272)

```c
typedef struct {
    Level* level;
    TileMetadata* tile_meta;
    Tileset* tileset;
} LevelRenderer;

LevelRenderer* renderer_create(Level* level, 
                                TileMetadata* meta,
                                Tileset* tileset);
void renderer_free(LevelRenderer* renderer);

// Рендер без камеры (статичный, top-left origin)
void renderer_draw_simple(LevelRenderer* renderer, 
                          SDL_Renderer* sdl_renderer);
```

**Алгоритм рендера:**
```c
void renderer_draw_simple(LevelRenderer* r, SDL_Renderer* sdl) {
    int tile_size = 16;  // Фиксированный размер тайла
    
    // Вычислить видимую область (без камеры - просто 0,0)
    int start_tile_x = 0;
    int start_tile_y = 0;
    int end_tile_x = (480 / tile_size) + 1;  // ~30 тайлов
    int end_tile_y = (272 / tile_size) + 1;  // ~17 тайлов
    
    for (int ty = start_tile_y; ty < end_tile_y; ty++) {
        for (int tx = start_tile_x; tx < end_tile_x; tx++) {
            // Проверка границ карты
            if (tx >= r->level->width || ty >= r->level->height) continue;
            
            uint8_t tile_byte = level_get_tile(r->level, tx, ty);
            uint8_t tile_id = tile_byte & 0x7F;
            bool bg_fill = (tile_byte & 0x80);
            
            TileMetadata meta = r->tile_meta[tile_id];
            
            // Пропустить пустые тайлы
            if (meta.render_type == 0) continue;
            
            // Для анимаций (render_type=3) пока пропускаем
            if (meta.render_type == 3) continue;
            
            // Рендер статичного тайла (render_type=1)
            SDL_Texture* tex = tileset_get(r->tileset, meta.image_index);
            if (!tex) continue;
            
            SDL_Rect dest = {
                tx * tile_size,
                ty * tile_size,
                tile_size,
                tile_size
            };
            
            // TODO: применить transform (rotate/flip)
            SDL_RenderCopy(sdl, tex, NULL, &dest);
        }
    }
}
```

### 2.5. Обновление main.c

**Изменения:**
```c
// Вместо загрузки ball sprite:
Level* level = level_load("res/lf", 0);  // Уровень 0
TileMetadata* tile_meta = tilemetadata_load("res/tf");

char theme_path[32];
snprintf(theme_path, sizeof(theme_path), "res/if%d", level->theme_id);
Tileset* tileset = tileset_load(renderer, "res/if0", theme_path);

LevelRenderer* level_renderer = renderer_create(level, tile_meta, tileset);

// Main loop:
while (running) {
    // Events...
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    renderer_draw_simple(level_renderer, renderer);
    
    SDL_RenderPresent(renderer);
}
```

## Критерий успеха

При запуске на PSP должна быть видна тайловая карта первого уровня:
- Статичные тайлы отрисованы корректно
- Видимая область ~30×17 тайлов (480÷16 × 272÷16)
- Никакой физики, коллизий, камеры - только статичный рендер

## Проверка

1. Скомпилировать и запустить на PSP
2. Должны быть видны платформы, блоки, декорации уровня
3. debug.log должен показать:
   ```
   Level 0 loaded: width=X height=Y theme=0
   TileMetadata loaded: 127 tiles
   Tileset loaded: 104 base + 7 theme textures
   Rendering...
   ```

## Следующий шаг (Step 3)

После завершения шага 2:
- **Шаг 3:** Камера и viewport (плавное следование за точкой, deadzone, clamp/wrap)
- Это позволит видеть уровень с правильной позиции и прокручивать его

## Референсы

- `DEOBFUSCATION.md` строки 483-527 (формат /res/lf)
- `DEOBFUSCATION.md` строки 264-291 (класс Level g.java)
- `DEOBFUSCATION.md` строки 580-619 (формат /res/tf)
- `bounce_back/generate_maps_from_lf.py` - референсная реализация парсера
- `bounce_back/dump_res_container_signatures.py` - проверка форматов PNG
