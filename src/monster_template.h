#include <string>
#include <vector>
#include "numeric.h"

using namespace std;

class MonsterTemplate {
    private:
        string name;
        string description;
        vector<string> colors;
        char symbol;
        Numeric * speed;
        vector<string> abilities;
        Numeric * hitpoints;
        Numeric * attack_damage;

    public:
        bool isValid();
        string getName();
        void setName(string);
        string getDescription();
        void setDescription(string);
        vector<string> getColors();
        void setColors(vector<string>);
        char getSymbol();
        void setSymbol(char c);
        Numeric * getSpeed();
        void setSpeed(Numeric *);
        vector<string> getAbilities();
        void setAbilities(vector<string>);
        Numeric * getHitpoints();
        void setHitpoints(Numeric *);
        Numeric * getAttackDamage();
        void setAttackDamage(Numeric *);
        string toString();
        MonsterTemplate();
};
