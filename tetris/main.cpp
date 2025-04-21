#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <map>
#include <string>
#include <iostream>
#include <cmath>

const int GRID_WIDTH = 10;
const int GRID_HEIGHT = 20;
const int TILE_SIZE = 30;
const std::string FONT_PATH = "extern/fonts/PixelatedElegance.ttf";

// Add color mapping for Tetriminos - déplacé avant le système de particules
const std::array<sf::Color, 7> TETRIMINO_COLORS = {
    sf::Color::Cyan, sf::Color::Red, sf::Color::Green, sf::Color::Magenta,
    sf::Color::Blue, sf::Color::Yellow, sf::Color::White};

// Système de particules
struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float size;
    float lifetime;
    float maxLifetime;
    
    Particle(sf::Vector2f pos, sf::Vector2f vel, sf::Color col, float sz, float life) 
        : position(pos), velocity(vel), color(col), size(sz), lifetime(life), maxLifetime(life) {}
    
    void update(float deltaTime) {
        position += velocity * deltaTime;
        lifetime -= deltaTime;
        
        // Ralentissement progressif
        velocity *= 0.98f;
        
        // Diminution de la taille
        size = std::max(0.5f, size * (lifetime / maxLifetime));
    }
    
    bool isDead() const {
        return lifetime <= 0;
    }
    
    void draw(sf::RenderWindow& window) {
        sf::CircleShape shape(size);
        shape.setPosition(position);
        shape.setOrigin(size, size);
        
        // Ajuster l'opacité en fonction de la durée de vie restante
        sf::Color fadingColor = color;
        fadingColor.a = static_cast<sf::Uint8>(255 * (lifetime / maxLifetime));
        shape.setFillColor(fadingColor);
        
        window.draw(shape);
    }
};

// Structure pour l'effet d'onde de choc
struct ShockWave {
    sf::Vector2f center;
    float radius;
    float maxRadius;
    float thickness;
    sf::Color color;
    float lifetime;
    float maxLifetime;
    
    ShockWave(sf::Vector2f pos, float maxRad, sf::Color col, float life)
        : center(pos), radius(0), maxRadius(maxRad), thickness(3.0f), color(col), lifetime(life), maxLifetime(life) {}
        
    void update(float deltaTime) {
        radius += (maxRadius / maxLifetime) * deltaTime * 2.5f; // Vitesse de propagation
        lifetime -= deltaTime;
        
        // Réduire l'épaisseur avec le temps
        thickness = std::max(0.5f, 3.0f * (lifetime / maxLifetime));
    }
    
    bool isDead() const {
        return lifetime <= 0 || radius >= maxRadius;
    }
    
    void draw(sf::RenderWindow& window) {
        sf::CircleShape shape(radius);
        shape.setPosition(center.x - radius, center.y - radius);
        shape.setOrigin(0, 0);
        
        // Ajuster l'opacité en fonction de la durée de vie restante
        sf::Color wavingColor = color;
        wavingColor.a = static_cast<sf::Uint8>(155 * (lifetime / maxLifetime));
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(wavingColor);
        shape.setOutlineThickness(thickness);
        
        window.draw(shape);
    }
};

class ParticleSystem {
private:
    std::vector<Particle> particles;
    std::vector<ShockWave> shockWaves;
    
public:
    void addParticle(const Particle& particle) {
        particles.push_back(particle);
    }
    
    void addShockWave(const ShockWave& wave) {
        shockWaves.push_back(wave);
    }
    
