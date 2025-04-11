#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <locale>
#include <codecvt>

const int ROWS = 6;
const int COLS = 7;
const int CELL_SIZE = 100;

enum class Player { None, Player1, Player2 };

std::vector<std::vector<Player>> grid(ROWS, std::vector<Player>(COLS, Player::None));
Player currentPlayer = Player::Player1;

void dropToken(int col) {
    for (int row = ROWS - 1; row >= 0; --row) {
        if (grid[row][col] == Player::None) {
            grid[row][col] = currentPlayer;
            currentPlayer = (currentPlayer == Player::Player1) ? Player::Player2 : Player::Player1;
            break;
        }
    }
}

bool checkWin(Player player) {
    // Check rows
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col <= COLS - 4; ++col) {
            if (grid[row][col] == player && grid[row][col + 1] == player &&
                grid[row][col + 2] == player && grid[row][col + 3] == player) {
                return true;
            }
        }
    }

    // Check columns
    for (int col = 0; col < COLS; ++col) {
        for (int row = 0; row <= ROWS - 4; ++row) {
            if (grid[row][col] == player && grid[row + 1][col] == player &&
                grid[row + 2][col] == player && grid[row + 3][col] == player) {
                return true;
            }
        }
    }

    // Check diagonals (top-left to bottom-right)
    for (int row = 0; row <= ROWS - 4; ++row) {
        for (int col = 0; col <= COLS - 4; ++col) {
            if (grid[row][col] == player && grid[row + 1][col + 1] == player &&
                grid[row + 2][col + 2] == player && grid[row + 3][col + 3] == player) {
                return true;
            }
        }
    }

    // Check diagonals (bottom-left to top-right)
    for (int row = 3; row < ROWS; ++row) {
        for (int col = 0; col <= COLS - 4; ++col) {
            if (grid[row][col] == player && grid[row - 1][col + 1] == player &&
                grid[row - 2][col + 2] == player && grid[row - 3][col + 3] == player) {
                return true;
            }
        }
    }

    return false;
}

void drawGrid(sf::RenderWindow &window) {
    for (int row = 0; row <= ROWS; ++row) {
        sf::RectangleShape line(sf::Vector2f(COLS * CELL_SIZE, 2));
        line.setPosition(0, row * CELL_SIZE);
        line.setFillColor(sf::Color::White);
        window.draw(line);
    }

    for (int col = 0; col <= COLS; ++col) {
        sf::RectangleShape line(sf::Vector2f(2, ROWS * CELL_SIZE));
        line.setPosition(col * CELL_SIZE, 0);
        line.setFillColor(sf::Color::White);
        window.draw(line);
    }
}

void drawTokens(sf::RenderWindow &window) {
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            if (grid[row][col] != Player::None) {
                sf::CircleShape token(CELL_SIZE / 2 - 10);
                token.setPosition(col * CELL_SIZE + 10, row * CELL_SIZE + 10);
                token.setFillColor(grid[row][col] == Player::Player1 ? sf::Color::Red : sf::Color::Yellow);
                window.draw(token);
            }
        }
    }
}

void resetGame() {
    grid = std::vector<std::vector<Player>>(ROWS, std::vector<Player>(COLS, Player::None));
    currentPlayer = Player::Player1;
}

// Removed UTF-8 conversion and replaced the message with English text

void showEndGamePopup(sf::RenderWindow &window, const std::string &message, sf::Font &font) {
    sf::RectangleShape popup(sf::Vector2f(400, 200));
    popup.setFillColor(sf::Color(50, 50, 50));
    popup.setOutlineColor(sf::Color::White);
    popup.setOutlineThickness(2);
    popup.setPosition((COLS * CELL_SIZE - 400) / 2, (ROWS * CELL_SIZE - 200) / 2);

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
                    resetGame();
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
        drawTokens(window);
        window.draw(popup);
        window.draw(popupText);
        window.draw(restartButton);
        window.draw(quitButton);
        window.display();
    }
}

int main() {
    // Adjust window size to fit the grid
    sf::RenderWindow window(sf::VideoMode(COLS * CELL_SIZE, ROWS * CELL_SIZE), "Puissance 4");

    sf::Font font;
    if (!font.loadFromFile("GOODDP__.TTF")) {
        std::cerr << "Failed to load font!" << std::endl;
        return -1;
    }

    sf::Text winText;
    winText.setFont(font);
    winText.setCharacterSize(50);
    winText.setFillColor(sf::Color::White);
    winText.setStyle(sf::Text::Bold);

    // Main game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                int col = event.mouseButton.x / CELL_SIZE;
                if (col >= 0 && col < COLS) {
                    dropToken(col);
                    if (checkWin(Player::Player1)) {
                        showEndGamePopup(window, "Player 1 won!!", font);
                    } else if (checkWin(Player::Player2)) {
                        showEndGamePopup(window, "Player 2 won!!", font);
                    }
                }
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
                resetGame();
            }
        }

        // Clear the screen
        window.clear();

        // Draw the game grid
        drawGrid(window);

        // Draw the tokens
        drawTokens(window);

        // Display the contents of the window
        window.display();
    }

    return 0;
}