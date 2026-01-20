# Tile Types Used in Bounce Back Game

## Summary
- **Total tiles across all levels**: 61,848
- **Unique tile IDs**: 99
- **Tile size**: 16x16 pixels
- **Number of levels**: 22

## Flag 0x80
The flag 0x80 (bit 7 set) means "draw background color" before rendering the tile.
- **Implementation**: g.java:601-604
- **Behavior**: When 0x80 is set, the game fills a 16x16 rect with background color (this.i) before drawing the tile
- **Color value**: 0x7F000CFF (ARGB: semi-transparent dark blue/water color)
  - Default value: 2130706559 (g.java:28)
  - Can be overridden per level from data file (g.java:165)
- **Purpose**: Creates water/dark background areas instead of standard light blue sky background

---

## Collision System

The game implements different collision behaviors for different tile types (a.java:756-1632):

### 1. DEADLY TILES (Spikes/Hazards)
- **Tile IDs**: 1, 62, 63, 64, 65, 85, 86, 87, 88, 89, 90, 91, 92
- **Code**: a.java:764-787, 1589-1608
- **Behavior**:
  - Normal player → death `k()` function
  - Inverted player (this.F) → bounce off (velocity = 30)

### 2. SLOPE TILES (Diagonal Platforms)
- **Left/Up Slopes**: 3, 6, 52, 113, 116 (a.java:910-959)
- **Right/Up Slopes**: 4, 5, 114, 115 (a.java:960-1008)
- **Behavior**:
  - Bounce physics with velocity dampening
  - Converted to solid platforms based on player direction
  - Tiles 3, 4 → convert to 2 when moving
  - Tiles 113, 114, 115, 116 → convert to 110 based on velocity direction

### 3. SOLID PLATFORMS
- **Tile IDs**: 2, 110, 111, 112
- **Code**: a.java:1009-1019
- **Behavior**:
  - Solid collision, player slides along surface
  - Main level geometry blocks

### 4. HOOPS (Collectible Rings)
- **Tile 93**: Horizontal top (→ 0x5F after passing) - a.java:1553-1560
- **Tile 94, 96**: Vertical hoops - a.java:788-799
- **Tile 97, 98**: Horizontal bottom double (→ 0x63, 0x64) - a.java:1561-1582
- **Tile 99, 100**: Double hoop continuation
- **Tile 101, 102**: Vertical left double
- **Behavior**:
  - Pass-through collision at specific position
  - Changes tile ID after player passes through
  - Gives 500 points, decrements hoop counter

### 5. COLLECTIBLES (Point Items)
- **Tile IDs**: 12, 30, 34
- **Code**: a.java:1609-1613
- **Behavior**: Calls `b(i6, i7)` collection function
- **Points**:
  - Tile 12: 1,000 points
  - Tile 30: 2,500 points
  - Tile 34: 200 points

### 6. POWERUPS (Special Abilities)
- **Tile IDs**: 11, 15, 18, 22, 26, 39
- **Code**: a.java:1614-1620
- **Behavior**: Calls `c(i6, i7)` powerup activation function

### 7. SPECIAL COLLECTIBLES
- **Tile IDs**: 7, 8
- **Code**: a.java:1629-1630
- **Behavior**: Only collectible when player is inverted (this.F)
- **Function**: Calls `a(i6, i7)`

### 8. EMPTY/AIR
- **Tile ID**: 0
- **Behavior**: No collision, pass-through

---

## Game Loop & Timing

### Frame Rate & Ticks
- **Game loop interval**: 50 milliseconds (f.java:12, h.java:811)
- **Target frame rate**: 20 FPS (1000ms / 50ms = 20 frames per second)
- **Timer implementation**: `java.util.Timer` with fixed-rate scheduling (f.java:11-12)
- **Main game loop**: h.java:752-807 `run()` method
- **Frame counter**: Variable `O++` increments each frame (h.java:753)

### Game Loop Sequence (per tick)
Each 50ms tick executes:
1. Input handling: `c()` (h.java:793)
2. Game state update: `d()` if needed (h.java:794-795)
3. Player physics: `this.e.d()` (h.java:796)
4. Camera/viewport update: `g()` (h.java:797)
5. Viewport calculations: `this.Z.c()` and `this.Z.d()` (h.java:799-800)
6. Position adjustments: `a()` (h.java:801)
7. Additional calculations: `this.A.c()` and `this.A.d()` (h.java:802-803)
8. Screen repaint: `repaint()` and `serviceRepaints()` (h.java:805-806)

