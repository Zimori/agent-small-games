#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <functional>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int BRICK_ROWS = 1;
const int BRICK_COLS = 1;
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

struct Brick {
    sf::RectangleShape shape;
    bool destroyed = false;
    int points = 50;
};

// Génère les briques pour un niveau donné
void generateBricks(std::vector<Brick>& bricks, int level) {
    bricks.clear();
    int rows = BRICK_ROWS + (level - 1); // Plus de lignes à chaque niveau
    int cols = BRICK_COLS;
    float offsetX = (WINDOW_WIDTH - (cols * (BRICK_WIDTH + BRICK_SPACING) - BRICK_SPACING)) / 2.0f;
    float offsetY = 60.0f;
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
            bricks.push_back(brick);
        }
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
                        brick.destroyed = true;
                        score += brick.points;
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
        }

        // Draw
        window.clear(sf::Color(30, 30, 40));
        // Bricks
        for (const auto& brick : bricks) {
            if (!brick.destroyed) window.draw(brick.shape);
        }
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
