#!/usr/bin/env python3
"""
Generate a standalone HTML level inspector with hover metadata.

Source-of-truth format is aligned to original Java code:
- tf parsing: g.java constructor (fields v/T/b/l/af)
- lf parsing: h.b(level) + g constructor map bytes
"""

from __future__ import annotations

import argparse
import json
import os
import struct
from dataclasses import dataclass


@dataclass
class Cursor:
    data: bytes
    off: int = 0

    def u8(self) -> int:
        if self.off >= len(self.data):
            raise ValueError("unexpected EOF (u8)")
        v = self.data[self.off]
        self.off += 1
        return v

    def i8(self) -> int:
        v = self.u8()
        return v - 256 if v >= 128 else v

    def i32_be(self) -> int:
        if self.off + 4 > len(self.data):
            raise ValueError("unexpected EOF (i32)")
        (v,) = struct.unpack(">i", self.data[self.off : self.off + 4])
        self.off += 4
        return v


def read_container(path: str) -> list[bytes]:
    with open(path, "rb") as f:
        header = f.read(2)
        if len(header) != 2:
            raise ValueError(f"{path}: too small for container header")
        (count,) = struct.unpack(">H", header)
        sizes_raw = f.read(count * 2)
        if len(sizes_raw) != count * 2:
            raise ValueError(f"{path}: too small for chunk sizes")
        sizes = list(struct.unpack(f">{count}H", sizes_raw))
        chunks: list[bytes] = []
        for i, sz in enumerate(sizes):
            chunk = f.read(sz)
            if len(chunk) != sz:
                raise ValueError(f"{path}: EOF in chunk {i}")
            chunks.append(chunk)
        return chunks


def parse_tf(tf_path: str) -> dict:
    chunks = read_container(tf_path)
    if len(chunks) < 2:
        raise ValueError("tf must contain at least 2 chunks")

    c0 = Cursor(chunks[0])
    c1 = Cursor(chunks[1])

    images_total = c0.i8()
    images_base = c0.i8()
    clamp_x = c0.u8() != 0
    clamp_y = c0.u8() != 0
    tile_w = c0.i8()
    tile_h = c0.i8()
    if tile_w == 12:
        tile_w = 16
        tile_h = 16
    tile_count = c0.i8()
    split_index = c0.i8()
    tile_id_mask = c0.u8()
    tile_flag_mask = c0.u8()
    bg_color_i32 = c0.i32_be()

    anim_count = c0.i8()
    animations = []
    for _ in range(max(0, anim_count)):
        _unused = c0.i8()
        period = c0.i8()
        n_frames = c0.i8()
        frames = [c0.u8() for _ in range(max(0, n_frames))]
        animations.append({"period": period, "frames": frames})

    cur = c0
    tiles = []
    for tile_id in range(max(0, tile_count)):
        if tile_id == split_index:
            cur = c1
        echo = cur.i8()
        render_type = cur.i8()
        image_index = cur.u8()
        transform = cur.u8()
        collision_type = cur.i8()
        if collision_type == 1:
            # inline mask booleans (f * A bytes in g.java)
            need = tile_w * tile_h
            if cur.off + need > len(cur.data):
                raise ValueError(f"tf: EOF in inline mask for tile {tile_id}")
            cur.off += need
        aux = cur.i32_be()
        tiles.append(
            {
                "id": tile_id,
                "echo": echo,
                "render_type": render_type,
                "image_index": image_index,
                "transform": transform,
                "collision_type": collision_type,
                "aux": aux,
            }
        )

    return {
        "images_total": images_total,
        "images_base": images_base,
        "clamp_x": clamp_x,
        "clamp_y": clamp_y,
        "tile_w": tile_w,
        "tile_h": tile_h,
        "tile_count": tile_count,
        "split_index": split_index,
        "tile_id_mask": tile_id_mask,
        "tile_flag_mask": tile_flag_mask,
        "bg_color_i32": bg_color_i32,
        "animations": animations,
        "tiles": tiles,
    }


