#include <vector>

using namespace std;

struct Coordinate {
    int x;
    int y;
};

typedef struct {
    int distance;
    int priority;
    struct Coordinate coord;
} Node;

class PriorityQueue {
    private:
        vector<Node> nodes;

    public:
        int size();
        void insertWithPriority(struct Coordinate, int);
        void decreasePriority(struct Coordinate, int);
        Node extractMin();
};
