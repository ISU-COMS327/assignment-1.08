#ifndef __MONSTER_H
#define __MONSTER_H

class Monster {
    private:
        int x;
        int y;
        int decimal_type;
        int last_known_player_x;
        int last_known_player_y;
        int speed;

    public:
        int getX();
        int getY();
        int getDecimalType();
        int getLastKnownPlayerX();
        int getLastKnownPlayerY();
        int getSpeed();
        void setX(int v);
        void setY(int v);
        void setDecimalType(int v);
        void setLastKnownPlayerX(int v);
        void setLastKnownPlayerY(int v);
        void setSpeed(int v);
};
#endif
