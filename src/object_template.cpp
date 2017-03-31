#include "object_template.h"

/*
 *      string name;
        string description;
        string type;
        string color;
        Numeric * hit_bonus;
        Numeric * damage_bonus;
        Numeric * dodge_bonus;
        Numeric * defense_bonus;
        Numeric * weight;
        Numeric * speed_bonus;
        Numeric * special_attribute;
        Numeric * value;
 *
 */

bool ObjectTemplate :: isValid() {
    return name.size() > 0 && description.size() > 0 && type.size() > 0 &&\
        color.size() > 0 && hit_bonus->isValid() && damage_bonus->isValid() &&\
        dodge_bonus->isValid() && defense_bonus->isValid() && weight->isValid() &&\
        speed_bonus->isValid() && special_attribute->isValid() && value->isValid();
}

string ObjectTemplate :: toString() {
    return name + "\n" + description + "\n" + type + "\n" + color\
           + "\n" + weight->toString() + "\n" + dodge_bonus->toString()\
           + "\n" + damage_bonus->toString() + "\n" + defense_bonus->toString()\
           + "\n" + speed_bonus->toString() + "\n" + hit_bonus->toString()\
           + "\n" + special_attribute->toString() + "\n" + value->toString();

}