    void createExplosion(sf::Vector2f position, sf::Color color, int count, float speed) {
        for (int i = 0; i < count; ++i) {
            // Direction aléatoire
            float angle = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
            float velocity = speed * (0.5f + static_cast<float>(rand() % 100) / 100.0f);
            sf::Vector2f direction(std::cos(angle) * velocity, std::sin(angle) * velocity);
            
            // Légère variation de position
            sf::Vector2f offset(
                (static_cast<float>(rand() % 100) - 50.0f) / 50.0f * TILE_SIZE / 4.0f,
                (static_cast<float>(rand() % 100) - 50.0f) / 50.0f * TILE_SIZE / 4.0f
            );
            
            // Légère variation de couleur
            sf::Color particleColor = color;
            int variation = 30;
            particleColor.r = std::min(255, std::max(0, particleColor.r + (rand() % variation - variation/2)));
            particleColor.g = std::min(255, std::max(0, particleColor.g + (rand() % variation - variation/2)));
            particleColor.b = std::min(255, std::max(0, particleColor.b + (rand() % variation - variation/2)));
            
            // Taille et durée de vie aléatoires
            float size = 1.0f + static_cast<float>(rand() % 30) / 10.0f;
            float lifetime = 0.5f + static_cast<float>(rand() % 100) / 100.0f;
            
            addParticle(Particle(position + offset, direction, particleColor, size, lifetime));
        }
    }
    
    void createLineExplosion(int lineY, const std::vector<std::vector<int>>& grid) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            if (grid[lineY][x] != 0) {
                int colorIndex = grid[lineY][x] - 1;
                sf::Color color = TETRIMINO_COLORS[colorIndex];
                sf::Vector2f tileCenter(x * TILE_SIZE + TILE_SIZE / 2, lineY * TILE_SIZE + TILE_SIZE / 2);
                createExplosion(tileCenter, color, 15, 100.0f); // 15 particules par tuile
            }
        }
    }
    
    // Créer une onde de choc au centre d'une ligne
    void createShockWaveForLine(int lineY, sf::Color color, float maxRadius = 300.0f) {
        sf::Vector2f center(GRID_WIDTH * TILE_SIZE / 2, lineY * TILE_SIZE + TILE_SIZE / 2);
        addShockWave(ShockWave(center, maxRadius, color, 0.7f));
    }
    
    // Créer l'effet pour un effacement avec la touche espace
    void createSpaceClearEffect(const std::vector<int>& lines, const std::vector<std::vector<int>>& grid) {
        // Effet plus intense selon le nombre de lignes
        float baseRadius = 150.0f;
        float extraRadius = 75.0f * lines.size(); // Plus de lignes = plus grand rayon
        float maxRadius = baseRadius + extraRadius;
        
        // Choisir une couleur basée sur le nombre de lignes
        sf::Color waveColor;
        switch(lines.size()) {
            case 1: waveColor = sf::Color(255, 255, 200); break;    // Jaune clair
            case 2: waveColor = sf::Color(255, 200, 100); break;    // Orange
            case 3: waveColor = sf::Color(255, 100, 50); break;     // Orange-rouge
            case 4: waveColor = sf::Color(255, 50, 255); break;     // Rose (Tetris)
            default: waveColor = sf::Color::White;
        }
        
        // Onde centrale - Grande onde pour l'effet global
        float centerY = 0;
        for (int lineY : lines) {
            centerY += lineY;
            
            // Ajouter une onde par ligne pour un effet plus dynamique
            sf::Vector2f lineCenter(GRID_WIDTH * TILE_SIZE / 2, lineY * TILE_SIZE + TILE_SIZE / 2);
            addShockWave(ShockWave(lineCenter, maxRadius * 0.7f, waveColor, 0.6f));
            
            // Ajouter des particules pour chaque cellule dans la ligne
            createLineExplosion(lineY, grid);
        }
        
        // Onde centrale supplémentaire basée sur la moyenne des lignes
        if (!lines.empty()) {
            centerY /= lines.size(); // Position moyenne
            sf::Vector2f mainCenter(GRID_WIDTH * TILE_SIZE / 2, centerY * TILE_SIZE + TILE_SIZE / 2);
            
            // Ajouter une onde de choc principale
            addShockWave(ShockWave(mainCenter, maxRadius, waveColor, 0.8f));
            
            // Pour un Tetris (4 lignes), ajouter des effets supplémentaires
            if (lines.size() == 4) {
                // Ondes supplémentaires pour un Tetris
                addShockWave(ShockWave(mainCenter, maxRadius * 1.2f, sf::Color(200, 50, 200), 1.0f));
                
                // Explosion massive de particules
                for (int i = 0; i < 120; ++i) { // 120 particules supplémentaires
                    float angle = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
                    float velocity = 150.0f * (0.5f + static_cast<float>(rand() % 100) / 100.0f);
                    sf::Vector2f direction(std::cos(angle) * velocity, std::sin(angle) * velocity);
                    
                    sf::Vector2f offset(
                        (static_cast<float>(rand() % 200) - 100.0f),
                        (static_cast<float>(rand() % 200) - 100.0f)
                    );
                    
                    // Variation de couleur pour l'effet "explosion"
                    sf::Color particleColor;
                    int colorChoice = rand() % 3;
                    if (colorChoice == 0) particleColor = sf::Color(255, 50, 255); // Rose
                    else if (colorChoice == 1) particleColor = sf::Color(255, 255, 200); // Jaune
                    else particleColor = sf::Color(50, 200, 255); // Bleu clair
                    
                    float size = 2.0f + static_cast<float>(rand() % 40) / 10.0f;
                    float lifetime = 0.7f + static_cast<float>(rand() % 100) / 100.0f;
                    
                    addParticle(Particle(mainCenter + offset, direction, particleColor, size, lifetime));
                }
            }
        }
    }
    
    void update(float deltaTime) {
        // Mettre à jour toutes les particules
        for (auto& particle : particles) {
            particle.update(deltaTime);
        }
        
        // Supprimer les particules mortes
        particles.erase(
            std::remove_if(particles.begin(), particles.end(), 
                [](const Particle& p) { return p.isDead(); }),
            particles.end()
        );
        
        // Mettre à jour les ondes de choc
        for (auto& wave : shockWaves) {
            wave.update(deltaTime);
        }
        
        // Supprimer les ondes de choc terminées
        shockWaves.erase(
            std::remove_if(shockWaves.begin(), shockWaves.end(),
                [](const ShockWave& w) { return w.isDead(); }),
            shockWaves.end()
        );
    }
    
    void draw(sf::RenderWindow& window) {
        // Dessiner d'abord les ondes de choc (arrière-plan)
        for (auto& wave : shockWaves) {
            wave.draw(window);
        }
        
        // Puis dessiner les particules (premier plan)
        for (auto& particle : particles) {
            particle.draw(window);
        }
    }
    
    bool isEmpty() const {
        return particles.empty() && shockWaves.empty();
    }
};

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

