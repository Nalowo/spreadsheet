# spreadsheet
# Учебный проект: Электронная таблица
Хранит значения ячеек – любой текст, вычисляет формулы вида – “=(A1 + 40)  /  (G12 * H3 - 2)”. Вычисление формул работает на основе синтаксического анализатора ANTLR.  
Максимальное количество строк и столбцов - 16384.  

* Создание таблицы:

```C++
auto sheet = CreateSheet();
```

* Создание текстовой ячейки:  

```C++
sheet->SetCell(Position::FromString("C2"), "Me gusta");
```

Ячейка будет создана как текстовая, формульные ячейки начинаются с знака “=”, если мы хотим создать текстовую ячейку начинающуюся с “=”, но не хотим чтобы она интерпретировалась как формульная, нужно перед “=” поставить знак “`”  

```C++
sheet->SetCell(Position::FromString("C2"), "`=Me gusta");
```

* Создание формульной ячейки: 

Выражение формульной ячейки всегда начинается со знака “=”.  

```C++
sheet->SetCell(Position::FromString("C2"), “=(A1 + 40)  /  (G12 * H3 - 2)”);
```

-- В случае есть не удастся распарсить формулу, значение ячейки в которую добавлялась формула не изменится и будет выброшено исключение FormulaException  
-- В случае если формула имеет циклическую зависимость (А1 = B1 + A1), будет выброшено исключение CircularDependencyException, при этом, содержимое изменяемой ячейки не изменится  

* Получение указателя на ячейку:  

```C++
sheet->GetCell(Position::FromString("C2"));
```

* Печать таблицы:  

С представлением содержимого ячеек как текста:  
```C++
sheet->PrintTexts();
```

С представлением как значений:  

```C++
sheet->PrintValues();
```

При запросе значения ячейки происходит вычисление значения формульных ячеек.  
- Если в выражение формулы есть ссылка на пустую ячейку, то ячейка интерпретируется как 0  
- В случае есть при вычислении произошло деление на ноль, то выводится “#DIV/0!”  
- В случае если формула содержит не корректную позицию ячейки выводится значение “#REF!”  
- В случае если содержимое одной из ячеек в формуле не удалось преобразовать в числовой тип, выводится значение #VALUE!  

# Компиляция:
Понадобится ANTLR не ниже Version 4.13.0, инструкция по установке https://github.com/antlr/antlr4/blob/master/doc/getting-started.md  
На сайте ANTLR нужно скачать архив antlr4-cpp-runtime*.zip из раздела Download https://www.antlr.org/download.html  

* Команда генерации файлов анализатора для C++:  

```
java -jar antlr-4.13.0-complete.jar -Dlanguage=Cpp Formula.g4
```
Полученные файлы нужно положить в директорию src, с кодом проекта  

* Структура проекта:

src/  
├── antlr4_runtime/  
│   └── Содержимое архива antlr4-cpp-runtime*.zip.  
├── build/  
├── antlr-4.12.0-complete.jar  
├── CMakeLists.txt  
├── FindANTLR.cmake  
├── Formula.g4  
├── Остальные файлы проекта  
└── ...  

* Команды компиляции:

```
cmake .. -DCMAKE_BUILD_TYPE=Debug\Release -G "MinGW Makefiles"
cmake --build .
```

* Компилятор:
g++.exe (MinGW-W64 x86_64-ucrt-posix-seh, built by Brecht Sanders) 12.2.0

