#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <map>
#include <string>
#include <iostream>

const int GRID_WIDTH = 10;
const int GRID_HEIGHT = 20;
const int TILE_SIZE = 30;
const std::string FONT_PATH = "extern/fonts/PixelatedElegance.ttf";

// Update Tetrimino shapes to include all standard Tetris pieces
const std::array<std::array<int, 4>, 7> TETRIMINOS = {{
    {1, 3, 5, 7}, // I
    {2, 4, 5, 7}, // Z
    {3, 5, 4, 6}, // S
    {3, 5, 4, 7}, // T
    {2, 3, 5, 7}, // L
    {3, 5, 7, 6}, // J
    {2, 3, 4, 5}  // O
}};

// Add color mapping for Tetriminos
const std::array<sf::Color, 7> TETRIMINO_COLORS = {
    sf::Color::Cyan, sf::Color::Red, sf::Color::Green, sf::Color::Magenta,
    sf::Color::Blue, sf::Color::Yellow, sf::Color::White};

struct Tetrimino
{
    int shapeIndex;
    int rotation; // 0, 1, 2, 3
    int x, y;
    Tetrimino(int shapeIndex_) : shapeIndex(shapeIndex_), rotation(0), x(GRID_WIDTH / 2 - 1), y(0) {}
};

// Helper to get rotated block positions
std::array<sf::Vector2i, 4> getBlockPositions(const Tetrimino &t)
{
    std::array<sf::Vector2i, 4> positions;
    for (int i = 0; i < 4; ++i)
    {
        int px = TETRIMINOS[t.shapeIndex][i] % 2;
        int py = TETRIMINOS[t.shapeIndex][i] / 2;
        // Rotate around (1,1) as pivot
        for (int r = 0; r < t.rotation; ++r)
        {
            int tmp = px;
            px = 1 - (py - 1);
            py = tmp;
        }
        positions[i] = sf::Vector2i(t.x + px, t.y + py);
    }
    return positions;
}

bool isValidPosition(const Tetrimino &tetrimino, const std::vector<std::vector<int>> &grid)
{
    auto positions = getBlockPositions(tetrimino);
    for (const auto &pos : positions)
    {
        int x = pos.x, y = pos.y;
        if (x < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT || (y >= 0 && grid[y][x] != 0))
            return false;
    }
    return true;
}

void placeTetrimino(const Tetrimino &tetrimino, std::vector<std::vector<int>> &grid)
{
    auto positions = getBlockPositions(tetrimino);
    for (const auto &pos : positions)
    {
        if (pos.y >= 0)
            grid[pos.y][pos.x] = tetrimino.shapeIndex + 1;
    }
}

void rotateTetrimino(Tetrimino &tetrimino, const std::vector<std::vector<int>> &grid)
{
    // Do not rotate the O (square) piece
    if (tetrimino.shapeIndex == 6)
        return;
    Tetrimino temp = tetrimino;
    temp.rotation = (temp.rotation + 1) % 4;
    if (isValidPosition(temp, grid))
    {
        tetrimino = temp;
        return;
    }
    const std::array<sf::Vector2i, 5> kicks = {
        sf::Vector2i(-1, 0), sf::Vector2i(1, 0), sf::Vector2i(0, -1), sf::Vector2i(-2, 0), sf::Vector2i(2, 0)};
    for (const auto &kick : kicks)
    {
        Tetrimino kicked = temp;
        kicked.x += kick.x;
        kicked.y += kick.y;
        if (isValidPosition(kicked, grid))
        {
            tetrimino = kicked;
            return;
        }
    }
    // If all fail, do not rotate
}

// Add scoring system
int score = 0;
const std::map<int, int> SCORE_TABLE = {
    {1, 40}, {2, 100}, {3, 300}, {4, 1200}};

// Add line clearing logic
void clearLines(std::vector<std::vector<int>> &grid)
{
    int linesCleared = 0;
    for (int y = GRID_HEIGHT - 1; y >= 0; --y)
    {
        bool isFullLine = true;
        for (int x = 0; x < GRID_WIDTH; ++x)
        {
            if (grid[y][x] == 0)
            {
                isFullLine = false;
                break;
            }
        }
        if (isFullLine)
        {
            for (int row = y; row > 0; --row)
            {
                grid[row] = grid[row - 1];
            }
            grid[0] = std::vector<int>(GRID_WIDTH, 0);
            ++y; // Recheck the same row after shifting
            ++linesCleared;
        }
    }
    if (linesCleared > 0)
    {
        score += SCORE_TABLE.at(linesCleared);
    }
}

