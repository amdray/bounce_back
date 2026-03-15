# Шаг 4: Pixel-perfect коллизии с тайлами

## Статус: Не начат

**Предыдущий шаг:** ✅ Шаг 3 - Персонаж (мяч) с базовой физикой (гравитация + падение)

## Цель

Реализовать pixel-perfect коллизии мяча с тайлами согласно g.java:315-430 (метод `a(int,int,int,int,boolean[][],boolean)`).

## Файлы для создания

- [ ] `src/collision_masks.h` - структура для хранения масок
- [ ] `src/collision_masks.c` - загрузка масок из res/tf
- [ ] `src/collision.h` - функция collision_test
- [ ] `src/collision.c` - реализация проверки коллизий

## Ключевые факты из оригинального кода

### Хранение масок (g.java:207, 226-234)

**ВАЖНО из DEOBFUSCATION.md и g.java:**

- Маски хранятся ОТДЕЛЬНО от TileMetadata (g.java:207 `this.s = new boolean[this.h][][]`)
- Маска в файле ТОЛЬКО при `collisionType==1` (g.java:226-230)
- readBoolean() = 1 байт/пиксель (256 байт на маску 16×16)
- При `collisionType==3` создается alias при загрузке: `s[tileId] = s[aux]` (g.java:234)

**Реализация:**

Создать collision_masks.c с функцией загрузки (аналог g.java:213-235):

```c
// collision_masks.h
typedef struct {
    bool*** masks;      // [tileId][y][x] - аналог g.s (g.java:207). ВАЖНО: индекс = mask[y][x] (см. g.java)
    uint8_t tile_count;
    uint8_t mask_w;     // 16 после нормализации (g.java:167)
    uint8_t mask_h;
} CollisionMasks;

CollisionMasks* collision_masks_load(const char* tf_path);
void collision_masks_free(CollisionMasks* masks);
```

**Загрузка в main.c:**
```c
// После tilemetadata_load
CollisionMasks* collision_masks = collision_masks_load("res/tf");
if (!collision_masks) {
    // error handling
}

// При вызове player_update передать masks
player_update(player, level, tile_meta, collision_masks);

// При выходе
collision_masks_free(collision_masks);
```

**Обновить player.h:**
```c
void player_update(Player* p, Level* level, TileMetadata* tile_meta, 
                   CollisionMasks* masks);
```

Формат чтения см. DEOBFUSCATION.md § "Формат метаданных тайлов (res/tf)".
Примечание по `/res/tf`: сразу после 14-байтового заголовка идёт `u8 animCount (U)`; отдельного “reserved/global byte” между header и `animCount` нет (см. `g.java`).

### Проверка коллизии (g.java:315-430)

Реализовать аналог `a(int,int,int,int,boolean[][],boolean)`:

**Ключевые шаги:**
1. Найти затронутые тайлы: `start/end = pos / 16` (g.java:323-326)
2. Для каждого тайла:
   - collisionType==0 → skip (g.java:357)
   - collisionType==2 → return true (g.java:361)
   - collisionType==1 или 3 → pixel-perfect (g.java:363-428)
3. Область пересечения rect ∩ tile (g.java:365-393)
4. Для каждого пикселя:
   - Локальные координаты в тайле (g.java:379-382)
   - Если type==3: apply_transform (g.java:395-419)
   - Проверить маску `s[tileId][y][x]` (g.java:422)
   - Проверить player_mask если есть (g.java:424)

**Подпись:**
```c
bool collision_test(Level* level, TileMetadata* tile_meta,
                    CollisionMasks* masks,  // Отдельно от TileMetadata!
                    int rect_x, int rect_y, int rect_w, int rect_h,
                    bool* player_mask);
```

### Transform для collisionType==3 (g.java:395-419)

**Порядок:** flip сначала (g.java:401-404), потом rotation (g.java:405-419)

```c
// g.java:401-404
if ((b2 & 0x8) != 0) i10 = k - i10;  // flip vertical → X
if ((b2 & 0x4) != 0) i11 = m - i11;  // flip horizontal → Y

// g.java:405-419 - rotation по младшим 2 битам
```

См. COLLISION_CONTRACT.md § 4.3 для полной таблицы.

### Интеграция в player.c

Обновить подпись и вызовы `player_update()`:

**player.h:**
```c
void player_update(Player* p, Level* level, TileMetadata* tile_meta, 
                   CollisionMasks* masks);
```

**player.c - попиксельное движение с коллизиями:**
```c
void player_update(Player* p, Level* level, TileMetadata* tile_meta,
                   CollisionMasks* masks) {
    // Применить гравитацию
    p->y_speed += GRAVITY_NORMAL;
    if (p->y_speed > MAX_FALL_SPEED) p->y_speed = MAX_FALL_SPEED;
    
    // Попиксельно с проверкой
    int step = (p->y_speed > 0) ? 1 : -1;
    for (int i = 0; i < abs(p->y_speed); i++) {
        int test_y = p->y_pos + step;
        int rect_x = p->x_pos - p->half_width;
        int rect_y = test_y - p->half_height;
        
        if (collision_test(level, tile_meta, masks,
                          rect_x, rect_y, p->sprite_width, p->sprite_height, NULL)) {
            p->y_speed = 0;
            p->is_grounded = (step > 0);
            break;
        }
        p->y_pos = test_y;
    }
}
```

