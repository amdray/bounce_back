# ID Characteristics Table

Одна строка = один `tileId`.  
Этот файл — рабочий реестр: добавляем новые столбцы по мере анализа.

## LF map coordinates (storage order)
- Тайлы в `lf` идут по порядку строк (row-major), без пропусков.
- После `height` и `width` идёт массив из `height * width` байт.
- Формулы:
  - `index = y * width + x`
  - `x = index % width`
  - `y = index // width`
- В каждом `tile_byte`:
  - `id = tile_byte & 0x7F`
  - `flag80 = tile_byte & 0x80`

| tileId | name | collision_note | texture_source | transform_source | collision_source | flag80_ever | notes | parts_count | lf_total_count | lf_levels_if_le5 | mechanic | map_p1_map_only_texture_mode | java_map_pass_rotation_disabled |
|---:|---|---|---|---|---|---|---|---:|---:|---|---|---|---|
| 0 | пустота | пусто | - | - | l=0, af=0 | yes |  | 1 | 44255 |  | case 0 | as_is | no |
| 1 | невидимый блок | полный блок | - | - | l=2, af=0 | no |  | 1 | 0 |  | case 1 | - | no |
| 2 | резиновый блок |  | - | - | l=2, af=0 | no |  | 1 | 191 |  | case 2 | as_is | no |
| 3 |  |  | T=1 | - | l=1, af=0 | no |  | 1 | 4 | 4,8,14 | case 3 | as_is | no |
| 4 |  |  | T=53 | b=0x01 | l=3, af=3 | no | render+collision | 1 | 4 | 4,8,14 | case 4 | with_rotation | yes |
| 5 |  |  | T=54 | b=0x02 | l=3, af=3 | no | render+collision | 1 | 1 | 14 | case 5 | with_rotation | yes |
| 6 |  |  | T=55 | b=0x03 | l=3, af=3 | no | render+collision | 1 | 2 | 14 | case 6 | with_rotation | yes |
| 7 |  |  | T=2 | - | l=2, af=0 | no |  | 1 | 9 |  |  | as_is | no |
| 8 |  |  | T=56 | b=0x01 | l=2, af=0 | no | render_only | 1 | 36 |  |  | with_rotation | yes |
| 9 |  |  | T=3 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 10 |  |  | T=4 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 11 |  |  | T=3 | - | l=2, af=0 | yes |  | 1 | 25 |  | case 11 | as_is | no |
| 12 |  |  | T=5 | - | l=2, af=0 | yes |  | 1 | 23 |  | case 12 | as_is | no |
| 13 |  |  | T=6 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 14 |  |  | T=7 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 15 |  |  | T=6 | - | l=2, af=1 | no |  | 1 | 3 | 17,19,20 | case 15 | as_is | no |
| 16 |  |  | T=8 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 17 |  |  | T=9 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 18 |  |  | T=8 | - | l=2, af=2 | yes |  | 1 | 29 |  | case 18 | as_is | no |
| 19 |  |  | T=10 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 20 |  |  | T=11 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 21 |  |  | T=12 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 22 |  |  | T=10 | - | l=2, af=3 | no |  | 1 | 7 |  | case 22 | as_is | no |
| 23 |  |  | T=13 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 24 |  |  | T=14 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 25 |  |  | T=15 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 26 |  |  | T=13 | - | l=2, af=4 | no |  | 1 | 6 |  | case 26 | as_is | no |
| 27 |  |  | T=16 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 28 |  |  | T=17 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 29 |  |  | T=18 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 30 |  |  | T=16 | - | l=2, af=5 | yes |  | 1 | 63 |  | case 30 | as_is | no |
| 31 |  |  | T=19 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 32 |  |  | T=20 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 33 |  |  | T=21 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 34 |  |  | T=19 | - | l=2, af=6 | yes |  | 1 | 80 |  | case 34 | as_is | no |
| 35 |  |  | T=22 | - | l=0, af=0 | no |  | 1 | 0 |  |  | - | no |
| 36 |  |  | T=23 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 37 |  |  | T=24 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 38 |  |  | T=25 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 39 |  |  | T=23 | - | l=2, af=7 | no |  | 1 | 3 | 12,15,16 | case 39 | as_is | no |
| 40 |  |  | T=26 | - | l=0, af=0 | no |  | 1 | 0 |  |  | - | no |
| 41 |  |  | T=27 | - | l=0, af=0 | no |  | 1 | 0 |  |  | - | no |
| 42 |  |  | T=28 | - | l=0, af=0 | no |  | 1 | 0 |  |  | - | no |
| 43 |  |  | T=26 | - | l=0, af=8 | no |  | 1 | 8 |  |  | as_is | no |
| 44 |  |  | T=57 | b=0x08 | l=0, af=9 | no | render_only | 1 | 4 | 17,18 |  | with_rotation | yes |
| 45 |  |  | T=60 | b=0x03 | l=0, af=10 | no | render_only | 1 | 24 |  |  | with_rotation | yes |
| 46 |  |  | T=29 | - | l=0, af=0 | no |  | 1 | 0 |  |  | - | no |
| 47 |  |  | T=30 | - | l=0, af=0 | no |  | 1 | 0 |  |  | - | no |
| 48 |  |  | T=31 | - | l=0, af=0 | no |  | 1 | 0 |  |  | - | no |
| 49 |  |  | T=29 | - | l=0, af=11 | no |  | 1 | 6 |  |  | as_is | no |
| 50 |  |  | T=63 | b=0x08 | l=0, af=12 | no | render_only | 1 | 0 |  |  | - | no |
| 51 |  |  | T=66 | b=0x03 | l=0, af=13 | no | render_only | 1 | 4 | 5,20 |  | with_rotation | yes |
| 52 |  |  | T=32 | - | l=1, af=0 | no |  | 1 | 3 | 5,17 | case 52 | - | no |
| 53 |  |  | T=69 | b=0x03 | l=3, af=52 | no | render+collision | 1 | 2 | 5,20 | case 53 | - | yes |
| 54 |  |  | T=33 | - | l=1, af=0 | no |  | 1 | 3 | 5,17 | case 54 | - | no |
| 55 |  |  | T=70 | b=0x03 | l=3, af=54 | no | render+collision | 1 | 2 | 5,20 | case 55 | - | yes |
| 56 |  |  | T=34 | - | l=1, af=0 | no |  | 1 | 4 | 5,17 |  | - | no |
| 57 |  |  | T=71 | b=0x03 | l=3, af=56 | no | render+collision | 1 | 12 |  |  | - | yes |
| 58 |  |  | T=72 | b=0x02 | l=3, af=56 | no | render+collision | 1 | 2 | 17,18 |  | - | yes |
| 59 |  |  | T=35 | - | l=1, af=0 | no |  | 1 | 4 | 5,17 |  | - | no |
| 60 |  |  | T=73 | b=0x03 | l=3, af=59 | no | render+collision | 1 | 12 |  |  | - | yes |
| 61 |  |  | T=74 | b=0x02 | l=3, af=59 | no | render+collision | 1 | 2 | 17,18 |  | - | yes |
| 62 |  |  | T=36 | - | l=2, af=0 | yes |  | 1 | 43 |  | case 62 | as_is | no |
| 63 |  |  | T=75 | b=0x04 | l=2, af=0 | no | render_only | 1 | 26 |  | case 63 | with_rotation | yes |
| 64 |  |  | T=76 | b=0x01 | l=2, af=0 | yes | render_only | 1 | 16 |  | case 64 | with_rotation | yes |
| 65 |  |  | T=77 | b=0x03 | l=2, af=0 | yes | render_only | 1 | 16 |  | case 65 | with_rotation | yes |
| 66 |  |  | T=37 | - | l=1, af=0 | no |  | 1 | 14 |  |  | - | no |
| 67 |  |  | T=78 | b=0x03 | l=3, af=66 | no | render+collision | 1 | 15 |  |  | - | yes |
| 68 |  |  | T=79 | b=0x02 | l=3, af=66 | no | render+collision | 1 | 2 | 17,18 |  | - | yes |
| 69 |  |  | T=38 | - | l=1, af=0 | no |  | 1 | 14 |  |  | - | no |
| 70 |  |  | T=80 | b=0x01 | l=3, af=69 | no | render+collision | 1 | 0 |  |  | - | no |
| 71 |  |  | T=81 | b=0x03 | l=3, af=69 | no | render+collision | 1 | 15 |  |  | - | yes |
| 72 |  |  | T=82 | b=0x02 | l=3, af=69 | no | render+collision | 1 | 2 | 17,18 |  | - | yes |
| 73 |  |  | T=39 | - | l=0, af=0 | no |  | 1 | 14 |  |  | as_is | no |
| 74 |  |  | T=83 | b=0x01 | l=0, af=0 | no | render_only | 1 | 0 |  |  | - | no |
| 75 |  |  | T=84 | b=0x03 | l=0, af=0 | no | render_only | 1 | 28 |  |  | with_rotation | yes |
| 76 |  |  | T=85 | b=0x02 | l=0, af=0 | no | render_only | 1 | 4 | 17,18 |  | with_rotation | yes |
| 77 |  |  | T=40 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 78 |  |  | T=41 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 79 |  |  | T=40 | - | l=2, af=14 | no |  | 1 | 9 |  |  | as_is | no |
| 80 |  |  | T=42 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 81 |  |  | T=43 | - | l=2, af=0 | no |  | 1 | 0 |  |  | - | no |
| 82 |  |  | T=42 | - | l=2, af=15 | no |  | 1 | 52 |  |  | as_is | no |
| 83 |  |  | T=86 | b=0x08 | l=2, af=16 | no | render_only | 1 | 36 |  |  | with_rotation | yes |
| 84 |  |  | T=88 | b=0x08 | l=2, af=17 | no | render_only | 1 | 5 | 11,17,19 |  | with_rotation | yes |
| 85 |  |  | T=44 | - | l=2, af=0 | yes |  | 1 | 140 |  | case 85 | as_is | no |
| 86 |  |  | T=90 | b=0x04 | l=2, af=0 | yes | render_only | 1 | 112 |  | case 86 | with_rotation | yes |
| 87 |  |  | T=91 | b=0x01 | l=2, af=0 | yes | render_only | 1 | 37 |  | case 87 | with_rotation | yes |
| 88 |  |  | T=92 | b=0x03 | l=2, af=0 | yes | render_only | 1 | 37 |  | case 88 | with_rotation | yes |
| 89 |  |  | T=45 | - | l=2, af=0 | yes |  | 1 | 43 |  | case 89 | as_is | no |
| 90 |  |  | T=93 | b=0x04 | l=2, af=0 | no | render_only | 1 | 26 |  | case 90 | with_rotation | yes |
| 91 |  |  | T=94 | b=0x01 | l=2, af=0 | yes | render_only | 1 | 16 |  | case 91 | with_rotation | yes |
| 92 |  |  | T=95 | b=0x03 | l=2, af=0 | yes | render_only | 1 | 16 |  | case 92 | with_rotation | yes |
| 93 |  |  | T=46 | - | l=1, af=0 | yes |  | 2 | 89 |  | case 93 | - | no |
| 94 |  |  | T=96 | b=0x01 | l=3, af=93 | yes | render+collision | 2 | 60 |  | case 94 | - | yes |
| 95 |  |  | T=47 | - | l=1, af=0 | no |  | 2 | 0 |  | case 95 | - | no |
| 96 |  |  | T=97 | b=0x01 | l=3, af=95 | no | render+collision | 2 | 0 |  | case 96 | - | no |
| 97 |  |  | T=48 | - | l=1, af=0 | no |  | 3 | 77 |  | case 97 | - | no |
| 98 |  |  | T=98 | b=0x04 | l=3, af=97 | no | render+collision | 1 | 77 |  | case 98 | - | yes |
| 99 |  |  | T=49 | - | l=1, af=0 | no |  | 3 | 0 |  | case 99 | - | no |
| 100 |  |  | T=99 | b=0x04 | l=3, af=99 | no | render+collision | 1 | 0 |  | case 100 | - | no |
| 101 |  |  | T=100 | b=0x03 | l=3, af=97 | yes | render+collision | 3 | 48 |  | case 101 | - | yes |
| 102 |  |  | T=101 | b=0x05 | l=3, af=97 | yes | render+collision | 1 | 48 |  | case 102 | - | yes |
| 103 |  |  | T=102 | b=0x03 | l=3, af=99 | no | render+collision | 3 | 0 |  | case 103 | - | no |
| 104 |  |  | T=103 | b=0x05 | l=3, af=99 | no | render+collision | 1 | 0 |  | case 104 | - | no |
| 105 |  |  | T=50 | - | l=0, af=0 | no |  | 1 | 0 |  |  | - | no |
| 106 |  |  | T=51 | - | l=0, af=0 | no |  | 1 | 0 |  |  | - | no |
| 107 |  |  | T=52 | - | l=0, af=0 | no |  | 1 | 0 |  |  | - | no |
| 108 |  |  | T=40 | - | l=2, af=18 | no |  | 1 | 5 | 11,17,19 |  | as_is | no |
| 109 |  |  | T=88 | b=0x08 | l=2, af=19 | no | render_only | 1 | 9 |  |  | with_rotation | yes |
| 110 | базовый статичный блок |  | T=104 | - | l=2, af=0 | yes |  | 1 | 5660 |  |  | as_is | no |
| 111 | крайний статичный блок |  | T=105 | - | l=2, af=0 | yes |  | 1 | 4313 |  |  | as_is | no |
| 112 | базовый стеновой блок |  | T=106 | - | l=2, af=0 | yes |  | 1 | 3280 |  |  | as_is | no |
| 113 | базовый блок ◢ |  | T=107 | - | l=1, af=0 | yes |  | 1 | 689 |  | case 113 | as_is | no |
| 114 |  |  | T=108 | b=0x01 | l=3, af=113 | yes | render+collision | 1 | 688 |  | case 114 | with_rotation | yes |
| 115 |  |  | T=109 | b=0x02 | l=3, af=113 | yes | render+collision | 1 | 605 |  | case 115 | with_rotation | yes |
| 116 |  |  | T=110 | b=0x03 | l=3, af=113 | yes | render+collision | 1 | 629 |  | case 116 | with_rotation | yes |


