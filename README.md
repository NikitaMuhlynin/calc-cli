# calc-cli

Консольный калькулятор на C++17 с расширением через DLL-плагины.

## Текущий этап

Подготовлен каркас сборки. `calc.exe` выводит сообщение о текущем этапе
и завершается. `plugin_stub.dll` — временная заглушка для проверки сборки DLL;
её экспорт не является интерфейсом будущих плагинов.
Разбор выражений, вычисления и загрузка плагинов будут добавлены отдельно.

## Требования

- Windows x64.
- CMake 3.24 или новее, включая CTest.
- Visual Studio 2022 или Build Tools 2022 с компонентами разработки на C++
  и Windows SDK.

## Сборка и проверка

Команды выполняются из корня проекта в PowerShell. Пресеты используют MSVC;
запускать Developer PowerShell не требуется.

```powershell
cmake --preset windows-msvc
cmake --build --preset debug
ctest --preset debug
.\build\windows-msvc\bin\Debug\calc.exe
```

Для Release после конфигурации:

```powershell
cmake --build --preset release
ctest --preset release
```

CTest проверяет успешный запуск программы и расположение результатов сборки.
Перед запуском тестов нужно собрать соответствующую конфигурацию.

Результат сборки Debug:

```text
build/windows-msvc/bin/Debug/
  calc.exe
  plugins/
    plugin_stub.dll
```

Для Release используется аналогичная папка `bin/Release`.

При необходимости сборку без тестов можно настроить отдельно:

```powershell
cmake -S . -B build/no-tests -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTING=OFF
cmake --build build/no-tests --config Release
```

## Структура

- `src/` — консольное приложение.
- `plugins/` — DLL-плагины; пока только заглушка.
- `tests/` — проверки через CTest.
- `CMakePresets.json` — воспроизводимые команды сборки и тестирования.

Стиль задаётся в `.clang-format`: отступ в четыре пробела, открывающая
скобка функций, классов и структур — с новой строки.
