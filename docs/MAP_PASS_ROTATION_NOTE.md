# Map Pass Rotation Note

Этот файл содержит информацию, ранее находившуюся в разделе `A-03` отчета `AUDIT_REPORT.md`.

## Факт из Java кода

В основном проходе карты при `ab=false` runtime-поворот отключен принудительно:

```java
k = (paramInt4 == 0) ? this.b[i] : paramInt4;
k = 0;
```

Источник: `original_code/bounce_back_s60.jar.src/g.java:624-625`.

Следствие:
- Для map-pass поворот/отражение в рантайме не выполняется.
- Рендер идет «как в текстуре», даже если у тайла есть `b != 0`.

## Состояние C-порта

Текущее поведение в `src/level_renderer.c` соответствует этому правилу Java map-pass:

```c
SDL_RenderCopy(r, tex, NULL, &dest);
```

Источник: `src/level_renderer.c:71`.

Для переднего плана используется отдельный проход, где трансформации применяются по `meta.transform != 0`:
- `src/foreground_pass.c:73-76`.
