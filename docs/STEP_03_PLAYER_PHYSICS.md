# Шаг 3: Персонаж (мяч) с базовой физикой

## Статус: Не начат

**Предыдущий шаг:** ✅ Шаг 2 - Тайловый движок (статичный рендер карты)

## Цель шага

Добавить игрока (мяч) на уровень с базовой физикой (гравитация + падение). Мяч должен отрисовываться поверх карты и падать вниз под действием гравитации. Пока **БЕЗ коллизий** - мяч улетит за пределы экрана, это нормально.

## Что нужно реализовать

### 3.1. Структура игрока (`player.h`)

**Базовые поля** (из DEOBFUSCATION.md, класс Player a.java, строки 91-108):

```c
typedef struct {
    // Позиция (пиксели)
    int x_pos;              // this.D - X координата центра мяча
    int y_pos;              // this.i - Y координата центра мяча
    
    // Скорости (пиксели/тик)
    int x_speed;            // this.s - горизонтальная скорость
    int y_speed;            // this.h - вертикальная скорость
    int prev_y_speed;       // this.z - предыдущая вертикальная (для отскока)
    
    // Размеры и спрайт
    int sprite_index;       // this.g - индекс текущего спрайта (0-25)
    int sprite_width;       // this.d - ширина спрайта
    int sprite_height;      // this.u - высота спрайта
    int half_width;         // this.J - половина ширины (для центрирования)
    int half_height;        // this.c - половина высоты
    
    // Состояние
    bool is_large;          // this.t - true=16px, false=12px
    bool is_inverted;       // this.I - инвертированная гравитация
    bool gravity_down;      // this.p - направление гравитации
    bool is_grounded;       // this.x - на земле (будет использоваться в шаге 4)
    
    // Ресурсы
    SDL_Texture** ball_sprites;  // Массив 26 текстур мяча
    int sprite_count;            // Количество загруженных спрайтов
} Player;
```

### 3.2. Инициализация игрока (`player.c`)

**Функция создания:**
```c
Player* player_create(SDL_Renderer* renderer, 
                      int spawn_x_tiles, int spawn_y_tiles,
                      bool is_large_ball) {
    Player* p = calloc(1, sizeof(Player));
    
    // Загрузка спрайтов из /res/b
    ResourceContainer* ball_res = resource_load("res/b");
    if (!ball_res) return NULL;
    
    // Элементы 1-25 в /res/b - PNG спрайты (элемент 0 - кастомный формат)
    p->sprite_count = ball_res->count - 1;  // 25 спрайтов
    p->ball_sprites = calloc(p->sprite_count, sizeof(SDL_Texture*));
    
    for (int i = 0; i < p->sprite_count; i++) {
        size_t png_size;
        const uint8_t* png_data = resource_get_element(ball_res, i + 1, &png_size);
        if (!png_data) continue;
        
        SDL_RWops* rw = SDL_RWFromConstMem(png_data, png_size);
        SDL_Surface* surf = IMG_Load_RW(rw, 1);
        if (surf) {
            p->ball_sprites[i] = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
        }
    }
    resource_free(ball_res);
    
    // Установка начальной позиции (из DEOBFUSCATION.md, h.b строки 483-527)
    // spawnPx = spawnTiles * 16 + offset
    int offset = is_large_ball ? 12 : 8;  // Большой мяч: +12, маленький: +8
    p->x_pos = spawn_x_tiles * 16 + offset;
    p->y_pos = spawn_y_tiles * 16 + offset;
    
    // Начальное состояние
    p->is_large = is_large_ball;
    p->sprite_index = is_large_ball ? 11 : 0;  // Большой: sprite 11, маленький: sprite 0
    p->is_inverted = false;
    p->gravity_down = true;
    p->is_grounded = false;
    
    // Размеры спрайта (из DEOBFUSCATION.md, a.java:267-298)
    // Функция a.a(int spriteIndex) устанавливает размеры
    if (is_large_ball) {
        p->sprite_width = 16;
        p->sprite_height = 16;
    } else {
        p->sprite_width = 12;
        p->sprite_height = 12;
    }
    p->half_width = p->sprite_width / 2;
    p->half_height = p->sprite_height / 2;
    
    // Начальные скорости
    p->x_speed = 0;
    p->y_speed = 0;
    p->prev_y_speed = 0;
    
    return p;
}

void player_free(Player* p) {
    if (!p) return;
    if (p->ball_sprites) {
        for (int i = 0; i < p->sprite_count; i++) {
            if (p->ball_sprites[i]) SDL_DestroyTexture(p->ball_sprites[i]);
        }
        free(p->ball_sprites);
    }
    free(p);
}
```

### 3.3. Физика игрока (`player_update`)

**Константы физики** (из a.java строки 662-678, проверено в оригинальном коде):

