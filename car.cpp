#include "car.h"
#include <QDebug>
// ===========================
// Car Class Implementation
// ===========================

// Constructor initializes the car's position to (0, 0)
Car::Car() : carX(0), carY(0) {}

// Checks if the car's image is null (not loaded)
bool Car::isNull() const {
    return carImage.isNull();
}

// Loads and scales the car image from the given path
void Car::loadImage(const QString &path, int x, int y) {
    if (!carImage.load(path)) {
        qDebug() << "Failed to load image from" << path;
        return;
    }
    carImage = carImage.scaled(x, y, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    qDebug() << "Loaded and scaled image from" << path << "to size (" << x << "," << y << ")";
}

// Updates the car's Y position to simulate downward movement
void Car::updateCar(int speed) {
    carY += speed;
    qDebug() << "Updating car position. New Y:" << carY;
    if (carY > WINDOWS_SIZE_Y) {
        qDebug() << "Car moved off-screen, wrapping around!";
        carY = -CAR_SIZE_Y - (rand() % 800);
        qDebug() << "Car Y reset to:" << carY;
    }
}

// Moves the car vertically by the specified step
void Car::moveY(int step) {
    carY += step;
    qDebug() << "Car moved vertically by" << step << "pixels. New Y:" << carY;
}

// Moves the car horizontally by the specified step
void Car::moveX(int step) {
    carX += step;
    qDebug() << "Car moved horizontally by" << step << "pixels. New X:" << carX;
}

// Returns the current X position of the car
int Car::getX() const {
    return carX;
}

// Returns the current Y position of the car
int Car::getY() const {
    return carY;
}

// Sets the car's X position
void Car::setX(int x) {
    carX = x;
    qDebug() << "Car X position set to:" << x;
}

// Sets the car's Y position
void Car::setY(int y) {
    carY = y;
    qDebug() << "Car Y position set to:" << y;
}

// Returns the car's image pixmap
QPixmap Car::getPixmap() const {
    return carImage;
}
