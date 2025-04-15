# Tetris

A simple **Tetris** game implemented in C++ using the **SFML** library for graphics.

## Features

- Classic Tetris gameplay with falling blocks.
- Grid-based rendering using SFML.
- Detects and clears completed rows.
- Scoring system to track progress.

## Requirements

- **C++ Compiler** (e.g., g++)
- **SFML Library** (Simple and Fast Multimedia Library)

## Installation

1. Clone the repository:

   ```bash
   git clone git@github.com-Zimori:Zimori/agent-small-games.git
   cd agent-small-games/tetris
   ```

2. Install SFML (if not already installed):

   ```bash
   sudo apt-get install libsfml-dev
   ```

3. Compile the game (recommended):

   ```bash
   make tetris
   ```

   Or compile manually:

   ```bash
   g++ main.cpp -o tetris -lsfml-graphics -lsfml-window -lsfml-system
   ```

4. Run the game:

   ```bash
   ./tetris
   ```

## How to Play

1. Launch the game.
2. Use the keyboard to control the falling blocks:
   - **Arrow Left/Right**: Move the block left or right.
   - **Arrow Down**: Speed up the block's fall.
   - **Space**: Rotate the block.
3. Complete rows to score points and clear them from the grid.
4. The game ends when the blocks reach the top of the grid.

## Controls

- **Arrow Keys**: Move and rotate blocks.
- **Escape**: Exit the game.

## File Structure

```
agent-small-games/
└── tetris/
    ├── main.cpp        # Main game logic
    └── README.md       # Project documentation
```

## Known Issues

- Ensure the SFML library is properly installed before compiling.
- The game currently lacks sound effects and advanced animations.

## Future Improvements

- Add sound effects and background music.
- Implement a pause menu.
- Add different levels of difficulty.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Acknowledgments

- [SFML](https://www.sfml-dev.org/) for the graphics library.

---
Enjoy the game!