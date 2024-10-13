#include "game.h"
#include <QMovie>
#include <QPainter>
#include <QDebug>
#include <QTimer>

// ===========================
// Game Class Implementation
// ===========================

// Constructor initializes the game state and UI components
Game::Game(QWidget* parent, QTimer* timer)
    : QWidget(parent),
    isGameOver(false),
    showGameOverText(false),
    recordTime(0.0),
    finalTime(0.0),
    settings("YourOrganization", "YourGame"),
    timer(timer),
    elapsed(0)
{
    // Set focus policy to accept key events
    setFocusPolicy(Qt::StrongFocus);
    setFocus(); // Ensure the game widget has focus when the game starts
    qDebug() << "Game widget initialized and focus set.";

    // Load the record time from persistent storage
    recordTime = settings.value("recordTime", 0.0).toDouble();
    qDebug() << "Loaded recordTime:" << recordTime;

    // Connect the timer to the updateGame slot
    connect(timer, &QTimer::timeout, this, &Game::updateGame);
    timer->start(16); // Approximately 60 FPS
    qDebug() << "Timer started with interval 16ms.";

    // Start the elapsed timer
    ecTimer.start();
    qDebug() << "Elapsed timer started.";

    // Initialize game level
    level = 2;
    qDebug() << "Initial game level set to:" << level;

    // Initialize main car position
    mainCar.setX((WINDOWS_SIZE_X - CAR_SIZE_X) / 2); // Centered horizontally
    mainCar.setY(WINDOWS_SIZE_Y - CAR_SIZE_Y - 20); // Positioned near the bottom
    qDebug() << "Main car initialized at (" << mainCar.getX() << "," << mainCar.getY() << ")";

    // Load car images
    mainCar.loadImage(":/img/car_image.png", CAR_SIZE_X, CAR_SIZE_Y); // Main car
    secondaryCar1.loadImage(":/img/secondary_car2.png", CAR_SIZE_X, CAR_SIZE_Y); // Secondary cars
    secondaryCar2.loadImage(":/img/secondary_car2.png", CAR_SIZE_X, CAR_SIZE_Y);
    secondaryCar3.loadImage(":/img/secondary_car2.png", CAR_SIZE_X, CAR_SIZE_Y);

    // Initialize random seed
    srand(static_cast<unsigned int>(time(0)));
    qDebug() << "Random seed initialized.";

    // Set initial positions for secondary cars in different lanes
    initializeCarPositions();

    // Load animated background
    loadBackground();

    // Initialize Restart Button
    initializeRestartButton();
}

// Destructor cleans up the background movie
Game::~Game() {
    if (background) {
        background->stop();
        delete background;
        qDebug() << "Background movie stopped and deleted.";
    }
}

// Initializes the positions of the secondary cars in different lanes
void Game::initializeCarPositions() {
    secondaryCar1.setX(WINDOWS_SIZE_X - CAR_SIZE_X - 15); // Right lane
    secondaryCar1.setY(-CAR_SIZE_Y - (rand() % 800) + 2000); // Random Y within bounds
    qDebug() << "SecondaryCar1 initialized at (" << secondaryCar1.getX() << "," << secondaryCar1.getY() << ")";

    secondaryCar2.setX(15); // Left lane
    secondaryCar2.setY(-CAR_SIZE_Y - (rand() % 800) + 2000); // Random Y within bounds
    qDebug() << "SecondaryCar2 initialized at (" << secondaryCar2.getX() << "," << secondaryCar2.getY() << ")";

    secondaryCar3.setX((WINDOWS_SIZE_X - CAR_SIZE_X) / 2 - 10); // Center lane
    secondaryCar3.setY(-CAR_SIZE_Y - (rand() % 800) + 2000); // Random Y within bounds
    qDebug() << "SecondaryCar3 initialized at (" << secondaryCar3.getX() << "," << secondaryCar3.getY() << ")";
}