def parse_lf(lf_path: str) -> list[dict]:
    chunks = read_container(lf_path)
    if len(chunks) % 2 != 0:
        raise ValueError("lf chunk count must be even (meta/map pairs)")

    levels = []
    n_levels = len(chunks) // 2
    for li in range(n_levels):
        meta = chunks[2 * li]
        tile_chunk = chunks[2 * li + 1]
        if len(tile_chunk) < 2:
            raise ValueError(f"lf level {li}: map chunk too small")
        h = tile_chunk[0]
        w = tile_chunk[1]
        need = 2 + h * w
        if len(tile_chunk) < need:
            raise ValueError(f"lf level {li}: map bytes too small")
        tiles = list(tile_chunk[2:need])

        level_meta = {
            "theme": meta[0] if len(meta) > 0 else 0,
            "spawn_y": meta[1] if len(meta) > 1 else 0,
            "spawn_x": meta[2] if len(meta) > 2 else 0,
            "ball_type": meta[3] if len(meta) > 3 else 0,
            "exit_y": meta[4] if len(meta) > 4 else 0,
            "exit_x": meta[5] if len(meta) > 5 else 0,
            "enemy_count": meta[6] if len(meta) > 6 else 0,
        }

        levels.append(
            {
                "index": li,
                "width": w,
                "height": h,
                "meta": level_meta,
                "tiles": tiles,
            }
        )
    return levels