```c
// Константы из оригинального кода (a.java:662-678)
#define GRAVITY_NORMAL       9      // Обычная гравитация в воздухе (пиксели/тик)
#define GRAVITY_SPECIAL      7      // Гравитация на специальных тайлах (флаг 0x80)
#define GRAVITY_INVERTED    -6      // Инвертированная гравитация на спец. тайлах
#define MAX_FALL_SPEED      80      // Максимальная скорость падения (нормальная)
#define MAX_FALL_SPEED_SPECIAL 20   // Макс. скорость на специальных тайлах (не лопнувший мяч)
```

**Функция обновления** (из a.java метод `d()` строка 599):

```c
void player_update(Player* p) {
    // Сохранить предыдущую вертикальную скорость (для отскока)
    p->prev_y_speed = p->y_speed;
    
    // Упрощенная физика для шага 3 (без проверки специальных тайлов)
    // В шаге 4 добавим проверку флага 0x80 в tileMap и разные значения gravity
    int gravity = GRAVITY_NORMAL;      // 9 пикселей/тик
    int max_speed = MAX_FALL_SPEED;    // 80 пикселей/тик
    
    if (p->gravity_down) {
        // Гравитация вниз
        p->y_speed += gravity;
        if (p->y_speed > max_speed) {
            p->y_speed = max_speed;
        }
    } else {
        // Гравитация вверх (инвертированная)
        // В оригинале: gravity = -6 на специальных тайлах
        // Для упрощения сейчас используем -GRAVITY_NORMAL
        p->y_speed -= gravity;
        if (p->y_speed < -max_speed) {
            p->y_speed = -max_speed;
        }
    }
    
    // Обновить позицию
    p->x_pos += p->x_speed;
    p->y_pos += p->y_speed;
    
    // На этом шаге коллизий нет, поэтому is_grounded всегда false
    p->is_grounded = false;
}
```

**Примечание:** Сейчас мяч будет просто падать вниз с постоянной гравитацией 9 и улетит за экран. Коллизии добавим в шаге 4.

**Важно о физике:** В оригинальном коде (a.java:662-678) гравитация зависит от флага 0x80 в байте tileMap под игроком:
- Флаг 0x80 = 0 (обычный тайл): gravity = 9, max_speed = 80
- Флаг 0x80 = 1 (специальный тайл, ice/bounce): gravity = 7 или -6 (инвертированная), max_speed = 20

Для упрощения шага 3 игнорируем этот флаг и используем константы как для обычных тайлов. Полную логику реализуем в шаге 4 вместе с коллизиями.

### 3.4. Рендер игрока (`player_render`)

```c
void player_render(Player* p, SDL_Renderer* renderer, int camera_x, int camera_y) {
    if (!p || !p->ball_sprites) return;
    
    // Проверка валидности индекса спрайта
    if (p->sprite_index < 0 || p->sprite_index >= p->sprite_count) return;
    
    SDL_Texture* sprite = p->ball_sprites[p->sprite_index];
    if (!sprite) return;
    
    // Вычислить экранную позицию
    // Позиция игрока - это ЦЕНТР мяча, нужно вычесть half_width/half_height
    int screen_x = p->x_pos - p->half_width - camera_x;
    int screen_y = p->y_pos - p->half_height - camera_y;
    
    SDL_Rect dest = {
        screen_x,
        screen_y,
        p->sprite_width,
        p->sprite_height
    };
    
    // Рендер с учетом инверсии (flip vertical если is_inverted)
    SDL_RendererFlip flip = p->is_inverted ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, sprite, NULL, &dest, 0.0, NULL, flip);
}
```

### 3.5. Обновление main.c

**Добавить инициализацию игрока:**
```c
// После загрузки level
Player* player = player_create(renderer, 
                               level->spawn_x, 
                               level->spawn_y,
                               level->ball_type != 0);  // ball_type: 0=маленький, другое=большой

if (log) fprintf(log, "Player created at (%d, %d)\n", player->x_pos, player->y_pos);
```

**Обновить main loop:**
```c
// Обработка выхода
while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) running = 0;
    if (event.type == SDL_CONTROLLERBUTTONDOWN) {
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
            running = 0;
        }
    }
}

// Обновление физики (50ms = 20 FPS как в оригинале)
player_update(player);

// Рендер
SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
SDL_RenderClear(renderer);

// Рендер карты (камера пока в 0,0)
renderer_draw_simple(level_renderer, renderer);

// Рендер игрока поверх карты (камера 0,0)
player_render(player, renderer, 0, 0);

SDL_RenderPresent(renderer);

// Задержка для 20 FPS (50ms tick)
SDL_Delay(50);
```

**Cleanup:**
```c
player_free(player);
```

## Критерий успеха

