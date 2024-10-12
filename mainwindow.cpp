#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setFixedSize(WINDOWS_SIZE_X, WINDOWS_SIZE_Y);

    // Set up the timer
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateGame); // Connect to the update method
    timer->start(20); // Approximately 60 FPS

    game = new Game(this, timer);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    game->keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    game->keyReleaseEvent(event);
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    game->render(&painter); // Render the game
}

void MainWindow::updateGame() {
    game->updateGame(); // Update game state
    update(); // Trigger a repaint
}
