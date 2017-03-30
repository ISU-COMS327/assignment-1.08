#include "monster.h"

int Monster :: getX() {
    return x;
}

int Monster :: getY() {
    return y;
}

int Monster :: getDecimalType() {
    return decimal_type;
}

int Monster ::getLastKnownPlayerX() {
    return last_known_player_x;
}

int Monster :: getLastKnownPlayerY() {
    return last_known_player_y;
}

int Monster :: getSpeed() {
    return speed;
}

void Monster :: setX(int v) {
   x = v;
}

void Monster :: setY(int v) {
    y = v;
}

void Monster :: setDecimalType(int v) {
    decimal_type = v;
}

void Monster :: setLastKnownPlayerX(int v) {
    last_known_player_x = v;
}

void Monster :: setLastKnownPlayerY(int v) {
    last_known_player_y = v;
}

void Monster :: setSpeed(int v) {
    speed = v;
}