### Time Tracking
- **Current time**: `System.currentTimeMillis()` (h.java:812, 841, 856)
- **Delta time**: `this.g += System.currentTimeMillis() - this.t` (h.java:841)
- **Frame timestamp**: `this.t = System.currentTimeMillis()` (h.java:812, 856)

**Note**: All physics values (velocity, acceleration, etc.) are calculated per frame/tick, meaning they apply every 50ms.

---

## Level Data Structure (LF File)

Each level has two chunks in the LF file:
1. **Metadata chunk** (chunk 0, 2, 4, ...) - Level header and spawn info
2. **Tilemap chunk** (chunk 1, 3, 5, ...) - Tile data

### Metadata Chunk Structure (h.java:195-241)
- **Byte 0** (b1): Unknown parameter
- **Byte 1** (b2): **Player spawn Y** (in tiles, multiply by 16 for pixels)
- **Byte 2** (b3): **Player spawn X** (in tiles, multiply by 16 for pixels)
- **Byte 3** (b4): **Player type** (0 = small ball sprite 0, other = big ball sprite 11)
- **Byte 4** (ar): Unknown parameter
- **Byte 5** (D): Unknown parameter
- **Byte 6** (j): **Number of enemies/moving objects**
- **Bytes 7+**: Enemy/object data (if j > 0)

### Spawn Point Calculation (h.java:292-302)
```
pixel_x = b3 * 16 + offset
pixel_y = b2 * 16 + offset
```
Where offset depends on player type (b4):
- If b4 == 0: offset = 8 (sprite size ~16x16)
- If b4 != 0: offset = 12 (sprite size ~20x20)

### Player Sprites (a.java:104-109, 144-145)
Player ball sprite is selected based on b4 value:
- **b4 = 0**: Uses sprite index 0 (starts at ~16x16, grows during gameplay)
- **b4 != 0**: Uses sprite index 11 (starts at 20x20, larger ball)
- Ball size (width, height) is read from sprite image dimensions
- Total 25 player sprites with sizes ranging from 16x16 to 28x20

**Example (Level 0)**:
- b2 = 8, b3 = 1, b4 = 0
- Spawn: X = 1 * 16 + 8 = 24 pixels, Y = 8 * 16 + 8 = 136 pixels
- Ball: sprite 0, approximately 16x16 pixels

---

## Physics Constants

Game physics parameters extracted from a.java (all values per frame @ 20 FPS):

### Gravity & Velocity
- **Gravity (down)**: 7 units/frame (a.java:671)
- **Gravity (up, inverted)**: -6 units/frame (a.java:668)
- **Terminal velocity (normal)**: 20 units/frame (a.java:666)
- **Bounce velocity**: 30 units/frame (a.java:780, 783, etc.)
- **Maximum velocity**: 140 units/frame (i2 max = 14, * 10 = 140) (a.java:697-698)

### Horizontal Movement
- **Acceleration (normal)**: 18 units/frame (a.java:1397)
- **Acceleration (boosted)**: 22 units/frame (a.java:1397)
- **Max speed (normal)**: 60 units/frame (a.java:1400-1401)
- **Max speed (boosted)**: 100 units/frame (a.java:1398-1399)
- **Deceleration (grounded)**: 3 units/frame (a.java:1426, 1430)
- **Deceleration (airborne)**: 8 units/frame (a.java:1426, 1430)

### Special Tiles Physics
- **Tiles 45, 51, 53, 67, 71**: +/- 100 velocity boost (a.java:685-687)
- **Tile 75**: +/- 50 velocity boost (a.java:685, 687)
- **Tiles 43, 49, 52, 54, 66, 69, 73**: Force horizontal velocity to 250 (a.java:1419)
- **Tiles 44, 50, 76**: Force horizontal velocity to -250/-125 (a.java:1417)

### Slope Physics
- **Velocity dampening**: velocity = velocity * 3/4 (a.java:935, 984)
- **Minimum bounce threshold**: 30 units (a.java:938, 987)
- **Slope entry threshold**: 80 units (a.java:917, 966)
- **Slope soft entry**: 40 units (a.java:925, 974)

---

## Base Tile IDs (sorted by ID)

### ID 0 (0x00) - Empty/Air
- **Total usage**: 44,255 times (67.45% of all tiles)
- **Variants**:
  - plain 0x00 (41,717) - empty space with standard sky background
  - flagged 0x80 (2,538) - empty space with dark blue/water background color
