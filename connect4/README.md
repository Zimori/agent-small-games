# Connect Four

A simple **Connect Four** game implemented in C++ using the **SFML** library for graphics.

## Features

- Two-player gameplay (Player 1 vs Player 2).
- Visual grid and tokens rendered using SFML.
- Detects wins for rows, columns, and diagonals.
- Restart the game with the `R` key.
- End-game popup with options to restart or quit.

## Requirements

- **C++ Compiler** (e.g., g++)
- **SFML Library** (Simple and Fast Multimedia Library)

## Installation

1. Clone the repository:

   ```bash
   git clone git@github.com-Zimori:Zimori/agent-small-games.git
   cd agent-small-games/connect4
   ```

2. Install SFML (if not already installed):

   ```bash
   sudo apt-get install libsfml-dev
   ```

3. Compile the game:

   ```bash
   g++ main.cpp -o connect4 -lsfml-graphics -lsfml-window -lsfml-system
   ```

4. Run the game:

   ```bash
   ./connect4
   ```

## How to Play

1. Launch the game.
2. Players take turns clicking on a column to drop their token.
   - **Player 1**: Red tokens.
   - **Player 2**: Yellow tokens.
3. The first player to connect four tokens in a row, column, or diagonal wins.
4. Use the `R` key to restart the game at any time.

## Controls

- **Mouse Left Click**: Drop a token in the selected column.
- **R Key**: Restart the game.
- **Close Button**: Exit the game.

## File Structure

```
agent-small-games/
└── connect4/
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
