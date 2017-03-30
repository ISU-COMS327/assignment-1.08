#include <iostream>
#include <sstream>
#include "monster_template.h"

using namespace std;

string numeric_to_string(numeric_t numeric) {
    return to_string(numeric.base) + "+" + to_string(numeric.dice) + "d" + to_string(numeric.sides);
}

string vector_to_string(vector<string> vec) {
    stringstream ss;
    for (size_t i = 0; i < vec.size(); i++) {
        if (i != 0) {
            ss << " ";
        }
        ss << vec[i];
    }
    return ss.str();
}

string MonsterTemplate::getName() {
    return name;
}

void MonsterTemplate::setName(string n) {
    name = n;
}

string MonsterTemplate::getDescription() {
    return description;
}

void MonsterTemplate::setDescription(string d) {
    description = d;
}

vector<string> MonsterTemplate::getColors() {
    return colors;
}

void MonsterTemplate::setColors(vector<string> c) {
    colors = c;
}

char MonsterTemplate::getSymbol() {
    return symbol;
}

void MonsterTemplate::setSymbol(char c) {
    symbol = c;
}

numeric_t MonsterTemplate::getSpeed() {
    return speed;
}

void MonsterTemplate::setSpeed(numeric_t s) {
    speed = s;
}

vector<string> MonsterTemplate::getAbilities() {
    return abilities;
}

void MonsterTemplate::setAbilities(vector<string> a) {
    abilities = a;
}

numeric_t MonsterTemplate::getHitpoints() {
    return hitpoints;
}

void MonsterTemplate::setHitpoints(numeric_t h) {
    hitpoints = h;
}

numeric_t MonsterTemplate::getAttackDamage() {
    return attack_damage;
}


void MonsterTemplate::setAttackDamage(numeric_t a) {
    attack_damage = a;
}

bool MonsterTemplate::isValid() {
    return !name.empty() && !description.empty() && colors.size() > 0 && speed.base >= 0 && speed.dice >= 0\
          && speed.sides >= 0 && abilities.size() > 0 && hitpoints.base >= 0\
          && hitpoints.dice >= 0 && hitpoints.sides >= 0 &&\
          attack_damage.base >= 0 && attack_damage.sides >= 0 && attack_damage.dice >= 0 \
          && symbol;
}

string MonsterTemplate::toString() {
    return name + "\n" + description + "\n" + symbol + "\n" + vector_to_string(colors)\
           + "\n" + numeric_to_string(speed) + "\n" + numeric_to_string(hitpoints)\
           + "\n" + numeric_to_string(attack_damage);
}

MonsterTemplate::MonsterTemplate() {
    name = "";
    description = "";
    colors.empty();
    speed.base = -1;
    speed.dice = -1;
    speed.sides = -1;
    abilities.empty();
    hitpoints.base = -1;
    hitpoints.dice = -1;
    hitpoints.sides = -1;
    attack_damage.base = -1;
    attack_damage.dice = -1;
    attack_damage.sides = -1;
}
