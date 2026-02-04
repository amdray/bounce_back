# Bounce Back — game loop spec (50 ms / 20 FPS)

Цель этого файла — зафиксировать **исполняемую модель** (что происходит каждый тик) как контракт порта на PSP/PSPSDK/SDL2. Это снижает риск «поплывшего» поведения при замене платформы/рендера/ввода/таймеров/ФС.

Источник: `bounce_back/original_code/bounce_back_s60.jar.src/h.java` (главным образом `run()`, `keyPressed()`, `keyReleased()`, `c()`), плюс `a.java` (inputMask) и `d.java` (demo playback).

Связанные контракты:
- Коллизии тайлов: [`COLLISION_CONTRACT.md`](COLLISION_CONTRACT.md)
- Общее именование/форматы ресурсов: [`DEOBFUSCATION.md`](DEOBFUSCATION.md)

---

## Контракт времени

- **Tick**: 50 ms (20 FPS).
- В оригинале tick запускается `java.util.Timer.scheduleAtFixedRate(...)` (см. `f.java`).
- **Единица симуляции**: один tick = один шаг физики/логики (без `dt` в формулах).

---

## Input pipeline (press/release → inputMask)

### 1) Сбор событий

`h.keyPressed/keyReleased` пишут в массив `h.b[]` значения:
- `0` — нет события,
- `1` — pressed,
- `2` — released.

Клавиши (не GUI-слой, а именно игровой ввод):
- `b[0]` — jump (бит `8` в `a.m`)
- `b[1]` — down (бит `4`)
- `b[2]` — left (бит `1`)
- `b[3]` — right (бит `2`)

### 2) Применение в начале tick

Каждый tick `h.c()` вызывает:

```java
a(0, 8);  // jump
a(1, 4);  // down
a(2, 1);  // left
a(3, 2);  // right
```

`h.a(slot, bit)` транслирует событие в `a.java`:
- `pressed`  → `Player.c(bit)` → `inputMask |= bit`
- `released` → `Player.b(bit)` → `inputMask &= ~bit`

Это важно для порта: **ввод дискретный (press/release), а не “current state”**, и применяется строго в начале tick.

---

## Demo playback (подмена ввода)

Если активен `h.au` (`d.java`), то **до** `h.c()` выполняется `au.a(h.b)` — реплей выставляет `b[0..3]` как `pressed/released` события на конкретном tick.

Доп. события демо:
- `actionCode=20` → `b[4]=1` (логика “выйти/закрыть демо”)
- `actionCode=21` → `b[5]=value` (триггерит `h.e()` в `h.c()`)

---

## Run() — порядок стадий tick (исполняемая модель)

`h.run()` содержит несколько “веток” (системные переходы), но **в нормальном тике** порядок такой:

1. **Demo inject**: `au.a(b)` (если `au != null`)
2. **Input stage**: `c()` → перевод `b[]` в `Player.inputMask`
3. **Enemy stage**: `d()` (только если `enemy_count != 0`)
4. **Player stage**: `this.e.d()` (физика/коллизии/подборы/смерть)
5. **Timers/tiles stage**: `g()` (декремент “временных” тайлов/эффектов)
6. **Camera + map tick**:
   - `Z.c(playerX, playerY); Z.d();`
   - `a(playerX, playerY, Z.J, Z.t);` (поддержка `ai/aq` для бэкграунда)
   - `A.c(ai, aq); A.d();`
7. **Render**: `repaint(); serviceRepaints();`

Системные ветки в начале `run()` (важно для state machine порта):
- `ap=true` — обработка Back/Exit: закрывает демо/выходит в меню/дергает `b[4]=1` если `aa!=null`.
- `ab=true` — загрузка уровня: `b(d)` (levelIndex).
- `V=true` — загрузка сейва из RMS: `b()` (внутри вызывает `b(levelIndex)`).
- `ad=true` — режим “ожидание первого нажатия” (в `keyPressed` первый key просто выключает `ad` и ресетит `t`).

---

## Таймеры/счетчики (привязаны к tick)

Минимально-важные (для совпадения поведения):
- `Player.bonusCounter` = 450 ticks (бонусы).
- Анимации тайлов: период 5 ticks (в `/res/tf`, `animGroup`).
- Enemy motion: скорость задается **в пикселях на tick** (см. `h.d()` и формат `/res/lf`).
