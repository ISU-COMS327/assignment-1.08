#include "priority_queue.h"

int PriorityQueue :: size() {
    return nodes.size();
}

void PriorityQueue :: insertWithPriority(struct Coordinate coord, int priority) {
    Node node;
    node.coord = coord;
    node.priority = priority;
    if (nodes.size() == 0) {
        nodes.push_back(node);
        return;
    }
    bool was_added = false;
    for (int i = 0; i < nodes.size(); i++) {
        Node existing_node = nodes[i];
        if (priority <= existing_node.priority) {
            nodes.insert(nodes.begin() + i, node);
            was_added = true;
            break;
        }
    }
    if (!was_added) {
        nodes.push_back(node);
    }
}

Node PriorityQueue :: extractMin() {
    Node min = nodes[0];
    nodes.erase(nodes.begin());
    return min;
}

void PriorityQueue :: decreasePriority(struct Coordinate coord, int priority) {
    Node node;
    node.coord = coord;
    node.priority = priority;
    bool coord_found = false;
    for (int i = 0; i < nodes.size(); i++) {
        Node existing_node = nodes[i];
        if (coord.x == existing_node.coord.x && coord.y == existing_node.coord.y) {
            nodes.erase(nodes.begin() + i);
            coord_found = true;
            break;
        }
    }
    if (!coord_found) {
        return;
    }
    insertWithPriority(coord, priority);
}
