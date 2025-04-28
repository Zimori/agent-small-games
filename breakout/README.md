# Breakout

A classic brick breaker game made in C++ using the SFML library.

## Features

- Control a paddle to bounce the ball and destroy all bricks.
- Multiple progressive levels with varied brick layouts.
- Score system and lives management.
- Current level display.
- Retro graphical interface with pixel fonts.

## Controls

| Key                  | Action                                  |
|----------------------|-----------------------------------------|
| Left/Right Arrow     | Move the paddle                         |
| Space                | Launch the ball                         |
| R                    | Restart after game over/victory         |
| Escape               | Quit the game                           |

## Build Instructions

Make sure SFML is installed:

```bash
sudo apt-get install libsfml-dev
```

Build the game:

```bash
make breakout
```

Or manually:

```bash
g++ main.cpp -o breakout -lsfml-graphics -lsfml-window -lsfml-system
```

## Running

```bash
./bin/breakout
```

## Gameplay

- Destroy all bricks to advance to the next level.
- If the ball falls below the paddle, you lose a life.
- The game ends when you run out of lives or complete all levels.

## Project Structure

```
breakout/
  main.cpp         # Main source code
  README.md        # This file
extern/fonts/      # Fonts used
```

## License

This project is licensed under the MIT License. See the LICENSE file.

---
Enjoy the game!
