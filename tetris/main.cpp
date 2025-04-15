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

struct Tetrimino {
    int shapeIndex;
    int rotation; // 0, 1, 2, 3
    int x, y;
    Tetrimino(int shapeIndex_) : shapeIndex(shapeIndex_), rotation(0), x(GRID_WIDTH / 2 - 1), y(0) {}
};

// Helper to get rotated block positions
std::array<sf::Vector2i, 4> getBlockPositions(const Tetrimino& t) {
    std::array<sf::Vector2i, 4> positions;
    for (int i = 0; i < 4; ++i) {
        int px = TETRIMINOS[t.shapeIndex][i] % 2;
        int py = TETRIMINOS[t.shapeIndex][i] / 2;
        // Rotate around (1,1) as pivot
        for (int r = 0; r < t.rotation; ++r) {
            int tmp = px;
            px = 1 - (py - 1);
            py = tmp;
        }
        positions[i] = sf::Vector2i(t.x + px, t.y + py);
    }
    return positions;
}

bool isValidPosition(const Tetrimino &tetrimino, const std::vector<std::vector<int>> &grid) {
    auto positions = getBlockPositions(tetrimino);
    for (const auto& pos : positions) {
        int x = pos.x, y = pos.y;
        if (x < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT || (y >= 0 && grid[y][x] != 0))
            return false;
    }
    return true;
}

void placeTetrimino(const Tetrimino &tetrimino, std::vector<std::vector<int>> &grid) {
    auto positions = getBlockPositions(tetrimino);
    for (const auto& pos : positions) {
        if (pos.y >= 0)
            grid[pos.y][pos.x] = tetrimino.shapeIndex + 1;
    }
}

void rotateTetrimino(Tetrimino &tetrimino, const std::vector<std::vector<int>> &grid) {
    Tetrimino temp = tetrimino;
    temp.rotation = (temp.rotation + 1) % 4;
    // Try normal rotation
    if (isValidPosition(temp, grid)) {
        tetrimino = temp;
        return;
    }
    // Wall kick attempts: left, right, up
    const std::array<sf::Vector2i, 5> kicks = {
        sf::Vector2i(-1, 0), sf::Vector2i(1, 0), sf::Vector2i(0, -1), sf::Vector2i(-2, 0), sf::Vector2i(2, 0)
    };
    for (const auto& kick : kicks) {
        Tetrimino kicked = temp;
        kicked.x += kick.x;
        kicked.y += kick.y;
        if (isValidPosition(kicked, grid)) {
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
    sf::RenderWindow window(sf::VideoMode(GRID_WIDTH * TILE_SIZE, GRID_HEIGHT * TILE_SIZE), "Tetris");
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

    // Create the first Tetrimino
    Tetrimino currentTetrimino(std::rand() % 7);

    sf::Clock clock;
    float fallDelay = 0.5f; // Time between automatic falls
    float fallTimer = 0.0f;

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
            }
        }

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

                // Spawn a new Tetrimino
                currentTetrimino = Tetrimino(std::rand() % 7);

                // Check for game over
                if (!isValidPosition(currentTetrimino, grid))
                {
                    window.close();
                }
            }
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
        for (const auto& pos : positions) {
            if (pos.y >= 0) {
                sf::RectangleShape tile(sf::Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                tile.setPosition(pos.x * TILE_SIZE, pos.y * TILE_SIZE);
                tile.setFillColor(TETRIMINO_COLORS[currentTetrimino.shapeIndex]);
                window.draw(tile);
            }
        }

        // Ensure score is displayed properly
        // Draw the score
        sf::Text scoreText;
        scoreText.setFont(font);
        scoreText.setString("Score: " + std::to_string(score));
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(GRID_WIDTH * TILE_SIZE + 10, 10);
        window.draw(scoreText);

        // Display the window
        window.display();
    }

    return 0;
}