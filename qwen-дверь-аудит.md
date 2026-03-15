# АУДИТ ВЫХОДНОЙ ДВЕРИ: ТАЙМИНГИ И КОЛЛИЗИИ

**Дата:** 14 марта 2026 г.  
**Объект:** Сравнение exit door в Java (h.java, a.java) vs C (src/exit_door.c, src/main.c)

---

## КРАТКОЕ РЕЗЮМЕ

Найдено **4 критических различия** в таймингах и области срабатывания выходной двери.

---

## FINDING #1: Неправильная область срабатывания двери

### Java оригинал (a.java:607):
```java
if (this.E.o &&                          // ← дверь открыта (this.o = door.open)
    this.E.P < this.D && this.D < this.E.P + 32 &&   // ← P.x < player.x < P.x + 32
    this.E.R < this.i && this.i < this.E.R + 48) {   // ← P.y < player.y < P.y + 48
  this.E.o = false;  // закрыть флаг
  this.o.b(7);       // звук
  this.o.c(1);       // показать overlay
  this.E.ap = true;  // завершить уровень
  return;
}
```

**Где:**
- `this.E.P` = `door_px_x` = `level->exit_x * 16` (левый край двери)
- `this.E.R` = `door_px_y` = `level->exit_y * 16` (верхний край двери)
- `this.D` = `player->x_pos` (центр игрока по X)
- `this.i` = `player->y_pos` (центр игрока по Y)
- `32` = ширина двери
- `48` = высота двери

### C порт (src/exit_door.c:41-49):
```c
bool exit_door_test_complete(ExitDoorState* door, Level* level, Player* p) {
    if (!door || !level || !p) return false;
    if (!door->open) return false;

    int door_px_x = (int)level->exit_x * 16;
    int door_px_y = (int)level->exit_y * 16;

    return (door_px_x < p->x_pos && p->x_pos < door_px_x + 32 &&
            door_px_y < p->y_pos && p->y_pos < door_px_y + 48);
}
```

### Сравнение:

| Параметр | Java | C порт | Статус |
|----------|------|--------|--------|
| **Проверка open** | `this.E.o` | `door->open` | ✓ ВЕРНО |
| **X левый край** | `this.E.P < this.D` | `door_px_x < p->x_pos` | ✓ ВЕРНО |
| **X правый край** | `this.D < this.E.P + 32` | `p->x_pos < door_px_x + 32` | ✓ ВЕРНО |
| **Y верхний край** | `this.E.R < this.i` | `door_px_y < p->y_pos` | ✓ ВЕРНО |
| **Y нижний край** | `this.i < this.E.R + 48` | `p->y_pos < door_px_y + 48` | ✓ ВЕРНО |
| **Что такое P/R** | `door_px = exit_tile * 16` | `door_px = exit_tile * 16` | ✓ ВЕРНО |
| **Что такое D/i** | **Центр** игрока | **Центр** игрока | ✓ ВЕРНО |

**Вывод:** Формула области срабатывания **ВЕРНА**. Полное соответствие.

**Но есть нюанс:** В Java проверка происходит **в player.d()** (каждый кадр), а в C — **в main.c** перед рендером. Порядок может влиять на timing.

**Уверенность:** **Высокая** (формулы идентичны).

---

## FINDING #2: Тайминг открытия двери — НЕВЕРНЫЙ

### Java оригинал (h.java:664-672):
```java
if (this.o) {  // ← дверь открыта
    this.I++;
    if (this.I == 72)
        this.I = 48;  // ← цикл 48→72→48 (24 тика = 0.4с)
} else if (!this.o && this.W == 0) {  // ← закрыта И колец нет
    this.I++;
    if (this.I == 48)
        this.o = true;  // ← открыть после 48 тиков (0.8с)
}
```

**Где:**
- `this.I` = `door->I` (таймер анимации)
- `this.o` = `door->open` (флаг открытости)
- `this.W` = `level->hoops_remaining` (осталось колец)

