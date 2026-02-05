# STEP 05: Управление и прыжки

## Источники

- **Оригинал:** `a.java:207-221` (ввод), `a.java:650-658` (jump logic), `a.java:543-556` (jump strength), `a.java:1393-1430` (movement)
- **DEOBFUSCATION.md** § 2.3 (Player.inputMask)
- **SDL2:** SDL_GameController (PSP контроллер)

---

## 1. Концепт

Оригинал использует битовую маску `this.m` (inputMask) для хранения текущего состояния кнопок:
- Бит 0x1 = влево
- Бит 0x2 = вправо  
- Бит 0x4 = вниз (не используется в текущей реализации)
- Бит 0x8 = прыжок

Прыжок выполняется ТОЛЬКО если `isGrounded==true` (a.java:657), сила прыжка зависит от бонусов.

---

## 2. Битовая маска ввода (a.java:207-221)

```java
// a.java:207
public final void c(int paramInt) {
  this.m |= paramInt;
}

// a.java:214
public final void b(int paramInt) {
  this.m &= (paramInt ^ 0xFFFFFFFF);
}

// a.java:221
public final void i() {
  this.m = 0;
}
```

**Для PSP:** Заменить на чтение SDL_GetKeyboardState каждый кадр.

---

## 3. Константы прыжков

### Прыжок (a.java:543-556)

```java
private int h() {
  int i = 0;
  if (this.I) {           // isInverted
    i = -180;
  } else if (this.F) {    // isPopped
    i = -95;
  }
  if (this.C)             // hasJumpBonus
    i += i >> 2;          // +25%
  if (this.l)             // hasGravBonus
    i -= i >> 2;          // -25%
  return i;
}
```

**Константы:**
- JUMP_NORMAL = -125 (по умолчанию)
- JUMP_INVERTED = -180 (a.java:544)
- JUMP_POPPED = -95 (a.java:546)
- Модификатор JumpBonus: +25% (a.java:549)
- Модификатор GravBonus: -25% (a.java:551)

---

## 4. Логика прыжка (a.java:650-658)

```java
// a.java:650
boolean bool1 = ((this.m & 0x8) != 0 && (this.m & 0x4) == 0) ? true : false;
// ...
if (bool1 && this.x) {  // jump pressed && grounded
  j = h();
  this.x = false;
}
```

**Правило:** Прыжок только при `isGrounded==true`, после прыжка `isGrounded=false`.

---

## 5. Движение по горизонтали (a.java:1393-1430)

```java
// a.java:1393
if ((this.m & 0x2) != 0 && (this.m & 0x1) == 0) {       // right only
  i += this.j ? 22 : 18;                                 // +speed
  if (this.j && i > 100) {                               // hasSpeedBonus
    i = 100;
  } else if (!this.j && i > 60) {
    i = 60;
  }
} else if ((this.m & 0x2) == 0 && (this.m & 0x1) != 0) { // left only
  i -= this.j ? 22 : 18;                                 // -speed
  if (this.j && i < -100) {
    i = -100;
  } else if (!this.j && i < -60) {
    i = -60;
  }
}
```

**Константы:**
- ACCEL_NORMAL = 18 (a.java:1394)
- ACCEL_BONUS = 22 (a.java:1394)
- MAX_SPEED_NORMAL = 60 (a.java:1397)
- MAX_SPEED_BONUS = 100 (a.java:1395)

**Торможение (a.java:1419-1430):**

```java
if (i > 0) {
  i -= this.x ? 8 : 3;  // grounded ? 8 : 3
  if (i < 0) i = 0;
} else if (i < 0) {
  i += this.x ? 8 : 3;
  if (i > 0) i = 0;
}
```

- DECEL_GROUNDED = 8 (a.java:1419)
- DECEL_AIRBORNE = 3 (a.java:1419)

---

## 6. Реализация для PSP

### 6.1 Файлы

- [ ] `src/input.h` — структура Input, функция input_update
- [ ] `src/input.c` — реализация (SDL_GetKeyboardState)
- [ ] `src/player.h` — добавить константы прыжков/движения, поля бонусов, обновить подпись player_update
- [ ] `src/player.c` — добавить calculate_jump_strength, обновить player_update (Input, прыжки, движение)