## Flag 0x80 meaning
- `flag80` = старший бит в `tile_byte`: `flag80 = tile_byte & 0x80`.
- Это не отдельный `tileId`: `id = tile_byte & 0x7F` остаётся тем же.
- В оригинале (`g.java`) этот флаг включает предварительную заливку клетки цветом `bgColor` из `tf` перед отрисовкой тайла.
- `flag80_ever`:
  - `yes` — для этого ID в картах хотя бы раз встречался байт с `0x80`.
  - `no` — для этого ID байт с `0x80` ни разу не встречался.



## collision_source values (примерная расшифровка)
- `l=0` -> коллизии нет. (tileId count: 21)
- `l=1` -> inline mask в этом же tileId (16x16 bool). (tileId count: 12)
- `l=2` -> полный solid-блок. (tileId count: 59)
- `l=3` -> mask alias; базовая маска берётся из `af`. (tileId count: 25)
- Всего tileId в таблице: 117
- `af` хранится всегда; для коллизии как alias используется при `l=3`.

## transform_source b values
b values: b=0x01, b=0x02, b=0x03, b=0x04, b=0x05, b=0x08
- b=0x01 -> rotate 270°
- b=0x02 -> rotate 180°
- b=0x03 -> rotate 90°
- b=0x04 -> flipY
- b=0x05 -> flipY + rotate 270°
- b=0x08 -> flipX

### transform_source references
- Nokia DirectGraphics Javadoc (constants and manipulation):
  https://nikita36078.github.io/J2ME_Docs/docs/Nokia_UI_API_1_1/com/nokia/mid/ui/DirectGraphics.html
- Nokia UI API constant values:
  https://www.j2megame.org/j2meapi/Nokia_UI_API_1_1/constant-values.html
- Original game code, transform table `p[]`:
  `original_code/bounce_back_s60.jar.src/g.java:135`
- Original game code, tile render uses `p[b]`:
  `original_code/bounce_back_s60.jar.src/h.java:745`
