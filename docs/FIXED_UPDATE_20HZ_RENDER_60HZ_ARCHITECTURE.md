# Bounce Back — 20 Hz fixed update + 60 Hz render

Цель этого документа: описать безопасный и архитектурно устойчивый переход с модели "1 кадр = 1 тик логики" на модель "логика 20 Hz, рендер 60 Hz" без изменения физконстант.

Контекст текущего порта:
- В `src/main.c:477` стоит `SDL_Delay(50)`, что делает общий цикл 20 FPS.
- В том же цикле вызываются логические тики: `level_objects_tick` (`src/main.c:370`), `player_update` (`src/main.c:376`), `exit_door_tick` (`src/main.c:401`), `animation_tick` (`src/main.c:456`).
- В `player` параметры и таймеры заданы в тиках, не в секундах: `JUMP_*`, `ACCEL_*`, `MAX_SPEED_*` (`src/player.h`), `timer_a/b/c` декрементируются на тик (`src/player.c:647`, `src/player.c:699`, `src/player.c:707`).
- Оригинал тоже тиковый 50 ms: `new f(this, 50)` (`original_code/bounce_back_s60.jar.src/h.java:811`), а `f` это `Timer.schedule(..., periodMs)` (`original_code/bounce_back_s60.jar.src/f.java:10`).

## 1. Принципиальный контракт времени

Инварианты, которые нельзя нарушать:
- Логика и физика выполняются только шагом `50 ms`.
- Количество логических апдейтов за секунду около 20, а не 60.
- Все тиковые таймеры (`timer_a/b/c`, `door.I`, `tile_anim.timer`) уменьшаются только в fixed update.
- Рендер может быть 60+ FPS, но не имеет права менять состояние мира.

Формально:
- `SIM_DT_MS = 50`.
- `RENDER_TARGET_MS = 16.666...` (опционально cap, если нужен).
- `world_state` меняется только внутри `simulate_one_tick()`.

## 2. Презумпция виновности текущего кода

Этот проект должен считать, что текущий код потенциально ошибочен, пока не доказано обратное. Поэтому при переходе вводятся защитные меры:

- Жесткое разделение слоев:
  - `pump_input_and_events()`
  - `simulate_one_tick()`
  - `render_frame(alpha)`
- Запрет на side-effects в рендере:
  - В рендер-пути не должно быть `*_tick`, `*_update` логики, изменения таймеров, изменения tilemap.
- Ассерты и счетчики:
  - Счетчик `sim_ticks_total`.
  - Счетчик `render_frames_total`.
  - Логи соотношения `ticks/sec` и `frames/sec`.
- Ограничение "spiral of death":
  - `MAX_SIM_STEPS_PER_FRAME` (например, 4 или 5).
  - При превышении steps: дроп части накопленного времени + лог warning.
- Feature flag для отката:
  - `BB_LOOP_MODE=legacy20` или `BB_LOOP_MODE=fixed20_render60`.

## 3. Рефакторинг цикла: целевая архитектура

## 3.1 Новая структура main loop

Псевдокод:

```c
// Timing constants
static const double SIM_DT_MS = 50.0;
static const double MAX_FRAME_CLAMP_MS = 250.0;    // защита от паузы/сворачивания
static const int MAX_SIM_STEPS_PER_FRAME = 5;      // защита от спирали

uint64_t perf_freq = SDL_GetPerformanceFrequency();
uint64_t prev_counter = SDL_GetPerformanceCounter();
double accumulator_ms = 0.0;

while (running) {
    // 1) Считать реальное время кадра
    uint64_t now = SDL_GetPerformanceCounter();
    double frame_ms = (double)(now - prev_counter) * 1000.0 / (double)perf_freq;
    prev_counter = now;

    // 2) Clamp большого dt (alt-tab, breakpoints)
    if (frame_ms > MAX_FRAME_CLAMP_MS) {
        frame_ms = MAX_FRAME_CLAMP_MS;
    }
    accumulator_ms += frame_ms;

    // 3) Обработать события ОС и raw input
    pump_input_and_events(&running, &input_raw, ...);

    // 4) Выполнить фиксированные тики логики
    int sim_steps = 0;
    while (accumulator_ms >= SIM_DT_MS && sim_steps < MAX_SIM_STEPS_PER_FRAME) {
        input_update_tick_state(&input_tick, &input_raw);
        simulate_one_tick(&game, &input_tick);
        save_previous_state_for_interpolation(&game);

        accumulator_ms -= SIM_DT_MS;
        sim_steps++;
    }

    // 5) Защита от спирали
    if (accumulator_ms >= SIM_DT_MS) {
        // Мы перегружены: дропаем излишек до 1 тика
        accumulator_ms = fmod(accumulator_ms, SIM_DT_MS);
        debug_warn_spiral_drop();
    }

    // 6) Интерполяция только для рендера
    double alpha = accumulator_ms / SIM_DT_MS;
    render_frame(&game, alpha);
}
```

## 3.2 Что считать "тик логики"

В `simulate_one_tick()` должны остаться:
- `level_objects_tick(...)`
- `player_update(...)`
- `exit_door_tick(...)`
- `animation_tick(...)`
- Проверки game over/level complete
- Переключение уровней и сбросы state

Это соответствует текущему порядку из `src/main.c` и поддерживает семантику оригинала.

## 3.3 Что оставить только в рендере

В `render_frame()` должны остаться:
- `bg_layer_draw(...)`
- `renderer_draw(...)`
- `enemy_renderer_draw(...)`
- `player_render(...)`
- `foreground_pass_draw(...)`
- `hud_render(...)`
- `SDL_RenderPresent(...)`

Запрещено:
- Любой `*_tick`.
- Любой `player_update`.
- Модификация `Level`/таймеров.