// Loads the animated background GIF
void Game::loadBackground() {
    background = new QMovie(":/img/background.gif"); // Update path if needed
    if (!background->isValid()) {
        qDebug() << "Failed to load background GIF!";
    } else {
        background->start(); // Start the animation
        qDebug() << "Background GIF loaded and animation started.";
    }
}

// Initializes the Restart Button and positions it at the upper right corner
void Game::initializeRestartButton() {
    // Initialize Restart Button
    restartButton = new QPushButton("Restart", this);
    restartButton->setFont(QFont("Arial", 16, QFont::Bold));
    restartButton->setFixedSize(100, 30);

    // Position the button at the upper right corner
    restartButton->move(0,0);
    qDebug() << "Restart button positioned at (" << restartButton->x() << "," << restartButton->y() << ")";

    // Set button styles
    restartButton->setStyleSheet("background-color: white; color: black;");
    restartButton->hide(); // Hide the button initially
    qDebug() << "Restart button initialized and hidden.";

    // Connect the button's clicked signal to the restartGame slot
    connect(restartButton, &QPushButton::clicked, this, &Game::restartGame);
    qDebug() << "Restart button connected to restartGame slot.";
}

// Handles the paint event by delegating to the render function
void Game::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    render(&painter);
    qDebug() << "paintEvent triggered and render function called.";
}

// Checks if the player's X position is within the game window boundaries
bool Game::inRange(int playerX) {
    bool range = (playerX >= 0) && ((playerX + CAR_SIZE_X) <= WINDOWS_SIZE_X);
    qDebug() << "Checking if player X:" << playerX << "is in range:" << range;
    return range;
}

// Handles key press events for left and right arrow keys
void Game::keyPressEvent(QKeyEvent* event) {
    qDebug() << "Key pressed:" << event->key();

    // If the game is over, ignore key presses
    if (isGameOver) {
        qDebug() << "Game is over. Ignoring key press.";
        return;
    }

    // Handle left arrow key press
    if (event->key() == Qt::Key_Left) {
        if (inRange(mainCar.getX() - MOVING_STEP)) {
            mainCar.moveX(-MOVING_STEP); // Move left
            qDebug() << "Moved main car left to X:" << mainCar.getX();
        } else {
            qDebug() << "Move left out of range.";
        }
    }
    // Handle right arrow key press
    else if (event->key() == Qt::Key_Right) {
        if (inRange(mainCar.getX() + MOVING_STEP)) {
            mainCar.moveX(MOVING_STEP); // Move right
            qDebug() << "Moved main car right to X:" << mainCar.getX();
        } else {
            qDebug() << "Move right out of range.";
        }
    }
}

// Checks for collisions between the main car and any secondary cars
bool Game::checkCollision() {
    const int padding = 30; // Padding to make collision detection less strict

    // Define rectangles for collision detection with padding
    QRect mainCarRect(mainCar.getX(), mainCar.getY(), CAR_SIZE_X - padding, CAR_SIZE_Y - padding);
    QRect secondaryCar1Rect(secondaryCar1.getX(), secondaryCar1.getY(), CAR_SIZE_X - padding, CAR_SIZE_Y - padding);
    QRect secondaryCar2Rect(secondaryCar2.getX(), secondaryCar2.getY(), CAR_SIZE_X - padding, CAR_SIZE_Y - padding);
    QRect secondaryCar3Rect(secondaryCar3.getX(), secondaryCar3.getY(), CAR_SIZE_X - padding, CAR_SIZE_Y - padding);

    // Check intersection with any secondary car
    bool collision = mainCarRect.intersects(secondaryCar1Rect) ||
                     mainCarRect.intersects(secondaryCar2Rect) ||
                     mainCarRect.intersects(secondaryCar3Rect);

    qDebug() << "Collision check result:" << collision;
    return collision;
}

// Handles key release events (currently no action needed)
void Game::keyReleaseEvent(QKeyEvent* event) {
    Q_UNUSED(event);
    qDebug() << "Key released:" << event->key();
    // Currently, no action needed on key release
}

