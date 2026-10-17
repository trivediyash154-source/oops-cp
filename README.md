# TypeMaster – Typing Speed & Accuracy Trainer

An OOP course project in C++ with an SFML 3 graphical interface.
Type a passage and see your speed (WPM), accuracy, time and mistakes live.

## Features

- **Practice mode**: type the whole passage at your own pace.
- **Timed mode**: type as much as you can in 60 seconds. After a passage is done, the next one loads and the timer keeps running.
- Typing happens directly on the passage: correct letters turn white, wrong ones turn red, and a smooth caret shows where you are.
- Five live stat cards (WPM, Accuracy, Time, Correct, Wrong), a progress bar, and a result screen with a rating.
- Results are saved to `user_stats.csv`. The app reads that file back to show your **best WPM** and a "New personal best!" label.
- Modern dark design: glass cards, rounded corners, soft glow lights, hover animations.

## OOP concepts used

| Concept | Where |
|---|---|
| Class and object | Every `.h` file; `TypeMasterApp app;` in `main()` |
| Encapsulation (private data + public methods) | `Statistics`, `PassageManager`, `Button`, `StatCard` |
| Constructor / initializer list / default arguments | `TimedChallenge(string text, int limit = 60)`, `Button(...) : Widget(...), label(...)` |
| Inheritance | `PracticeMode` and `TimedChallenge` inherit `TypingSession`; `Button` and `StatCard` inherit `Widget`; `RoundedRect` inherits `sf::Shape` |
| Abstract class (pure virtual function) | `TypingSession` (`startSession`, `getModeName`), `Widget` (`draw`) |
| Runtime polymorphism (virtual + override) | `session->getModeName()`, `session->isTimeUp()`, and the `for(Widget* widget : widgets) widget->draw(window);` loop |
| Virtual destructor | `TypingSession`, `Widget` |
| Static members | `RoundedRect::pointsPerCorner`, `Widget::placeText()` |
| Function template | `toString<T>()` in `main.cpp` |
| Exception handling | `PassageManager` and `loadFont()` throw `runtime_error`; `main()` catches it; `FileManager` skips broken CSV rows |
| File handling | `PassageManager` reads `prompts.txt`; `FileManager` writes and reads `user_stats.csv` |

## Files

| File | What it does |
|---|---|
| `main.cpp` | `TypeMasterApp` class: window, events, game logic and drawing |
| `TypingSession.h` | Abstract base class for a typing test |
| `PracticeMode.h`, `TimedChallenge.h` | The two modes (derived classes) |
| `PassageManager.h` | Loads passages from `prompts.txt` |
| `Statistics.h` | Calculates WPM, accuracy, correct and wrong letters |
| `FileManager.h` | Saves results and finds the best WPM |
| `Widget.h`, `Button.h`, `StatCard.h` | GUI widgets (base class + two derived classes) |
| `RoundedRect.h` | Rounded rectangle shape (SFML does not have one) |
| `Theme.h` | All colours and the soft glow effect |
| `assets/fonts/` | Poppins and JetBrains Mono fonts (free, SIL Open Font License) |

## How to build and run

In VS Code, open the project folder (**File > Open Folder**) and press
**Cmd + Shift + B** on Mac or **Ctrl + Shift + B** on Windows.
This builds the game and opens it (task *Build and Run TypeMaster* in `.vscode/tasks.json`).
VS Code will suggest the **C/C++** extension; install it for code completion.

You only need to install the compiler and SFML 3 once:

### macOS

1. Install the Apple compiler (only once): `xcode-select --install`
2. Install [Homebrew](https://brew.sh), then SFML 3: `brew install sfml`
3. Press **Cmd + Shift + B** in VS Code, or build and run from the Terminal inside this folder:
   ```
   clang++ -std=c++17 main.cpp -o main -I"$(brew --prefix)/include" -L"$(brew --prefix)/lib" -lsfml-graphics -lsfml-window -lsfml-system
   ./main
   ```

### Windows (MSYS2)

1. Install the compiler and SFML 3, from the **MSYS2 UCRT64** terminal:
   ```
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-sfml
   ```
2. Press **Ctrl + Shift + B** in VS Code, or build from a terminal inside this folder:
   ```
   g++ -std=c++17 main.cpp -o main.exe -lsfml-graphics -lsfml-window -lsfml-system
   ```
3. If Windows says an SFML `.dll` is missing when you run `main.exe` yourself,
   add `C:\msys64\ucrt64\bin` to your PATH (VS Code's task already does this).

### Linux

Install SFML 3, then use the same `g++` command as Windows (output `main`) and run `./main`.

## Controls

| Key / button | Action |
|---|---|
| `ENTER` or any letter | Start typing |
| `BACKSPACE` | Delete the last letter |
| `TAB` / **Restart** | Restart the current passage |
| `ENTER` on the result screen | Try the same passage again |
| **Practice** / **Timed 60s** | Change mode |
| **Next Passage** | Go to the next passage |
| **Save Result** | Stop the test now and save it |

Add your own passages to `prompts.txt`, one per line.
