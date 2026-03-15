# STEP 07: Анимация тайлов

## Источники

- **g.java:296-312** (tick animation update)
- **g.java:190-204** (animation loading from /res/tf)
- **DEOBFUSCATION.md** § 3.3 (Level g.java, animation fields)

---

## 1. Концепт

Тайлы с `renderType=3` (tileType в коде) используют **анимационные группы**.  
Каждая группа имеет:
- Массив кадров (tileId для каждого кадра)
- Период (сколько тиков на один кадр)
- Текущий индекс кадра
- Таймер до смены кадра

Каждый тик (50ms) таймер декрементируется, при 0 → следующий кадр, сброс таймера.

---

## 2. Формат данных (/res/tf, объект 0)

### 2.1 Заголовок анимаций (g.java:190-204)

```java
this.U = dataInputStream.readByte();  // animCount
if (this.U > 0) {
  this.m = new byte[this.U][];        // animFrames[groupId][frameIndex]
  this.O = new byte[this.U];          // animPeriod[groupId]
  this.ai = new byte[this.U];         // animFrameIndex[groupId] - текущий кадр
  this.aa = new byte[this.U];         // animTimer[groupId] - таймер до смены
  
  for (byte b4 = 0; b4 < this.U; b4++) {
    dataInputStream.readByte();       // reserved/unused
    this.O[b4] = dataInputStream.readByte();  // period
    byte b5 = dataInputStream.readByte();     // frameCount
    this.m[b4] = new byte[b5];
    this.aa[b4] = this.O[b4];         // init timer = period
    
    for (byte b6 = 0; b6 < b5; b6++)
      this.m[b4][b6] = dataInputStream.readByte(); // tileId
  }
}
```

**Формат:**
```
[1 byte] animCount
Для каждой группы (0..animCount-1):
  [1 byte] reserved (пропустить)
  [1 byte] period (тиков на кадр)
  [1 byte] frameCount
  [frameCount bytes] tileId для каждого кадра
```

### 2.2 Связь с тайлами (g.java:233, aux field)

```java
this.af[b4] = dataInputStream.readInt();  // aux
```

Для `renderType=3`: `aux` = индекс анимационной группы (0..animCount-1).

---

## 3. Логика обновления (g.java:296-308)

```java
public void d() {
  if (this.U != 0)
    for (byte b = 0; b < this.U; b++) {
      this.aa[b] = (byte)(this.aa[b] - 1);      // decrement timer
      if (this.aa[b] == 0) {                    // timer expired
        this.aa[b] = this.O[b];                 // reset timer = period
        this.ai[b] = (byte)(this.ai[b] + 1);    // next frame
        if (this.ai[b] == (this.m[b]).length)   // wrap around
          this.ai[b] = 0;
      }
    }
  // ... camera/render logic
}
```

**Правило:** Каждый тик вызывать `level_tick()` ДО рендеринга.

---

## 4. Получение текущего tileId для анимации

При рендеринге тайла с `renderType=3`:
```c
if (tile_meta[tileId].render_type == 3) {
    int anim_group = tile_meta[tileId].aux;
    int frame_index = anim_state->frame_index[anim_group];
    int actual_tile = anim_state->frames[anim_group][frame_index];
    // Рисовать texture для actual_tile
}
```

---

## 5. Реализация для PSP

### 5.1 Файлы

- [ ] `src/tile_animation.h` — структура TileAnimation, функции load/tick/get_tile
- [ ] `src/tile_animation.c` — реализация
- [ ] `src/main.c` — загрузить TileAnimation, вызывать tick каждый кадр
- [ ] `src/level_renderer.c` — использовать animation_get_tile при рендеринге

### 5.2 TileAnimation структура

```c
typedef struct {
    uint8_t count;              // animCount (U in g.java)
    uint8_t** frames;           // animFrames[group][frame] (m)
    uint8_t* period;            // animPeriod[group] (O)
    uint8_t* frame_index;       // animFrameIndex[group] (ai)
    uint8_t* timer;             // animTimer[group] (aa)
    uint8_t* frame_counts;      // frameCount[group] (для bounds)
} TileAnimation;

TileAnimation* animation_load(const char* tf_path);
void animation_free(TileAnimation* anim);
void animation_tick(TileAnimation* anim);
uint8_t animation_get_tile(TileAnimation* anim, TileMetadata* tile_meta, uint8_t tile_id);
```

