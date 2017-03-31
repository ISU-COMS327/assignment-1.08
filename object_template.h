#include "numeric.h"

class ObjectTemplate {
    public:
        bool isValid();
        string name;
        string description;
        string type;
        string color;
        Numeric hit_bonus;
        Numeric damage_bonus;
        Numeric dodge_bonus;
        Numeric defense_bonus;
        Numeric weight;
        Numeric speed_bonus;
        Numeric attribute_bonus;
        Numeric value;
}
