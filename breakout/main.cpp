#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <functional>
#include <random>
#include <ctime>
#include <iostream>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int BRICK_ROWS = 4; // moins de lignes
const int BRICK_COLS = 7; // moins de colonnes
const int BRICK_WIDTH = 90; // plus large
const int BRICK_HEIGHT = 32; // plus haut
const int BRICK_SPACING = 12; // un peu plus d'espace
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 18;
const float PADDLE_SPEED = 480.0f;
const float BALL_RADIUS = 14.0f;
const float BALL_SPEED = 420.0f; // vitesse augmentée
const int LIVES = 3;
const int MAX_LEVEL = 5;

sf::Font iconFont;

enum class PowerUpType { None, ExpandPaddle, ShrinkPaddle, MultiBall, SlowBall, FastBall };

struct PowerUp {
    PowerUpType type = PowerUpType::None;
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    bool active = false;
};

enum class BrickType { Normal, PowerUp, Indestructible };

struct Brick {
    sf::RectangleShape shape;
    bool destroyed = false;
    int points = 50;
    BrickType type = BrickType::Normal;
    float destructionTimer = 0.0f; // Timer pour l'animation de destruction
};

// Nouvelle structure pour gérer toutes les balles
struct Ball {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    bool launched = false;
};

// Synchronize the outline color with the falling power-up color
void generateBricks(std::vector<Brick>& bricks, int level) {
    bricks.clear();
    int rows = BRICK_ROWS + (level - 1); // Plus de lignes à chaque niveau
    int cols = BRICK_COLS;
    float offsetX = (WINDOW_WIDTH - (cols * (BRICK_WIDTH + BRICK_SPACING) - BRICK_SPACING)) / 2.0f;
    float offsetY = 60.0f;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 4); // Augmenter la probabilité de briques PowerUp
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Pour varier la disposition :
            if (level >= 2 && (level % 2 == 0) && (i + j) % (2 + level % 3) == 0) continue; // trous
            if (level >= 3 && (i == j || i + j == cols - 1)) continue; // diagonales vides
            Brick brick;
            brick.shape.setSize(sf::Vector2f(BRICK_WIDTH, BRICK_HEIGHT));
            brick.shape.setOrigin(BRICK_WIDTH / 2, BRICK_HEIGHT / 2);
            brick.shape.setPosition(offsetX + j * (BRICK_WIDTH + BRICK_SPACING) + BRICK_WIDTH / 2,
                                   offsetY + i * (BRICK_HEIGHT + BRICK_SPACING) + BRICK_HEIGHT / 2);
            brick.shape.setFillColor(sf::Color(100 + 25 * i, 100 + 10 * j, 200 - 20 * i + 10 * level));
            brick.points = 50 + 10 * i + 10 * (level - 1);
            brick.destroyed = false;
            // 10% chance d'être une brique power-up
            int r = dist(rng);
            if (r == 0) {
                int powerUpType = std::rand() % 5;
                switch (powerUpType) {
                    case 0:
                        brick.shape.setOutlineColor(sf::Color::Yellow); // Healing
                        brick.type = BrickType::PowerUp;
                        break;
                    case 1:
                        brick.shape.setOutlineColor(sf::Color::Red); // Shrink paddle
                        brick.type = BrickType::PowerUp;
                        break;
                    case 2:
                        brick.shape.setOutlineColor(sf::Color::Cyan); // Multi-ball
                        brick.type = BrickType::PowerUp;
                        break;
                    case 3:
                        brick.shape.setOutlineColor(sf::Color(100, 255, 100)); // Slow ball
                        brick.type = BrickType::PowerUp;
                        break;
                    case 4:
                        brick.shape.setOutlineColor(sf::Color(255, 100, 255)); // Fast ball
                        brick.type = BrickType::PowerUp;
                        break;
                }
            } else {
                brick.type = BrickType::Normal;
            }
            bricks.push_back(brick);
        }
    }
}