- Most common tile - represents empty space
- **Color**: When 0x80 flag is set, renders dark blue background (0x7F000CFF)

### ID 2 (0x02)
- **Total usage**: 191 times
- **Code reference**: Converts to ID 2 in certain conditions (a.java:901-907)

### ID 3 (0x03)
- **Total usage**: 4 times
- **Code reference**: Slope tile (a.java:910-959)

### ID 4 (0x04)
- **Total usage**: 4 times
- **Code reference**: Slope tile (a.java:960-1008)

### ID 5 (0x05)
- **Total usage**: 1 time

### ID 6 (0x06)
- **Total usage**: 2 times
- **Code reference**: Slope tile variant (a.java:911, 1246)

### ID 7 (0x07) - Collectible Item
- **Total usage**: 9 times
- **Code reference**: Saved to game state (h.java:458, 542)

### ID 8 (0x08) - Collectible Item
- **Total usage**: 36 times
- **Code reference**: Saved to game state (h.java:459, 543)

### ID 11 (0x0B) - Powerup/Switch
- **Total usage**: 25 times (22 plain + 3 flagged)
- **Code reference**: Trigger when player vertical (a.java:522-527, 885-891)

### ID 12 (0x0C) - Collectible (1000 points)
- **Total usage**: 23 times (21 plain + 2 flagged)
- **Code reference**: Gives 1000 points (a.java:334-342, 880-884)
- **Saved**: In game state (h.java:460, 544)

### ID 15 (0x0F) - Powerup
- **Total usage**: 3 times
- **Code reference**: Powerup activation (a.java:497-505, 886-891)

### ID 18 (0x12) - Powerup/Switch
- **Total usage**: 29 times (17 plain + 12 flagged)
- **Code reference**: Trigger when player not inverted (a.java:515-520, 887-891)

### ID 22 (0x16) - Powerup
- **Total usage**: 7 times
- **Code reference**: Powerup activation (a.java:506-514, 888-891)

### ID 26 (0x1A) - Powerup
- **Total usage**: 6 times
- **Code reference**: Powerup activation (a.java:489-495, 889-891)

### ID 30 (0x1E) - Collectible (2500 points)
- **Total usage**: 63 times (57 plain + 6 flagged)
- **Code reference**: Gives 2500 points (a.java:344-348, 881-884)
- **Saved**: In game state (h.java:461, 545)

### ID 34 (0x22) - Collectible (200 points)
- **Total usage**: 80 times (78 plain + 2 flagged)
- **Code reference**: Gives 200 points (a.java:322-332, 882-884)
- **Saved**: In game state (h.java:462, 546)

### ID 39 (0x27) - Powerup
- **Total usage**: 3 times
- **Code reference**: Powerup activation (a.java:481-487, 890-891)

### ID 43-45 (0x2B-0x2D)
- **Total usage**: 36 times combined

### ID 49-61 (0x31-0x3D)
- **Total usage**: Various (2-12 times each)

### ID 62 (0x3E)
- **Total usage**: 43 times (41 plain + 2 flagged)
- **Code reference**: Collision tile (a.java:764-787)

### ID 63 (0x3F)
- **Total usage**: 26 times
- **Code reference**: Collision tile (a.java:765)

### ID 64-65 (0x40-0x41)
- **Total usage**: 16 times each (10 plain + 6 flagged each)
- **Code reference**: Collision tiles (a.java:766-767)

### ID 66-73 (0x42-0x49)
- **Total usage**: 2-15 times each

### ID 75 (0x4B)
- **Total usage**: 28 times

### ID 76 (0x4C)
- **Total usage**: 4 times

### ID 79 (0x4F)
- **Total usage**: 9 times

### ID 82-84 (0x52-0x54)
- **Total usage**: 5-52 times each

### ID 85-92 (0x55-0x5C) - Collision Tiles
- **Total usage**: 140, 112, 37, 37, 43, 26, 16, 16 times respectively
- **Code reference**: Collision/spikes (a.java:768-775)
- Some have flagged variants

### ID 93 (0x5D) - Hoop (Horizontal Top)
- **Total usage**: 89 times (82 plain + 7 flagged)
- **Code reference**: Jump-through hoop (a.java:350-357, 1553-1628)
- **Render**: Special rendering (h.java:694-703)
- **Saved**: In game state (h.java:463-479, 547-565)

