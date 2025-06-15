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
const int BRICK_ROWS = 4;
const int BRICK_COLS = 7;
const int BRICK_WIDTH = 90;
const int BRICK_HEIGHT = 32;
const int BRICK_SPACING = 12;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 18;
const float PADDLE_SPEED = 480.0f;
const float BALL_RADIUS = 9.0f;
const float BALL_SPEED = 420.0f;
const int LIVES = 3;
const int MAX_LEVEL = 5;

sf::Font iconFont;

enum class PowerUpType
{
    None,
    ExpandPaddle,
    ShrinkPaddle,
    MultiBall,
    SlowBall,
    FastBall
};

struct PowerUp
{
    PowerUpType type = PowerUpType::None;
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    bool active = false;
};

enum class BrickType
{
    Normal,
    PowerUp,
    Indestructible,
    Explosive,
    MultiHit
};

struct Brick
{
    sf::RectangleShape shape;
    bool destroyed = false;
    int points = 50;
    BrickType type = BrickType::Normal;
    float destructionTimer = 0.0f; // Timer pour l'animation de destruction
    int hitsLeft = 1;              // Pour les briques multi-coups
};

// Nouvelle structure pour gérer toutes les balles
struct Ball
{
    sf::CircleShape shape;
    sf::Vector2f velocity;
    bool launched = false;
};

// Prototype pour la génération de layout fixe
std::vector<std::vector<int>> getLevelLayout(int level);

void generateBricks(std::vector<Brick> &bricks, int level)
{
    bricks.clear();
    auto layout = getLevelLayout(level);
    int rows = layout.size();
    int cols = layout.empty() ? 0 : layout[0].size();
    float offsetX = (WINDOW_WIDTH - (cols * (BRICK_WIDTH + BRICK_SPACING) - BRICK_SPACING)) / 2.0f;
    float offsetY = 60.0f;
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            int cell = layout[i][j];
            if (cell < 0)
                continue; // -1 = pas de brique
            Brick brick;
            brick.shape.setSize(sf::Vector2f(BRICK_WIDTH, BRICK_HEIGHT));
            brick.shape.setOrigin(BRICK_WIDTH / 2, BRICK_HEIGHT / 2);
            brick.shape.setPosition(offsetX + j * (BRICK_WIDTH + BRICK_SPACING) + BRICK_WIDTH / 2,
                                    offsetY + i * (BRICK_HEIGHT + BRICK_SPACING) + BRICK_HEIGHT / 2);
            brick.destroyed = false;
            brick.destructionTimer = 0.f;
            // Types de briques selon la matrice
            switch (cell)
            {
            case 0: // Normal
                brick.type = BrickType::Normal;
                brick.shape.setFillColor(sf::Color(120, 120, 220));
                brick.points = 50;
                brick.hitsLeft = 1;
                break;
            case 1: // PowerUp
                brick.type = BrickType::PowerUp;
                brick.shape.setFillColor(sf::Color(180, 180, 80));
                brick.shape.setOutlineColor(sf::Color::Yellow);
                brick.shape.setOutlineThickness(2);
                brick.points = 80;
                brick.hitsLeft = 1;
                break;
            case 2: // Explosive
                brick.type = BrickType::Explosive;
                brick.shape.setFillColor(sf::Color(255, 140, 0));
                brick.shape.setOutlineColor(sf::Color(255, 140, 0));
                brick.shape.setOutlineThickness(3);
                brick.points = 120;
                brick.hitsLeft = 1;
                break;
            case 3: // Indestructible
                brick.type = BrickType::Indestructible;
                brick.shape.setFillColor(sf::Color(80, 80, 80));
                brick.shape.setOutlineColor(sf::Color::White);
                brick.shape.setOutlineThickness(2);
                brick.points = 0;
                brick.hitsLeft = 9999;
                break;
            case 4: // Multi-hit (2 coups)
                brick.type = BrickType::MultiHit;
                brick.shape.setFillColor(sf::Color(80, 180, 255));
                brick.shape.setOutlineColor(sf::Color(0, 120, 255));
                brick.shape.setOutlineThickness(2);
                brick.points = 100;
                brick.hitsLeft = 2;
                break;
            case 5: // Multi-hit (3 coups)
                brick.type = BrickType::MultiHit;
                brick.shape.setFillColor(sf::Color(255, 80, 180));
                brick.shape.setOutlineColor(sf::Color(180, 0, 120));
                brick.shape.setOutlineThickness(2);
                brick.points = 150;
                brick.hitsLeft = 3;
                break;
            default:
                brick.type = BrickType::Normal;
                brick.shape.setFillColor(sf::Color(120, 120, 220));
                brick.points = 50;
                brick.hitsLeft = 1;
                break;
            }
            bricks.push_back(brick);
        }
    }
}

