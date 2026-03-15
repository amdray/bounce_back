# STEP 06: Камера с мертвой зоной

## Источники

- **bounce_zero/src/game.c:60-90, 280-330** (camera implementation)
- **DEOBFUSCATION.md** § 1.2 (Камера и физика камеры из bounce_zero)
- **PSP:** SCREEN_WIDTH=480, SCREEN_HEIGHT=272

---

## 1. Концепт

PSP экран 480×272 меньше большинства уровней (48×20 тайлов = 768×320 пикселей).  
Нужна камера, следящая за игроком с **мертвой зоной** (deadzone):

- **Горизонталь (X):** Камера центрируется на игроке (простое следование)
- **Вертикаль (Y):** С мертвой зоной 30% — игрок может двигаться внутри зоны без сдвига камеры
- **Маленькие уровни:** Центрируются вертикально, камера не двигается

Без HUD (bounce_back не имеет HUD в игре) gameAreaHeight = SCREEN_HEIGHT.

---

## 2. Константы (bounce_zero/src/game.c:61-63)

```c
#define CAMERA_UNINITIALIZED -999
#define CAMERA_DEADZONE_PERCENT 30  // 30% от высоты экрана
```

---

## 3. Горизонтальная камера (bounce_zero/src/game.c:290-293)

```c
// Простое центрирование на игроке
int cameraX = player->xPos - SCREEN_WIDTH / 2;

// Clamp по границам уровня
int maxCameraX = g_level.width * TILE_SIZE - SCREEN_WIDTH;
if (cameraX < 0) cameraX = 0;
if (cameraX > maxCameraX && maxCameraX > 0) cameraX = maxCameraX;
```

**Для bounce_back:**
- `player->x_pos` вместо `player->xPos`
- `level->width` вместо `g_level.width`

---

## 4. Вертикальная камера с deadzone (bounce_zero/src/game.c:295-320)

```c
// Инициализация при первом запуске
if (s_currentCameraY == CAMERA_UNINITIALIZED) {
    s_currentCameraY = player->yPos - SCREEN_HEIGHT / 2;
}

// Deadzone логика
int deadZoneTop = (SCREEN_HEIGHT * CAMERA_DEADZONE_PERCENT) / 100;
int deadZoneBottom = SCREEN_HEIGHT - deadZoneTop;

int tempPlayerScreenY = player->yPos - s_currentCameraY;
if (tempPlayerScreenY < deadZoneTop) {
    s_currentCameraY = player->yPos - deadZoneTop;
} else if (tempPlayerScreenY > deadZoneBottom) {
    s_currentCameraY = player->yPos - deadZoneBottom;
}

// Clamp
int maxCameraY = g_level.height * TILE_SIZE - SCREEN_HEIGHT;
if (s_currentCameraY < 0) s_currentCameraY = 0;
if (s_currentCameraY > maxCameraY && maxCameraY > 0) s_currentCameraY = maxCameraY;
```

**Для bounce_back:**
- `player->y_pos` вместо `player->yPos`
- `level->height` вместо `g_level.height`

---

## 5. Маленькие уровни (bounce_zero/src/game.c:70-82)

```c
static inline bool is_level_small(void) {
    return (g_level.height * TILE_SIZE) < SCREEN_HEIGHT;
}

static inline int get_center_offset(void) {
    int levelPixelHeight = g_level.height * TILE_SIZE;
    return -(SCREEN_HEIGHT - levelPixelHeight) / 2;
}

void game_reset_camera(void) {
    if (is_level_small()) {
        s_currentCameraY = get_center_offset();
    } else {
        s_currentCameraY = CAMERA_UNINITIALIZED;
    }
}
```

**Правило:** Если уровень меньше экрана по высоте → центрировать, не двигать камеру.

---

## 6. Реализация для PSP

### 6.1 Файлы

- [ ] `src/camera.h` — структура Camera, функции camera_init/camera_update/camera_reset
- [ ] `src/camera.c` — реализация
- [ ] `src/main.c` — создать Camera, вызывать camera_update, передавать в рендеринг
- [ ] `src/player.c` — player_render принимает camera_x, camera_y
- [ ] `src/level_renderer.c` — renderer_draw принимает camera_x, camera_y

### 6.2 Camera структура

```c
typedef struct {
    int x;              // Текущая позиция камеры X (мировые координаты)
    int y;              // Текущая позиция камеры Y (с deadzone)
    bool initialized;   // Флаг инициализации Y камеры
} Camera;

void camera_init(Camera* cam);
void camera_reset(Camera* cam, int level_width, int level_height);
void camera_update(Camera* cam, int player_x, int player_y, 
                   int level_width, int level_height);
```