// Système de score amélioré
struct ScoreSystem {
    // Points pour les lignes complétées (selon niveau)
    int getSingleLineClear(int level) { return 100 * level; }
    int getDoubleLineClear(int level) { return 300 * level; }
    int getTripleLineClear(int level) { return 500 * level; }
    int getTetrisLineClear(int level) { return 800 * level; } // Bonus pour Tetris (4 lignes)
    
    // Points par cellule descendue
    int getSoftDropPoints() { return 1; }     // 1 point par cellule en soft drop
    int getHardDropPoints() { return 2; }     // 2 points par cellule en hard drop
};

// Variables globales pour le score et le niveau
ScoreSystem scoreSystem;
int level = 1;
int linesCleared = 0;
const int LINES_PER_LEVEL = 10;
float fallDelay = 0.5f; 

// Add scoring system
int score = 0;

// Remplacer la map SCORE_TABLE par la fonction suivante
int getLinesClearPoints(int numLines, int level) {
    switch(numLines) {
        case 1: return scoreSystem.getSingleLineClear(level);
        case 2: return scoreSystem.getDoubleLineClear(level);
        case 3: return scoreSystem.getTripleLineClear(level);
        case 4: return scoreSystem.getTetrisLineClear(level);
        default: return 0;
    }
}

// Add line clearing logic
void clearLines(std::vector<std::vector<int>> &grid)
{
    int linesCleared_local = 0;
    std::vector<int> fullLines;  // Stocker les indices des lignes pleines

    // Identifier d'abord toutes les lignes complètes
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
            fullLines.push_back(y);  // Ajouter l'indice à la liste des lignes pleines
            ++linesCleared_local;
        }
    }

    // Supprimer toutes les lignes pleines trouvées (du bas vers le haut)
    std::sort(fullLines.begin(), fullLines.end(), std::greater<int>());  // Trier en ordre décroissant
    
    // Créer une nouvelle grille temporaire sans les lignes complètes
    std::vector<std::vector<int>> newGrid(GRID_HEIGHT, std::vector<int>(GRID_WIDTH, 0));
    int destRow = GRID_HEIGHT - 1;
    
    // Copier uniquement les lignes non complètes de bas en haut
    for (int y = GRID_HEIGHT - 1; y >= 0; --y) {
        if (std::find(fullLines.begin(), fullLines.end(), y) == std::end(fullLines)) {
            // Cette ligne n'est pas pleine, la copier
            for (int x = 0; x < GRID_WIDTH; ++x) {
                newGrid[destRow][x] = grid[y][x];
            }
            destRow--;
        }
        // Si c'est une ligne pleine, ne pas la copier (sauter)
    }
    
    // Remplacer la grille d'origine par la nouvelle grille
    grid = newGrid;

    if (linesCleared_local > 0)
    {
        score += getLinesClearPoints(linesCleared_local, level);
        
        // Mettre à jour la variable globale pour le changement de niveau
        linesCleared += linesCleared_local;
        
        // Vérifier si on doit augmenter le niveau
        if (linesCleared >= level * LINES_PER_LEVEL) {
            level++;
            // Accélérer la vitesse de chute
            fallDelay = std::max(0.05f, 0.5f - (level-1) * 0.05f);
        }
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
    
    // Draw LEVEL box at the bottom of the screen
    const int levelBoxHeight = 60;
    const int levelBoxY = GRID_HEIGHT * TILE_SIZE - levelBoxHeight - 15; // Position en bas avec une marge
    
    sf::RectangleShape levelBox(sf::Vector2f(boxWidth, levelBoxHeight));
    levelBox.setPosition(GRID_WIDTH * TILE_SIZE + 15, levelBoxY);
    levelBox.setFillColor(sf::Color(60, 60, 80));
    levelBox.setOutlineColor(sf::Color::White);
    levelBox.setOutlineThickness(2);
    window.draw(levelBox);
    
    sf::Text levelLabel;
    levelLabel.setFont(font);
    levelLabel.setString("LEVEL");
    levelLabel.setCharacterSize(18);
    levelLabel.setFillColor(sf::Color(220, 220, 255));
    levelLabel.setStyle(sf::Text::Bold);
    levelLabel.setPosition(GRID_WIDTH * TILE_SIZE + 35, levelBoxY + 6);
    window.draw(levelLabel);
    
    sf::Text levelValue;
    levelValue.setFont(font);
    levelValue.setString(std::to_string(level));
    levelValue.setCharacterSize(30);
    levelValue.setFillColor(sf::Color::Yellow);
    levelValue.setStyle(sf::Text::Bold);
    
    // Centrer le nombre du niveau
    sf::FloatRect textRect = levelValue.getLocalBounds();
    levelValue.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    levelValue.setPosition(GRID_WIDTH * TILE_SIZE + 15 + boxWidth / 2, levelBoxY + 40);
    window.draw(levelValue);
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
bool isHardDropClear = false; // Indique si l'effacement a été déclenché par un hard drop

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

    // Création du système de particules
    ParticleSystem particleSystem;

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
                        level = 1; // Réinitialiser le niveau
                        linesCleared = 0; // Réinitialiser les lignes
                        fallDelay = 0.5f; // Réinitialiser la vitesse
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
                    if (isValidPosition(temp, grid)) {
                        currentTetrimino = temp;
                        // Points pour soft drop
                        score += scoreSystem.getSoftDropPoints();
                    }
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
                    int dropDistance = 0; // Compter la distance parcourue
                    
                    while (true) {
                        Tetrimino next = drop;
                        next.y += 1;
                        if (!isValidPosition(next, grid)) break;
                        drop = next;
                        dropDistance++; // Augmenter la distance
                    }
                    
                    // Ajouter des points pour le hard drop
                    score += dropDistance * scoreSystem.getHardDropPoints();
                    
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
                    
                    // Detect lines to clear
                    clearingLines.clear();
                    for (int y = 0; y < GRID_HEIGHT; ++y) {
                        bool full = true;
                        for (int x = 0; x < GRID_WIDTH; ++x)
                            if (grid[y][x] == 0) { full = false; break; }
                        if (full) clearingLines.push_back(y);
                    }
                    
                    // Activer le flag pour indiquer que l'effacement a été déclenché par un hard drop
                    isHardDropClear = !clearingLines.empty();
                    
                    // Gérer les points pour les lignes complétées
                    if (!clearingLines.empty()) {
                        // Les points seront ajoutés après l'animation
                        clearAnimTimer = 0.0f;
                    } else {
                        currentTetrimino = nextTetrimino;
                        nextTetrimino = Tetrimino(std::rand() % 7);
                        if (!isValidPosition(currentTetrimino, grid))
                            gameOver = true;
                    }
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

        // Clear the window
        window.clear(sf::Color::Black);

        // Calculer deltaTime même si on est en pause ou game over pour les animations de particules
        float deltaTime = clock.restart().asSeconds();
        
        if (!paused && !gameOver) {
            fallTimer += deltaTime;
            
            // Update line clear animation if active
            if (!clearingLines.empty()) {
                clearAnimTimer += deltaTime;
                
                // Générer des effets pendant l'animation
                if (clearAnimTimer < CLEAR_ANIM_DURATION / 2) {
                    // Si l'effacement a été déclenché par un hard drop (espace), utiliser l'effet spécial
                    if (isHardDropClear && clearAnimTimer < 0.05f) {
                        // Créer l'effet spécial une seule fois au début de l'animation
                        particleSystem.createSpaceClearEffect(clearingLines, grid);
                        
                        // Effet de ralenti (bullet time) pour les Tetris (4 lignes)
                        if (clearingLines.size() == 4) {
                            // Ralentir le temps pour un effet dramatique
                            sf::sleep(sf::milliseconds(100));
                        }
                    }
                    // Animation standard pour les autres modes d'effacement
                    else if (!isHardDropClear) {
                        // Génération de particules standard pour chaque ligne complète
                        for (int lineY : clearingLines) {
                            particleSystem.createLineExplosion(lineY, grid);
                        }
                    }
                }
                
                if (clearAnimTimer >= CLEAR_ANIM_DURATION) {
                    // Animation finished - clear lines and update score
                    int numLinesCleared = clearingLines.size();
                    
                    // Ajouter les points selon le nombre de lignes
                    score += getLinesClearPoints(numLinesCleared, level);
                    
                    // Mettre à jour le total des lignes complétées
                    linesCleared += numLinesCleared;
                    
                    // Vérifier si on doit augmenter le niveau
                    if (linesCleared >= level * LINES_PER_LEVEL) {
                        level++;
                        // Accélérer la vitesse de chute
                        fallDelay = std::max(0.05f, 0.5f - (level-1) * 0.05f);
                    }
                    
                    // Créer une nouvelle grille temporaire sans les lignes complètes
                    std::vector<std::vector<int>> newGrid(GRID_HEIGHT, std::vector<int>(GRID_WIDTH, 0));
                    int destRow = GRID_HEIGHT - 1;
                    
                    // Copier uniquement les lignes non complètes de bas en haut
                    for (int y = GRID_HEIGHT - 1; y >= 0; --y) {
                        if (std::find(clearingLines.begin(), clearingLines.end(), y) == std::end(clearingLines)) {
                            // Cette ligne n'est pas pleine, la copier
                            for (int x = 0; x < GRID_WIDTH; ++x) {
                                newGrid[destRow][x] = grid[y][x];
                            }
                            destRow--;
                        }
                        // Si c'est une ligne pleine, ne pas la copier (sauter)
                    }
                    
                    // Remplacer la grille d'origine par la nouvelle grille
                    grid = newGrid;
                    
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

        // Mettre à jour et dessiner les particules
        particleSystem.update(deltaTime);
        particleSystem.draw(window);

        // Display the window
        window.display();
    }

    if (heldTetrimino) delete heldTetrimino;

    return 0;
}