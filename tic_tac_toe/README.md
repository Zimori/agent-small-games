# Tic-Tac-Toe

A simple **Tic-Tac-Toe** game implemented in C++ using the **SFML** library for graphics.

## Features

- Two-player gameplay (Player X vs Player O).
- Visual grid and symbols rendered using SFML.
- Detects wins for rows, columns, and diagonals.
- End-game popup with options to restart or quit.

## Requirements

- **C++ Compiler** (e.g., g++)
- **SFML Library** (Simple and Fast Multimedia Library)

## Installation

1. Clone the repository:

   ```bash
   git clone git@github.com-Zimori:Zimori/agent-small-games.git
   cd agent-small-games/tic_tac_toe
   ```

2. Install SFML (if not already installed):

   ```bash
   sudo apt-get install libsfml-dev
   ```

3. Compile the game:

   ```bash
   g++ main.cpp -o tic_tac_toe -lsfml-graphics -lsfml-window -lsfml-system
   ```

4. Run the game:

   ```bash
   ./tic_tac_toe
   ```

## How to Play

1. Launch the game.
2. Players take turns clicking on a cell to place their symbol (X or O).
   - **Player X**: Starts first.
   - **Player O**: Plays after Player X.
3. The first player to align three symbols in a row, column, or diagonal wins.
4. Use the popup options to restart or quit the game when it ends.

## Controls

- **Mouse Left Click**: Place a symbol in the selected cell.
- **Close Button**: Exit the game.

## File Structure

```
agent-small-games/
└── tic_tac_toe/
    ├── GOODDP__.TTF    # Font file for text rendering
    ├── main.cpp        # Main game logic
    └── README.md       # Project documentation
```

## Known Issues

- Ensure the font file `GOODDP__.TTF` is in the same directory as the executable.
- The game does not handle invalid input (e.g., clicking outside the grid).

## Future Improvements

- Add AI for single-player mode.
- Improve the UI with animations and sound effects.
- Add a menu screen for better user experience.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Acknowledgments

- [SFML](https://www.sfml-dev.org/) for the graphics library.
- Fonts used: `GOODDP__.TTF`.

---
Enjoy the game!