**main.c:**
```c
// В main() после загрузки tile_meta
CollisionMasks* collision_masks = collision_masks_load("res/tf");

// В игровом цикле
player_update(player, level, tile_meta, collision_masks);

// При выходе
collision_masks_free(masks);
```

## Пошаговая интеграция

### 1. Создать новые файлы

- [ ] `src/collision_masks.h` - структура CollisionMasks + функции load/free
- [ ] `src/collision_masks.c` - реализация загрузки масок из res/tf (g.java:213-235)
- [ ] `src/collision.h` - функция collision_test
- [ ] `src/collision.c` - реализация проверки коллизий (g.java:315-430)

### 2. Изменить существующие файлы

- [ ] `src/player.h` - обновить подпись: `void player_update(Player*, Level*, TileMetadata*, CollisionMasks*)`
- [ ] `src/player.c` - добавить параметр `CollisionMasks* masks`, вызывать `collision_test(..., masks, ...)`
- [ ] `src/main.c` - загрузить `CollisionMasks* masks = collision_masks_load("res/tf")`
- [ ] `src/main.c` - обновить вызов: `player_update(player, level, tile_meta, masks)`
- [ ] `src/main.c` - освободить `collision_masks_free(masks)` при выходе
- [ ] `Makefile` - добавить `src/collision_masks.c` и `src/collision.c` в OBJS

### 3. Порядок реализации

**Шаг 3.1:** Создать collision_masks.h/c
```c
// collision_masks.h
typedef struct {
    bool*** masks;
    uint8_t tile_count;
    uint8_t mask_w, mask_h;
} CollisionMasks;

CollisionMasks* collision_masks_load(const char* tf_path);  // g.java:213-235
void collision_masks_free(CollisionMasks* masks);
```

**Шаг 3.2:** Создать collision.h/c
```c
// collision.h
#include "collision_masks.h"

bool collision_test(Level* level, TileMetadata* tile_meta,
                    CollisionMasks* masks,
                    int rect_x, int rect_y, int rect_w, int rect_h,
                    bool* player_mask);  // g.java:315-430

void apply_transform(uint8_t transform, int* x, int* y);  // g.java:395-419
```

**Шаг 3.3:** Обновить player.h
```c
// БЫЛО:
void player_update(Player* p, Level* level, TileMetadata* tile_meta);

// СТАЛО:
void player_update(Player* p, Level* level, TileMetadata* tile_meta,
                   CollisionMasks* masks);
```

**Шаг 3.4:** Обновить player.c
```c
// Добавить #include "collision.h" в начало
// Изменить подпись функции
// Использовать collision_test внутри цикла попиксельного движения
```

**Шаг 3.5:** Обновить main.c
```c
// После tilemetadata_load:
CollisionMasks* collision_masks = collision_masks_load("res/tf");
if (!collision_masks) {
    fprintf(stderr, "Failed to load collision masks\n");
    // cleanup and exit
}

// В игровом цикле:
player_update(player, level, tile_meta, collision_masks);

// При завершении (перед resource_free):
collision_masks_free(collision_masks);
```

**Шаг 3.6:** Обновить Makefile
```makefile
OBJS = src/main.o \
       src/resource_loader.o \
       src/level_loader.o \
       src/tile_metadata.o \
       src/player.o \
       src/collision_masks.o \
       src/collision.o
```

### 4. Проверка перед компиляцией

☐ collision_masks.h включен в collision.h?  
☐ collision.h включен в player.c?  
☐ collision_masks.h включен в main.c?  
☐ Все подписи функций согласованы?  
☐ Makefile содержит все новые .o файлы?

## Критерии успеха

**debug.log:**
```
Player pos=(X, Y+9) speed=9
Player pos=(X, Y+18) speed=18
...
Player pos=(X, Y_ground) speed=0 grounded=1  ← остановился на тайле
```

**Визуально:**
- Мяч падает и останавливается на платформе
- Не проваливается сквозь тайлы
- Нижняя граница мяча касается верхней границы тайла

## Тестовые случаи

1. **Solid tile (type=2):** мяч останавливается мгновенно
2. **Mask tile (type=1):** учитывает прозрачные пиксели
3. **Transform tile (type=3):** правильно применяет rotate/flip перед проверкой маски
4. **Падение с высоты:** корректное попиксельное торможение

## Следующий шаг

**Шаг 5:** Управление (влево/вправо) и прыжки

## Референсы

- DEOBFUSCATION.md § "Формат метаданных тайлов (res/tf)"
- COLLISION_CONTRACT.md (полный контракт коллизий)
- g.java:315-430 (collisionTest - оригинальная реализация)
- g.java:226-234 (загрузка collision masks)
- a.java:662-678 (GRAVITY_NORMAL=9, MAX_FALL_SPEED=80)