// Update the drawPowerUpIcon function to use the new font
void drawPowerUpIcon(sf::RenderWindow& window, const Brick& brick) {
    if (brick.type != BrickType::PowerUp) return;

    sf::Vector2f pos = brick.shape.getPosition();
    sf::Vector2f size = brick.shape.getSize();

    sf::Text icon;
    icon.setFont(iconFont);
    icon.setCharacterSize(static_cast<unsigned int>(size.y * 0.6f));
    icon.setFillColor(sf::Color::White);
    icon.setStyle(sf::Text::Bold);
    icon.setPosition(pos.x - size.x * 0.1f, pos.y - size.y * 0.4f);

    // Use ASCII characters as symbols for power-ups
    if (brick.shape.getOutlineColor() == sf::Color::Yellow) {
        icon.setString("+"); // Healing symbol
    } else if (brick.shape.getOutlineColor() == sf::Color::Cyan) {
        icon.setString("O"); // Multi-ball symbol
    } else if (brick.shape.getOutlineColor() == sf::Color::Red) {
        icon.setString("><"); // Shrink paddle symbol
    } else if (brick.shape.getOutlineColor() == sf::Color::Green) {
        icon.setString("< >"); // Expand paddle symbol
    } else {
        icon.setString("?"); // Default symbol
    }

    window.draw(icon);
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Ensure consistent randomization by initializing the random seed

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Breakout");
    window.setFramerateLimit(60);

    // Font
    sf::Font font;
    if (!font.loadFromFile("extern/fonts/PixelatedElegance.ttf")) {
        // fallback
        if (!font.loadFromFile("extern/fonts/GOODDP__.TTF")) {
            return 1;
        }
    }

    // Load the icon font
    if (!iconFont.loadFromFile("extern/fonts/GOODDP__.TTF")) {
        std::cerr << "Failed to load 'GOODDP__.TTF' font!" << std::endl;
        return 1;
    }

    // Paddle
    sf::RectangleShape paddle(sf::Vector2f(PADDLE_WIDTH, PADDLE_HEIGHT));
    paddle.setFillColor(sf::Color(200, 200, 255));
    paddle.setOrigin(PADDLE_WIDTH / 2, PADDLE_HEIGHT / 2);
    paddle.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT - 40);

    // Bricks
    std::vector<Brick> bricks;
    int score = 0;
    int lives = LIVES;
    int level = 1;
    bool gameOver = false;
    bool gameWon = false;

    generateBricks(bricks, level);

    std::vector<PowerUp> powerUps;
    float paddleExpandTimer = 0.f;
    const float PADDLE_EXPAND_DURATION = 8.f;
    bool paddleExpanded = false;
    float slowBallTimer = 0.f;
    float fastBallTimer = 0.f;
    bool ballSlowed = false;
    bool ballFaster = false;
    float paddleShrinkTimer = 0.f;
    bool paddleShrunk = false;
    const float PADDLE_SHRINK_DURATION = 8.f;
    const float SLOWBALL_DURATION = 8.f;
    const float FASTBALL_DURATION = 8.f;

    sf::Text scoreText, livesText, infoText, levelText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(22);
    scoreText.setFillColor(sf::Color::White);
    // scoreText position will be set dynamically
    livesText.setFont(font);
    livesText.setCharacterSize(22);
    livesText.setFillColor(sf::Color::White);
    // livesText position will be set dynamically
    infoText.setFont(font);
    infoText.setCharacterSize(32);
    infoText.setFillColor(sf::Color::Yellow);
    infoText.setStyle(sf::Text::Bold);
    // infoText position will be set dynamiquement
    levelText.setFont(font);
    levelText.setCharacterSize(22);
    levelText.setFillColor(sf::Color::Cyan);
    // levelText position will be set dynamiquement

    std::vector<Ball> balls;
    auto spawnBall = [&](sf::Vector2f pos, sf::Vector2f vel = {0,0}, bool launched = false) {
        Ball b;
        b.shape = sf::CircleShape(BALL_RADIUS);
        b.shape.setFillColor(sf::Color(255, 200, 100));
        b.shape.setOrigin(BALL_RADIUS, BALL_RADIUS);
        b.shape.setPosition(pos);
        b.velocity = vel;
        b.launched = launched;
        balls.push_back(b);
    };
    // Initialisation : une balle au centre
    spawnBall({WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f});

    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                if (gameOver || gameWon) {
                    if (event.key.code == sf::Keyboard::R) {
                        // Reset game
                        level = 1;
                        generateBricks(bricks, level);
                        score = 0;
                        lives = LIVES;
                        gameOver = false;
                        gameWon = false;
                        balls.clear();
                        spawnBall({WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f});
                        paddle.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT - 40);
                    }
                }
            }
        }

        if (!gameOver && !gameWon) {
            // Paddle movement
            float move = 0;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) move -= PADDLE_SPEED * dt;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) move += PADDLE_SPEED * dt;
            float newX = paddle.getPosition().x + move;
            newX = std::max(PADDLE_WIDTH / 2.f, std::min((float)WINDOW_WIDTH - PADDLE_WIDTH / 2.f, newX));
            paddle.setPosition(newX, paddle.getPosition().y);

            // Ball follows paddle before launch
            for (auto& ball : balls) {
                if (!ball.launched) {
                    ball.shape.setPosition(paddle.getPosition().x, paddle.getPosition().y - PADDLE_HEIGHT / 2 - BALL_RADIUS);
                }
            }
            // Lancement de la balle
            if (!balls.empty() && !balls[0].launched && sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                float angle = (std::rand() % 120 + 30) * 3.14159f / 180.f;
                balls[0].velocity = sf::Vector2f(BALL_SPEED * std::cos(angle), BALL_SPEED * -std::sin(angle));
                balls[0].launched = true;
            }
            // Mouvement et collisions pour chaque balle
            for (size_t i = 0; i < balls.size(); ++i) {
                auto& ball = balls[i];
                if (!ball.launched) continue;
                ball.shape.move(ball.velocity * dt);
                sf::Vector2f pos = ball.shape.getPosition();
                // Wall collisions
                if (pos.x - BALL_RADIUS < 0) {
                    ball.shape.setPosition(BALL_RADIUS, pos.y);
                    ball.velocity.x = -ball.velocity.x;
                }
                if (pos.x + BALL_RADIUS > WINDOW_WIDTH) {
                    ball.shape.setPosition(WINDOW_WIDTH - BALL_RADIUS, pos.y);
                    ball.velocity.x = -ball.velocity.x;
                }
                if (pos.y - BALL_RADIUS < 0) {
                    ball.shape.setPosition(pos.x, BALL_RADIUS);
                    ball.velocity.y = -ball.velocity.y;
                }
                // Bottom (lose ball)
                if (pos.y - BALL_RADIUS > WINDOW_HEIGHT) {
                    balls.erase(balls.begin() + i);
                    --i;
                    continue;
                }
                // Paddle collision
                if (ball.shape.getGlobalBounds().intersects(paddle.getGlobalBounds())) {
                    float px = (ball.shape.getPosition().x - paddle.getPosition().x) / (PADDLE_WIDTH / 2);
                    px = std::clamp(px, -1.0f, 1.0f);
                    float angle = px * 60 * 3.14159f / 180.f;
                    float speed = std::sqrt(ball.velocity.x * ball.velocity.x + ball.velocity.y * ball.velocity.y);
                    ball.velocity.x = speed * std::sin(angle);
                    ball.velocity.y = -std::abs(speed * std::cos(angle));
                    float overlap = (paddle.getPosition().y - PADDLE_HEIGHT / 2) - (ball.shape.getPosition().y + BALL_RADIUS);
                    ball.shape.move(0, overlap - 1);
                }
                // Brick collisions
                for (auto& brick : bricks) {
                    if (!brick.destroyed && ball.shape.getGlobalBounds().intersects(brick.shape.getGlobalBounds())) {
                        if (brick.type != BrickType::Indestructible) {
                            brick.destroyed = true;
                            brick.destructionTimer = 0.5f;
                            score += brick.points;
                            if (brick.type == BrickType::PowerUp) {
                                PowerUp pu;
                                pu.type = PowerUpType::None;
                                if (brick.shape.getOutlineColor() == sf::Color::Yellow) pu.type = PowerUpType::ExpandPaddle;
                                else if (brick.shape.getOutlineColor() == sf::Color::Red) pu.type = PowerUpType::ShrinkPaddle;
                                else if (brick.shape.getOutlineColor() == sf::Color::Cyan) pu.type = PowerUpType::MultiBall;
                                else if (brick.shape.getOutlineColor() == sf::Color(100, 255, 100)) pu.type = PowerUpType::SlowBall;
                                else if (brick.shape.getOutlineColor() == sf::Color(255, 100, 255)) pu.type = PowerUpType::FastBall;
                                if (pu.type == PowerUpType::MultiBall) {
                                    // Ajoute une nouvelle balle identique à la première balle lancée
                                    if (!balls.empty()) {
                                        Ball newBall = balls[0];
                                        newBall.velocity = sf::Vector2f(-balls[0].velocity.y, balls[0].velocity.x);
                                        newBall.launched = true;
                                        balls.push_back(newBall);
                                    }
                                } else if (pu.type != PowerUpType::None) {
                                    pu.shape.setFillColor(brick.shape.getOutlineColor());
                                    pu.shape.setSize(sf::Vector2f(28, 16));
                                    pu.shape.setOutlineThickness(0);
                                    pu.shape.setOrigin(14, 8);
                                    pu.shape.setPosition(brick.shape.getPosition());
                                    pu.velocity = sf::Vector2f(0, 120.f);
                                    pu.active = true;
                                    powerUps.push_back(pu);
                                }
                            }
                        }
                        // Simple collision response
                        sf::FloatRect b = brick.shape.getGlobalBounds();
                        sf::Vector2f ballPos = ball.shape.getPosition();
                        float overlapLeft = (ballPos.x + BALL_RADIUS) - b.left;
                        float overlapRight = (b.left + b.width) - (ballPos.x - BALL_RADIUS);
                        float overlapTop = (ballPos.y + BALL_RADIUS) - b.top;
                        float overlapBottom = (b.top + b.height) - (ballPos.y - BALL_RADIUS);
                        bool ballFromLeft = std::abs(overlapLeft) < std::abs(overlapRight);
                        bool ballFromTop = std::abs(overlapTop) < std::abs(overlapBottom);
                        float minOverlapX = ballFromLeft ? overlapLeft : overlapRight;
                        float minOverlapY = ballFromTop ? overlapTop : overlapBottom;
                        if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                            ball.velocity.x = -ball.velocity.x;
                        } else {
                            ball.velocity.y = -ball.velocity.y;
                        }
                        break;
                    }
                }
            }
            // Si plus aucune balle, on perd une vie et on relance une balle
            if (balls.empty()) {
                lives--;
                if (lives <= 0) gameOver = true;
                else {
                    spawnBall({paddle.getPosition().x, paddle.getPosition().y - PADDLE_HEIGHT / 2 - BALL_RADIUS});
                }
            }
            // PowerUps chute
            for (auto& pu : powerUps) {
                if (!pu.active) continue;
                pu.shape.move(pu.velocity * dt);
                if (pu.shape.getGlobalBounds().intersects(paddle.getGlobalBounds())) {
                    switch (pu.type) {
                        case PowerUpType::ExpandPaddle:
                            if (!paddleExpanded) {
                                paddle.setSize(sf::Vector2f(PADDLE_WIDTH * 1.7f, PADDLE_HEIGHT));
                                paddle.setOrigin(paddle.getSize().x / 2, paddle.getSize().y / 2);
                                paddleExpanded = true;
                                paddleExpandTimer = PADDLE_EXPAND_DURATION;
                            } else {
                                paddleExpandTimer = PADDLE_EXPAND_DURATION;
                            }
                            break;
                        case PowerUpType::ShrinkPaddle:
                            if (!paddleShrunk) {
                                paddle.setSize(sf::Vector2f(PADDLE_WIDTH * 0.6f, PADDLE_HEIGHT));
                                paddle.setOrigin(paddle.getSize().x / 2, paddle.getSize().y / 2);
                                paddleShrunk = true;
                                paddleShrinkTimer = PADDLE_SHRINK_DURATION;
                            } else {
                                paddleShrinkTimer = PADDLE_SHRINK_DURATION;
                            }
                            break;
                        case PowerUpType::MultiBall:
                            // Ajoute une nouvelle balle identique à la première balle lancée
                            if (!balls.empty()) {
                                Ball newBall = balls[0];
                                newBall.velocity = sf::Vector2f(-balls[0].velocity.y, balls[0].velocity.x);
                                newBall.launched = true;
                                balls.push_back(newBall);
                            }
                            break;
                        case PowerUpType::SlowBall:
                            if (!ballSlowed) {
                                for (auto& b : balls) b.velocity *= 0.6f;
                                ballSlowed = true;
                                slowBallTimer = SLOWBALL_DURATION;
                            } else {
                                slowBallTimer = SLOWBALL_DURATION;
                            }
                            break;
                        case PowerUpType::FastBall:
                            if (!ballFaster) {
                                for (auto& b : balls) b.velocity *= 1.5f;
                                ballFaster = true;
                                fastBallTimer = FASTBALL_DURATION;
                            } else {
                                fastBallTimer = FASTBALL_DURATION;
                            }
                            break;
                        default: break;
                    }
                    pu.active = false;
                }
                if (pu.shape.getPosition().y > WINDOW_HEIGHT + 30) {
                    pu.active = false;
                }
            }
            // Timers d'effets
            if (paddleExpanded) {
                paddleExpandTimer -= dt;
                if (paddleExpandTimer <= 0.f) {
                    paddle.setSize(sf::Vector2f(PADDLE_WIDTH, PADDLE_HEIGHT));
                    paddle.setOrigin(PADDLE_WIDTH / 2, PADDLE_HEIGHT / 2);
                    paddleExpanded = false;
                }
            }
            if (paddleShrunk) {
                paddleShrinkTimer -= dt;
                if (paddleShrinkTimer <= 0.f) {
                    paddle.setSize(sf::Vector2f(PADDLE_WIDTH, PADDLE_HEIGHT));
                    paddle.setOrigin(PADDLE_WIDTH / 2, PADDLE_HEIGHT / 2);
                    paddleShrunk = false;
                }
            }
            if (ballSlowed) {
                slowBallTimer -= dt;
                if (slowBallTimer <= 0.f) {
                    for (auto& b : balls) b.velocity /= 0.6f;
                    ballSlowed = false;
                }
            }
            if (ballFaster) {
                fastBallTimer -= dt;
                if (fastBallTimer <= 0.f) {
                    for (auto& b : balls) b.velocity /= 1.5f;
                    ballFaster = false;
                }
            }
        }

        // Draw
        window.clear();
        sf::VertexArray background(sf::Quads, 4);
        background[0].position = sf::Vector2f(0, 0);
        background[1].position = sf::Vector2f(WINDOW_WIDTH, 0);
        background[2].position = sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        background[3].position = sf::Vector2f(0, WINDOW_HEIGHT);
        background[0].color = sf::Color(30, 30, 60);
        background[1].color = sf::Color(40, 40, 80);
        background[2].color = sf::Color(20, 20, 40);
        background[3].color = sf::Color(10, 10, 20);
        window.draw(background);
        // Bricks
        // Ajout d'une icône pour les briques spéciales
        for (const auto& brick : bricks) {
            if (!brick.destroyed) {
                window.draw(brick.shape);
                if (brick.type == BrickType::PowerUp) {
                    drawPowerUpIcon(window, brick); // Dessiner l'icône correspondant au power-up
                }
            }
        }
        // Gestion des animations de destruction avec chute
        for (auto& brick : bricks) {
            if (brick.destroyed && brick.destructionTimer > 0.0f) {
                brick.destructionTimer -= dt;
                float scale = brick.destructionTimer / 0.5f; // Réduction d'échelle sur 0,5 seconde
                brick.shape.setScale(scale, scale);
                sf::Color fadeColor = brick.shape.getFillColor();
                fadeColor.a = static_cast<sf::Uint8>(255 * scale); // Effet de fondu
                brick.shape.setFillColor(fadeColor);

                // Faire tomber la brique vers le bas
                sf::Vector2f position = brick.shape.getPosition();
                position.y += 100 * dt; // Vitesse de chute
                brick.shape.setPosition(position);
                window.draw(brick.shape);
            }
        }
        // PowerUps
        for (const auto& pu : powerUps) {
            if (pu.active) window.draw(pu.shape);
        }
        // Paddle & Ball
        window.draw(paddle);
        for (const auto& ball : balls) window.draw(ball.shape);
        // Score & Lives & Level
        std::ostringstream oss;
        oss << "Score: " << score;
        scoreText.setString(oss.str());
        scoreText.setPosition(20, 10); // Always left-aligned
        window.draw(scoreText);
        oss.str("");
        oss << "Lives: " << lives;
        livesText.setString(oss.str());
        // Right-align livesText
        sf::FloatRect livesBounds = livesText.getLocalBounds();
        livesText.setPosition(WINDOW_WIDTH - livesBounds.width - 20, 10);
        window.draw(livesText);
        // Affichage du niveau
        oss.str("");
        oss << "Level: " << level << "/" << MAX_LEVEL;
        levelText.setString(oss.str());
        sf::FloatRect levelBounds = levelText.getLocalBounds();
        levelText.setPosition(WINDOW_WIDTH / 2.0f - levelBounds.width / 2.0f, 10);
        window.draw(levelText);
        // Info
        if (gameOver) {
            infoText.setString("GAME OVER\nPress R to restart");
            sf::FloatRect infoBounds = infoText.getLocalBounds();
            infoText.setOrigin(infoBounds.left + infoBounds.width / 2.0f, infoBounds.top + infoBounds.height / 2.0f);
            infoText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
            infoText.setFillColor(sf::Color::Red);
            window.draw(infoText);
        }
        if (gameWon) {
            infoText.setString("YOU WIN!\nPress R to restart");
            sf::FloatRect infoBounds = infoText.getLocalBounds();
            infoText.setOrigin(infoBounds.left + infoBounds.width / 2.0f, infoBounds.top + infoBounds.height / 2.0f);
            infoText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
            infoText.setFillColor(sf::Color::Green);
            window.draw(infoText);
        }
        if (!balls.empty() && !balls[0].launched && !gameOver && !gameWon) {
            infoText.setString("Press SPACE to launch the ball");
            sf::FloatRect infoBounds = infoText.getLocalBounds();
            infoText.setOrigin(infoBounds.left + infoBounds.width / 2.0f, infoBounds.top + infoBounds.height / 2.0f);
            infoText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
            infoText.setFillColor(sf::Color::Yellow);
            window.draw(infoText);
        }
        window.display();
    }
    return 0;
}