### ID 94 (0x5E) - Hoop (Vertical Right)
- **Total usage**: 60 times (52 plain + 8 flagged)
- **Code reference**: Jump-through hoop (a.java:350-357, 788-812)
- **Render**: Rotated 270° (h.java:705-712)
- **Saved**: In game state (h.java:464-479, 548-565)

### ID 97 (0x61) - Hoop (Horizontal Bottom, Double)
- **Total usage**: 77 times
- **Code reference**: Gives 500 points, decrements hoop counter (a.java:352-357, 1567-1608)
- **Render**: Flipped vertically, drawn twice (h.java:714-719)

### ID 98 (0x62) - Hoop (Horizontal Bottom, Double)
- **Total usage**: 77 times
- **Code reference**: Hoop variant (a.java:353-357)

### ID 101 (0x65) - Hoop (Vertical Left, Double)
- **Total usage**: 48 times (45 plain + 3 flagged)
- **Code reference**: Gives 500 points, decrements hoop counter (a.java:354-357, 705, 814-878)
- **Render**: Mirrored and rotated, drawn twice (h.java:720-726)
- **Saved**: In game state (h.java:471-479, 556-565)

### ID 102 (0x66) - Hoop (Vertical Left, Double)
- **Total usage**: 48 times (45 plain + 3 flagged)
- **Code reference**: Hoop variant (a.java:355-357, 705, 820-878)
- **Saved**: In game state (h.java:472-479, 557-565)

### ID 108-109 (0x6C-0x6D)
- **Total usage**: 5 and 9 times

### ID 110 (0x6E) - Platform/Ground
- **Total usage**: 5,660 times (5,562 plain + 98 flagged) - **2nd most common tile**
- **Code reference**: Converts to ID 110 from other IDs (a.java:902, 907, 1235, 1240)
- 8.99% of all tiles

### ID 111 (0x6F) - Platform/Ground
- **Total usage**: 4,313 times (4,193 plain + 120 flagged) - **3rd most common tile**
- 6.78% of all tiles

### ID 112 (0x70) - Platform/Ground
- **Total usage**: 3,280 times (3,250 plain + 30 flagged) - **4th most common tile**
- 5.25% of all tiles

### ID 113 (0x71) - Slope/Platform
- **Total usage**: 689 times (626 plain + 63 flagged)
- **Code reference**: Slope tile, converts to 110 (a.java:913, 1234, 1247)
- 1.01% of all tiles

### ID 114 (0x72) - Slope/Platform
- **Total usage**: 688 times (600 plain + 88 flagged)
- **Code reference**: Slope tile, converts to 110 (a.java:906, 962, 1234)
- 0.97% of all tiles

### ID 115 (0x73) - Slope/Platform
- **Total usage**: 605 times (584 plain + 21 flagged)
- **Code reference**: Slope tile, converts to 110 (a.java:901, 963, 1239)
- 0.94% of all tiles

### ID 116 (0x74) - Slope/Platform
- **Total usage**: 629 times (606 plain + 23 flagged)
- **Code reference**: Slope tile, converts to 110 (a.java:901, 914, 1239, 1248)
- 0.98% of all tiles

---

## Special Tile Categories

### Collectibles (Point Items)
- **ID 12**: 1,000 points
- **ID 30**: 2,500 points
- **ID 34**: 200 points

### Hoops (Jump-through objectives)
- **ID 93**: Horizontal top hoop
- **ID 94**: Vertical right hoop
- **ID 97-98**: Horizontal bottom double hoops (500 points each)
- **ID 101-102**: Vertical left double hoops (500 points each)

### Platforms/Ground (Most common solid tiles)
- **ID 110-112**: Main platform tiles (19,253 total occurrences)
- **ID 113-116**: Slope/angled platform tiles (2,611 total occurrences)

### Collision/Hazards
- **ID 62-65**: Collision tiles
- **ID 85-92**: Collision/spike tiles

### Powerups/Switches
- **ID 11, 15, 18, 22, 26, 39**: Various powerup/trigger tiles

### Saved Items (persist in game state)
- **ID 7, 8, 12, 30, 34, 93-104**: These are tracked in save data (h.java:457-565)

---

## Notes

1. **Flag 0x80**: Many tiles appear with bit 7 set (0x80), likely indicating mirroring, rotation, or collision variant
2. **Dynamic tiles**: IDs 93-104 are hoops that change state when passed through
3. **Slope conversion**: IDs 113-116 convert to ID 110 under certain game conditions
4. **Empty space dominates**: ID 0 represents 67% of all tiles across all levels