// Update the drawPowerUpIcon function to use the new font
void drawPowerUpIcon(sf::RenderWindow &window, const Brick &brick)
{
    if (brick.type != BrickType::PowerUp)
        return;

    sf::Vector2f pos = brick.shape.getPosition();
    sf::Vector2f size = brick.shape.getSize();

    sf::Text icon;
    icon.setFont(iconFont);
    icon.setCharacterSize(static_cast<unsigned int>(size.y * 0.6f));
    icon.setFillColor(sf::Color::White);
    icon.setStyle(sf::Text::Bold);
    icon.setPosition(pos.x - size.x * 0.1f, pos.y - size.y * 0.4f);

    // Use ASCII characters as symbols for power-ups
    if (brick.shape.getOutlineColor() == sf::Color::Yellow)
    {
        icon.setString("+"); // Healing symbol
    }
    else if (brick.shape.getOutlineColor() == sf::Color::Cyan)
    {
        icon.setString("O"); // Multi-ball symbol
    }
    else if (brick.shape.getOutlineColor() == sf::Color::Red)
    {
        icon.setString("><"); // Shrink paddle symbol
    }
    else if (brick.shape.getOutlineColor() == sf::Color::Green)
    {
        icon.setString("< >"); // Expand paddle symbol
    }
    else
    {
        icon.setString("?"); // Default symbol
    }

    window.draw(icon);
}

