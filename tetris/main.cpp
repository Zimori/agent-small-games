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

// Helper to draw the side panel (score, next, hold)
void drawSidePanel(sf::RenderWindow& window, sf::Font& font, int score, const Tetrimino& nextTetrimino, Tetrimino* heldTetrimino) {
    const int SCORE_PANEL_WIDTH = 150;
    const int panelTop = 20;
    const int scoreBoxHeight = 60;
    const int bigBoxHeight = 170;
    const int boxSpacing = 30;
    const int scoreBoxY = panelTop;
    const int nextBoxY = scoreBoxY + scoreBoxHeight + boxSpacing;
    const int holdBoxY = nextBoxY + bigBoxHeight + boxSpacing;
    const int boxWidth = SCORE_PANEL_WIDTH - 30;
    const int pieceTileSize = TILE_SIZE - 1;

    // Draw the score panel background
    sf::RectangleShape scorePanel(sf::Vector2f(SCORE_PANEL_WIDTH, GRID_HEIGHT * TILE_SIZE));
    scorePanel.setPosition(GRID_WIDTH * TILE_SIZE, 0);
    scorePanel.setFillColor(sf::Color(25, 25, 25));
    window.draw(scorePanel);

    // Draw the SCORE box
    sf::RectangleShape scoreBox(sf::Vector2f(boxWidth, scoreBoxHeight));
    scoreBox.setPosition(GRID_WIDTH * TILE_SIZE + 15, scoreBoxY);
    scoreBox.setFillColor(sf::Color(40, 40, 60));
    scoreBox.setOutlineColor(sf::Color::White);
    scoreBox.setOutlineThickness(2);
    window.draw(scoreBox);
    sf::Text scoreLabel;
    scoreLabel.setFont(font);
    scoreLabel.setString("SCORE");
    scoreLabel.setCharacterSize(18);
    scoreLabel.setFillColor(sf::Color(200, 200, 255));
    scoreLabel.setStyle(sf::Text::Bold);
    scoreLabel.setPosition(GRID_WIDTH * TILE_SIZE + 35, scoreBoxY + 6);
    window.draw(scoreLabel);
    sf::Text scoreValue;
    scoreValue.setFont(font);
    scoreValue.setString(std::to_string(score));
    scoreValue.setCharacterSize(28);
    scoreValue.setFillColor(sf::Color::White);
    scoreValue.setStyle(sf::Text::Bold);
    scoreValue.setPosition(GRID_WIDTH * TILE_SIZE + 35, scoreBoxY + 28);
    window.draw(scoreValue);

    // Draw the NEXT box
    sf::RectangleShape nextBox(sf::Vector2f(boxWidth, bigBoxHeight));
    nextBox.setPosition(GRID_WIDTH * TILE_SIZE + 15, nextBoxY);
    nextBox.setFillColor(sf::Color(40, 60, 40));
    nextBox.setOutlineColor(sf::Color::White);
    nextBox.setOutlineThickness(2);
    window.draw(nextBox);
    sf::Text nextLabel;
    nextLabel.setFont(font);
    nextLabel.setString("NEXT");
    nextLabel.setCharacterSize(18);
    nextLabel.setFillColor(sf::Color(200, 255, 200));
    nextLabel.setStyle(sf::Text::Bold);
    nextLabel.setPosition(GRID_WIDTH * TILE_SIZE + 35, nextBoxY + 6);
    window.draw(nextLabel);
    // Center the next piece in the box, but move it higher
    int nextBoxCenterX = GRID_WIDTH * TILE_SIZE + 15 + boxWidth / 2;
    int nextBoxCenterY = nextBoxY + 55;
    for (int i = 0; i < 4; ++i) {
        int px = TETRIMINOS[nextTetrimino.shapeIndex][i] % 2;
        int py = TETRIMINOS[nextTetrimino.shapeIndex][i] / 2;
        sf::RectangleShape tile(sf::Vector2f(pieceTileSize, pieceTileSize));
        tile.setPosition(nextBoxCenterX + (px - 1) * pieceTileSize, nextBoxCenterY + (py - 1) * pieceTileSize);
        tile.setFillColor(TETRIMINO_COLORS[nextTetrimino.shapeIndex]);
        tile.setOutlineColor(sf::Color::Black);
        tile.setOutlineThickness(2);
        window.draw(tile);
    }

    // Draw the HOLD box
    sf::RectangleShape holdBox(sf::Vector2f(boxWidth, bigBoxHeight));
    holdBox.setPosition(GRID_WIDTH * TILE_SIZE + 15, holdBoxY);
    holdBox.setFillColor(sf::Color(60, 40, 40));
    holdBox.setOutlineColor(sf::Color::White);
    holdBox.setOutlineThickness(2);
    window.draw(holdBox);
    sf::Text holdLabel;
    holdLabel.setFont(font);
    holdLabel.setString("HOLD");
    holdLabel.setCharacterSize(18);
    holdLabel.setFillColor(sf::Color(255, 200, 200));
    holdLabel.setStyle(sf::Text::Bold);
    holdLabel.setPosition(GRID_WIDTH * TILE_SIZE + 35, holdBoxY + 6);
    window.draw(holdLabel);
    int holdBoxCenterX = GRID_WIDTH * TILE_SIZE + 15 + boxWidth / 2;
    int holdBoxCenterY = holdBoxY + 55;
    if (heldTetrimino) {
        for (int i = 0; i < 4; ++i) {
            int px = TETRIMINOS[heldTetrimino->shapeIndex][i] % 2;
            int py = TETRIMINOS[heldTetrimino->shapeIndex][i] / 2;
            sf::RectangleShape tile(sf::Vector2f(pieceTileSize, pieceTileSize));
            tile.setPosition(holdBoxCenterX + (px - 1) * pieceTileSize, holdBoxCenterY + (py - 1) * pieceTileSize);
            tile.setFillColor(TETRIMINO_COLORS[heldTetrimino->shapeIndex]);
            tile.setOutlineColor(sf::Color::Black);
            tile.setOutlineThickness(2);
            window.draw(tile);
        }
    }
}