При запуске на PSP должно быть видно:
1. ✅ Тайловая карта уровня (из шага 2)
2. ✅ Мяч на стартовой позиции (spawn_x, spawn_y)
3. ✅ Мяч падает вниз под действием гравитации
4. ✅ Мяч ускоряется до MAX_FALL_SPEED (80 пикселей/тик)
5. ✅ Мяч улетает за нижний край экрана (коллизий пока нет)

## Проверка

**debug.log должен показать:**
```
Level 0 loaded: ...
Player created at (X, Y)
Player update: pos=(X, Y) speed=(0, 9)
Player update: pos=(X, Y+9) speed=(0, 18)
Player update: pos=(X, Y+27) speed=(0, 27)
...
Player update: pos=(X, Y+lots) speed=(0, 80)  // Достигнут MAX_FALL_SPEED
```

**Визуально:**
- Мяч должен стартовать в правильной позиции (примерно в центре/сверху уровня)
- Правильный размер (12px или 16px в зависимости от ball_type)
- Плавное ускорение падения
- Через ~1-2 секунды мяч исчезнет снизу (нормально, коллизий нет)

## Параметры физики для тестирования

**Gravity rate:** 9 пикселей/тик (из a.java:663, проверено в оригинале)
- Тик 0: y_speed = 0
- Тик 1: y_speed = 9
- Тик 2: y_speed = 18
- Тик 3: y_speed = 27
- Тик 4: y_speed = 36
- Тик 5: y_speed = 45
- Тик 6: y_speed = 54
- Тик 7: y_speed = 63
- Тик 8: y_speed = 72
- Тик 9: y_speed = 80 (MAX_FALL_SPEED достигнут)

**Скорость падения в пикселях:**
- За 1 секунду (20 тиков): мяч пролетит ~800 пикселей (на макс. скорости)
- За 0.5 секунды (10 тиков): ~360 пикселей (ускорение + макс. скорость)
- Экран PSP - 272 пикселя высотой, поэтому мяч улетит за ~0.4 секунды после достижения терминальной скорости

**Специальные тайлы (флаг 0x80 в tileMap):**
- На обычных тайлах: gravity = 9, max_speed = 80
- На специальных тайлах (ice, bounce): gravity = 7, max_speed = 20 (если не лопнувший)
- Инвертированная гравитация на спец. тайлах: gravity = -6

Для шага 3 используем упрощенную физику (gravity=9, max_speed=80 всегда). Полную логику добавим в шаге 4.

## Следующий шаг (Step 4)

**Шаг 4: Pixel-perfect коллизии с тайлами**
- Реализовать `g.collisionTest()` из DEOBFUSCATION.md (строки 315-430)
- Загрузка collision masks из `/res/tf`
- Проверка коллизий с учетом transform (rotate/flip)
- Мяч должен останавливаться на платформах
- `is_grounded` флаг корректно устанавливается

После этого мяч перестанет проваливаться сквозь платформы и будет корректно стоять на поверхности.

## Референсы

- `DEOBFUSCATION.md` строки 91-235 (класс Player a.java)
- `DEOBFUSCATION.md` строки 599-650 (метод a.d() - физика)
- `a.java:267-298` (метод a.a(int) - установка размеров спрайта)
- `a.java:599-690` (метод d() - основной цикл физики)
- **Проверенные константы из a.java:662-678:**
  - GRAVITY_NORMAL = 9 (в воздухе)
  - GRAVITY_SPECIAL = 7 (на тайлах с флагом 0x80)
  - GRAVITY_INVERTED = -6 (инвертированная на спец. тайлах)
  - MAX_FALL_SPEED = 80 (нормальная)
  - MAX_FALL_SPEED_SPECIAL = 20 (на спец. тайлах, не лопнувший мяч)

## Примечания по реализации

### Tick rate: 50ms (20 FPS)

Оригинальная игра работает на 20 FPS (50ms tick). Это важно для физики:
- `SDL_Delay(50)` в main loop
- Все скорости в пикселях/тик
- Гравитация применяется каждый тик

### Спрайты мяча (animation)

Сейчас используем только статичный спрайт (`sprite_index` фиксирован). В будущем:
- Спрайты 0-10: маленький мяч (12px)
- Спрайты 11-19: большой мяч (16px)
- Спрайты 20-24: лопнувший мяч (weak jump)

Анимацию добавим позже, когда будет движение по горизонтали.

### Отладка физики

Добавить в `player_update()` логирование каждые N тиков:
```c
static int tick_counter = 0;
tick_counter++;
if (tick_counter % 10 == 0 && log) {
    fprintf(log, "Player tick %d: pos=(%d,%d) speed=(%d,%d)\n",
            tick_counter, p->x_pos, p->y_pos, p->x_speed, p->y_speed);
}
```

Это покажет как ускоряется падение и достигается MAX_FALL_SPEED.
