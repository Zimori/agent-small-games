#include <SFML/Graphics.hpp>
#include <array>
#include <iostream>

const int WINDOW_SIZE = 600;
const int GRID_SIZE = 3;
const int CELL_SIZE = WINDOW_SIZE / GRID_SIZE;

enum class Player { None, X, O };

class TicTacToe {
public:
    TicTacToe() {
        reset();
    }

    void reset() {
        board.fill(Player::None);
        currentPlayer = Player::X;
        gameOver = false;
    }

    bool makeMove(int row, int col) {
        if (gameOver || board[row * GRID_SIZE + col] != Player::None) {
            return false;
        }
        board[row * GRID_SIZE + col] = currentPlayer;
        checkGameOver();
        currentPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
        return true;
    }

    Player getCell(int row, int col) const {
        return board[row * GRID_SIZE + col];
    }

    Player getWinner() const {
        return winner;
    }

    bool isGameOver() const {
        return gameOver;
    }

private:
    std::array<Player, GRID_SIZE * GRID_SIZE> board;
    Player currentPlayer;
    Player winner = Player::None;
    bool gameOver = false;

    void checkGameOver() {
        // Check rows and columns
        for (int i = 0; i < GRID_SIZE; ++i) {
            if (checkLine(i * GRID_SIZE, 1) || checkLine(i, GRID_SIZE)) {
                gameOver = true;
                return;
            }
        }

        // Check diagonals
        if (checkLine(0, GRID_SIZE + 1) || checkLine(GRID_SIZE - 1, GRID_SIZE - 1)) {
            gameOver = true;
            return;
        }

        // Check for draw
        gameOver = std::all_of(board.begin(), board.end(), [](Player p) { return p != Player::None; });
    }

    bool checkLine(int start, int step) {
        Player first = board[start];
        if (first == Player::None) return false;
        for (int i = 1; i < GRID_SIZE; ++i) {
            if (board[start + i * step] != first) return false;
        }
        winner = first;
        return true;
    }
};

void drawGrid(sf::RenderWindow& window) {
    for (int i = 1; i < GRID_SIZE; ++i) {
        sf::RectangleShape line(sf::Vector2f(WINDOW_SIZE, 2));
        line.setPosition(0, i * CELL_SIZE);
        line.setFillColor(sf::Color::Black); // Changed to black for visibility
        window.draw(line);

        line.setSize(sf::Vector2f(2, WINDOW_SIZE));
        line.setPosition(i * CELL_SIZE, 0);
        line.setFillColor(sf::Color::Black); // Changed to black for visibility
        window.draw(line);
    }
}

void drawBoard(sf::RenderWindow& window, const TicTacToe& game) {
    sf::Font font;
    if (!font.loadFromFile("GOODDP__.TTF")) {
        std::cerr << "Failed to load font!\n";
        return;
    }

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(CELL_SIZE / 2);
    text.setFillColor(sf::Color::Black);

    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            Player cell = game.getCell(row, col);
            if (cell == Player::None) continue;

            text.setString(cell == Player::X ? "X" : "O");
            text.setPosition(col * CELL_SIZE + CELL_SIZE / 4, row * CELL_SIZE + CELL_SIZE / 4);
            window.draw(text);
        }
    }
}

void showEndGamePopup(sf::RenderWindow &window, const std::string &message, sf::Font &font, TicTacToe &game) {
    sf::RectangleShape popup(sf::Vector2f(400, 200));
    popup.setFillColor(sf::Color(50, 50, 50));
    popup.setOutlineColor(sf::Color::White);
    popup.setOutlineThickness(2);
    popup.setPosition((WINDOW_SIZE - 400) / 2, (WINDOW_SIZE - 200) / 2);

    sf::Text popupText;
    popupText.setFont(font);
    popupText.setString(message);
    popupText.setCharacterSize(30);
    popupText.setFillColor(sf::Color::White);
    popupText.setPosition(popup.getPosition().x + 50, popup.getPosition().y + 30);

    sf::Text restartButton;
    restartButton.setFont(font);
    restartButton.setString("Restart");
    restartButton.setCharacterSize(25);
    restartButton.setFillColor(sf::Color::Green);
    restartButton.setPosition(popup.getPosition().x + 50, popup.getPosition().y + 120);

    sf::Text quitButton;
    quitButton.setFont(font);
    quitButton.setString("Quit");
    quitButton.setCharacterSize(25);
    quitButton.setFillColor(sf::Color::Red);
    quitButton.setPosition(popup.getPosition().x + 250, popup.getPosition().y + 120);

    while (true) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

                if (restartButton.getGlobalBounds().contains(mousePos)) {
                    game.reset();
                    return;
                }

                if (quitButton.getGlobalBounds().contains(mousePos)) {
                    window.close();
                    return;
                }
            }
        }

        window.clear();
        drawGrid(window);
        drawBoard(window, game);
        window.draw(popup);
        window.draw(popupText);
        window.draw(restartButton);
        window.draw(quitButton);
        window.display();
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Tic-Tac-Toe");
    TicTacToe game;

    sf::Font font;
    if (!font.loadFromFile("GOODDP__.TTF")) {
        std::cerr << "Failed to load font!\n";
        return -1;
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed && !game.isGameOver()) {
                int row = event.mouseButton.y / CELL_SIZE;
                int col = event.mouseButton.x / CELL_SIZE;
                game.makeMove(row, col);

                if (game.isGameOver()) {
                    std::string message = (game.getWinner() == Player::X) ? "Player X won!" :
                                          (game.getWinner() == Player::O) ? "Player O won!" : "It's a draw!";
                    showEndGamePopup(window, message, font, game);
                }
            }
        }

        window.clear(sf::Color::White);
        drawGrid(window);
        drawBoard(window, game);
        window.display();
    }

    return 0;
}