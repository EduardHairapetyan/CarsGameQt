#ifndef CAR_H
#define CAR_H

#include <QPixmap>
#include "shared.h"

#define CAR_SIZE_X 100
#define CAR_SIZE_Y 200

class Car {
private:
    int carX;
    int carY;
    QPixmap carImage;

public:
    Car();
    QPixmap getPixmap() const; // Return by value
    void loadImage(const QString& path, int x, int y);
    void updateCar(int speed);
    void setX(int x);
    void setY(int y);
    int getX() const;
    int getY() const;
    void moveX(int step);
    void moveY(int step);
    bool isNull() const;
};

#endif // CAR_H