### 5.3 Парсинг (animation_load)

Открыть `/res/tf` объект 0, пропустить заголовок уровня (g.java:151-166), прочитать анимации (g.java:190-204).

**ВАЖНО:** Использовать ТОТ ЖЕ путь "res/tf" как в tilemetadata_load/collision_masks_load.

### 5.4 Обновление (animation_tick)

```c
void animation_tick(TileAnimation* anim) {
    if (!anim || anim->count == 0) return;
    
    for (int i = 0; i < anim->count; i++) {
        anim->timer[i]--;
        if (anim->timer[i] == 0) {
            anim->timer[i] = anim->period[i];
            anim->frame_index[i]++;
            if (anim->frame_index[i] >= anim->frame_counts[i]) {
                anim->frame_index[i] = 0;
            }
        }
    }
}
```

### 5.5 Получение tileId (animation_get_tile)

```c
uint8_t animation_get_tile(TileAnimation* anim, TileMetadata* tile_meta, uint8_t tile_id) {
    if (!anim || !tile_meta) return tile_id;
    
    TileMetadata* tm = &tile_meta[tile_id];
    if (tm->render_type != 3) return tile_id;  // не анимация
    
    int group = tm->aux;
    if (group < 0 || group >= anim->count) return tile_id;
    
    int frame = anim->frame_index[group];
    if (frame < 0 || frame >= anim->frame_counts[group]) return tile_id;
    
    return anim->frames[group][frame];
}
```

---

## 6. Пошаговая интеграция

### 1. Создать tile_animation.h/c

- [ ] `tile_animation.h`: Объявить TileAnimation struct, функции
- [ ] `tile_animation.c`: Реализовать animation_load (парсинг из res/tf)
  - Пропустить заголовок (14 байт, как в tilemetadata_load)
  - Прочитать animCount
  - Для каждой группы: reserved, period, frameCount, frames
  - Инициализировать timer = period, frame_index = 0
- [ ] Реализовать animation_tick (g.java:296-308)
- [ ] Реализовать animation_get_tile

### 2. Обновить main.c

- [ ] После tilemetadata_load: `TileAnimation* tile_anim = animation_load("res/tf");`
- [ ] Проверить на NULL, cleanup при ошибке
- [ ] Передать tile_anim в renderer_create
- [ ] В игровом цикле ПЕРЕД рендерингом: `animation_tick(tile_anim);`
- [ ] При выходе: `animation_free(tile_anim);`

### 3. Обновить level_renderer.h/c

- [ ] `level_renderer.h`: Добавить поле `TileAnimation* tile_anim` в LevelRenderer
- [ ] `renderer_create`: Принимать TileAnimation*, сохранить в структуре
- [ ] `renderer_draw`: При рендеринге тайла:
  ```c
  uint8_t display_tile = animation_get_tile(lr->tile_anim, lr->tile_meta, tile_id);
  // Рисовать texture для display_tile вместо tile_id
  ```

### 4. Makefile

- [ ] Добавить `src/tile_animation.c` в OBJS

### 5. Тесты

- [ ] Запустить уровень с анимированными тайлами
- [ ] Видеть смену кадров анимации (вода, лава, etc)
- [ ] debug.log: "anim_group=0 frame=2" при смене кадра
- [ ] Проверить что анимация зациклена (frame wraps)

---

## Критерии успеха

✅ Анимированные тайлы меняют кадры каждые N тиков  
✅ Период анимации соответствует оригиналу  
✅ Анимация зациклена (возврат к кадру 0)  
✅ Не-анимированные тайлы не затронуты  
✅ debug.log показывает "frame_index=..." при смене

---

## Примечания

- Анимация тайлов работает НЕЗАВИСИМО от FPS (20 FPS = 50ms tick)
- В оригинале g.java:296 вызывается каждый тик ПЕРЕД рендером
- `renderType=3` в tile_metadata.c уже считывается (поле `render_type`)
- `aux` в tile_metadata.c уже считывается (поле `aux`)
- НЕ путать с анимацией спрайта мяча (будет в Step 8)

**Источники проверены:** ✅ g.java:190-204, 296-308, 233