### 6.3 Константы (camera.h)

```c
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define TILE_SIZE 16
#define CAMERA_DEADZONE_PERCENT 30
```

### 6.4 Изменения рендеринга

**player_render:** Вычитать camera_x/camera_y из мировых координат:
```c
void player_render(Player* p, SDL_Renderer* renderer, int camera_x, int camera_y) {
    int screen_x = p->x_pos - p->half_width - camera_x;
    int screen_y = p->y_pos - p->half_height - camera_y;
    // ...
}
```

**level_renderer:** Рисовать только видимые тайлы (bounce_zero/src/level.c:417):
```c
void renderer_draw(LevelRenderer* lr, SDL_Renderer* r, int camera_x, int camera_y) {
    int startTileX = camera_x / TILE_SIZE;
    int endTileX = (camera_x + SCREEN_WIDTH - 1) / TILE_SIZE;
    int startTileY = camera_y / TILE_SIZE;
    int endTileY = (camera_y + SCREEN_HEIGHT - 1) / TILE_SIZE;
    
    for (int y = startTileY; y <= endTileY; y++) {
        for (int x = startTileX; x <= endTileX; x++) {
            int screen_x = x * TILE_SIZE - camera_x;
            int screen_y = y * TILE_SIZE - camera_y;
            // draw_tile(...)
        }
    }
}
```

---

## 7. Пошаговая интеграция

### 1. Создать camera.h/c

- [ ] `camera.h`: Объявить Camera struct, константы, функции
- [ ] `camera.c`: Реализовать camera_update (bounce_zero/src/game.c:290-330)
  - Горизонталь: центрирование + clamp
  - Вертикаль: deadzone + clamp
  - Маленькие уровни: центрирование

### 2. Обновить player.h/c

- [ ] `player.h`: Изменить подпись `void player_render(Player* p, SDL_Renderer* renderer, int camera_x, int camera_y);`
- [ ] `player.c`: В player_render вычитать camera_x/camera_y:
  ```c
  int screen_x = p->x_pos - p->half_width - camera_x;
  int screen_y = p->y_pos - p->half_height - camera_y;
  SDL_RenderCopy(renderer, sprite, NULL, &(SDL_Rect){screen_x, screen_y, w, h});
  ```

### 3. Обновить level_renderer.h/c

- [ ] `level_renderer.h`: Изменить подпись `void renderer_draw(LevelRenderer* lr, SDL_Renderer* r, int camera_x, int camera_y);`
- [ ] `level_renderer.c`: Рисовать только видимые тайлы:
  - Вычислить startTileX/Y, endTileX/Y
  - Цикл по видимым тайлам
  - Вычесть camera_x/camera_y из screen_x/screen_y

### 4. Обновить main.c

- [ ] Создать `Camera camera = {0};`
- [ ] После загрузки уровня: `camera_reset(&camera, level->width, level->height);`
- [ ] В игровом цикле ПОСЛЕ player_update: `camera_update(&camera, player->x_pos, player->y_pos, level->width, level->height);`
- [ ] Передать в рендеринг:
  ```c
  renderer_draw(level_renderer, renderer, camera.x, camera.y);
  player_render(player, renderer, camera.x, camera.y);
  ```

### 5. Makefile

- [ ] Добавить `src/camera.c` в OBJS

### 6. Тесты

- [ ] Запустить на большом уровне (48×20) → камера следует за мячом
- [ ] Двигаться по вертикали внутри deadzone → камера не двигается
- [ ] Выйти за deadzone → камера начинает следовать
- [ ] Мяч не выходит за пределы экрана
- [ ] debug.log: "cameraX=%d cameraY=%d" каждый кадр

---

## Критерии успеха

✅ Камера центрируется на игроке по горизонтали  
✅ Вертикальная мертвая зона 30% работает  
✅ Камера НЕ выходит за границы уровня (clamp)  
✅ Маленькие уровни центрируются вертикально  
✅ Видны только тайлы в области экрана (оптимизация)  
✅ debug.log показывает "cameraX=..." при движении

---

## Примечания

- HUD в bounce_back НЕТ в игровом процессе (только в меню) → gameAreaHeight = SCREEN_HEIGHT
- TILE_SIZE = 16 (константа из оригинала)
- Wrap уровни (clampX=0, clampY=0) будут обрабатываться в Step 7+ (пока все уровни clamp)
- bounce_zero использует 30 FPS, bounce_back — 20 FPS → логика камеры не зависит от FPS

**Источники проверены:** ✅ bounce_zero/src/game.c:60-90, 290-330
