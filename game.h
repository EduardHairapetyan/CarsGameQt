#ifndef GAME_H
#define GAME_H

#include <QPainter>
#include <QKeyEvent>
#include <QPixmap>
#include <QMovie>
#include <QWidget>
#include <QSettings>
#include <QElapsedTimer>
#include <QPushButton>
#include "shared.h"
#include "car.h"

#define MOVING_STEP (WINDOWS_SIZE_X / 3)

class Game : public QWidget
{
    Q_OBJECT

public:
    explicit Game(QWidget* parent = nullptr, QTimer* timer = nullptr);
    ~Game();
    void paintEvent(QPaintEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void updateGame();
    void render(QPainter* painter);
    void restartGame();
    void initializeCarPositions();
    void loadBackground();
    void initializeRestartButton();
private:
    Car mainCar;
    Car secondaryCar1;
    Car secondaryCar2;
    Car secondaryCar3;
    QMovie* background;
    QTimer* timer;
    QElapsedTimer ecTimer;
    int level;

    // Game state variables
    bool isGameOver;
    bool showGameOverText;
    double recordTime;
    double finalTime;

    // Settings for persistent storage
    QSettings settings;

    // Timing
    qint64 elapsed; // Elapsed time in milliseconds

    QPushButton* restartButton;

    // Methods
    bool inRange(int playerX);
    bool checkCollision();
    bool isValidPosition(int padding);
};

#endif // GAME_H
