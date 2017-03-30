#include "monster.h"
#include "monster_wrapper.h"

extern "C" {
    Monster * newMonster() {
        return new Monster();
    }

    int Monster_get_x(Monster * m) {
        return m->getX();
    }
    int Monster_get_y(Monster * m) {
        return m->getY();
    }
    int Monster_get_decimal_type(Monster * m) {
        return m->getDecimalType();
    }
    int Monster_get_last_known_player_x(Monster * m) {
        return m->getLastKnownPlayerX();
    }
    int Monster_get_last_known_player_y(Monster * m) {
        return m->getLastKnownPlayerY();
    }
    int Monster_get_speed(Monster * m) {
        return m->getSpeed();
    }

    void Monster_set_x(Monster * m, int v) {
        m->setX(v);
    }
    void Monster_set_y(Monster *m, int v) {
        m->setY(v);
    }
    void Monster_set_decimal_type(Monster * m, int v) {
        m->setDecimalType(v);
    }
    void Monster_set_last_known_player_x(Monster * m, int v) {
        m->setLastKnownPlayerX(v);
    }
    void Monster_set_last_known_player_y(Monster * m, int v) {
        m->setLastKnownPlayerY(v);
    }
    void Monster_set_speed(Monster * m, int v) {
        m->setSpeed(v);
    }

    void Monster_reset_player_location(Monster * m) {
        m->setLastKnownPlayerX(0);
        m->setLastKnownPlayerY(0);
    }

    void deleteMonster(Monster *m) {
        delete m;
    }
};
