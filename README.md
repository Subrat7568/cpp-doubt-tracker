# C++ OOP Doubt-Tracker

A simple command-line app to log coding doubts (DSA, C++ concepts, general), track solved/unsolved status, search, and see which topics to revise first.

## OOP concepts used
- **Encapsulation**: `Doubt` keeps its data private, exposed via getters/setters.
- **Inheritance**: `DsaDoubt`, `CppDoubt`, `GeneralDoubt` extend `Doubt`.
- **Polymorphism**: each subclass overrides `type()` and `hint()`.
- **File I/O**: doubts are saved to `doubts.txt` and reloaded on start.

## Features
Add doubt, answer/follow-up thread, filter by type, keyword search, filter by type and status, edit/delete doubts, sort by priority, input validation, mark solved/unsolved, stats of unsolved doubts by topic.

## Build & run
```
g++ -std=c++17 -o doubt src/main.cpp
./doubt          # on Windows: doubt.exe
```
`web-version.html` is a browser version of the same idea (open it in any browser; data is kept in localStorage; the AI answer feature only works on claude.ai).
