#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <functional>
#include <random>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int BRICK_ROWS = 6;
const int BRICK_COLS = 10;
const int BRICK_WIDTH = 60;
const int BRICK_HEIGHT = 20;
const int BRICK_SPACING = 8;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 18;
const float PADDLE_SPEED = 400.0f;
const float BALL_RADIUS = 10.0f;
const float BALL_SPEED = 320.0f;
const int LIVES = 3;
const int MAX_LEVEL = 5;

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
};

// Génère les briques pour un niveau donné
void generateBricks(std::vector<Brick>& bricks, int level) {
    bricks.clear();
    int rows = BRICK_ROWS + (level - 1); // Plus de lignes à chaque niveau
    int cols = BRICK_COLS;
    float offsetX = (WINDOW_WIDTH - (cols * (BRICK_WIDTH + BRICK_SPACING) - BRICK_SPACING)) / 2.0f;
    float offsetY = 60.0f;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 9);
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
                brick.type = BrickType::PowerUp;
                brick.shape.setOutlineThickness(2);
                brick.shape.setOutlineColor(sf::Color::Yellow);
            } else {
                brick.type = BrickType::Normal;
            }
            bricks.push_back(brick);
        }
    }
}

// Ajout d'une fonction pour dessiner une icône sur une brique power-up
void drawPowerUpIcon(sf::RenderWindow& window, const Brick& brick) {
    if (brick.type != BrickType::PowerUp) return;
    // Déterminer le type de power-up à partir de la couleur de la brique (même logique que lors du spawn)
    sf::Vector2f pos = brick.shape.getPosition();
    sf::Vector2f size = brick.shape.getSize();
    // sf::Color c = brick.shape.getOutlineColor(); // supprimé car inutilisé
    sf::Color fill = brick.shape.getFillColor();
    // On devine le type par la couleur de remplissage
    if (fill == sf::Color::Yellow) {
        // Expand Paddle: flèche horizontale
        sf::RectangleShape arrow(sf::Vector2f(size.x * 0.6f, 4));
        arrow.setFillColor(sf::Color::White);
        arrow.setOrigin(arrow.getSize().x / 2, 2);
        arrow.setPosition(pos.x, pos.y);
        window.draw(arrow);
        sf::CircleShape left(5, 3); left.setRotation(90); left.setFillColor(sf::Color::White);
        left.setOrigin(5, 5); left.setPosition(pos.x - size.x * 0.3f, pos.y);
        window.draw(left);
        sf::CircleShape right(5, 3); right.setRotation(-90); right.setFillColor(sf::Color::White);
        right.setOrigin(5, 5); right.setPosition(pos.x + size.x * 0.3f, pos.y);
        window.draw(right);
    } else if (fill == sf::Color::Red) {
        // Shrink Paddle: flèche horizontale vers l'intérieur
        sf::RectangleShape arrow(sf::Vector2f(size.x * 0.3f, 4));
        arrow.setFillColor(sf::Color::White);
        arrow.setOrigin(arrow.getSize().x / 2, 2);
        arrow.setPosition(pos.x, pos.y);
        window.draw(arrow);
        sf::CircleShape left(5, 3); left.setRotation(-90); left.setFillColor(sf::Color::White);
        left.setOrigin(5, 5); left.setPosition(pos.x - size.x * 0.15f, pos.y);
        window.draw(left);
        sf::CircleShape right(5, 3); right.setRotation(90); right.setFillColor(sf::Color::White);
        right.setOrigin(5, 5); right.setPosition(pos.x + size.x * 0.15f, pos.y);
        window.draw(right);
    } else if (fill == sf::Color::Cyan) {
        // MultiBall: dessiner une petite balle
        sf::CircleShape ballIcon(size.y * 0.22f);
        ballIcon.setFillColor(sf::Color::White);
        ballIcon.setOrigin(ballIcon.getRadius(), ballIcon.getRadius());
        ballIcon.setPosition(pos.x, pos.y);
        window.draw(ballIcon);
        sf::CircleShape ballIcon2(size.y * 0.13f);
        ballIcon2.setFillColor(sf::Color(100,255,255));
        ballIcon2.setOrigin(ballIcon2.getRadius(), ballIcon2.getRadius());
        ballIcon2.setPosition(pos.x + size.x * 0.18f, pos.y + size.y * 0.13f);
        window.draw(ballIcon2);
    } else if (fill == sf::Color(100,255,100)) {
        // SlowBall: icône tortue (cercle + 2 pattes)
        sf::CircleShape shell(size.y * 0.18f);
        shell.setFillColor(sf::Color::White);
        shell.setOrigin(shell.getRadius(), shell.getRadius());
        shell.setPosition(pos.x, pos.y);
        window.draw(shell);
        sf::RectangleShape leg(sf::Vector2f(8, 3));
        leg.setFillColor(sf::Color::White);
        leg.setOrigin(4, 1.5f);
        leg.setPosition(pos.x - size.x * 0.10f, pos.y + size.y * 0.10f);
        window.draw(leg);
        leg.setPosition(pos.x + size.x * 0.10f, pos.y + size.y * 0.10f);
        window.draw(leg);
    } else if (fill == sf::Color(255,100,255)) {
        // FastBall: icône éclair (zigzag)
        sf::VertexArray lightning(sf::LineStrip, 4);
        lightning[0].position = pos + sf::Vector2f(-size.x * 0.13f, -size.y * 0.10f);
        lightning[1].position = pos + sf::Vector2f(0, size.y * 0.10f);
        lightning[2].position = pos + sf::Vector2f(size.x * 0.10f, -size.y * 0.10f);
        lightning[3].position = pos + sf::Vector2f(0, size.y * 0.18f);
        for (int i = 0; i < 4; ++i) lightning[i].color = sf::Color::White;
        window.draw(lightning);
    }
}

