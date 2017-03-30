#include <string>
#include <vector>
#include <string>

using namespace std;

class Numeric {
    public:
        int base;
        int dice;
        int sides;
        string toString();
        bool isValid();
        Numeric(string);
        Numeric();
};

