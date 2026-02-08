# Java Transforms Reference (Original Game)

Дата: 2026-02-07  
Источник: `original_code/bounce_back_s60.jar.src/g.java`, `original_code/bounce_back_s60.jar.src/h.java`

## 1) Где в Java это живет

1. Таблица всех transform-значений:
- `original_code/bounce_back_s60.jar.src/g.java:134-136`

2. Применение transform в draw:
- `DirectGraphics.drawImage(image, x, y, anchor, manipulation)`
- вызовы в tile-engine: `original_code/bounce_back_s60.jar.src/g.java:620`, `original_code/bounce_back_s60.jar.src/g.java:632`
- вызовы в hoops overlay: `original_code/bounce_back_s60.jar.src/h.java:711`, `:717`, `:718`, `:724`, `:725`

3. Битовая семантика transform-байта в коллизии (из `tf`):
- `0x8` и `0x4` + `rot = b & 0x3`
- `original_code/bounce_back_s60.jar.src/g.java:402-419`

## 2) API DirectGraphics: что означает manipulation

В Java-версии используется Nokia API `com.nokia.mid.ui.DirectGraphics`:

- Метод: `drawImage(..., manipulation)`
- `manipulation` задает операцию над изображением: поворот и/или зеркалирование.

Практически по коду игры используются базовые API-состояния:

- `0` — без transform
- `90` — поворот на 90°
- `180` — поворот на 180°
- `270` — поворот на 270°
- `8192` — зеркалирование по X (горизонтальный flip)
- `16384` — зеркалирование по Y (вертикальный flip)

Комбинированные значения формируются сложением:
- `flip + rotate`, например `8462 = 8192 + 270`.

## 3) Полная таблица transform в оригинале (`g.p[]`)

Таблица из `g.java:134-136`:

`p = {0, 270, 180, 90, 16384, 16654, 16564, 16474, 8192, 8462, 8372, 8282}`

| index (tf transform) | manipulation (DirectGraphics) | Человекопонятно |
|---|---:|---|
| 0 | 0 | без transform |
| 1 | 270 | поворот 270° |
| 2 | 180 | поворот 180° |
| 3 | 90 | поворот 90° |
| 4 | 16384 | flipY |
| 5 | 16654 (=16384+270) | flipY + поворот 270° |
| 6 | 16564 (=16384+180) | flipY + поворот 180° |
| 7 | 16474 (=16384+90) | flipY + поворот 90° |
| 8 | 8192 | flipX |
| 9 | 8462 (=8192+270) | flipX + поворот 270° |
| 10 | 8372 (=8192+180) | flipX + поворот 180° |
| 11 | 8282 (=8192+90) | flipX + поворот 90° |

## 4) Какие значения реально видны в Java-коде игры

### 4.1 Через `g.p[transform]` (tile-engine)

- При рендере тайлов transform-индекс берется из `/res/tf` (`b[tileId]`) и мапится через `p[]`.
- См. `g.java:620`, `g.java:632`.

### 4.2 Хардкодом в hoops overlay (`h.a(...)`)

В `h.java` напрямую используются:

- `270` (`h.java:711`, `h.java:725`)
- `8192` (`h.java:717`)
- `180` (`h.java:718`)
- `8462` (`h.java:724`)

Человекопонятно:
- `270` = rot270
- `8192` = flipX
- `180` = rot180
- `8462` = flipX + rot270

## 5) Важное для порта

1. Java использует единый формат manipulation в `DirectGraphics.drawImage(...)`.
2. Значения `0..11` из `tf` не являются "произвольными" — это индексы в таблицу `g.p[]`.
3. Для точного переноса в SDL/PSP нужно сначала воспроизвести именно эту таблицу соответствий, а затем уже оптимизировать рендер.
