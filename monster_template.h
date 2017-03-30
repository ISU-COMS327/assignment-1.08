#include <string>
#include <vector>

using namespace std;

typedef struct numeric {
    int base;
    int dice;
    int sides;
} numeric_t;

class MonsterTemplate {
    private:
        string name;
        string description;
        vector<string> colors;
        char symbol;
        numeric_t speed;
        vector<string> abilities;
        numeric_t hitpoints;
        numeric_t attack_damage;

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
        numeric_t getSpeed();
        void setSpeed(numeric_t);
        vector<string> getAbilities();
        void setAbilities(vector<string>);
        numeric_t getHitpoints();
        void setHitpoints(numeric_t);
        numeric_t getAttackDamage();
        void setAttackDamage(numeric_t);
        string toString();
        MonsterTemplate();
};