int main() {
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

    // Paddle
    sf::RectangleShape paddle(sf::Vector2f(PADDLE_WIDTH, PADDLE_HEIGHT));
    paddle.setFillColor(sf::Color(200, 200, 255));
    paddle.setOrigin(PADDLE_WIDTH / 2, PADDLE_HEIGHT / 2);
    paddle.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT - 40);

    // Ball
    sf::CircleShape ball(BALL_RADIUS);
    ball.setFillColor(sf::Color(255, 200, 100));
    ball.setOrigin(BALL_RADIUS, BALL_RADIUS);
    ball.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    sf::Vector2f ballVelocity(0, 0);
    bool ballLaunched = false;

    // Bricks
    std::vector<Brick> bricks;
    int score = 0;
    int lives = LIVES;
    int level = 1;
    bool gameOver = false;
    bool gameWon = false;

    generateBricks(bricks, level);

    std::vector<PowerUp> powerUps;
    std::vector<sf::CircleShape> extraBalls; // Pour MultiBall
    std::vector<sf::Vector2f> extraBallsVel;
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
                        ball.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
                        ballVelocity = sf::Vector2f(0, 0);
                        ballLaunched = false;
                        paddle.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT - 40);
                    }
                }
                if (!ballLaunched && !gameOver && !gameWon && event.key.code == sf::Keyboard::Space) {
                    float angle = (std::rand() % 120 + 30) * 3.14159f / 180.f;
                    ballVelocity = sf::Vector2f(BALL_SPEED * std::cos(angle), BALL_SPEED * -std::sin(angle));
                    ballLaunched = true;
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
            if (!ballLaunched) {
                ball.setPosition(paddle.getPosition().x, paddle.getPosition().y - PADDLE_HEIGHT / 2 - BALL_RADIUS);
            } else {
                // Ball movement
                ball.move(ballVelocity * dt);
                sf::Vector2f pos = ball.getPosition();

                // Wall collisions
                if (pos.x - BALL_RADIUS < 0) {
                    ball.setPosition(BALL_RADIUS, pos.y);
                    ballVelocity.x = -ballVelocity.x;
                }
                if (pos.x + BALL_RADIUS > WINDOW_WIDTH) {
                    ball.setPosition(WINDOW_WIDTH - BALL_RADIUS, pos.y);
                    ballVelocity.x = -ballVelocity.x;
                }
                if (pos.y - BALL_RADIUS < 0) {
                    ball.setPosition(pos.x, BALL_RADIUS);
                    ballVelocity.y = -ballVelocity.y;
                }
                // Bottom (lose life)
                if (pos.y - BALL_RADIUS > WINDOW_HEIGHT) {
                    lives--;
                    ballLaunched = false;
                    ballVelocity = sf::Vector2f(0, 0);
                    ball.setPosition(paddle.getPosition().x, paddle.getPosition().y - PADDLE_HEIGHT / 2 - BALL_RADIUS);
                    if (lives <= 0) gameOver = true;
                }

                // Paddle collision
                if (ball.getGlobalBounds().intersects(paddle.getGlobalBounds())) {
                    float px = (ball.getPosition().x - paddle.getPosition().x) / (PADDLE_WIDTH / 2);
                    float angle = px * 75 * 3.14159f / 180.f;
                    float speed = std::sqrt(ballVelocity.x * ballVelocity.x + ballVelocity.y * ballVelocity.y);
                    ballVelocity.x = speed * std::sin(angle);
                    ballVelocity.y = -std::abs(speed * std::cos(angle));
                    ball.setPosition(ball.getPosition().x, paddle.getPosition().y - PADDLE_HEIGHT / 2 - BALL_RADIUS - 1);
                }

                // Brick collisions
                for (auto& brick : bricks) {
                    if (!brick.destroyed && ball.getGlobalBounds().intersects(brick.shape.getGlobalBounds())) {
                        if (brick.type != BrickType::Indestructible) {
                            brick.destroyed = true;
                            score += brick.points;
                            // Si brique powerup, spawn un powerup
                            if (brick.type == BrickType::PowerUp) {
                                PowerUp pu;
                                int ptype = std::rand() % 5;
                                switch (ptype) {
                                    case 0:
                                        pu.type = PowerUpType::ExpandPaddle;
                                        pu.shape.setFillColor(sf::Color::Yellow);
                                        break;
                                    case 1:
                                        pu.type = PowerUpType::ShrinkPaddle;
                                        pu.shape.setFillColor(sf::Color::Red);
                                        break;
                                    case 2:
                                        pu.type = PowerUpType::MultiBall;
                                        pu.shape.setFillColor(sf::Color::Cyan);
                                        break;
                                    case 3:
                                        pu.type = PowerUpType::SlowBall;
                                        pu.shape.setFillColor(sf::Color(100,255,100));
                                        break;
                                    case 4:
                                        pu.type = PowerUpType::FastBall;
                                        pu.shape.setFillColor(sf::Color(255,100,255));
                                        break;
                                }
                                pu.shape.setSize(sf::Vector2f(28, 16));
                                pu.shape.setOutlineThickness(2);
                                pu.shape.setOutlineColor(sf::Color::White);
                                pu.shape.setOrigin(14, 8);
                                pu.shape.setPosition(brick.shape.getPosition());
                                pu.velocity = sf::Vector2f(0, 120.f);
                                pu.active = true;
                                powerUps.push_back(pu);
                            }
                        }
                        // Simple collision response
                        sf::FloatRect b = brick.shape.getGlobalBounds();
                        sf::Vector2f ballPos = ball.getPosition();
                        float overlapLeft = (ballPos.x + BALL_RADIUS) - b.left;
                        float overlapRight = (b.left + b.width) - (ballPos.x - BALL_RADIUS);
                        float overlapTop = (ballPos.y + BALL_RADIUS) - b.top;
                        float overlapBottom = (b.top + b.height) - (ballPos.y - BALL_RADIUS);
                        bool ballFromLeft = std::abs(overlapLeft) < std::abs(overlapRight);
                        bool ballFromTop = std::abs(overlapTop) < std::abs(overlapBottom);
                        float minOverlapX = ballFromLeft ? overlapLeft : overlapRight;
                        float minOverlapY = ballFromTop ? overlapTop : overlapBottom;
                        if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                            ballVelocity.x = -ballVelocity.x;
                        } else {
                            ballVelocity.y = -ballVelocity.y;
                        }
                        break;
                    }
                }

                // Win condition (niveau)
                bool allDestroyed = true;
                for (const auto& b : bricks) if (!b.destroyed) { allDestroyed = false; break; }
                if (allDestroyed) {
                    if (level < MAX_LEVEL) {
                        level++;
                        generateBricks(bricks, level);
                        ballLaunched = false;
                        ballVelocity = sf::Vector2f(0, 0);
                        ball.setPosition(paddle.getPosition().x, paddle.getPosition().y - PADDLE_HEIGHT / 2 - BALL_RADIUS);
                        // Optionnel : petit message de passage de niveau
                    } else {
                        gameWon = true;
                    }
                }
            }

            // PowerUps chute
            for (auto& pu : powerUps) {
                if (!pu.active) continue;
                pu.shape.move(pu.velocity * dt);
                // Collision avec la raquette
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
                            if (extraBalls.empty()) {
                                sf::CircleShape newBall(BALL_RADIUS);
                                newBall.setFillColor(sf::Color(100,255,255));
                                newBall.setOrigin(BALL_RADIUS, BALL_RADIUS);
                                newBall.setPosition(ball.getPosition());
                                extraBalls.push_back(newBall);
                                // Vitesse différente
                                sf::Vector2f v = ballVelocity;
                                std::swap(v.x, v.y);
                                extraBallsVel.push_back(v);
                            }
                            break;
                        case PowerUpType::SlowBall:
                            if (!ballSlowed) {
                                ballVelocity *= 0.6f;
                                for (auto& v : extraBallsVel) v *= 0.6f;
                                ballSlowed = true;
                                slowBallTimer = SLOWBALL_DURATION;
                            } else {
                                slowBallTimer = SLOWBALL_DURATION;
                            }
                            break;
                        case PowerUpType::FastBall:
                            if (!ballFaster) {
                                ballVelocity *= 1.5f;
                                for (auto& v : extraBallsVel) v *= 1.5f;
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
                    ballVelocity /= 0.6f;
                    for (auto& v : extraBallsVel) v /= 0.6f;
                    ballSlowed = false;
                }
            }
            if (ballFaster) {
                fastBallTimer -= dt;
                if (fastBallTimer <= 0.f) {
                    ballVelocity /= 1.5f;
                    for (auto& v : extraBallsVel) v /= 1.5f;
                    ballFaster = false;
                }
            }

            // Gestion des extra balls (multi-balles)
            for (size_t i = 0; i < extraBalls.size(); ++i) {
                extraBalls[i].move(extraBallsVel[i] * dt);
                sf::Vector2f pos = extraBalls[i].getPosition();
                // Wall collisions
                if (pos.x - BALL_RADIUS < 0) {
                    extraBalls[i].setPosition(BALL_RADIUS, pos.y);
                    extraBallsVel[i].x = -extraBallsVel[i].x;
                }
                if (pos.x + BALL_RADIUS > WINDOW_WIDTH) {
                    extraBalls[i].setPosition(WINDOW_WIDTH - BALL_RADIUS, pos.y);
                    extraBallsVel[i].x = -extraBallsVel[i].x;
                }
                if (pos.y - BALL_RADIUS < 0) {
                    extraBalls[i].setPosition(pos.x, BALL_RADIUS);
                    extraBallsVel[i].y = -extraBallsVel[i].y;
                }
                // Bottom (lose life for extra ball)
                if (pos.y - BALL_RADIUS > WINDOW_HEIGHT) {
                    extraBalls.erase(extraBalls.begin() + i);
                    extraBallsVel.erase(extraBallsVel.begin() + i);
                    --i;
                    continue;
                }
                // Paddle collision
                if (extraBalls[i].getGlobalBounds().intersects(paddle.getGlobalBounds())) {
                    float px = (extraBalls[i].getPosition().x - paddle.getPosition().x) / (PADDLE_WIDTH / 2);
                    float angle = px * 75 * 3.14159f / 180.f;
                    float speed = std::sqrt(extraBallsVel[i].x * extraBallsVel[i].x + extraBallsVel[i].y * extraBallsVel[i].y);
                    extraBallsVel[i].x = speed * std::sin(angle);
                    extraBallsVel[i].y = -std::abs(speed * std::cos(angle));
                    extraBalls[i].setPosition(extraBalls[i].getPosition().x, paddle.getPosition().y - PADDLE_HEIGHT / 2 - BALL_RADIUS - 1);
                }
                // Brick collisions
                for (auto& brick : bricks) {
                    if (!brick.destroyed && extraBalls[i].getGlobalBounds().intersects(brick.shape.getGlobalBounds())) {
                        if (brick.type != BrickType::Indestructible) {
                            brick.destroyed = true;
                            score += brick.points;
                        }
                        // Simple collision response
                        sf::FloatRect b = brick.shape.getGlobalBounds();
                        sf::Vector2f ballPos = extraBalls[i].getPosition();
                        float overlapLeft = (ballPos.x + BALL_RADIUS) - b.left;
                        float overlapRight = (b.left + b.width) - (ballPos.x - BALL_RADIUS);
                        float overlapTop = (ballPos.y + BALL_RADIUS) - b.top;
                        float overlapBottom = (b.top + b.height) - (ballPos.y - BALL_RADIUS);
                        bool ballFromLeft = std::abs(overlapLeft) < std::abs(overlapRight);
                        bool ballFromTop = std::abs(overlapTop) < std::abs(overlapBottom);
                        float minOverlapX = ballFromLeft ? overlapLeft : overlapRight;
                        float minOverlapY = ballFromTop ? overlapTop : overlapBottom;
                        if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                            extraBallsVel[i].x = -extraBallsVel[i].x;
                        } else {
                            extraBallsVel[i].y = -extraBallsVel[i].y;
                        }
                        break;
                    }
                }
            }
        }

        // Draw
        window.clear(sf::Color(30, 30, 40));
        // Bricks
        for (const auto& brick : bricks) {
            if (!brick.destroyed) {
                window.draw(brick.shape);
                drawPowerUpIcon(window, brick);
            }
        }
        // PowerUps
        for (const auto& pu : powerUps) {
            if (pu.active) window.draw(pu.shape);
        }
        // Extra balls
        for (const auto& b : extraBalls) window.draw(b);
        // Paddle & Ball
        window.draw(paddle);
        window.draw(ball);
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
        if (!ballLaunched && !gameOver && !gameWon) {
            infoText.setString("Press SPACE to launch the ball");
            sf::FloatRect infoBounds = infoText.getLocalBounds();
            infoText.setOrigin(infoBounds.left + infoBounds.width / 2.0f, infoBounds.top + infoBounds.height / 2.0f);
            infoText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
            infoText.setFillColor(sf::Color::Yellow);
            window.draw(infoText);
        }
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
        window.display();
    }
    return 0;
}
