# Workshop "C++ concurrency: condition variables"

### Требования

ОС: Любой Linux дистрибутив, Windows с MSVC или WSL.
Компилятор: GCC 8.x, Clang 5.x, с поддержкой C++17.
На остальных платформах работа возможна, но не проверена.


### До начала

1) Установите необходимый toolchain, если ещё не установлен: `make`, `git`, `cmake`, `g++` (либо `clang`, либо `visual studio` на Windows).

2) Убедитесь, что сборка проходит успешно.
```
> cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug .
> cmake --build build
```

3) Для сборки во время workshop'а вы можете использовать cmake в терминале, либо использовать свой IDE (настраивать нужно будет самостоятельно).