## 4. Ввод при разной частоте рендера/логики

Критичный момент: если читать input только раз на 20 Hz, управление может стать вязким. Но и применять input в физику нужно только на fixed tick.

Правильная схема:
- На каждом render-кадре собирать OS events и обновлять `input_raw`.
- На каждом fixed tick формировать `input_tick` snapshot (press/hold/release) из `input_raw`.
- `player_update` получает `input_tick`, а не "живой" input в середине кадра.

Рекомендация:
- Добавить edge-флаги `pressed_this_tick`/`released_this_tick` для кнопок прыжка и плеч, чтобы поведение было детерминированным.

## 5. Интерполяция без изменения физики

Интерполировать только визуальные координаты:
- `player_render_x = lerp(prev_x_pos, x_pos, alpha)`
- `player_render_y = lerp(prev_y_pos, y_pos, alpha)`
- Аналогично для камеры.

Не интерполировать:
- Коллизии
- Триггеры
- Таймеры
- Tile-state

Для первого этапа можно без интерполяции (будет 60 FPS с дискретным движением 20 Hz), затем добавить interpolation как отдельный безопасный шаг.

## 6. Пошаговый план внедрения

## Шаг 0: Базовая телеметрия
- Добавить debug-метрики в `main.c`:
  - `sim_ticks_total`, `render_frames_total`
  - `sim_ticks_last_sec`, `render_frames_last_sec`
  - `spiral_drop_count`
- Логировать раз в 1 секунду.

## Шаг 1: Изолировать функции цикла
- Вынести код из `while (running)` в три функции:
  - `pump_input_and_events(...)`
  - `simulate_one_tick(...)`
  - `render_frame(...)`
- Убедиться, что поведение не изменилось в legacy-режиме 20 FPS.

## Шаг 2: Ввести accumulator и fixed steps
- Заменить `SDL_Delay(50)` на timing через `SDL_GetPerformanceCounter`.
- Включить fixed simulation 50 ms.
- Временно оставить рендер без interpolation.

## Шаг 3: Проверить регрессии
- Проверить длительности бонусов:
  - `timer_b=450` должно ощущаться как ~22.5 сек.
  - `timer_c=550` как ~27.5 сек.
- Проверить открытие двери:
  - `door.I` до `48` тиков как раньше.
- Проверить скорость падения/прыжка на эталонных уровнях.

## Шаг 4: Добавить interpolation
- Хранить `prev` состояния игрока и камеры.
- Применить только в рендере.
- Проверить, что геймплей не изменился.

## Шаг 5: Фича-флаг и fallback
- Оставить `legacy20` режим на время стабилизации.
- По умолчанию можно переключить на `fixed20_render60`, когда тесты стабильны.

## 7. Что сломается, если сделать неправильно

Типовые ошибки:
- Запуск `player_update` 60 раз/сек -> ускорение игры, ломаются прыжок и бонусы.
- Декремент `timer_*` в рендере -> ускорение времени эффектов.
- Обработка input сразу в рендере и физике без snapshot -> недетерминированность.
- Отсутствие `MAX_SIM_STEPS_PER_FRAME` -> фризы в "догоняющем" цикле.

## 8. Acceptance criteria

Переход считается корректным, если:
- В среднем `sim_ticks/sec` держится в диапазоне `20 +/- 1`.
- `render_frames/sec` держится близко к 60 (или лимиту платформы).
- Сценарии из baseline повторяются по таймингам:
  - Время до открытия двери.
  - Длительность бонусов.
  - Время падения с фиксированной высоты.
- Нет логических расхождений между `legacy20` и `fixed20_render60` по ключевым метрикам.

## 9. Минимальный набор автопроверок

Рекомендации для `tests`/debug harness:
- Тест "N fixed ticks":
  - Прогнать 600 тиков без рендера.
  - Проверить состояние `player`, `door`, `tile_anim` на детерминированные значения.
- Тест "frame jitter":
  - Подать последовательность dt (`5, 8, 33, 4, 60... ms`) и убедиться, что выполнено нужное число sim-шагов.
- Тест "pause spike":
  - Один кадр с `dt=1000ms` должен быть clamped и не должен повесить цикл.

## 10. Практическая интеграция в текущие файлы

Где менять в первую очередь:
- `src/main.c`
  - заменить монолитный цикл на трехфазный
  - убрать `SDL_Delay(50)`
  - добавить accumulator
- `src/player.[ch]`
  - опционально добавить `prev_x_pos/prev_y_pos` для interpolation
- `src/camera.[ch]`
  - опционально добавить `prev_x/prev_y` для interpolation

Где не менять физконстанты:
- `src/player.h` (`JUMP_*`, `ACCEL_*`, `MAX_SPEED_*`, `DECEL_*`)
- Логика `timer_a/b/c` в `src/player.c`
- Логика `exit_door_tick` и `animation_tick`

## 11. Решения по умолчанию

Рекомендуемые значения:
- `SIM_DT_MS = 50.0`
- `MAX_FRAME_CLAMP_MS = 250.0`
- `MAX_SIM_STEPS_PER_FRAME = 5`
- `alpha = accumulator / SIM_DT_MS`

Почему так:
- Совместимость с оригиналом по симуляции.
- Стабильность при просадках кадров.
- Визуальная плавность без физрегрессий.

## 12. Итог

Архитектурно правильный путь для этого проекта: оставить симуляцию полностью тиковой (20 Hz), а рендер отвязать и поднять до 60 Hz. 

Это минимизирует риск, сохраняет паритет с оригиналом и дает заметный выигрыш в плавности. При подходе "презумпции виновности" обязательны: жесткие инварианты времени, ограничители спирали, метрики и feature-flag отката.
