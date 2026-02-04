#!/usr/bin/env python3
"""
Анализ требований к памяти для Bounce Back на PSP.

Считает:
1. Размеры контейнеров /res/* (на диске)
2. Размеры распакованных текстур (в RAM как RGBA)
3. "Самый толстый" уровень (наибольшая tileMap + враги)
4. Общие требования к памяти для игры

PSP имеет:
- 32 MB RAM (PSP-1000)
- 64 MB RAM (PSP-2000/3000)
- ~24 MB доступно для приложений после системы
"""

from __future__ import annotations

import os
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Optional
from PIL import Image
import io


@dataclass
class ContainerInfo:
    """Информация о контейнере ресурсов."""
    name: str
    file_size: int  # размер на диске
    element_count: int
    elements: list[int]  # размеры элементов


@dataclass
class TextureMemory:
    """Память для текстуры в RAM."""
    width: int
    height: int
    bytes_rgba: int  # width * height * 4


@dataclass
class LevelMemory:
    """Память для одного уровня."""
    level_index: int
    tilemap_width: int
    tilemap_height: int
    tilemap_bytes: int  # width * height (каждый тайл = 1 байт)
    enemy_count: int
    enemy_bytes: int  # count * размер структуры врага в RAM
    total_tiles: int


def read_be16(f) -> int:
    """Читает big-endian uint16."""
    return struct.unpack('>H', f.read(2))[0]


def read_container_info(path: str) -> Optional[ContainerInfo]:
    """Читает метаданные контейнера (формат class c)."""
    if not os.path.exists(path):
        return None
    
    with open(path, 'rb') as f:
        blob = f.read()
    
    if len(blob) < 2:
        return None
    
    # Парсим заголовок
    count = (blob[0] << 8) | blob[1]
    
    if len(blob) < 2 + count * 2:
        return None
    
    # Читаем размеры элементов
    sizes = []
    offset = 2
    for i in range(count):
        size = (blob[offset] << 8) | blob[offset + 1]
        sizes.append(size)
        offset += 2
    
    return ContainerInfo(
        name=os.path.basename(path),
        file_size=len(blob),
        element_count=count,
        elements=sizes
    )


def get_texture_memory(data: bytes) -> Optional[TextureMemory]:
    """Вычисляет размер текстуры в памяти (как RGBA)."""
    try:
        img = Image.open(io.BytesIO(data))
        w, h = img.size
        # В PSP храним как RGBA (4 байта на пиксель)
        return TextureMemory(
            width=w,
            height=h,
            bytes_rgba=w * h * 4
        )
    except:
        return None


def analyze_level_memory(lf_path: str, level_index: int) -> Optional[LevelMemory]:
    """Анализирует требования к памяти для одного уровня."""
    container = read_container_info(lf_path)
    if not container:
        return None
    
    # Для каждого уровня: metadata chunk + tilemap chunk
    metadata_idx = level_index * 2
    tilemap_idx = metadata_idx + 1
    
    if tilemap_idx >= container.element_count:
        return None
    
    # Читаем данные
    with open(lf_path, 'rb') as f:
        blob = f.read()
    
    # Пропускаем заголовок контейнера
    count = (blob[0] << 8) | blob[1]
    offset = 2 + count * 2
    
    # Переходим к нужному chunk
    for i in range(tilemap_idx):
        offset += container.elements[i]
    
    # Читаем tilemap chunk
    tilemap_data = blob[offset:offset + container.elements[tilemap_idx]]
    
    if len(tilemap_data) < 2:
        return None
    
    height = tilemap_data[0]
    width = tilemap_data[1]
    tilemap_bytes = height * width
    
    # Читаем metadata для подсчета врагов
    metadata_offset = 2 + count * 2
    for i in range(metadata_idx):
        metadata_offset += container.elements[i]
    
    metadata = blob[metadata_offset:metadata_offset + container.elements[metadata_idx]]
    
    if len(metadata) < 7:
        return None
    
    enemy_count = metadata[6]  # 7-й байт = количество врагов
    
    # В runtime для каждого врага нужно хранить:
    # - type (1 byte) → int (4)
    # - bbox corners (4 bytes) → int[2][2] (16)
    # - current offset (2 bytes) → int[2] (8)
    # - speed (2 bytes) → int[2] (8)
    # Итого ~36 байт на врага (округлим до 40 для выравнивания)
    enemy_bytes = enemy_count * 40
    
    return LevelMemory(
        level_index=level_index,
        tilemap_width=width,
        tilemap_height=height,
        tilemap_bytes=tilemap_bytes,
        enemy_count=enemy_count,
        enemy_bytes=enemy_bytes,
        total_tiles=tilemap_bytes
    )