// Génération de layouts fixes pour chaque niveau
std::vector<std::vector<int>> getLevelLayout(int level)
{
    // 0: normal, 1: powerup, 2: explosive, 3: indestructible, 4: multi-hit(2 coups), 5: multi-hit(3 coups)
    if (level == 1)
    {
        return {
            {0, 0, 0, 0, 0, 0, 0},
            {0, 4, 0, 1, 0, 4, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 2, 0, 2, 0, 0}};
    }
    else if (level == 2)
    {
        return {
            {3, 0, 4, 0, 4, 0, 3},
            {0, 2, 0, 1, 0, 2, 0},
            {4, 0, 5, 0, 5, 0, 4},
            {0, 0, 0, 0, 0, 0, 0}};
    }
    else if (level == 3)
    {
        return {
            {5, 0, 3, 0, 3, 0, 5},
            {0, 4, 2, 1, 2, 4, 0},
            {3, 0, 5, 0, 5, 0, 3},
            {0, 2, 0, 4, 0, 2, 0}};
    }
    else if (level == 4)
    {
        return {
            {4, 5, 4, 3, 4, 5, 4},
            {0, 2, 0, 1, 0, 2, 0},
            {5, 0, 4, 0, 4, 0, 5},
            {0, 3, 0, 2, 0, 3, 0}};
    }
    else
    {
        return {
            {5, 4, 3, 2, 3, 4, 5},
            {4, 5, 2, 1, 2, 5, 4},
            {3, 2, 5, 0, 5, 2, 3},
            {2, 1, 0, 3, 0, 1, 2}};
    }
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Ensure consistent randomization by initializing the random seed

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Breakout");
    window.setFramerateLimit(60);

    // Font
    sf::Font font;
    if (!font.loadFromFile("extern/fonts/PixelatedElegance.ttf"))
    {
        // fallback
        if (!font.loadFromFile("extern/fonts/GOODDP__.TTF"))
        {
            return 1;
        }
    }

    // Load the icon font
    if (!iconFont.loadFromFile("extern/fonts/GOODDP__.TTF"))
    {
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
    livesText.setFont(font);
    livesText.setCharacterSize(22);
    livesText.setFillColor(sf::Color::White);
    infoText.setFont(font);
    infoText.setCharacterSize(32);
    infoText.setFillColor(sf::Color::Yellow);
    infoText.setStyle(sf::Text::Bold);
    levelText.setFont(font);
    levelText.setCharacterSize(22);
    levelText.setFillColor(sf::Color::Cyan);

    std::vector<Ball> balls;
    auto spawnBall = [&](sf::Vector2f pos, sf::Vector2f vel = {0, 0}, bool launched = false)
    {
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
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                if (gameOver || gameWon)
                {
                    if (event.key.code == sf::Keyboard::R)
                    {
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

        if (!gameOver && !gameWon)
        {
            // Paddle movement
            float move = 0;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                move -= PADDLE_SPEED * dt;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                move += PADDLE_SPEED * dt;
            float newX = paddle.getPosition().x + move;
            newX = std::max(PADDLE_WIDTH / 2.f, std::min((float)WINDOW_WIDTH - PADDLE_WIDTH / 2.f, newX));
            paddle.setPosition(newX, paddle.getPosition().y);

            // Ball follows paddle before launch
            for (auto &ball : balls)
            {
                if (!ball.launched)
                {
                    ball.shape.setPosition(paddle.getPosition().x, paddle.getPosition().y - PADDLE_HEIGHT / 2 - BALL_RADIUS);
                }
            }
            // Lancement de la balle
            if (!balls.empty() && !balls[0].launched && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                float angle = (std::rand() % 120 + 30) * 3.14159f / 180.f;
                balls[0].velocity = sf::Vector2f(BALL_SPEED * std::cos(angle), BALL_SPEED * -std::sin(angle));
                balls[0].launched = true;
            }
            // Mouvement et collisions pour chaque balle
            for (size_t i = 0; i < balls.size(); ++i)
            {
                auto &ball = balls[i];
                if (!ball.launched)
                    continue;
                ball.shape.move(ball.velocity * dt);
                sf::Vector2f pos = ball.shape.getPosition();
                // Wall collisions
                if (pos.x - BALL_RADIUS < 0)
                {
                    ball.shape.setPosition(BALL_RADIUS, pos.y);
                    ball.velocity.x = -ball.velocity.x;
                }
                if (pos.x + BALL_RADIUS > WINDOW_WIDTH)
                {
                    ball.shape.setPosition(WINDOW_WIDTH - BALL_RADIUS, pos.y);
                    ball.velocity.x = -ball.velocity.x;
                }
                if (pos.y - BALL_RADIUS < 0)
                {
                    ball.shape.setPosition(pos.x, BALL_RADIUS);
                    ball.velocity.y = -ball.velocity.y;
                }
                // Bottom (lose ball)
                if (pos.y - BALL_RADIUS > WINDOW_HEIGHT)
                {
                    balls.erase(balls.begin() + i);
                    --i;
                    if (balls.empty()) {
                        lives--;
                        if (lives <= 0)
                            gameOver = true;
                        else {
                            // Affiche un message et attend que le joueur appuie sur ESPACE pour relancer
                            // On place une nouvelle balle non lancée sur la raquette
                            spawnBall({paddle.getPosition().x, paddle.getPosition().y - PADDLE_HEIGHT / 2 - BALL_RADIUS});
                            balls.back().launched = false;
                        }
                    }
                    continue;
                }
                // Paddle collision
                if (ball.shape.getGlobalBounds().intersects(paddle.getGlobalBounds()))
                {
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
                for (auto &brick : bricks)
                {
                    if (!brick.destroyed && ball.shape.getGlobalBounds().intersects(brick.shape.getGlobalBounds()))
                    {
                        if (brick.type == BrickType::Indestructible)
                        {
                            // Réponse simple : rebond
                        }
                        else if (brick.type == BrickType::MultiHit)
                        {
                            brick.hitsLeft--;
                            // Flash effect: set color to white for a short time
                            brick.shape.setFillColor(sf::Color::White);
                            brick.destructionTimer = 0.12f; // Use destructionTimer as a generic animation timer
                            // Changement de couleur selon coups restants (restored after flash)
                            if (brick.hitsLeft == 2)
                            {
                                brick.points = 100;
                                // color restored after flash
                            }
                            else if (brick.hitsLeft == 1)
                            {
                                brick.points = 100;
                                // color restored after flash
                            }
                            if (brick.hitsLeft <= 0)
                            {
                                brick.destroyed = true;
                                brick.destructionTimer = 0.5f;
                                score += brick.points;
                            }
                        }
                        else
                        {
                            brick.destroyed = true;
                            brick.destructionTimer = 0.5f;
                            score += brick.points;
                            if (brick.type == BrickType::PowerUp)
                            {
                                PowerUp pu;
                                pu.type = PowerUpType::None;
                                if (brick.shape.getOutlineColor() == sf::Color::Yellow)
                                    pu.type = PowerUpType::ExpandPaddle;
                                else if (brick.shape.getOutlineColor() == sf::Color::Red)
                                    pu.type = PowerUpType::ShrinkPaddle;
                                else if (brick.shape.getOutlineColor() == sf::Color::Cyan)
                                    pu.type = PowerUpType::MultiBall;
                                else if (brick.shape.getOutlineColor() == sf::Color(100, 255, 100))
                                    pu.type = PowerUpType::SlowBall;
                                else if (brick.shape.getOutlineColor() == sf::Color(255, 100, 255))
                                    pu.type = PowerUpType::FastBall;
                                if (pu.type == PowerUpType::MultiBall)
                                {
                                    // Ajoute une nouvelle balle identique à la première balle lancée
                                    if (!balls.empty())
                                    {
                                        Ball newBall = balls[0];
                                        newBall.velocity = sf::Vector2f(-balls[0].velocity.y, balls[0].velocity.x);
                                        newBall.launched = true;
                                        balls.push_back(newBall);
                                    }
                                }
                                else if (pu.type != PowerUpType::None)
                                {
                                    pu.shape.setFillColor(brick.shape.getOutlineColor());
                                    pu.shape.setSize(sf::Vector2f(28, 16));
                                    pu.shape.setOutlineThickness(0);
                                    pu.shape.setOrigin(14, 8);
                                    pu.shape.setPosition(brick.shape.getPosition());
                                    pu.velocity = sf::Vector2f(0, 220.f); // vitesse augmentée
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
                        if (std::abs(minOverlapX) < std::abs(minOverlapY))
                        {
                            ball.velocity.x = -ball.velocity.x;
                        }
                        else
                        {
                            ball.velocity.y = -ball.velocity.y;
                        }
                        // Gestion de l'explosion pour les briques explosives
                        if (brick.type == BrickType::Explosive && brick.destroyed)
                        {
                            for (auto &other : bricks)
                            {
                                if (&other == &brick || other.destroyed || other.type == BrickType::Indestructible)
                                    continue;
                                int row_b = std::round((brick.shape.getPosition().y - 60.0f) / (BRICK_HEIGHT + BRICK_SPACING));
                                int col_b = std::round((brick.shape.getPosition().x - ((WINDOW_WIDTH - (BRICK_COLS * (BRICK_WIDTH + BRICK_SPACING) - BRICK_SPACING)) / 2.0f) - BRICK_WIDTH / 2) / (BRICK_WIDTH + BRICK_SPACING));
                                int row_o = std::round((other.shape.getPosition().y - 60.0f) / (BRICK_HEIGHT + BRICK_SPACING));
                                int col_o = std::round((other.shape.getPosition().x - ((WINDOW_WIDTH - (BRICK_COLS * (BRICK_WIDTH + BRICK_SPACING) - BRICK_SPACING)) / 2.0f) - BRICK_WIDTH / 2) / (BRICK_WIDTH + BRICK_SPACING));
                                if (std::abs(row_b - row_o) <= 1 && std::abs(col_b - col_o) <= 1)
                                {
                                    other.destroyed = true;
                                    other.destructionTimer = 0.3f;
                                    other.shape.setFillColor(sf::Color(255, 255, 180));
                                    other.shape.setOutlineColor(sf::Color(255, 180, 0));
                                    other.shape.setOutlineThickness(2);
                                }
                            }
                            brick.shape.setFillColor(sf::Color(255, 80, 0));
                            brick.shape.setOutlineColor(sf::Color::White);
                            brick.shape.setOutlineThickness(3);
                        }
                        break;
                    }
                }
                // Si plus aucune balle, on perd une vie et on relance une balle
                if (balls.empty())
                {
                    lives--;
                    if (lives <= 0)
                        gameOver = true;
                    else
                    {
                        spawnBall({paddle.getPosition().x, paddle.getPosition().y - PADDLE_HEIGHT / 2 - BALL_RADIUS});
                    }
                }
                // PowerUps chute
                for (auto &pu : powerUps)
                {
                    if (!pu.active)
                        continue;
                    pu.shape.move(pu.velocity * dt);
                    if (pu.shape.getGlobalBounds().intersects(paddle.getGlobalBounds()))
                    {
                        switch (pu.type)
                        {
                        case PowerUpType::ExpandPaddle:
                            if (!paddleExpanded)
                            {
                                paddle.setSize(sf::Vector2f(PADDLE_WIDTH * 1.7f, PADDLE_HEIGHT));
                                paddle.setOrigin(paddle.getSize().x / 2, paddle.getSize().y / 2);
                                paddleExpanded = true;
                                paddleExpandTimer = PADDLE_EXPAND_DURATION;
                            }
                            else
                            {
                                paddleExpandTimer = PADDLE_EXPAND_DURATION;
                            }
                            break;
                        case PowerUpType::ShrinkPaddle:
                            if (!paddleShrunk)
                            {
                                paddle.setSize(sf::Vector2f(PADDLE_WIDTH * 0.6f, PADDLE_HEIGHT));
                                paddle.setOrigin(paddle.getSize().x / 2, paddle.getSize().y / 2);
                                paddleShrunk = true;
                                paddleShrinkTimer = PADDLE_SHRINK_DURATION;
                            }
                            else
                            {
                                paddleShrinkTimer = PADDLE_SHRINK_DURATION;
                            }
                            break;
                        case PowerUpType::MultiBall:
                            // Ajoute une nouvelle balle identique à la première balle lancée
                            if (!balls.empty())
                            {
                                Ball newBall = balls[0];
                                newBall.velocity = sf::Vector2f(-balls[0].velocity.y, balls[0].velocity.x);
                                newBall.launched = true;
                                balls.push_back(newBall);
                            }
                            break;
                        case PowerUpType::SlowBall:
                            if (!ballSlowed)
                            {
                                for (auto &b : balls)
                                    b.velocity *= 0.6f;
                                ballSlowed = true;
                                slowBallTimer = SLOWBALL_DURATION;
                            }
                            else
                            {
                                slowBallTimer = SLOWBALL_DURATION;
                            }
                            break;
                        case PowerUpType::FastBall:
                            if (!ballFaster)
                            {
                                for (auto &b : balls)
                                    b.velocity *= 1.5f;
                                ballFaster = true;
                                fastBallTimer = FASTBALL_DURATION;
                            }
                            else
                            {
                                fastBallTimer = FASTBALL_DURATION;
                            }
                            break;
                        default:
                            break;
                        }
                        pu.active = false;
                    }
                    if (pu.shape.getPosition().y > WINDOW_HEIGHT + 30)
                    {
                        pu.active = false;
                    }
                }
                // Timers d'effets
                if (paddleExpanded)
                {
                    paddleExpandTimer -= dt;
                    if (paddleExpandTimer <= 0.f)
                    {
                        paddle.setSize(sf::Vector2f(PADDLE_WIDTH, PADDLE_HEIGHT));
                        paddle.setOrigin(PADDLE_WIDTH / 2, PADDLE_HEIGHT / 2);
                        paddleExpanded = false;
                    }
                }
                if (paddleShrunk)
                {
                    paddleShrinkTimer -= dt;
                    if (paddleShrinkTimer <= 0.f)
                    {
                        paddle.setSize(sf::Vector2f(PADDLE_WIDTH, PADDLE_HEIGHT));
                        paddle.setOrigin(PADDLE_WIDTH / 2, PADDLE_HEIGHT / 2);
                        paddleShrunk = false;
                    }
                }
                if (ballSlowed)
                {
                    slowBallTimer -= dt;
                    if (slowBallTimer <= 0.f)
                    {
                        for (auto &b : balls)
                            b.velocity /= 0.6f;
                        ballSlowed = false;
                    }
                }
                if (ballFaster)
                {
                    fastBallTimer -= dt;
                    if (fastBallTimer <= 0.f)
                    {
                        for (auto &b : balls)
                            b.velocity /= 1.5f;
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
            for (const auto &brick : bricks)
            {
                if (!brick.destroyed)
                {
                    window.draw(brick.shape);
                    if (brick.type == BrickType::PowerUp)
                    {
                        drawPowerUpIcon(window, brick);
                    }
                    // Affiche le nombre de coups restants sur les briques multi-coups
                    if (brick.type == BrickType::MultiHit && brick.hitsLeft > 0)
                    {
                        sf::Text hitsText;
                        hitsText.setFont(font);
                        hitsText.setCharacterSize(18);
                        hitsText.setFillColor(sf::Color::Black);
                        hitsText.setStyle(sf::Text::Bold);
                        hitsText.setString(std::to_string(brick.hitsLeft));
                        sf::FloatRect textBounds = hitsText.getLocalBounds();
                        sf::Vector2f pos = brick.shape.getPosition();
                        hitsText.setOrigin(textBounds.width / 2, textBounds.height / 2);
                        hitsText.setPosition(pos.x, pos.y - 6);
                        window.draw(hitsText);
                    }
                }
            }
            // Gestion des animations de destruction avec chute
            for (auto &brick : bricks)
            {
                if (brick.destroyed && brick.destructionTimer > 0.0f)
                {
                    brick.destructionTimer -= dt;
                    float scale = brick.destructionTimer / 0.5f;
                    brick.shape.setScale(scale, scale);
                    sf::Color fadeColor = brick.shape.getFillColor();
                    fadeColor.a = static_cast<sf::Uint8>(255 * scale);
                    brick.shape.setFillColor(fadeColor);
                    sf::Vector2f position = brick.shape.getPosition();
                    position.y += 100 * dt; // Vitesse de chute
                    brick.shape.setPosition(position);
                    window.draw(brick.shape);
                }
                // Flash effect for multi-hit bricks (not destroyed)
                else if (brick.type == BrickType::MultiHit && !brick.destroyed && brick.destructionTimer > 0.0f)
                {
                    brick.destructionTimer -= dt;
                    if (brick.destructionTimer <= 0.0f)
                    {
                        // Restore color according to hitsLeft
                        if (brick.hitsLeft == 2)
                        {
                            brick.shape.setFillColor(sf::Color(80, 180, 255));
                            brick.shape.setOutlineColor(sf::Color(0, 120, 255));
                        }
                        else if (brick.hitsLeft == 1)
                        {
                            brick.shape.setFillColor(sf::Color(255, 200, 80));
                            brick.shape.setOutlineColor(sf::Color(255, 120, 0));
                        }
                    }
                }
            }
            // PowerUps
            for (const auto &pu : powerUps)
            {
                if (pu.active)
                    window.draw(pu.shape);
            }
            // Paddle & Ball
            window.draw(paddle);
            for (const auto &ball : balls)
                window.draw(ball.shape);
            // Score & Lives & Level
            std::ostringstream oss;
            oss << "Score: " << score;
            scoreText.setString(oss.str());
            scoreText.setPosition(20, 10); // Always left-aligned
            window.draw(scoreText);
            oss.str("");
            oss << "Lives: " << lives;
            livesText.setString(oss.str());
            sf::FloatRect livesBounds = livesText.getLocalBounds();
            livesText.setPosition(WINDOW_WIDTH - livesBounds.width - 20, 10);
            window.draw(livesText);
            oss.str("");
            oss << "Level: " << level << "/" << MAX_LEVEL;
            levelText.setString(oss.str());
            sf::FloatRect levelBounds = levelText.getLocalBounds();
            levelText.setPosition(WINDOW_WIDTH / 2.0f - levelBounds.width / 2.0f, 10);
            window.draw(levelText);
            // Info
            if (gameOver)
            {
                infoText.setString("GAME OVER\nPress R to restart");
                sf::FloatRect infoBounds = infoText.getLocalBounds();
                infoText.setOrigin(infoBounds.left + infoBounds.width / 2.0f, infoBounds.top + infoBounds.height / 2.0f);
                infoText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
                infoText.setFillColor(sf::Color::Red);
                window.draw(infoText);
            }
            if (gameWon)
            {
                infoText.setString("YOU WIN!\nPress R to restart");
                sf::FloatRect infoBounds = infoText.getLocalBounds();
                infoText.setOrigin(infoBounds.left + infoBounds.width / 2.0f, infoBounds.top + infoBounds.height / 2.0f);
                infoText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
                infoText.setFillColor(sf::Color::Green);
                window.draw(infoText);
            }
            if (!balls.empty() && !balls[0].launched && !gameOver && !gameWon)
            {
                infoText.setString("Press SPACE to launch the ball");
                sf::FloatRect infoBounds = infoText.getLocalBounds();
                infoText.setOrigin(infoBounds.left + infoBounds.width / 2.0f, infoBounds.top + infoBounds.height / 2.0f);
                infoText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
                infoText.setFillColor(sf::Color::Yellow);
                window.draw(infoText);
            }
            window.display();

            // Correction de la condition de fin de niveau
            bool allDestroyed = true;
            for (const auto &b : bricks)
            {
                if (!b.destroyed && b.destructionTimer <= 0.f)
                {
                    allDestroyed = false;
                    break;
                }
            }
            if (allDestroyed)
            {
                if (level < MAX_LEVEL)
                {
                    level++;
                    generateBricks(bricks, level);
                    float dropStartY = -100.f;
                    float offsetY = 60.0f;
                    // Clear all balls BEFORE the animation
                    balls.clear();
                    // Prepare animation state for each brick
                    std::vector<float> animProgress(bricks.size(), 0.f);
                    float animDuration = 0.7f; // Duration of the drop animation (seconds)
                    bool dropping = true;
                    sf::Clock animClock;
                    while (dropping && window.isOpen())
                    {
                        float dtDrop = animClock.restart().asSeconds();
                        dropping = false;
                        window.clear();
                        // Redraw background
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
                        // Animate bricks with acceleration (ease-in)
                        for (size_t i = 0; i < bricks.size(); ++i)
                        {
                            auto &brick = bricks[i];
                            int row = i / BRICK_COLS;
                            float target = offsetY + row * (BRICK_HEIGHT + BRICK_SPACING) + BRICK_HEIGHT / 2;
                            animProgress[i] += dtDrop / animDuration;
                            if (animProgress[i] < 1.f)
                                dropping = true;
                            float t = std::min(animProgress[i], 1.f);
                            // Quadratic ease-in: y = start + (target - start) * t^2
                            float y = dropStartY + (target - dropStartY) * (t * t);
                            brick.shape.setPosition(brick.shape.getPosition().x, y);
                            window.draw(brick.shape);
                            if (brick.type == BrickType::PowerUp)
                                drawPowerUpIcon(window, brick);
                        }
                        // Redraw paddle (static)
                        window.draw(paddle);
                        // Redraw UI (score, lives, level)
                        std::ostringstream oss;
                        oss << "Score: " << score;
                        scoreText.setString(oss.str());
                        scoreText.setPosition(20, 10);
                        window.draw(scoreText);
                        oss.str("");
                        oss << "Lives: " << lives;
                        livesText.setString(oss.str());
                        sf::FloatRect livesBounds = livesText.getLocalBounds();
                        livesText.setPosition(WINDOW_WIDTH - livesBounds.width - 20, 10);
                        window.draw(livesText);
                        oss.str("");
                        oss << "Level: " << level << "/" << MAX_LEVEL;
                        levelText.setString(oss.str());
                        sf::FloatRect levelBounds = levelText.getLocalBounds();
                        levelText.setPosition(WINDOW_WIDTH / 2.0f - levelBounds.width / 2.0f, 10);
                        window.draw(levelText);
                        window.display();
                    }
                    // Reset brick positions to their final location after the drop
                    for (size_t i = 0; i < bricks.size(); ++i)
                    {
                        int row = i / BRICK_COLS;
                        int col = i % BRICK_COLS;
                        bricks[i].shape.setPosition(
                            (WINDOW_WIDTH - (BRICK_COLS * (BRICK_WIDTH + BRICK_SPACING) - BRICK_SPACING)) / 2.0f + col * (BRICK_WIDTH + BRICK_SPACING) + BRICK_WIDTH / 2,
                            offsetY + row * (BRICK_HEIGHT + BRICK_SPACING) + BRICK_HEIGHT / 2);
                    }
                    // Spawn a new ball after the animation
                    spawnBall({paddle.getPosition().x, paddle.getPosition().y - PADDLE_HEIGHT / 2 - BALL_RADIUS});
                }
                else
                {
                    gameWon = true;
                }
            }
        }
    }
    return 0;
}