### 6.2 Input структура

```c
typedef struct {
    bool left;
    bool right;
    bool down;
    bool jump;
} Input;

void input_update(Input* input);
```

### 6.3 Константы (player.h)

```c
// Прыжки (a.java:543-556)
#define JUMP_NORMAL    -125
#define JUMP_INVERTED  -180
#define JUMP_POPPED     -95

// Движение (a.java:1393-1430)
#define ACCEL_NORMAL   18
#define ACCEL_BONUS    22
#define MAX_SPEED_NORMAL 60
#define MAX_SPEED_BONUS 100
#define DECEL_GROUNDED  8
#define DECEL_AIRBORNE  3
```

### 6.4 Player изменения

**player.h:** Добавить поле `bool has_speed_bonus`, `bool has_jump_bonus`, `bool has_grav_bonus`

**player_update:** 
1. Читать Input
2. Применять горизонтальное движение (ускорение/торможение)
3. Если jump нажат И isGrounded → ySpeed = calculate_jump_strength()
4. Применять гравитацию (как в Step 3)
5. Проверять коллизии с тайлами (collision_test из Step 4)

---

## 7. Пошаговая интеграция

### 1. Создать input.h/c

- [ ] `input.h`: Объявить Input struct, input_init/input_cleanup/input_update
- [ ] `input.c`: Реализовать через SDL_GameController (PSP контроллер)
  - input_init(): SDL_GameControllerOpen(0)
  - D-pad left → SDL_CONTROLLER_BUTTON_DPAD_LEFT
  - D-pad right → SDL_CONTROLLER_BUTTON_DPAD_RIGHT
  - Cross (X) → SDL_CONTROLLER_BUTTON_A (jump)

### 2. Обновить player.h

- [ ] Добавить константы JUMP_*, BOUNCE_*, ACCEL_*, MAX_SPEED_*, DECEL_*
- [ ] Добавить поля: `bool has_speed_bonus`, `bool has_jump_bonus`, `bool has_grav_bonus`
- [ ] Обновить подпись: `void player_update(Player* p, Level* level, TileMetadata* tile_meta, CollisionMasks* masks, Input* input);`

### 3. Обновить player.c

- [ ] Добавить функцию `calculate_jump_strength(Player* p)` (a.java:543-556)
- [ ] В `player_update`:
  - Читать `input->left`, `input->right`, `input->jump`
  - Применять ускорение/торможение (a.java:1393-1430)
  - Если `input->jump && p->is_grounded`: `p->y_speed = calculate_jump_strength(p)`
  - Применять гравитацию и проверять коллизии (как Step 3)

### 4. Обновить main.c

- [ ] Вызвать `input_init()` после создания игрока
- [ ] Создать `Input input = {0};`
- [ ] В игровом цикле: `input_update(&input);`
- [ ] Передать в `player_update(player, level, tile_meta, masks, &input);`
- [ ] При выходе: `input_cleanup()`

### 5. Makefile

- [ ] Добавить `src/input.c` в OBJS

### 6. Тесты

- [ ] Запустить, нажать влево/вправо → мяч двигается
- [ ] debug.log: "xSpeed=18" при ускорении
- [ ] Нажать X на земле → прыжок, "ySpeed=-125"
- [ ] В воздухе X не работает

---

## Критерии успеха

✅ Мяч двигается влево/вправо при нажатии D-pad  
✅ Прыжок работает ТОЛЬКО на земле  
✅ Скорость ограничена MAX_SPEED_NORMAL=60  
✅ Торможение работает (DECEL_GROUNDED=8)  
✅ debug.log показывает "jump=-125" при прыжке

---

## Примечания

- Бонусы (speed/jump/grav) пока НЕ реализованы — будут в Step 6
- Враги и специальные тайлы (батуты, порталы) — Step 7+
- Инверсия гравитации — Step 6
- **Отскок от врагов/батутов (bounce)** — Step 7+ (использует a.java:558-590, функция f())

**Источники проверены:** ✅ a.java:207-221, 543-556, 650-658, 1393-1430