// Validates the positions of secondary cars to ensure they aren't too close vertically
bool Game::isValidPosition(int padding) {
    bool valid = !(abs(secondaryCar1.getY() - secondaryCar2.getY()) < 2 * CAR_SIZE_Y + padding &&
                   abs(secondaryCar2.getY() - secondaryCar3.getY()) < 2 * CAR_SIZE_Y + padding);
    qDebug() << "Checking valid positions with padding" << padding << ":" << valid;
    return valid;
}

// Main game update loop called periodically by the timer
void Game::updateGame() {
    qDebug() << "updateGame called.";

    // If the game is over, do not update the game state
    if (isGameOver) {
        qDebug() << "Game is over. updateGame will not proceed.";
        return;
    }

    // Update level based on elapsed time to increase difficulty
    level = 3 + 2 * elapsed / 10000;
    qDebug() << "Game level updated to:" << level;

    // Move the secondary cars downward based on the current level (speed)
    secondaryCar1.updateCar(level);
    secondaryCar2.updateCar(level);
    secondaryCar3.updateCar(level);

    // Ensure cars are in valid positions to avoid overlapping lanes
    while (!isValidPosition(350)) { // Increased padding for better spacing
        qDebug() << "Invalid car positions detected. Resetting positions.";
        // Find the car with the smallest Y (highest on the screen)
        if (secondaryCar1.getY() <= secondaryCar2.getY() && secondaryCar1.getY() <= secondaryCar3.getY()) {
            // Move secondaryCar1 to a new random Y-position above the screen
            secondaryCar1.setY(-CAR_SIZE_Y - (rand() % 800) - 400);
            qDebug() << "SecondaryCar1 position reset to Y:" << secondaryCar1.getY();
        }
        else if (secondaryCar2.getY() <= secondaryCar1.getY() && secondaryCar2.getY() <= secondaryCar3.getY()) {
            // Move secondaryCar2 to a new random Y-position above the screen
            secondaryCar2.setY(-CAR_SIZE_Y - (rand() % 800) - 400);
            qDebug() << "SecondaryCar2 position reset to Y:" << secondaryCar2.getY();
        }
        else {
            // Move secondaryCar3 to a new random Y-position above the screen
            secondaryCar3.setY(-CAR_SIZE_Y - (rand() % 800) - 400);
            qDebug() << "SecondaryCar3 position reset to Y:" << secondaryCar3.getY();
        }
    }

    // Check for collision between the main car and any secondary car
    if (checkCollision()) {
        qDebug() << "Collision detected! Stopping the game.";

        // Stop the game timer to halt further updates
        timer->stop();
        qDebug() << "Timer stopped.";

        // Disconnect the timer signal to prevent further calls to updateGame
        disconnect(timer, &QTimer::timeout, this, &Game::updateGame);
        qDebug() << "Timer signal disconnected from updateGame slot.";

        // Load the crashed car image to indicate the collision visually
        if (!mainCar.isNull()) { // Ensure the car image is valid before loading
            mainCar.loadImage(":/img/crashed_car.png", CAR_SIZE_X, CAR_SIZE_Y);
            qDebug() << "Crashed car image loaded.";
        } else {
            qDebug() << "Failed to load crashed car image!";
        }

        // Record the final time in seconds
        finalTime = elapsed / 1000.0; // Convert milliseconds to seconds
        qDebug() << "Final time recorded:" << finalTime << "seconds.";

        // Update the record time if the current time is greater
        if (finalTime > recordTime) {
            recordTime = finalTime;
            settings.setValue("recordTime", recordTime); // Save the new record time persistently
            qDebug() << "New record time set:" << recordTime << "seconds.";
        }

        // Immediately update the UI to show the crashed car
        update();
        qDebug() << "UI updated to show crashed car.";

        // Set the game over flag to true to prevent further game updates
        isGameOver = true;
        qDebug() << "Game over flag set to true.";

        // Schedule the "Game Over" message to appear after a 2-second delay without blocking the main thread
        QTimer::singleShot(2000, this, [this]() {
            showGameOverText = true; // Flag to display the "Game Over" message
            update(); // Trigger a repaint to show "Game Over"
            restartButton->show(); // Show the Restart Button
            qDebug() << "Game Over message displayed and Restart button shown.";
            this->setFocus();      // Set focus back to the game widget
            qDebug() << "Focus set back to game widget.";
        });

        return; // Exit the updateGame method as the game is now over
    }

    // Update the elapsed time based on the QElapsedTimer
    elapsed = ecTimer.elapsed();
    qDebug() << "Elapsed time updated:" << elapsed << "milliseconds.";

    // Trigger a repaint to update the game's visuals
    update();
    qDebug() << "UI repaint triggered.";
}