// Helper to get ghost piece position
template<typename T>
T getGhostTetrimino(const T& t, const std::vector<std::vector<int>>& grid) {
    T ghost = t;
    while (true) {
        T next = ghost;
        next.y += 1;
        if (!isValidPosition(next, grid)) break;
        ghost = next;
    }
    return ghost;
}

// Animation state for line clear
std::vector<int> clearingLines; // y indices of lines being cleared
float clearAnimTimer = 0.0f;
const float CLEAR_ANIM_DURATION = 0.15f; // seconds - shorter animation duration for better flow

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
    clearingLines.clear();
    clearAnimTimer = 0.0f;

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
                        if (heldTetrimino) { delete heldTetrimino; heldTetrimino = nullptr; } // Reset hold
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
                        drawSidePanel(window, font, score, nextTetrimino, heldTetrimino);
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
            float deltaTime = clock.restart().asSeconds();
            fallTimer += deltaTime;
            
            // Update line clear animation if active
            if (!clearingLines.empty()) {
                clearAnimTimer += deltaTime;
                if (clearAnimTimer >= CLEAR_ANIM_DURATION) {
                    // Animation finished - clear lines and update score
                    score += SCORE_TABLE.at((int)clearingLines.size());
                    
                    // Sort lines in descending order to remove from bottom to top
                    std::sort(clearingLines.begin(), clearingLines.end(), std::greater<int>());
                    
                    // Remove each full line
                    for (int y : clearingLines) {
                        // Move all lines above down one row
                        for (int row = y; row > 0; --row) {
                            grid[row] = grid[row - 1];
                        }
                        grid[0] = std::vector<int>(GRID_WIDTH, 0);
                    }
                    
                    // Reset animation state
                    clearingLines.clear();
                    clearAnimTimer = 0.0f;
                    
                    // Get next tetrimino
                    currentTetrimino = nextTetrimino;
                    nextTetrimino = Tetrimino(std::rand() % 7);
                    if (!isValidPosition(currentTetrimino, grid))
                        gameOver = true;
                    canHold = true;
                }
            }
            // Only process falling if not animating line clear
            else if (fallTimer >= fallDelay) {
                fallTimer = 0.0f;
                Tetrimino temp = currentTetrimino;
                temp.y += 1;
                if (isValidPosition(temp, grid))
                {
                    currentTetrimino = temp;
                }
                else
                {
                    placeTetrimino(currentTetrimino, grid);
                    // Detect lines to clear
                    clearingLines.clear();
                    for (int y = 0; y < GRID_HEIGHT; ++y) {
                        bool full = true;
                        for (int x = 0; x < GRID_WIDTH; ++x)
                            if (grid[y][x] == 0) { full = false; break; }
                        if (full) clearingLines.push_back(y);
                    }
                    
                    // If no lines to clear, continue with next piece
                    if (clearingLines.empty()) {
                        currentTetrimino = nextTetrimino;
                        nextTetrimino = Tetrimino(std::rand() % 7);
                        if (!isValidPosition(currentTetrimino, grid))
                            gameOver = true;
                        canHold = true;
                    }
                    // Otherwise start animation
                    else {
                        clearAnimTimer = 0.0f;
                    }
                }
            }
        } else if (paused) {
            clock.restart();
        }

        // Clear the window
        window.clear(sf::Color::Black);

        // Draw the grid with line clear animation
        for (int y = 0; y < GRID_HEIGHT; ++y)
        {
            for (int x = 0; x < GRID_WIDTH; ++x)
            {
                if (grid[y][x] != 0)
                {
                    sf::RectangleShape tile(sf::Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                    tile.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                    
                    // Animate clearing lines - flash between white and the original color
                    bool isClearing = std::find(clearingLines.begin(), clearingLines.end(), y) != clearingLines.end();
                    if (isClearing) {
                        float flashRate = 15.0f; // Flash speed
                        bool flash = static_cast<int>(clearAnimTimer * flashRate) % 2 == 0;
                        if (flash) {
                            tile.setFillColor(sf::Color::White);
                        } else {
                            tile.setFillColor(TETRIMINO_COLORS[grid[y][x] - 1]);
                        }
                    } else {
                        tile.setFillColor(TETRIMINO_COLORS[grid[y][x] - 1]);
                    }
                    window.draw(tile);
                }
            }
        }

        // Draw the ghost piece
        if (!gameOver && clearingLines.empty()) {
            Tetrimino ghost = getGhostTetrimino(currentTetrimino, grid);
            auto ghostPositions = getBlockPositions(ghost);
            for (const auto& pos : ghostPositions) {
                if (pos.y >= 0) {
                    sf::RectangleShape tile(sf::Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                    tile.setPosition(pos.x * TILE_SIZE, pos.y * TILE_SIZE);
                    tile.setFillColor(sf::Color(200, 200, 200, 80));
                    tile.setOutlineColor(sf::Color(100, 100, 100, 120));
                    tile.setOutlineThickness(2);
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

        drawSidePanel(window, font, score, nextTetrimino, heldTetrimino);

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