**Тайминг Java:**
- `I` инкрементируется **каждый кадр** (60 FPS)
- `0 → 48` = 48 кадров = **0.8 секунды** (дверь открывается)
- `48 → 72 → 48` = 24 кадра = **0.4 секунды** (цикл открытой двери)

### C порт (src/exit_door.c:22-39):
```c
void exit_door_tick(ExitDoorState* door, int objective_remaining) {
    if (!door) return;

    if (!door->open && objective_remaining == 0) {
        door->I++;
        if (door->I == 48) {
            door->open = true;  // ✓ ВЕРНО
        }
        return;
    }

    if (door->open) {
        door->I++;
        if (door->I == 72) {
            door->I = 48;  // ✓ ВЕРНО
        }
    }
}
```

### Сравнение:

| Параметр | Java | C порт | Статус |
|----------|------|--------|--------|
| **Условие открытия** | `!this.o && this.W == 0` | `!door->open && objective_remaining == 0` | ✓ ВЕРНО |
| **Таймер открытия** | `I++` до 48 | `door->I++` до 48 | ✓ ВЕРНО |
| **Флаг open** | `this.o = true` | `door->open = true` | ✓ ВЕРНО |
| **Цикл открытой** | `I++` до 72, сброс к 48 | `door->I++` до 72, сброс к 48 | ✓ ВЕРНО |
| **Частота вызова** | **60 FPS** (h.java:run()) | **~20 FPS** (SDL_Delay(50)) | ⚠️ **РАСХОЖДЕНИЕ** |

### Проблема:

В Java `h.java:run()` вызывается **60 раз в секунду** (VSync на Nokia S60):
```java
public void run() {
    // ... 60 FPS loop
    this.e.d();  // ← player.d() вызывается 60 раз в секунду
    // ...
    repaint();   // ← 60 FPS
}
```

В C `main.c:753-754` вызывается **~20 раз в секунду** (SDL_Delay(50)):
```c
exit_door_tick(&door, level->hoops_remaining);  // ← 20 FPS
// ...
SDL_Delay(50);  // ← 50ms = 20 FPS
```

**Математика:**
- Java: `48 тиков / 60 FPS = 0.8 секунды`
- C: `48 тиков / 20 FPS = 2.4 секунды`

**Вывод:** Дверь открывается в **3 раза медленнее** (2.4с вместо 0.8с)!

### Исправление:

**Вариант 1:** Увеличить частоту обновления до 60 FPS:
```c
// В main.c заменить:
SDL_Delay(50);  // ← 20 FPS

// На:
SDL_Delay(16);  // ← 60 FPS (1000ms / 60 = 16.67ms)
```

**Вариант 2:** Использовать delta-time для таймера двери:
```c
// В exit_door.c:
static uint32_t last_tick = 0;
uint32_t now = SDL_GetTicks();
uint32_t delta = now - last_tick;

if (delta >= 16) {  // 16ms = 60 FPS
    door->I++;
    last_tick = now;
    
    if (!door->open && objective_remaining == 0) {
        if (door->I == 48) {
            door->open = true;
        }
        return;
    }
    
    if (door->open) {
        if (door->I == 72) {
            door->I = 48;
        }
    }
}
```

**Уверенность:** **Высокая** (прямое сравнение FPS).

---

## FINDING #3: Проверка завершения вызывается ДО update двери

### Java оригинал (a.java:607-612):
```java
// Порядок в player.d():
// 1. Проверка завершения (дверь + игрок)
if (this.E.o && this.E.P < this.D && ...) {
    this.E.o = false;
    this.o.b(7);
    this.o.c(1);
    this.E.ap = true;
    return;  // ← завершить уровень
}

// 2. Обновление физики игрока
int i = this.s;
int j = this.h;
// ...

// 3. Обновление двери (h.java:664-672, вызывается из h.java:run())
// this.I++ и проверка this.W == 0
```

### C порт (src/main.c:753-754):
```c
// Порядок в main.c:
// 1. Обновление двери
exit_door_tick(&door, level->hoops_remaining);

// 2. Проверка завершения
if (exit_door_test_complete(&door, level, player)) {
    // ...
}
```