// Renders the game visuals based on the current game state
void Game::render(QPainter* painter) {
    if (showGameOverText) {
        qDebug() << "Rendering Game Over screen.";

        // Step 1: Fill the entire window with black
        painter->fillRect(0, 0, WINDOWS_SIZE_X, WINDOWS_SIZE_Y, Qt::black);
        qDebug() << "Background filled with black.";

        // Step 2: Draw "Game Over" text
        painter->setPen(Qt::red);
        painter->setFont(QFont("Arial", 48, QFont::Bold));
        QString gameOverText = "Game Over";
        QFontMetrics fm(painter->font());
        int textWidth = fm.horizontalAdvance(gameOverText);
        int textHeight = fm.height();
        painter->drawText((WINDOWS_SIZE_X - textWidth) / 2, (WINDOWS_SIZE_Y - textHeight) / 2, gameOverText);
        qDebug() << "Game Over text drawn at center.";

        // Step 3: Draw the current time and record time below "Game Over"
        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 24));
        QString timeText = QString("Time: %1 s").arg(finalTime, 0, 'f', 2);
        QString recordText = QString("Record: %1 s").arg(recordTime, 0, 'f', 2);

        // Calculate positions for the time texts
        int centerX = WINDOWS_SIZE_X / 2;

        // Position texts with vertical spacing
        int gameOverY = WINDOWS_SIZE_Y / 2;
        int timeTextY = gameOverY + 50;
        int recordTextY = gameOverY + 90;

        // Draw time texts centered
        painter->drawText(centerX - 125, timeTextY, timeText); // Assuming 250 width
        painter->drawText(centerX - 125, recordTextY, recordText);
        qDebug() << "Time and Record texts drawn below Game Over.";

        // The Restart Button is shown via QTimer::singleShot in updateGame()
    }
    else {
        qDebug() << "Rendering normal game screen.";

        // Step 4: Normal game rendering

        // Fill the background with white to ensure black text is visible
        painter->fillRect(0, 0, WINDOWS_SIZE_X, WINDOWS_SIZE_Y, Qt::white);
        qDebug() << "Background filled with white.";

        // Draw the animated GIF as background
        if (background && !background->currentPixmap().isNull()) {
            painter->drawPixmap(0, 0, WINDOWS_SIZE_X, WINDOWS_SIZE_Y, background->currentPixmap());
            qDebug() << "Background GIF drawn.";
        }
        else {
            // If background GIF is not loaded, fill with a solid color
            painter->fillRect(0, 0, WINDOWS_SIZE_X, WINDOWS_SIZE_Y, Qt::darkGray);
            qDebug() << "Background GIF not loaded. Filled with dark gray.";
        }

        // Draw the main car image
        painter->drawPixmap(mainCar.getX(), mainCar.getY(), mainCar.getPixmap());
        qDebug() << "Main car drawn at (" << mainCar.getX() << "," << mainCar.getY() << ")";

        // Draw secondary cars
        painter->drawPixmap(secondaryCar1.getX(), secondaryCar1.getY(), secondaryCar1.getPixmap());
        painter->drawPixmap(secondaryCar2.getX(), secondaryCar2.getY(), secondaryCar2.getPixmap());
        painter->drawPixmap(secondaryCar3.getX(), secondaryCar3.getY(), secondaryCar3.getPixmap());
        qDebug() << "Secondary cars drawn.";

        // Get the elapsed time in seconds
        double_t elapsedTime = elapsed / 1000.0; // Convert milliseconds to seconds

        // Draw the timer background
        painter->setBrush(Qt::black); // Set brush to black
        painter->setPen(Qt::NoPen); // No border
        painter->drawRect(10, 10, 140, 30); // Draw the rectangle for the background
        qDebug() << "Timer background drawn.";

        // Draw the timer text in white
        painter->setPen(Qt::white); // Set text color to white
        painter->setFont(QFont("Arial", 16)); // Set font and size
        painter->drawText(15, 32, QString("Time: %1").arg(elapsedTime, 0, 'f', 2)); // Draw the time with two decimal places
        qDebug() << "Timer text drawn:" << QString("Time: %1").arg(elapsedTime, 0, 'f', 2);

        // Hide the Restart Button if it's visible
        if (restartButton->isVisible()) {
            restartButton->hide();
            qDebug() << "Restart button hidden during normal game rendering.";
        }
    }
}