def format_bytes(bytes_val: int) -> str:
    """Форматирует байты в читаемый вид."""
    if bytes_val < 1024:
        return f"{bytes_val} B"
    elif bytes_val < 1024 * 1024:
        return f"{bytes_val / 1024:.1f} KB"
    else:
        return f"{bytes_val / (1024 * 1024):.2f} MB"


def main():
    # Путь к ресурсам
    res_path = Path("original_code/bounce_back_s60.jar.src/res")
    
    if not res_path.exists():
        print(f"❌ Не найден путь: {res_path}")
        print("Убедитесь, что вы находитесь в папке bounce_back/")
        return
    
    print("=" * 80)
    print("АНАЛИЗ ТРЕБОВАНИЙ К ПАМЯТИ - Bounce Back → PSP")
    print("=" * 80)
    print()
    
    # ========================================================================
    # 1. РАЗМЕРЫ КОНТЕЙНЕРОВ НА ДИСКЕ
    # ========================================================================
    print("📦 РАЗМЕРЫ КОНТЕЙНЕРОВ (на диске):")
    print("-" * 80)
    
    containers_to_check = [
        ("lf", "Данные уровней (22 уровня)"),
        ("tf", "Метаданные тайлов"),
        ("if0", "Текстуры тайлов (базовые)"),
        ("if1", "Текстуры тайлов (тема 1)"),
        ("if2", "Текстуры тайлов (тема 2)"),
        ("b", "Спрайты мяча"),
        ("bg", "Фоновые данные"),
        ("ib0", "Анимированный фон 0"),
        ("ic", "UI элементы"),
        ("im", "Меню и заставки"),
        ("s", "Звуки"),
        ("r", "Демо-реплей"),
    ]
    
    total_disk = 0
    container_data = {}
    
    for name, description in containers_to_check:
        path = res_path / name
        info = read_container_info(str(path))
        if info:
            total_disk += info.file_size
            container_data[name] = info
            print(f"  {name:6s} - {format_bytes(info.file_size):>10s} ({info.element_count:3d} элементов) - {description}")
        else:
            print(f"  {name:6s} - {'ОТСУТСТВУЕТ':>10s}                      - {description}")
    
    print(f"\n  {'ИТОГО:':6s} - {format_bytes(total_disk):>10s}")
    print()
    
    # ========================================================================
    # 2. РАСПАКОВАННЫЕ ТЕКСТУРЫ (В RAM КАК RGBA)
    # ========================================================================
    print("🎨 ТЕКСТУРЫ В ПАМЯТИ (распакованные в RGBA):")
    print("-" * 80)
    
    textures_to_analyze = [
        ("if0", "Тайлы (базовые)"),
        ("if1", "Тайлы (тема 1)"),
        ("if2", "Тайлы (тема 2)"),
        ("b", "Мяч (26 кадров)"),
        ("ic", "UI элементы"),
        ("im", "Меню/заставки"),
        ("ib0", "Фон анимированный"),
    ]
    
    total_texture_memory = 0
    
    for container_name, description in textures_to_analyze:
        if container_name not in container_data:
            continue
        
        info = container_data[container_name]
        
        # Читаем элементы контейнера
        with open(res_path / container_name, 'rb') as f:
            blob = f.read()
        
        count = (blob[0] << 8) | blob[1]
        offset = 2 + count * 2
        
        container_memory = 0
        max_texture = None
        
        for i in range(info.element_count):
            element_data = blob[offset:offset + info.elements[i]]
            tex_mem = get_texture_memory(element_data)
            if tex_mem:
                container_memory += tex_mem.bytes_rgba
                if max_texture is None or tex_mem.bytes_rgba > max_texture.bytes_rgba:
                    max_texture = tex_mem
            offset += info.elements[i]
        
        total_texture_memory += container_memory
        
        max_info = ""
        if max_texture:
            max_info = f" [max: {max_texture.width}x{max_texture.height}]"
        
        print(f"  {container_name:6s} - {format_bytes(container_memory):>10s} {max_info:25s} - {description}")
    
    print(f"\n  {'ИТОГО:':6s} - {format_bytes(total_texture_memory):>10s}")
    print()
    
    # ========================================================================
    # 3. АНАЛИЗ УРОВНЕЙ
    # ========================================================================
    print("🗺️  АНАЛИЗ УРОВНЕЙ (tilemap + враги в RAM):")
    print("-" * 80)
    
    lf_path = str(res_path / "lf")
    levels = []
    
    for i in range(22):  # 22 уровня
        level_mem = analyze_level_memory(lf_path, i)
        if level_mem:
            levels.append(level_mem)
    
    if levels:
        # Сортируем по общей памяти
        levels_sorted = sorted(levels, key=lambda x: x.tilemap_bytes + x.enemy_bytes, reverse=True)
        
        # Топ-5 самых тяжелых уровней
        print("  Топ-5 самых тяжелых уровней:")
        print()
        for i, level in enumerate(levels_sorted[:5], 1):
            total_level = level.tilemap_bytes + level.enemy_bytes
            print(f"  #{i} Уровень {level.level_index:2d}:")
            print(f"      TileMap: {level.tilemap_width}x{level.tilemap_height} = {format_bytes(level.tilemap_bytes):>8s}")
            print(f"      Враги:   {level.enemy_count} шт × 40 байт = {format_bytes(level.enemy_bytes):>8s}")
            print(f"      ВСЕГО:   {format_bytes(total_level):>8s}")
            print()
        
        # Общая статистика
        max_level = levels_sorted[0]
        total_tilemap = sum(l.tilemap_bytes for l in levels)
        total_enemies = sum(l.enemy_bytes for l in levels)
        
        print(f"  Самый тяжелый уровень: #{max_level.level_index}")
        print(f"    Размер: {max_level.tilemap_width}x{max_level.tilemap_height} тайлов")
        print(f"    Память: {format_bytes(max_level.tilemap_bytes + max_level.enemy_bytes)}")
        print()
    
    # ========================================================================
    # 4. ОБЩИЙ РАСЧЕТ ПАМЯТИ
    # ========================================================================
    print("=" * 80)
    print("💾 ИТОГОВЫЕ ТРЕБОВАНИЯ К ПАМЯТИ:")
    print("=" * 80)
    print()
    
    # Статические ресурсы (загружаются один раз при старте)
    static_resources = total_texture_memory
    
    # Runtime данные для одного уровня (самый тяжелый)
    if levels:
        max_level = levels_sorted[0]
        runtime_level = max_level.tilemap_bytes + max_level.enemy_bytes
    else:
        runtime_level = 0
    
    # Дополнительная память для runtime (оценка):
    # - Игрок (структура Player): ~200 байт
    # - Камера/viewport: ~100 байт
    # - Коллизионные маски для тайлов: ~117 тайлов × 256 байт = ~30 KB
    # - Анимационные данные: ~5 KB
    # - Звуковые буферы: ~50 KB (для микширования)
    # - Стек/heap overhead: ~100 KB
    runtime_overhead = 200 + 100 + 30*1024 + 5*1024 + 50*1024 + 100*1024
    
    total_memory = static_resources + runtime_level + runtime_overhead
    
    print(f"  Статические ресурсы (текстуры):    {format_bytes(static_resources):>10s}")
    print(f"  Runtime уровень (самый тяжелый):   {format_bytes(runtime_level):>10s}")
    print(f"  Runtime overhead (player, audio):  {format_bytes(runtime_overhead):>10s}")
    print(f"  {'-' * 50}")
    print(f"  ИТОГО:                             {format_bytes(total_memory):>10s}")
    print()
    
    # Сравнение с PSP
    psp_available = 24 * 1024 * 1024  # ~24 MB доступно
    percentage = (total_memory / psp_available) * 100
    
    print(f"  PSP доступно для приложений:       {format_bytes(psp_available):>10s}")
    print(f"  Использование:                     {percentage:>9.1f}%")
    print()
    
    if total_memory < psp_available:
        margin = psp_available - total_memory
        print(f"  ✅ Запас памяти:                    {format_bytes(margin):>10s}")
        print()
        print("  🎉 ВСЕ РЕСУРСЫ ВЛЕЗАЮТ В ПАМЯТЬ PSP!")
    else:
        overflow = total_memory - psp_available
        print(f"  ⚠️  Превышение:                      {format_bytes(overflow):>10s}")
        print()
        print("  ⚠️  ТРЕБУЕТСЯ ОПТИМИЗАЦИЯ!")
    
    print()
    print("=" * 80)
    print("📋 РЕКОМЕНДАЦИИ:")
    print("=" * 80)
    print()
    print("  1. Загружать только текстуры текущей темы (if0 + if1 ИЛИ if2)")
    print("  2. Использовать сжатие текстур (swizzled, 16-bit вместо 32-bit)")
    print("  3. Загружать уровень по требованию (не все 22 сразу)")
    print("  4. Звуки микшировать on-the-fly, не держать все в RAM")
    print("  5. Использовать VRAM для часто используемых текстур (2 MB)")
    print()
    print("  С учетом этих оптимизаций реальное использование: ~8-12 MB")
    print()


if __name__ == "__main__":
    main()
