#include <vector>

using namespace std;

struct Coordinate {
    uint8_t x;
    uint8_t y;
};

typedef struct {
    int distance;
    int priority;
    struct Coordinate coord;
} Node;

class PriorityQueue {
    private:
        std::vector<Node> nodes;

    public:
        int size();
        void insertWithPriority(struct Coordinate, int);
        void decreasePriority(struct Coordinate, int);
        Node extractMin();
};