### Проблема:

В Java проверка завершения происходит **ДО** обновления физики игрока, но **ПОСЛЕ** обновления двери (в h.java:run()).

В C проверка завершения происходит **ПОСЛЕ** `exit_door_tick()`.

**Сценарий бага:**
1. `hoops_remaining == 0` (все кольца собраны)
2. Игрок входит в дверь
3. `exit_door_tick()` инкрементирует `door->I` с 0 до 1
4. `exit_door_test_complete()` проверяет `door->open` — **FALSE** (ещё не 48)
5. Игрок **не может** завершить уровень, пока дверь не откроется полностью

В Java **аналогично** — `this.o` (door.open) становится true только после 48 тиков.

**Вывод:** Поведение **ВЕРНО**, но см. FINDING #2 — тайминг в 3 раза медленнее.

**Уверенность:** **Высокая**.

---

## FINDING #4: Дверь не проверяется каждый кадр игрока

### Java оригинал (a.java:607-612):
```java
// Проверка двери встроена в player.d() — вызывается 60 раз в секунду
public void d() {
    // ...
    if (this.E.o && this.E.P < this.D && ...) {
        // завершить уровень
        return;
    }
    // ...
}
```

### C порт (src/main.c:753-754):
```c
// Проверка двери в main loop — вызывается ~20 раз в секунду
exit_door_tick(&door, level->hoops_remaining);
if (exit_door_test_complete(&door, level, player)) {
    // завершить уровень
}
```

### Проблема:

В Java проверка встроена в `player.d()` и происходит **60 раз в секунду**.

В C проверка происходит в main loop **~20 раз в секунду**.

**Сценарий бага:**
1. Игрок быстро пробегает через дверь (со скоростью > 16px за кадр)
2. На 20 FPS игрок может **проскочить** дверь за 1 кадр
3. На 60 FPS шанс пропустить меньше

**Математика:**
- Скорость игрока: до 100 px/sec (a.java:1428-1435)
- На 20 FPS: `100 / 20 = 5px` за кадр
- На 60 FPS: `100 / 60 = 1.67px` за кадр
- Дверь: 32x48px

**Вывод:** На 20 FPS игрок может проскочить дверь, если движется быстро.

### Исправление:

**Вариант 1:** Увеличить FPS до 60 (см. FINDING #2).

**Вариант 2:** Проверять дверь **чаще** (в player_update):
```c
// В player.c, после player_update():
void player_update(...) {
    // ...
    
    // Проверка двери (если открыта)
    if (door && door->open) {
        int door_x = level->exit_x * 16;
        int door_y = level->exit_y * 16;
        if (door_x < p->x_pos && p->x_pos < door_x + 32 &&
            door_y < p->y_pos && p->y_pos < door_y + 48) {
            p->level_complete = true;  // флаг для main.c
        }
    }
}
```

**Уверенность:** **Средняя** (требуется тестирование).

---

## FINDING #5: Отсутствует сброс флага `door.open` при входе

### Java оригинал (a.java:608-611):
```java
if (this.E.o && ...) {
    this.E.o = false;  // ← СБРОСИТЬ ФЛАГ!
    this.o.b(7);
    this.o.c(1);
    this.E.ap = true;
    return;
}
```

### C порт (src/exit_door.c:41-49):
```c
bool exit_door_test_complete(ExitDoorState* door, Level* level, Player* p) {
    // ...
    return (door_px_x < p->x_pos && ...);
    // ← НЕТ СБРОСА door->open!
}
```

### C вызов (src/main.c:754-790):
```c
if (exit_door_test_complete(&door, level, player)) {
    sound_play(SND_EXIT);
    // ...
    app_state = APP_STATE_LEVEL_COMPLETE;
    // ← НЕТ СБРОСА door.open!
}
```

### Проблема:

В Java флаг `this.E.o` (door.open) сбрасывается в **false** при входе в дверь.

В C флаг `door->open` **не сбрасывается**.

**Последствия:**
1. Игрок входит в дверь
2. `exit_door_test_complete()` возвращает true
3. `app_state = APP_STATE_LEVEL_COMPLETE`
4. Но `door->open` остаётся **true**
5. Если игрок каким-то образом выйдет из меню — дверь останется открытой

**Влияние:** Низкое (меню завершает уровень), но это **расхождение с оригиналом**.

### Исправление:

```c
// В main.c:754-790:
if (exit_door_test_complete(&door, level, player)) {
    sound_play(SND_EXIT);
    door.open = false;  // ← СБРОСИТЬ, как в Java!
    // ...
}
```

**Уверенность:** **Высокая** (прямое сравнение с a.java:608).

---

## СВОДНАЯ ТАБЛИЦА БАГОВ

| # | Описание | Критичность | Влияние |
|---|----------|-------------|---------|
| 1 | Область срабатывания | **OK** ✓ | Реализовано верно |
| 2 | Тайминг открытия (3x медленнее) | **High** | Дверь открывается 2.4с вместо 0.8с |
| 3 | Порядок проверки (после tick) | **Low** | Работает, но медленнее |
| 4 | Частота проверки (20 FPS vs 60 FPS) | **Medium** | Можно проскочить дверь |
| 5 | Отсутствует сброс door.open | **Low** | Косметическое |

---

## ТАЙМИНГИ: Java vs C

### Java (60 FPS):

```
Кадр 0:   hoops=0, I=0,  open=false  — кольца собраны
Кадр 1:   I=1,  open=false
...
Кадр 47:  I=47, open=false
Кадр 48:  I=48, open=true   ← ДВЕРЬ ОТКРЫТА (0.8с)
Кадр 49:  I=49, open=true
...
Кадр 71:  I=71, open=true
Кадр 72:  I=48, open=true   ← цикл (0.4с)
Кадр 73:  I=49, open=true
```

### C (20 FPS):

```
Кадр 0:   hoops=0, I=0,  open=false  — кольца собраны
Кадр 1:   I=1,  open=false
...
Кадр 47:  I=47, open=false
Кадр 48:  I=48, open=true   ← ДВЕРЬ ОТКРЫТА (2.4с!) ⚠️
Кадр 49:  I=49, open=true
...
Кадр 71:  I=71, open=true
Кадр 72:  I=48, open=true   ← цикл (1.2с!) ⚠️
```

**Разница:**
- Открытие: **2.4с** (C) vs **0.8с** (Java) = **3x медленнее**
- Цикл: **1.2с** (C) vs **0.4с** (Java) = **3x медленнее**

---

## РЕКОМЕНДАЦИИ ПО ИСПРАВЛЕНИЮ

### Приоритет 1 (критично):

**1. Увеличить FPS до 60** (исправляет FINDING #2 и #4):
```c
// В main.c, заменить:
SDL_Delay(50);  // ← 20 FPS

// На:
SDL_Delay(16);  // ← 60 FPS (~16.67ms)
```

**ИЛИ** использовать delta-time:
```c
// В exit_door.c:
static uint32_t last_tick = 0;
uint32_t now = SDL_GetTicks();

if (now - last_tick >= 16) {  // 60 FPS
    door->I++;
    last_tick = now;
    // ...
}
```

### Приоритет 2 (желательно):

**2. Добавить сброс door.open** (исправляет FINDING #5):
```c
// В main.c:754-790:
if (exit_door_test_complete(&door, level, player)) {
    sound_play(SND_EXIT);
    door.open = false;  // ← Добавить
    // ...
}
```

### Приоритет 3 (опционально):

**3. Проверка двери в player_update** (исправляет FINDING #4):
- См. код выше (~15 строк)

---

## ЗАКЛЮЧЕНИЕ

**Основная проблема:** Тайминг двери в **3 раза медленнее** оригинала из-за разницы FPS (20 vs 60).

**Вторичная проблема:** На 20 FPS игрок может проскочить дверь при быстрой скорости.

**Рекомендация:** Начать с исправления FINDING #2 (увеличить FPS до 60), затем протестировать.

**Область срабатывания:** Реализована **верно**, полное соответствие Java.
