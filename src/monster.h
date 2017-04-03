#ifndef __MONSTER_H
#define __MONSTER_H

#include "numeric.h"

class Monster {
    private:

    public:
        string name;
        string description;
        string color;
        char symbol;
        int speed;
        vector<string> abilities;
        int hitpoints;
        Numeric * attack_damage;
        int last_known_player_x;
        int last_known_player_y;
        int decimal_type;
        int x;
        int y;

        void resetPlayerLocation();
        int getDecimalType();
};
#endif