def build_html(data: dict) -> str:
    data_json = json.dumps(data, ensure_ascii=False, separators=(",", ":"))
    return f"""<!doctype html>
<html lang="ru">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Bounce Back Level Flags Inspector</title>
  <style>
    :root {{
      --bg: #11151a;
      --panel: #1a2129;
      --line: #2e3c4a;
      --text: #d8e2ec;
      --muted: #9eb1c4;
      --accent: #65c3ff;
    }}
    * {{ box-sizing: border-box; }}
    body {{
      margin: 0;
      background: linear-gradient(180deg, #0c1014, #11151a 28%, #0f1419);
      color: var(--text);
      font: 13px/1.4 Consolas, "Liberation Mono", Menlo, monospace;
    }}
    .app {{
      display: grid;
      grid-template-columns: minmax(440px, 1fr) 380px;
      gap: 12px;
      padding: 12px;
      min-height: 100vh;
    }}
    .left, .right {{
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 10px;
      overflow: hidden;
    }}
    .toolbar {{
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      padding: 10px;
      border-bottom: 1px solid var(--line);
      align-items: center;
    }}
    label {{ color: var(--muted); }}
    select, input[type="range"] {{
      background: #111820;
      color: var(--text);
      border: 1px solid var(--line);
      border-radius: 6px;
      padding: 4px 6px;
    }}
    .scroll {{
      overflow: auto;
      max-height: calc(100vh - 110px);
      background:
        linear-gradient(45deg, rgba(101,195,255,.04) 25%, transparent 25%) 0 0/16px 16px,
        linear-gradient(-45deg, rgba(101,195,255,.04) 25%, transparent 25%) 0 0/16px 16px,
        #0f141a;
    }}
    canvas {{ display: block; image-rendering: pixelated; image-rendering: crisp-edges; }}
    .right {{ padding: 10px; }}
    .title {{ color: var(--accent); margin-bottom: 8px; font-weight: 700; }}
    .kv {{ border-top: 1px solid var(--line); padding: 7px 0; }}
    .k {{ color: var(--muted); }}
    .v {{ color: var(--text); word-break: break-word; }}
    .small {{ color: var(--muted); font-size: 12px; }}
  </style>
</head>
<body>
  <div class="app">
    <section class="left">
      <div class="toolbar">
        <label>Level:
          <select id="levelSel"></select>
        </label>
        <label>Zoom:
          <input id="zoom" type="range" min="1" max="4" step="1" value="2" />
          <span id="zoomVal">2x</span>
        </label>
        <label>
          <input id="showTextures" type="checkbox" checked />
          textures
        </label>
      </div>
      <div class="scroll" id="scrollWrap">
        <canvas id="cv"></canvas>
      </div>
    </section>
    <aside class="right">
      <div class="title">Tile Inspector</div>
      <div id="info" class="small">Наведи мышку на клетку</div>
      <div class="kv small">
        Формат подтверждён по оригиналу:
        <br/>`g.java` (tf: v/T/b/l/af, masks, flags)
        <br/>`h.java` (lf: level meta + map bytes)
      </div>
    </aside>
  </div>
  <script>
    const DATA = {data_json};
    const DG = [0,270,180,90,16384,16654,16564,16474,8192,8462,8372,8282];
    const JAVA_TRANSFORM_LABEL = {{
      0:  'none',
      270:'rotate 270°',
      180:'rotate 180°',
      90: 'rotate 90°',
      16384:'flip vertical',
      16654:'flip vertical + rotate 270°',
      16564:'flip vertical + rotate 180°',
      16474:'flip vertical + rotate 90°',
      8192:'flip horizontal',
      8462:'flip horizontal + rotate 270°',
      8372:'flip horizontal + rotate 180°',
      8282:'flip horizontal + rotate 90°'
    }};
    const TF_BY_ID = Object.fromEntries(DATA.tf.tiles.map(t => [t.id, t]));
    const TEX_CACHE = new Map();
    let level = DATA.levels[0];
    let zoom = 2;
    let showTextures = true;

    const cv = document.getElementById('cv');
    const ctx = cv.getContext('2d');
    const levelSel = document.getElementById('levelSel');
    const zoomEl = document.getElementById('zoom');
    const zoomVal = document.getElementById('zoomVal');
    const showTexEl = document.getElementById('showTextures');
    const info = document.getElementById('info');

    for (const lv of DATA.levels) {{
      const opt = document.createElement('option');
      opt.value = String(lv.index);
      opt.textContent = `L${{String(lv.index).padStart(2, '0')}} (${{lv.width}}x${{lv.height}})`;
      levelSel.appendChild(opt);
    }}

    function hex2(v) {{
      return '0x' + (v & 0xFF).toString(16).toUpperCase().padStart(2, '0');
    }}
    function hex8(v) {{
      return '0x' + (v >>> 0).toString(16).toUpperCase().padStart(8, '0');
    }}
    function decodeCollision(c) {{
      if (c === 0) return '0 (none)';
      if (c === 1) return '1 (inline mask)';
      if (c === 2) return '2 (solid full-tile)';
      if (c === 3) return '3 (mask alias via aux)';
      return String(c);
    }}
    function decodeRender(r) {{
      if (r === 0) return '0 (skip)';
      if (r === 1) return '1 (static)';
      if (r === 3) return '3 (animated)';
      return String(r);
    }}
    function decodeTransform(t) {{
      if (t < 0 || t >= DG.length) return `${{t}} (${{hex2(t)}}) / out-of-range`;
      const direct = DG[t];
      const label = JAVA_TRANSFORM_LABEL[direct] || 'unknown';
      return `${{t}} (${{hex2(t)}}) -> p[b]=${{direct}} -> ${{label}}`;
    }}
    function texPath(imageIndex) {{
      return `extracted_textures/${{String(imageIndex).padStart(3, '0')}}.png`;
    }}
    function getTex(imageIndex) {{
      if (TEX_CACHE.has(imageIndex)) return TEX_CACHE.get(imageIndex);
      const img = new Image();
      img.src = texPath(imageIndex);
      img.onload = () => draw();
      img.onerror = () => draw();
      TEX_CACHE.set(imageIndex, img);
      return img;
    }}
    function cellColor(id) {{
      const r = (id * 53) % 256;
      const g = (id * 97) % 256;
      const b = (id * 193) % 256;
      return `rgb(${{r}},${{g}},${{b}})`;
    }}

    function draw() {{
      const ts = 16 * zoom;
      cv.width = level.width * ts;
      cv.height = level.height * ts;
      ctx.clearRect(0, 0, cv.width, cv.height);

      for (let y = 0; y < level.height; y++) {{
        for (let x = 0; x < level.width; x++) {{
          const b = level.tiles[y * level.width + x];
          const id = b & DATA.tf.tile_id_mask;
          const tf = TF_BY_ID[id];
          const px = x * ts;
          const py = y * ts;

          const drawable = tf && tf.render_type !== 0 && Number.isInteger(tf.image_index);
          if (showTextures && drawable) {{
            const img = getTex(tf.image_index);
            if (img.complete && img.naturalWidth > 0) {{
              ctx.drawImage(img, px, py, ts, ts);
            }} else {{
              ctx.fillStyle = cellColor(id);
              ctx.fillRect(px, py, ts, ts);
            }}
          }} else {{
            if (id !== 0) {{
              ctx.fillStyle = cellColor(id);
              ctx.fillRect(px, py, ts, ts);
            }}
          }}

          if ((b & DATA.tf.tile_flag_mask) !== 0) {{
            ctx.fillStyle = 'rgba(0,0,0,0.28)';
            ctx.fillRect(px, py, ts, ts);
          }}

          ctx.strokeStyle = 'rgba(0,0,0,0.2)';
          ctx.strokeRect(px + 0.5, py + 0.5, ts - 1, ts - 1);
        }}
      }}
    }}

    function renderInfo(tileX, tileY) {{
      const idx = tileY * level.width + tileX;
      const b = level.tiles[idx];
      const id = b & DATA.tf.tile_id_mask;
      const flag = (b & DATA.tf.tile_flag_mask) !== 0;
      const tf = TF_BY_ID[id];
      if (!tf) {{
        info.innerHTML = '<div class="kv"><div class="k">tile</div><div class="v">no tf entry</div></div>';
        return;
      }}

      const auxMeaning =
        tf.collision_type === 3 ? `mask alias -> tileId ${{tf.aux}}` :
        tf.render_type === 3 ? `animation group ${{tf.aux}}` :
        'raw aux';

      const rows = [
        ['level', `L${{String(level.index).padStart(2,'0')}} (${{level.width}}x${{level.height}})`],
        ['coord', `x=${{tileX}}, y=${{tileY}}, index=${{idx}}`],
        ['map byte', `${{hex2(b)}} (id=${{id}}, flag80=${{flag ? 1 : 0}})`],
        ['tf.id/echo', `${{tf.id}} / ${{tf.echo}}`],
        ['tf.render_type (v)', decodeRender(tf.render_type)],
        ['tf.image_index (T)', `${{tf.image_index}} -> ${{texPath(tf.image_index)}}`],
        ['tf.transform (b)', decodeTransform(tf.transform)],
        ['tf.collision_type (l)', decodeCollision(tf.collision_type)],
        ['tf.aux (af)', `${{tf.aux}} (${{auxMeaning}})`],
        ['tf masks', `tile_id_mask=${{hex2(DATA.tf.tile_id_mask)}}, tile_flag_mask=${{hex2(DATA.tf.tile_flag_mask)}}`],
        ['tf global', `tile=${{DATA.tf.tile_w}}x${{DATA.tf.tile_h}}, splitIndex=${{DATA.tf.split_index}}, bgColor=${{hex8(DATA.tf.bg_color_i32)}}`],
      ];
      info.innerHTML = rows.map(([k,v]) => `<div class="kv"><div class="k">${{k}}</div><div class="v">${{v}}</div></div>`).join('');
    }}

    cv.addEventListener('mousemove', (e) => {{
      const r = cv.getBoundingClientRect();
      const px = e.clientX - r.left;
      const py = e.clientY - r.top;
      const ts = 16 * zoom;
      const x = Math.floor(px / ts);
      const y = Math.floor(py / ts);
      if (x < 0 || y < 0 || x >= level.width || y >= level.height) return;
      renderInfo(x, y);
    }});

    levelSel.addEventListener('change', () => {{
      const i = Number(levelSel.value);
      level = DATA.levels.find(v => v.index === i) || DATA.levels[0];
      draw();
      info.textContent = 'Наведи мышку на клетку';
    }});
    zoomEl.addEventListener('input', () => {{
      zoom = Number(zoomEl.value);
      zoomVal.textContent = `${{zoom}}x`;
      draw();
    }});
    showTexEl.addEventListener('change', () => {{
      showTextures = showTexEl.checked;
      draw();
    }});

    draw();
  </script>
</body>
</html>
"""


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tf", default="release/res/tf", help="Path to /res/tf")
    parser.add_argument("--lf", default="release/res/lf", help="Path to /res/lf")
    parser.add_argument("--out", default="artifacts/level_flags_inspector.html", help="Output html path")
    args = parser.parse_args()

    tf = parse_tf(args.tf)
    levels = parse_lf(args.lf)

    payload = {
        "tf": {
            "images_total": tf["images_total"],
            "images_base": tf["images_base"],
            "tile_w": tf["tile_w"],
            "tile_h": tf["tile_h"],
            "tile_count": tf["tile_count"],
            "split_index": tf["split_index"],
            "tile_id_mask": tf["tile_id_mask"],
            "tile_flag_mask": tf["tile_flag_mask"],
            "bg_color_i32": tf["bg_color_i32"],
            "tiles": tf["tiles"],
        },
        "levels": levels,
    }

    html = build_html(payload)
    os.makedirs(os.path.dirname(args.out) or ".", exist_ok=True)
    with open(args.out, "w", encoding="utf-8") as f:
        f.write(html)
    print(f"written: {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