int main()
{
    // Extend the window width to fit the score display
    const int SCORE_PANEL_WIDTH = 150;
    sf::RenderWindow window(sf::VideoMode(GRID_WIDTH * TILE_SIZE + SCORE_PANEL_WIDTH, GRID_HEIGHT * TILE_SIZE), "Tetris");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile(FONT_PATH))
    {
        std::cerr << "Failed to load font from: " << FONT_PATH << "\n";
        return -1; // Handle error if font fails to load
    }

    // Initialize grid
    std::vector<std::vector<int>> grid(GRID_HEIGHT, std::vector<int>(GRID_WIDTH, 0));

    // Initialize random seed
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Create the first Tetrimino and the next one
    Tetrimino currentTetrimino(std::rand() % 7);
    Tetrimino nextTetrimino(std::rand() % 7);

    Tetrimino* heldTetrimino = nullptr;
    bool canHold = true;

    sf::Clock clock;
    float fallDelay = 0.5f; // Time between automatic falls
    float fallTimer = 0.0f;

    bool paused = false;
    bool gameOver = false;

    // Main game loop
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
                if (gameOver) {
                    if (event.key.code == sf::Keyboard::R) {
                        // Reset game state
                        grid = std::vector<std::vector<int>>(GRID_HEIGHT, std::vector<int>(GRID_WIDTH, 0));
                        score = 0;
                        currentTetrimino = Tetrimino(std::rand() % 7);
                        nextTetrimino = Tetrimino(std::rand() % 7);
                        gameOver = false;
                        paused = false;
                        clock.restart();
                    }
                    continue;
                }
                if (paused) continue;
                Tetrimino temp = currentTetrimino;
                if (event.key.code == sf::Keyboard::Left)
                {
                    temp.x -= 1;
                    if (isValidPosition(temp, grid)) currentTetrimino = temp;
                }
                else if (event.key.code == sf::Keyboard::Right)
                {
                    temp.x += 1;
                    if (isValidPosition(temp, grid)) currentTetrimino = temp;
                }
                else if (event.key.code == sf::Keyboard::Down)
                {
                    temp.y += 1;
                    if (isValidPosition(temp, grid)) currentTetrimino = temp;
                }
                else if (event.key.code == sf::Keyboard::Up)
                {
                    rotateTetrimino(temp, grid);
                    if (isValidPosition(temp, grid)) currentTetrimino = temp;
                }
                else if (event.key.code == sf::Keyboard::Space)
                {
                    // Hard drop effect
                    Tetrimino drop = currentTetrimino;
                    while (true) {
                        Tetrimino next = drop;
                        next.y += 1;
                        if (!isValidPosition(next, grid)) break;
                        drop = next;
                    }
                    // Optional: cool effect - flash the piece as it lands
                    for (int flash = 0; flash < 3; ++flash) {
                        window.clear(sf::Color::Black);
                        // Draw grid
                        for (int y = 0; y < GRID_HEIGHT; ++y) {
                            for (int x = 0; x < GRID_WIDTH; ++x) {
                                if (grid[y][x] != 0) {
                                    sf::RectangleShape tile(sf::Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                                    tile.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                                    tile.setFillColor(TETRIMINO_COLORS[grid[y][x] - 1]);
                                    window.draw(tile);
                                }
                            }
                        }
                        // Draw the dropped Tetrimino with a flash color
                        auto flashPositions = getBlockPositions(drop);
                        for (const auto& pos : flashPositions) {
                            if (pos.y >= 0) {
                                sf::RectangleShape tile(sf::Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                                tile.setPosition(pos.x * TILE_SIZE, pos.y * TILE_SIZE);
                                tile.setFillColor(flash % 2 == 0 ? sf::Color::White : TETRIMINO_COLORS[drop.shapeIndex]);
                                window.draw(tile);
                            }
                        }
                        // Draw the rest of the UI (score panel, next piece, etc.)
                        sf::RectangleShape scorePanel(sf::Vector2f(SCORE_PANEL_WIDTH, GRID_HEIGHT * TILE_SIZE));
                        scorePanel.setPosition(GRID_WIDTH * TILE_SIZE, 0);
                        scorePanel.setFillColor(sf::Color(30, 30, 30));
                        window.draw(scorePanel);
                        sf::Text scoreText;
                        scoreText.setFont(font);
                        scoreText.setString("Score: \n" + std::to_string(score));
                        scoreText.setCharacterSize(24);
                        scoreText.setFillColor(sf::Color::White);
                        scoreText.setPosition(GRID_WIDTH * TILE_SIZE + 00, 30);
                        window.draw(scoreText);
                        sf::Text nextLabel;
                        nextLabel.setFont(font);
                        nextLabel.setString("Next:");
                        nextLabel.setCharacterSize(20);
                        nextLabel.setFillColor(sf::Color::White);
                        nextLabel.setPosition(GRID_WIDTH * TILE_SIZE + 20, 150);
                        window.draw(nextLabel);
                        int offsetX = GRID_WIDTH * TILE_SIZE + 40;
                        int offsetY = 200;
                        for (int i = 0; i < 4; ++i) {
                            int px = TETRIMINOS[nextTetrimino.shapeIndex][i] % 2;
                            int py = TETRIMINOS[nextTetrimino.shapeIndex][i] / 2;
                            sf::RectangleShape tile(sf::Vector2f(TILE_SIZE - 4, TILE_SIZE - 4));
                            tile.setPosition(offsetX + px * TILE_SIZE, offsetY + py * TILE_SIZE);
                            tile.setFillColor(TETRIMINO_COLORS[nextTetrimino.shapeIndex]);
                            window.draw(tile);
                        }
                        window.display();
                        sf::sleep(sf::milliseconds(40));
                    }
                    // Place the Tetrimino on the grid
                    placeTetrimino(drop, grid);
                    clearLines(grid);
                    currentTetrimino = nextTetrimino;
                    nextTetrimino = Tetrimino(std::rand() % 7);
                    if (!isValidPosition(currentTetrimino, grid))
                        gameOver = true;
                }
                if (!gameOver && !paused && event.key.code == sf::Keyboard::C && canHold) {
                    if (!heldTetrimino) {
                        heldTetrimino = new Tetrimino(currentTetrimino.shapeIndex);
                        currentTetrimino = nextTetrimino;
                        nextTetrimino = Tetrimino(std::rand() % 7);
                    } else {
                        std::swap(currentTetrimino.shapeIndex, heldTetrimino->shapeIndex);
                        currentTetrimino.rotation = 0;
                        currentTetrimino.x = GRID_WIDTH / 2 - 1;
                        currentTetrimino.y = 0;
                    }
                    canHold = false;
                }
            }
        }

        if (!paused && !gameOver) {
            // Handle automatic falling
            fallTimer += clock.restart().asSeconds();
            if (fallTimer >= fallDelay)
            {
                fallTimer = 0.0f;
                Tetrimino temp = currentTetrimino;
                temp.y += 1;
                if (isValidPosition(temp, grid))
                {
                    currentTetrimino = temp;
                }
                else
                {
                    // Place the Tetrimino on the grid
                    placeTetrimino(currentTetrimino, grid);

                    // Clear completed lines
                    clearLines(grid);

                    // Move nextTetrimino to current, and generate a new nextTetrimino
                    currentTetrimino = nextTetrimino;
                    nextTetrimino = Tetrimino(std::rand() % 7);

                    // Check for game over
                    if (!isValidPosition(currentTetrimino, grid))
                    {
                        gameOver = true;
                    }
                    canHold = true;
                }
            }
        } else if (paused) {
            // If paused, reset the clock to avoid time jump
            clock.restart();
        }

        // Clear the window
        window.clear(sf::Color::Black);

        // Draw the grid
        for (int y = 0; y < GRID_HEIGHT; ++y)
        {
            for (int x = 0; x < GRID_WIDTH; ++x)
            {
                if (grid[y][x] != 0)
                {
                    sf::RectangleShape tile(sf::Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                    tile.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                    tile.setFillColor(TETRIMINO_COLORS[grid[y][x] - 1]);
                    window.draw(tile);
                }
            }
        }

        // Draw the current Tetrimino
        auto positions = getBlockPositions(currentTetrimino);
        for (const auto &pos : positions)
        {
            if (pos.y >= 0)
            {
                sf::RectangleShape tile(sf::Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                tile.setPosition(pos.x * TILE_SIZE, pos.y * TILE_SIZE);
                tile.setFillColor(TETRIMINO_COLORS[currentTetrimino.shapeIndex]);
                window.draw(tile);
            }
        }

        // Draw the score panel background
        sf::RectangleShape scorePanel(sf::Vector2f(SCORE_PANEL_WIDTH, GRID_HEIGHT * TILE_SIZE));
        scorePanel.setPosition(GRID_WIDTH * TILE_SIZE, 0);
        scorePanel.setFillColor(sf::Color(30, 30, 30));
        window.draw(scorePanel);

        // Draw the score
        sf::Text scoreText;
        scoreText.setFont(font);
        // break line after "Score:", before the actual score
        scoreText.setString("Score: \n" + std::to_string(score));
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(GRID_WIDTH * TILE_SIZE + 00, 30);
        window.draw(scoreText);

        // Draw the label for next piece
        sf::Text nextLabel;
        nextLabel.setFont(font);
        nextLabel.setString("Next:");
        nextLabel.setCharacterSize(20);
        nextLabel.setFillColor(sf::Color::White);
        nextLabel.setPosition(GRID_WIDTH * TILE_SIZE + 20, 150);
        window.draw(nextLabel);

        // Center the next piece in the panel
        int offsetX = GRID_WIDTH * TILE_SIZE + 40;
        int offsetY = 200;
        for (int i = 0; i < 4; ++i)
        {
            int px = TETRIMINOS[nextTetrimino.shapeIndex][i] % 2;
            int py = TETRIMINOS[nextTetrimino.shapeIndex][i] / 2;
            sf::RectangleShape tile(sf::Vector2f(TILE_SIZE - 4, TILE_SIZE - 4));
            tile.setPosition(offsetX + px * TILE_SIZE, offsetY + py * TILE_SIZE);
            tile.setFillColor(TETRIMINO_COLORS[nextTetrimino.shapeIndex]);
            window.draw(tile);
        }

        // Draw the hold panel
        sf::Text holdLabel;
        holdLabel.setFont(font);
        holdLabel.setString("Hold:");
        holdLabel.setCharacterSize(20);
        holdLabel.setFillColor(sf::Color::White);
        holdLabel.setPosition(GRID_WIDTH * TILE_SIZE + 20, 320);
        window.draw(holdLabel);
        if (heldTetrimino) {
            int offsetX = GRID_WIDTH * TILE_SIZE + 40;
            int offsetY = 350;
            for (int i = 0; i < 4; ++i) {
                int px = TETRIMINOS[heldTetrimino->shapeIndex][i] % 2;
                int py = TETRIMINOS[heldTetrimino->shapeIndex][i] / 2;
                sf::RectangleShape tile(sf::Vector2f(TILE_SIZE - 4, TILE_SIZE - 4));
                tile.setPosition(offsetX + px * TILE_SIZE, offsetY + py * TILE_SIZE);
                tile.setFillColor(TETRIMINO_COLORS[heldTetrimino->shapeIndex]);
                window.draw(tile);
            }
        }

        // Draw pause overlay if paused
        if (paused) {
            sf::RectangleShape overlay(sf::Vector2f(GRID_WIDTH * TILE_SIZE + SCORE_PANEL_WIDTH, GRID_HEIGHT * TILE_SIZE));
            overlay.setFillColor(sf::Color(0, 0, 0, 120));
            overlay.setPosition(0, 0);
            window.draw(overlay);

            sf::Text pauseText;
            pauseText.setFont(font);
            pauseText.setString("PAUSE");
            pauseText.setCharacterSize(48);
            pauseText.setFillColor(sf::Color::White);
            pauseText.setStyle(sf::Text::Bold);
            // Center the text in the window
            sf::FloatRect textRect = pauseText.getLocalBounds();
            pauseText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            pauseText.setPosition((GRID_WIDTH * TILE_SIZE + SCORE_PANEL_WIDTH) / 2.0f, GRID_HEIGHT * TILE_SIZE / 2.0f);
            window.draw(pauseText);
        }

        // Draw GAME OVER overlay if game is over
        if (gameOver) {
            sf::RectangleShape overlay(sf::Vector2f(GRID_WIDTH * TILE_SIZE + SCORE_PANEL_WIDTH, GRID_HEIGHT * TILE_SIZE));
            overlay.setFillColor(sf::Color(0, 0, 0, 180));
            overlay.setPosition(0, 0);
            window.draw(overlay);

            // Draw GAME OVER text
            sf::Text overText;
            overText.setFont(font);
            overText.setString("GAME OVER");
            overText.setCharacterSize(48);
            overText.setFillColor(sf::Color::Red);
            overText.setStyle(sf::Text::Bold);
            sf::FloatRect overRect = overText.getLocalBounds();
            overText.setOrigin(overRect.left + overRect.width / 2.0f, overRect.top + overRect.height / 2.0f);
            float centerX = (GRID_WIDTH * TILE_SIZE + SCORE_PANEL_WIDTH) / 2.0f;
            float centerY = GRID_HEIGHT * TILE_SIZE / 2.0f;
            overText.setPosition(centerX, centerY - 20);
            window.draw(overText);

            // Draw restart instruction text, smaller and just below GAME OVER
            sf::Text restartText;
            restartText.setFont(font);
            restartText.setString("Press R to restart");
            restartText.setCharacterSize(22);
            restartText.setFillColor(sf::Color::White);
            restartText.setStyle(sf::Text::Regular);
            sf::FloatRect restartRect = restartText.getLocalBounds();
            restartText.setOrigin(restartRect.left + restartRect.width / 2.0f, restartRect.top + restartRect.height / 2.0f);
            restartText.setPosition(centerX, centerY + overRect.height / 2.0f + restartRect.height);
            window.draw(restartText);
        }

        // Display the window
        window.display();
    }

    if (heldTetrimino) delete heldTetrimino;

    return 0;
}