// Handles the restart logic when the Restart Button is clicked
void Game::restartGame() {
    qDebug() << "Restart button clicked. Restarting the game.";

    // Reset game state variables
    isGameOver = false;
    showGameOverText = false;
    finalTime = 0.0;
    elapsed = 0;
    level = 2;
    qDebug() << "Game state variables reset.";

    // Reset the main car position
    mainCar.setX((WINDOWS_SIZE_X - CAR_SIZE_X) / 2); // Centered horizontally
    mainCar.setY(WINDOWS_SIZE_Y - CAR_SIZE_Y - 20); // Positioned near the bottom
    mainCar.loadImage(":/img/car_image.png", CAR_SIZE_X, CAR_SIZE_Y); // Reload main car image
    qDebug() << "Main car position reset and image reloaded.";

    // Reset secondary cars positions
    secondaryCar1.setX(WINDOWS_SIZE_X - CAR_SIZE_X - 15); // Right lane
    secondaryCar1.setY(-CAR_SIZE_Y - (rand() % 800) + 2000); // Random Y within bounds
    qDebug() << "SecondaryCar1 position reset to (" << secondaryCar1.getX() << "," << secondaryCar1.getY() << ")";

    secondaryCar2.setX(15); // Left lane
    secondaryCar2.setY(-CAR_SIZE_Y - (rand() % 800) + 2000); // Random Y within bounds
    qDebug() << "SecondaryCar2 position reset to (" << secondaryCar2.getX() << "," << secondaryCar2.getY() << ")";

    secondaryCar3.setX((WINDOWS_SIZE_X - CAR_SIZE_X) / 2 - 10); // Center lane
    secondaryCar3.setY(-CAR_SIZE_Y - (rand() % 800) + 2000); // Random Y within bounds
    qDebug() << "SecondaryCar3 position reset to (" << secondaryCar3.getX() << "," << secondaryCar3.getY() << ")";

    // Reload background animation if needed
    if (background && !background->isValid()) {
        delete background;
        background = new QMovie(":/img/background.gif");
        if (background->isValid()) {
            background->start();
            qDebug() << "Background GIF reloaded and animation started.";
        } else {
            qDebug() << "Failed to reload background GIF!";
        }
    }

    // Restart the elapsed timer
    ecTimer.restart();
    qDebug() << "Elapsed timer restarted.";

    // Reconnect the timer signal to the updateGame slot
    // First, ensure no duplicate connections by disconnecting existing ones
    disconnect(timer, &QTimer::timeout, this, &Game::updateGame);
    connect(timer, &QTimer::timeout, this, &Game::updateGame);
    timer->start(16); // Approximately 60 FPS
    qDebug() << "Timer reconnected to updateGame slot and started.";

    // Hide the Restart Button
    restartButton->hide();
    qDebug() << "Restart button hidden after restarting the game.";

    // Set focus back to the game widget to receive key events
    this->setFocus();
    qDebug() << "Focus set back to game widget after restart.";

    // Trigger a repaint to update the UI
    update();
    qDebug() << "UI repaint triggered after restart.";
}
