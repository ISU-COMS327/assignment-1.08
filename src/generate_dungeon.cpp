#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <math.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <fstream>
#include <exception>
#include <map>

#include "util.h"
#include "monster.h"
#include "object.h"
#include "monster_description_parser.h"
#include "monster_template.h"
#include "object_description_parser.h"

#include "priority_queue.h"

#define HEIGHT 105
#define WIDTH 160
#define NCURSES_HEIGHT 20
#define NCURSES_WIDTH 80
#define IMMUTABLE_ROCK 255
#define ROCK 200
#define ROOM 0
#define CORRIDOR 0
#define MIN_NUMBER_OF_ROOMS 10
#define MAX_NUMBER_OF_ROOMS 50
#define MIN_ROOM_WIDTH 7
#define DEFAULT_MAX_ROOM_WIDTH 15
#define MIN_ROOM_HEIGHT 5
#define DEFAULT_MAX_ROOM_HEIGHT 10
#define DEFAULT_NUMBER_OF_MONSTERS 5
using namespace std;

static string TYPE_ROOM = "room";
static string TYPE_CORRIDOR = "corridor";
static string TYPE_ROCK = "rock";
static string TYPE_UPSTAIR = "upstair";
static string TYPE_DOWNSTAIR = "downstair";

struct Available_Coords {
    struct Coordinate * coords;
    int length;
};

typedef struct {
    int tunneling_distance;
    int non_tunneling_distance;
    int hardness;
    string type;
    uint8_t x;
    uint8_t y;
    uint8_t has_player;
    Monster *monster;
    Object * object;
} Board_Cell;

typedef struct {
    Board_Cell * cells;
    int length;
} Neighbors;

struct Room {
    uint8_t start_x;
    uint8_t end_x;
    uint8_t start_y;
    uint8_t end_y;
};

static Board_Cell board[HEIGHT][WIDTH];
static Board_Cell player_board[HEIGHT][WIDTH];
static struct Coordinate placeable_areas[HEIGHT * WIDTH];
static struct Coordinate ncurses_player_coord;
static struct Coordinate ncurses_start_coord;
static vector<struct Room> rooms;
static vector<Monster *> monsters;
static vector<MonsterTemplate> monster_templates;
static vector<Object *> objects;
static vector<ObjectTemplate> object_templates;
static struct Coordinate player;
static PriorityQueue * game_queue;
static map<string, int> color_map;

string RLG_DIRECTORY = "";
int IS_CONTROL_MODE = 1;
int DO_QUIT = 0;
int PLAYER_IS_ALIVE = 1;
int DO_SAVE = 0;
int DO_LOAD = 0;
int SHOW_HELP = 0;
int NUMBER_OF_ROOMS = MIN_NUMBER_OF_ROOMS;
int MAX_ROOM_WIDTH = DEFAULT_MAX_ROOM_WIDTH;
int MAX_ROOM_HEIGHT = DEFAULT_MAX_ROOM_HEIGHT;
int NUMBER_OF_MONSTERS = DEFAULT_NUMBER_OF_MONSTERS;
int NUMBER_OF_PLACEABLE_AREAS = 0;

int max(int x, int y) {
    if (x > y) {
        return x;
    }
    return y;
}
int min(int x, int y) {
    if (x < y) {
        return x;
    }
    return y;
}

void init_color_pairs();
void generate_monsters_from_templates();
void generate_objects_from_templates();
void print_usage();
struct Coordinate get_random_board_location();
void make_rlg_directory();
void make_monster_templates();
void make_object_templates();
void update_number_of_rooms();
void generate_new_board();
void generate_stairs();
void initialize_board();
void initialize_immutable_rock();
void load_board();
void save_board();
void place_player();
void set_placeable_areas();
void set_tunneling_distance_to_player();
void set_non_tunneling_distance_to_player();
void generate_monsters();
void print_non_tunneling_board();
void print_tunneling_board();
void add_message(string message);
void center_board_on_player();
int handle_user_input(int key);
void handle_user_input_for_look_mode(int key);
void print_board();
void print_cell(Board_Cell cell);
void dig_rooms(int number_of_rooms_to_dig);
void dig_room(int index);
int room_is_valid(struct Room room);
void add_rooms_to_board();
void dig_cooridors();
void connect_rooms_at_indexes(int index1, int index2);
void move_player();
struct Room get_room_player_is_in();
void move_monster(Monster * monster);
void kill_player_or_monster_at(struct Coordinate coord);

int main(int argc, char *args[]) {
    game_queue = new PriorityQueue();
    int player_x = -1;
    int player_y = -1;
    struct option longopts[] = {
        {"save", no_argument, &DO_SAVE, 1},
        {"load", no_argument, &DO_LOAD, 1},
        {"rooms", required_argument, 0, 'r'},
        {"nummon", required_argument, 0, 'm'},
        {"player_x", required_argument, 0, 'x'},
        {"player_y", required_argument, 0, 'y'},
        {"help", no_argument, &SHOW_HELP, 'h'},
        {0, 0, 0, 0}
    };
    int c;
    while((c = getopt_long(argc, args, "h:", longopts, NULL)) != -1) {
        switch(c) {
            case 'r':
                NUMBER_OF_ROOMS = atoi(optarg);
                break;
            case 'm':
                NUMBER_OF_MONSTERS =  atoi(optarg);
                if (NUMBER_OF_MONSTERS < 1) {
                    NUMBER_OF_MONSTERS = DEFAULT_NUMBER_OF_MONSTERS;
                    printf("Number of monsters cannot be less than 1\n");
                }
                break;
            case 'x':
                player_x = atoi(optarg);
                break;
            case 'y':
                player_y = atoi(optarg);
                break;
            case 'h':
                SHOW_HELP = 1;
                break;
            default:
                break;
        }
    }
    if (SHOW_HELP) {
        print_usage();
        exit(0);
    }
    if ((player_x != -1 || player_y != -1) && ((player_x <= 0 || player_x > WIDTH - 1) || (player_y <= 0 || player_y > HEIGHT - 1))) {
        printf("Invalid player coordinates. Note: both player_x and player_y must be provided as inputs\n");
        print_usage();
        exit(0);
    }
    if (player_y == -1) {
        player_y = 0;
    }
    if (player_x == -1) {
        player_x = 0;
    }
    cout << "here2!";
    make_rlg_directory();
    make_monster_templates();
    make_object_templates();
    player.x = player_x;
    player.y = player_y;
    update_number_of_rooms();
    generate_new_board();
    initscr();
    noecho();
    start_color();
    init_color_pairs();
    center_board_on_player();
    move(ncurses_player_coord.y, ncurses_player_coord.x);
    refresh();
    while(monsters.size() > 0 && PLAYER_IS_ALIVE && !DO_QUIT) {
        center_board_on_player();
        refresh();
        Node min = game_queue->extractMin();
        int speed;
        if (min.coord.x == player.x && min.coord.y == player.y) {
            add_message("It's your turn");
            speed = 10;
            int success = 0;
            while (!success) {
                int ch = getch();
                success = handle_user_input(ch);
                while (!IS_CONTROL_MODE && !DO_QUIT) {
                    success = 0;
                    int ch = getch();
                    handle_user_input_for_look_mode(ch);
                    if (DO_QUIT) {
                        success = 1;
                    }
                }
            }
            if (DO_QUIT) {
                break;
            }
            center_board_on_player();
            refresh();
            if (success == 2) {
                continue;
            }
            min.coord.x = player.x;
            min.coord.y = player.y;
            set_non_tunneling_distance_to_player();
            set_tunneling_distance_to_player();
        }
        else {
            Monster * monster = board[min.coord.y][min.coord.x].monster;
            if (monster == NULL) {
                continue;
            }
            move_monster(monster);

            speed = monster->speed;
            min.coord.x = monster->x;
            min.coord.y = monster->y;
        }
        if (PLAYER_IS_ALIVE) {
            move(ncurses_player_coord.y, ncurses_player_coord.x);
        }
        else {
            curs_set(0);
        }
        center_board_on_player();
        refresh();
        game_queue->insertWithPriority(min.coord, (1000/speed) + min.priority);
    }

    if (!PLAYER_IS_ALIVE) {
        add_message("You lost. The monsters killed you (press any key to exit)");
    }
    else if(!monsters.size()) {
        add_message("You won, killing all the monsters (press any key to exit)");
    }

    if (DO_SAVE) {
        save_board();
    }

    if (!DO_QUIT) {
        getch();
    }
    endwin();

    monsters.clear();
    objects.clear();
    monster_templates.clear();
    object_templates.clear();
    delete game_queue;
    free(board);
    free(player_board);

    return 0;
}

void init_color_pairs() {
    color_map["RED"] = COLOR_RED;
    color_map["GREEN"] = COLOR_GREEN;
    color_map["BLUE"] = COLOR_BLUE;
    color_map["CYAN"] = COLOR_CYAN;
    color_map["YELLOW"] = COLOR_YELLOW;
    color_map["MAGENTA"] = COLOR_MAGENTA;
    color_map["WHITE"] = COLOR_WHITE;
    color_map["BLACK"] = COLOR_BLACK;

    map<string, int>::iterator it;
    for (it = color_map.begin(); it != color_map.end(); it++) {
        int color_key = it->second;
        init_pair(color_key, color_key, COLOR_BLACK);
    }
}

void generate_monsters_from_templates() {
    monsters.clear();
    while (monsters.size() < NUMBER_OF_MONSTERS) {
        int i = random_int(0, monster_templates.size() - 1);
        struct Coordinate coordinate;
        MonsterTemplate monster_template = monster_templates[i];
        Monster * monster = monster_template.makeMonster();
        while (true) {
            coordinate = get_random_board_location();
            Board_Cell cell = board[coordinate.y][coordinate.x];
            if (cell.monster || player.x == coordinate.x || player.y == coordinate.y) {
                continue;
            }
            else {
                break;
            }
        }
        monster->x = coordinate.x;
        monster->y = coordinate.y;
        board[monster->y][monster->x].monster = monster;
        monsters.push_back(monster);
        game_queue->insertWithPriority(coordinate, monsters.size());
    }

}

void generate_objects_from_templates() {
    objects.clear();
    int number_of_objects = random_int(20, 40);
    while(objects.size() < number_of_objects) {
        int i = random_int(0, object_templates.size() - 1);
        struct Coordinate coordinate;
        ObjectTemplate object_template = object_templates[i];
        Object * object = object_template.makeObject();
        while(true) {
            coordinate = get_random_board_location();
            Board_Cell cell = board[coordinate.y][coordinate.x];
            if (!cell.object) {
                break;
            }
        }
        object->x = coordinate.x;
        object->y = coordinate.y;
        board[object->y][object->x].object = object;
        objects.push_back(object);
    }
}

void make_monster_templates() {
    string filename = RLG_DIRECTORY + "monster_desc.txt";
    MonsterDescriptionParser * p = new MonsterDescriptionParser(filename);
    try {
        p->parseFile();
    }
    catch(const char * e) {
        cout << "Error reading monster file: " << e << "\nExiting program" <<endl;
        exit(1);
    }
    monster_templates = p->getMonsterTemplates();
    delete p;
}

void make_object_templates() {
    string filename = RLG_DIRECTORY + "object_desc.txt";
    ObjectDescriptionParser * p = new ObjectDescriptionParser(filename);
    try{
        p->parseFile();
    }
    catch(const char * e) {
        cout << "Error reading object file: " << e << "\nExiting program" << endl;
        exit(2);
    }
    object_templates = p->getObjectTemplates();
    delete p;
}

void update_number_of_rooms() {
    if (NUMBER_OF_ROOMS < MIN_NUMBER_OF_ROOMS) {
        printf("Minimum number of rooms is %d\n", MIN_NUMBER_OF_ROOMS);
        NUMBER_OF_ROOMS = MIN_NUMBER_OF_ROOMS;
    }
    if (NUMBER_OF_ROOMS > MAX_NUMBER_OF_ROOMS) {
        printf("Maximum number of rooms is %d\n", MAX_NUMBER_OF_ROOMS);
        NUMBER_OF_ROOMS = MAX_NUMBER_OF_ROOMS;
    }
}

void generate_new_board() {
    initialize_board();
    rooms.empty();
    if (DO_LOAD) {
        load_board();
        DO_LOAD = 0;
    }
    else {
        dig_rooms(NUMBER_OF_ROOMS);
        dig_cooridors();
    }
    if (game_queue->size() > 0) {
        delete game_queue;
    }
    game_queue = new PriorityQueue();
    place_player();
    set_placeable_areas();
    set_non_tunneling_distance_to_player();
    set_tunneling_distance_to_player();
    generate_monsters_from_templates();
    generate_stairs();
    generate_objects_from_templates();
}

struct Coordinate get_random_unoccupied_location_in_room(struct Room room) {
    struct Available_Coords available_coords;
    available_coords.length = 0;
    available_coords.coords = (struct Coordinate *)  malloc(sizeof(struct Coordinate) * (room.end_y - room.start_y) * (room.end_x - room.start_x));
    for (int y = room.start_y; y < room.end_y; y++) {
        for (int x = room.start_x; x < room.end_x; x++) {
            Board_Cell cell =  board[y][x];
            if ((y != player.y || x != player.x) && !cell.monster) {
                struct Coordinate coord;
                coord.y = y;
                coord.x = x;
                available_coords.coords[available_coords.length]= coord;
                available_coords.length ++;
            }
        }
    }
    int index = random_int(0, available_coords.length);
    return available_coords.coords[index];
}

void generate_stairs() {
    int number_of_stairs_up = rooms.size() / 2;
    for (int i = 0; i < number_of_stairs_up; i++) {
        struct Room room = rooms[i];
        struct Coordinate coord = get_random_unoccupied_location_in_room(room);
        board[coord.y][coord.x].type = TYPE_UPSTAIR;
    }
    for (int i = number_of_stairs_up; i < NUMBER_OF_ROOMS; i++) {
        struct Room room = rooms[i];
        struct Coordinate coord = get_random_unoccupied_location_in_room(room);
        board[coord.y][coord.x].type = TYPE_DOWNSTAIR;
    }
}

void make_rlg_directory() {
    char * home = getenv("HOME");
    char dir[] = "/.rlg327/";
    RLG_DIRECTORY.append(home);
    RLG_DIRECTORY.append(dir);
    mkdir(RLG_DIRECTORY.c_str(), 0777);
}

void save_board() {
    string filename = "dungeon";
    string filepath = RLG_DIRECTORY + filename;
    cout << "Saving file to: " << filepath << endl;
    FILE * fp = fopen(filepath.c_str(), "wb+");
    if (fp == NULL) {
        cout << "Cannot save file\n";
        return;
    }
    string file_marker = "RLG327-S2017";
    uint32_t version = htonl(0);
    uint32_t file_size = htonl(16820 + (rooms.size() * 4));

    fwrite(file_marker.c_str(), 1, file_marker.length(), fp);
    fwrite(&version, 1, 4, fp);
    fwrite(&file_size, 1, 4, fp);
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            uint8_t num = board[y][x].hardness;
            fwrite(&num, 1, 1, fp);
        }
    }

    for (int i = 0; i < rooms.size(); i++) {
        struct Room room = rooms[i];
        uint8_t height = room.end_y - room.start_y + 1;
        uint8_t width = room.end_x - room.start_x + 1;
        fwrite(&room.start_x, 1, 1, fp);
        fwrite(&room.start_y, 1, 1, fp);
        fwrite(&(width), 1, 1, fp);
        fwrite(&(height), 1, 1, fp);
    }
    fclose(fp);
}

void load_board() {
    string filename = "dungeon";
    string filepath = RLG_DIRECTORY + filename;
    cout << "Loading dungeon: " << filepath << endl;
    FILE *fp = fopen(filepath.c_str(), "r");
    if (fp == NULL) {
        cout << "Cannot load " << filepath << endl;
        exit(1);
    }
    char title[13]; // one extra index for the null value at the end
    uint32_t version;
    uint32_t file_size;


    // Get title
    fread(title, 1, 12, fp);

    // Get version
    fread(&version, 4, 1, fp);
    version = ntohl(version);

    // Get file size
    fread(&file_size, 4, 1, fp);
    file_size = ntohl(file_size);

    printf("File Marker: %s :: Version: %d :: File Size: %d bytes\n", title, version, file_size);

    uint8_t num;
    int x = 0;
    int y = 0;
    for (int i = 0; i < 16800; i++) {
        fread(&num, 1, 1, fp);
        Board_Cell cell;
        cell.hardness = num;
        cell.has_player = 0;
        if (num == 0) {
            cell.type = TYPE_CORRIDOR;
        }
        else {
            cell.type = TYPE_ROCK;
        }
        cell.x = x;
        cell.y = y;
        board[y][x] = cell;
        board[y][x].monster = NULL;
        if (x == WIDTH - 1) {
            x = 0;
            y ++;
        }
        else {
            x ++;
        }
    }

    uint8_t start_x;
    uint8_t start_y;
    uint8_t width;
    uint8_t height;
    NUMBER_OF_ROOMS = (file_size - ftell(fp)) / 4;
    int counter = 0;
    while(ftell(fp) != file_size) {
        fread(&start_x, 1, 1, fp);
        fread(&start_y, 1, 1, fp);
        fread(&width, 1, 1, fp);
        fread(&height, 1, 1, fp);

        struct Room room;
        room.start_x = start_x;
        room.start_y = start_y;
        room.end_x = start_x + width - 1;
        room.end_y = start_y + height - 1;
        rooms.push_back(room);
        counter ++;
    }
    add_rooms_to_board();
    fclose(fp);
}

void print_usage() {
    printf("usage: generate_dungeon [--save] [--load] [--rooms=<number of rooms>] [--player_x=<player x position>] [--player_y=<player y position>] [--nummon=<number of monsters>]\n");
}

void initialize_board() {
    Board_Cell cell;
    cell.type = TYPE_ROCK;
    cell.monster = NULL;
    cell.object = NULL;
    cell.has_player = 0;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            cell.x = x;
            cell.y = y;
            cell.hardness = random_int(1, 254);
            board[y][x] = cell;
            cell.hardness = IMMUTABLE_ROCK;
            player_board[y][x] = cell;
        }
    }
    initialize_immutable_rock();
}

void initialize_immutable_rock() {
    int y;
    int x;
    int max_x = WIDTH - 1;
    int max_y = HEIGHT - 1;
    Board_Cell cell;
    cell.type = TYPE_ROCK;
    cell.hardness = IMMUTABLE_ROCK;
    cell.monster = NULL;
    cell.object = NULL;
    cell.has_player = 0;
    for (y = 0; y < HEIGHT; y++) {
        cell.y = y;
        cell.x = 0;
        board[y][0] = cell;
        cell.x = max_x;
        board[y][max_x] = cell;
    }
    for (x = 0; x < WIDTH; x++) {
        cell.y = 0;
        cell.x = x;
        board[0][x] = cell;
        cell.y = max_y;
        board[max_y][x] = cell;
    }
}

void place_player() {
    if (!player.x && !player.y) {
        struct Room room = rooms[0];
        int x = random_int(room.start_x, room.end_x);
        int y = random_int(room.start_y, room.end_y);
        player.x = x;
        player.y = y;
    }
    struct Coordinate coord;
    coord.x = player.x;
    coord.y = player.y;
    game_queue->insertWithPriority(coord, 0);
}

void set_placeable_areas() {
    NUMBER_OF_PLACEABLE_AREAS = 0;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            Board_Cell cell = board[y][x];
            if (cell.hardness == 0 && cell.x != player.x && cell.y != player.y) {
                struct Coordinate coord;
                coord.x = cell.x;
                coord.y = cell.y;
                placeable_areas[NUMBER_OF_PLACEABLE_AREAS] = coord;
                NUMBER_OF_PLACEABLE_AREAS++;
            }
        }
    }
}

int get_cell_weight(Board_Cell cell) {
    if (cell.hardness == 0) {
        return 1;
    }
    if (cell.hardness <= 84) {
        return 1;
    }
    if (cell.hardness <= 170) {
        return 2;
    }
    if (cell.hardness <= 254) {
        return 3;
    }
    return 1000;
}

int should_add_tunneling_neighbor(Board_Cell cell) {
    return cell.hardness < IMMUTABLE_ROCK;
}

void add_tunneling_neighbor(Neighbors * neighbors, Board_Cell cell) {
    if (!should_add_tunneling_neighbor(cell)) {
        return;
    }
    neighbors->cells[neighbors->length] = cell;
    neighbors->length ++;
}


vector<Board_Cell> get_tunneling_neighbors(struct Coordinate coord) {
    vector<Board_Cell> neighbors;
    int can_go_right = coord.x < WIDTH -1;
    int can_go_up = coord.y > 0;
    int can_go_left = coord.x > 0;
    int can_go_down = coord.y < HEIGHT -1;

    if (can_go_right) {
        Board_Cell right = board[coord.y][coord.x + 1];
        if (right.hardness < IMMUTABLE_ROCK) {
            neighbors.push_back(right);
        }
        if (can_go_up) {
            Board_Cell top_right = board[coord.y - 1][coord.x + 1];
            if (top_right.hardness < IMMUTABLE_ROCK) {
                neighbors.push_back(top_right);
            }
        }
        if (can_go_down) {
            Board_Cell bottom_right = board[coord.y + 1][coord.x + 1];
            if (bottom_right.hardness < IMMUTABLE_ROCK) {
                neighbors.push_back(bottom_right);
            }
        }
    }
    if (can_go_left) {
        Board_Cell left = board[coord.y][coord.x - 1];
        if (left.hardness < IMMUTABLE_ROCK) {
            neighbors.push_back(left);
        }
        if (can_go_up) {
            Board_Cell top_left = board[coord.y - 1][coord.x - 1];
            if (top_left.hardness < IMMUTABLE_ROCK) {
                neighbors.push_back(top_left);
            }
        }
        if (can_go_down) {
            Board_Cell bottom_left = board[coord.y + 1][coord.x - 1];
            if (bottom_left.hardness < IMMUTABLE_ROCK) {
                neighbors.push_back(bottom_left);
            }
        }
    }

    if (can_go_up) {
        Board_Cell above = board[coord.y - 1][coord.x];
        if (above.hardness < IMMUTABLE_ROCK) {
            neighbors.push_back(above);
        }
    }
    if (can_go_down) {
        Board_Cell below = board[coord.y + 1][coord.x];
        if (below.hardness < IMMUTABLE_ROCK) {
            neighbors.push_back(below);
        }
    }

    return neighbors;
}


void set_tunneling_distance_to_player() {
    PriorityQueue * tunneling_queue = new PriorityQueue();
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            struct Coordinate coord;
            coord.x = x;
            coord.y = y;
            if (y == player.y && x == player.x) {
                board[y][x].tunneling_distance = 0;
            }
            else {
                board[y][x].tunneling_distance = INT_MAX;
            }
            if (board[y][x].hardness < IMMUTABLE_ROCK) {
                tunneling_queue->insertWithPriority(coord, board[y][x].tunneling_distance);
            }
        }
    }
    int count = 0;
    while(tunneling_queue->size()) {
        Node min = tunneling_queue->extractMin();
        Board_Cell min_cell = board[min.coord.y][min.coord.x];
        vector<Board_Cell> neighbors = get_tunneling_neighbors(min.coord);
        int min_dist = min_cell.tunneling_distance + get_cell_weight(min_cell);
        for (int i = 0; i < neighbors.size(); i++) {
            Board_Cell neighbor_cell = neighbors[i];
            Board_Cell cell = board[neighbor_cell.y][neighbor_cell.x];
            if (min_dist < cell.tunneling_distance) {
                struct Coordinate coord;
                coord.x = cell.x;
                coord.y = cell.y;
                board[cell.y][cell.x].tunneling_distance = min_dist;
                tunneling_queue->decreasePriority(coord, min_dist);
            }
        }
        count ++;
        neighbors.empty();
    }
    delete tunneling_queue;
};

vector<Board_Cell> get_non_tunneling_neighbors(struct Coordinate coord) {
    vector<Board_Cell> neighbors;
    int can_go_right = coord.x < WIDTH -1;
    int can_go_up = coord.y > 0;
    int can_go_left = coord.x > 0;
    int can_go_down = coord.y < HEIGHT -1;
    if (can_go_right) {
        Board_Cell right = board[coord.y][coord.x + 1];
        if (right.hardness < 1) {
            neighbors.push_back(right);
        }
        if (can_go_up) {
            Board_Cell top_right = board[coord.y - 1][coord.x + 1];
            if (top_right.hardness < 1) {
                neighbors.push_back(top_right);
            }
        }
        if (can_go_down) {
            Board_Cell bottom_right = board[coord.y + 1][coord.x + 1];
            if (bottom_right.hardness < 1) {
                neighbors.push_back(bottom_right);
            }
        }
    }

    if (can_go_left) {
        Board_Cell left = board[coord.y][coord.x - 1];
        if (left.hardness < 1) {
            neighbors.push_back(left);
        }
        if (can_go_up) {
            Board_Cell top_left = board[coord.y - 1][coord.x - 1];
            if (top_left.hardness < 1) {
                neighbors.push_back(top_left);
            }
        }
        if (can_go_down) {
            Board_Cell bottom_left = board[coord.y + 1][coord.x - 1];
            if (bottom_left.hardness < 1) {
                neighbors.push_back(bottom_left);
            }
        }
    }

    if (can_go_up) {
        Board_Cell above = board[coord.y - 1][coord.x];
        if (above.hardness < 1) {
            neighbors.push_back(above);
        }
    }
    if (can_go_down) {
        Board_Cell below = board[coord.y + 1][coord.x];
        if (below.hardness < 1) {
            neighbors.push_back(below);
        }
    }
    return neighbors;
}

void set_non_tunneling_distance_to_player() {
    PriorityQueue * non_tunneling_queue = new PriorityQueue();
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            struct Coordinate coord;
            coord.x = x;
            coord.y = y;
            if (y == player.y && x == player.x) {
                board[y][x].non_tunneling_distance = 0;
            }
            else {
                board[y][x].non_tunneling_distance = INT_MAX;
            }
            if (board[y][x].hardness < 1) {
                non_tunneling_queue->insertWithPriority(coord, board[y][x].non_tunneling_distance);
            }
        }
    }
    while(non_tunneling_queue->size()) {
        Node min = non_tunneling_queue->extractMin();
        Board_Cell min_cell = board[min.coord.y][min.coord.x];
        vector<Board_Cell> neighbors = get_non_tunneling_neighbors(min.coord);
        int min_dist = min_cell.non_tunneling_distance + 1;
        for (int i = 0; i < neighbors.size(); i++) {
            Board_Cell neighbor_cell = neighbors[i];
            Board_Cell cell = board[neighbor_cell.y][neighbor_cell.x];
            if (min_dist < cell.non_tunneling_distance) {
                struct Coordinate coord;
                coord.x = cell.x;
                coord.y = cell.y;
                board[cell.y][cell.x].non_tunneling_distance = min_dist;
                non_tunneling_queue->decreasePriority(coord, min_dist);
            }
        }
        neighbors.empty();
    }
    delete non_tunneling_queue;
}

struct Coordinate get_random_board_location() {
    int index = random_int(0, NUMBER_OF_PLACEABLE_AREAS);
    return placeable_areas[index];
}

void generate_monsters() {
    /*
    monsters.empty();
    monsters.resize(NUMBER_OF_MONSTERS);
    struct Coordinate last_known_player_location;
    last_known_player_location.x = 0;
    last_known_player_location.y = 0;
    for (int i = 0; i < NUMBER_OF_MONSTERS; i++) {
        Monster * m = new Monster();
        struct Coordinate coordinate;
        while (1) {
            coordinate = get_random_board_location();
            int is_new = 1;
            for (int j = 0; j < i; j++) {
                int old_x = monsters[j].x;
                int old_y = monsters[j].y;
                if (old_x == coordinate.x && old_y == coordinate.y && old_x == player.x && old_y == player.y) {
                    is_new = 0;
                    break;
                }
            }
            if (is_new) {
                break;
            }
        }
        m->speed = random_int(5, 20);
        m->x = coordinate.x;
        m->setY(coordinate.y);
        m->setLastKnownPlayerX(last_known_player_location.x);
        m->setLastKnownPlayerY(last_known_player_location.y);
        m->setDecimalType(random_int(0, 15));
        board[coordinate.y][coordinate.x].monster = m;
        monsters[i] = * m;
        insert_with_priority(game_queue, coordinate, i + 1);
    }
    */
}

void print_non_tunneling_board() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
           Board_Cell cell = board[y][x];
           if(cell.x == player.x && cell.y == player.y) {
               printf("@");
           }
           else {
               if (strcmp(cell.type.c_str(), TYPE_ROCK.c_str()) != 0) {
                   printf("%d", cell.non_tunneling_distance % 10);
               }
               else {
                    printf(" ");
               }
           }
        }
        printf("\n");
    }
}
void print_tunneling_board() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
           Board_Cell cell = board[y][x];
           if(cell.x == player.x && cell.y == player.y) {
               printf("@");
           }
           else {
               if (cell.hardness == IMMUTABLE_ROCK) {
                   printf(" ");
               }
               else {
                   printf("%d", cell.tunneling_distance % 10);
               }
           }
        }
        printf("\n");
    }

}

void add_message(string message) {
    move(0,0);
    clrtoeol();
    mvprintw(0, 0, "%s", message.c_str());
    move(ncurses_player_coord.y, ncurses_player_coord.x);
    refresh();
}

void update_player_board() {
    for (int y = player.y - 5; y <= player.y + 5; y++) {
        for (int x = player.x - 5; x <= player.x + 5; x++) {
            player_board[y][x] = board[y][x];
        }
    }
}


void update_board_view(int ncurses_start_x, int ncurses_start_y) {
    update_player_board();
    ncurses_start_x = min(ncurses_start_x + NCURSES_WIDTH, WIDTH - 1);
    ncurses_start_y = min(ncurses_start_y + NCURSES_HEIGHT, HEIGHT - 1);
    ncurses_start_x = max(ncurses_start_x - NCURSES_WIDTH, 0);
    ncurses_start_y = max(ncurses_start_y - NCURSES_HEIGHT, 0);
    ncurses_start_coord.x = ncurses_start_x;
    ncurses_start_coord.y = ncurses_start_y;
    int row = 0;
    for (int y = ncurses_start_y; y <= ncurses_start_y + NCURSES_HEIGHT; y++) {
        int col = 0;
        for (int x = ncurses_start_x; x <= ncurses_start_x + NCURSES_WIDTH; x++) {
            if (PLAYER_IS_ALIVE && y == player.y && x == player.x) {
                mvprintw(row, col, "@");
                ncurses_player_coord.x = col;
                ncurses_player_coord.y = row;
            }
            else if (player_board[y][x].monster) {
                Monster *monster = player_board[y][x].monster;
                int color_key = color_map[monster->color];
                attron(COLOR_PAIR(color_key));
                mvprintw(row, col, "%c", monster->symbol);
                attroff(COLOR_PAIR(color_key));
            }
            else if(player_board[y][x].object) {
                Object * object = player_board[y][x].object;
                int color_key = color_map[object->color];
                attron(COLOR_PAIR(color_key));
                mvprintw(row, col,"%c", object->getSymbol());
                attroff(COLOR_PAIR(color_key));
            }
            else {
                Board_Cell cell = player_board[y][x];
                if (strcmp(cell.type.c_str(), TYPE_UPSTAIR.c_str()) == 0) {
                    mvprintw(row, col, "<");
                }
                else if (strcmp(cell.type.c_str(), TYPE_DOWNSTAIR.c_str()) == 0) {
                    mvprintw(row, col, ">");
                }
                else if (strcmp(cell.type.c_str(), TYPE_ROCK.c_str()) == 0) {
                    mvprintw(row, col, " ");
                }
                else if (strcmp(cell.type.c_str(), TYPE_ROOM.c_str()) == 0) {
                    mvprintw(row, col, ".");
                }
                else if (strcmp(cell.type.c_str(), TYPE_CORRIDOR.c_str()) == 0) {
                    mvprintw(row, col, "#");
                }
                else {
                    mvprintw(row, col, "F");
                }
            }
            col ++;
        }
        row ++;
    }
}

void handle_user_input_for_look_mode(int key) {
    int new_x = ncurses_start_coord.x;
    int new_y = ncurses_start_coord.y;
    if(key == 107 || key == 8) { // k - one page up
        new_y -= NCURSES_HEIGHT;
    }
    else if (key == 106 || key == 2) { // j - one page down
        new_y += NCURSES_HEIGHT;
    }
    else if (key == 104 || key == 4) { // h - one page left
        new_x -= NCURSES_WIDTH;
    }
    else if(key == 108 || key == 6) { // l - one page right
        new_x += NCURSES_WIDTH;
    }
    else if (key == 27) { // escape - enter control mode
        IS_CONTROL_MODE = 1;
        center_board_on_player();
        add_message("It's your turn");
        return;
    }
    else if (key == 81) { // Q - quit
        DO_QUIT = 1;
    }
    update_board_view(new_x, new_y);
    refresh();
}

int handle_user_input(int key) {
    struct Coordinate new_coord;
    new_coord.x = player.x;
    new_coord.y = player.y;
    string str = "";
    if (key == 107 || key == 8) { // k - one cell up
        if (board[player.y - 1][player.x].hardness > 0) {
           return 0;
        }
        new_coord.y = player.y - 1;
    }
    else if (key == 106 || key == 2) { // j - one cell down
        if (board[player.y + 1][player.x].hardness > 0) {
            return 0;
        }
        new_coord.y = player.y + 1;
    }
    else if (key == 104 || key == 4) { // h - one cell left
        if (board[player.y][player.x - 1].hardness > 0) {
            return 0;
        }
        new_coord.x = player.x - 1;
    }
    else if(key == 108 || key == 6) { // l - one cell right
        if (board[player.y][player.x + 1].hardness > 0) {
            return 0;
        }
        new_coord.x = player.x + 1;
    }
    else if (key == 121 || key == 7) { // y - one cell up-left
        if (board[player.y - 1][player.x - 1].hardness > 0) {
            return 0;
        }
        new_coord.x = player.x - 1;
        new_coord.y = player.y - 1;
    }
    else if (key == 117 || key == 9) { // u - one cell up-right
        if (board[player.y - 1][player.x + 1].hardness > 0) {
            return 0;
        }
        new_coord.x = player.x + 1;
        new_coord.y = player.y - 1;
    }
    else if (key == 110 || key == 3) { // n - one cell low-right
        if (board[player.y + 1][player.x + 1].hardness > 0) {
            return 0;
        }
        new_coord.x = player.x + 1;
        new_coord.y = player.y + 1;
    }
    else if (key == 98 || key == 1) { // b - one cell low-left
        if (board[player.y + 1][player.x - 1].hardness > 0) {
            return 0;
        }
        new_coord.x = player.x - 1;
        new_coord.y = player.y + 1;
    }
    else if (key == 60 && IS_CONTROL_MODE) {  // upstairs
        if (strcmp(board[player.y][player.x].type.c_str(), TYPE_UPSTAIR.c_str()) != 0) {
           return 0;
        }
        add_message("You travel upstairs");
        player.x = 0;
        player.y = 0;
        generate_new_board();
        return 2;
    }
    else if (key == 62) {  // downstairs
        if (strcmp(board[player.y][player.x].type.c_str(), TYPE_DOWNSTAIR.c_str()) != 0) {
            return 0;
        }
        add_message("You travel downstairs");
        player.y = 0;
        player.x = 0;
        generate_new_board();
        return 2;
    }
    else if (key == 32 || key == 5) { // space - rest
        // you rest
        add_message("You rest");
    }
    else if (key == 76 && IS_CONTROL_MODE) { // L - enter look mode
        add_message("Entering look mode");
        IS_CONTROL_MODE = 0;
    }
    else if (key == 81) { // Q - quit
        DO_QUIT = 1;
    }
    else {
        char ascii = key;
        string message = "'";
        message += ascii;
        message += "' is not supported";
        add_message(message);
        return 0;
    }
    if (new_coord.x != player.x || new_coord.y != player.y) {
        kill_player_or_monster_at(new_coord);
        player.x = new_coord.x;
        player.y = new_coord.y;
    }
    update_player_board();
    return 1;
}

void center_board_on_player() {
    int new_y = player.y - 10;
    int new_x = player.x - 40;
    update_board_view(new_x, new_y);
    move(ncurses_player_coord.y, ncurses_player_coord.x);
}

void print_board() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (PLAYER_IS_ALIVE && y == player.y && x == player.x) {
                printf("@");
            }
            else if (board[y][x].monster) {
                struct Coordinate coord;
                coord.x = x;
                coord.y = y;
                Monster * m = board[y][x].monster;
                int decimal_type = m->getDecimalType();
                printf("%x", decimal_type);
            }
            else {
                print_cell(board[y][x]);
            }
        }
        printf("\n");
    }
}

void print_cell(Board_Cell cell) {
    if (strcmp(cell.type.c_str(), TYPE_ROCK.c_str()) == 0) {
        printf(" ");
    }
    else if (strcmp(cell.type.c_str(), TYPE_ROOM.c_str()) == 0) {
        printf(".");
    }
    else if (strcmp(cell.type.c_str(), TYPE_CORRIDOR.c_str()) == 0) {
        printf("#");
    }
    else {
        printf("F");
    }
}

void dig_rooms(int number_of_rooms_to_dig) {
    for (int i = 0; i < number_of_rooms_to_dig; i++) {
        dig_room(i);
    }
    add_rooms_to_board();
}

void dig_room(int index) {
    // The index + recusrive_iteration is just a way to gain variety in the
    // random number. The hope is that it makes the program run faster.
    int start_x = random_int(1, WIDTH - MIN_ROOM_WIDTH - 1);
    int start_y = random_int(1, HEIGHT - MIN_ROOM_HEIGHT - 1);
    int room_height = random_int(MIN_ROOM_HEIGHT, MAX_ROOM_HEIGHT);
    int room_width = random_int(MIN_ROOM_WIDTH, MAX_ROOM_WIDTH);
    int end_y = start_y + room_height;
    if (end_y >= HEIGHT - 1) {
        end_y = HEIGHT - 2;

    }
    int end_x = start_x + room_width;
    if (end_x >= WIDTH - 1) {
        end_x = WIDTH - 2;

    }
    int height = end_y - start_y;
    int height_diff = MIN_ROOM_HEIGHT - height;
    if (height_diff > 0) {
        start_y -= height_diff + 1;
    }

    int width = end_x - start_x;
    int width_diff = MIN_ROOM_WIDTH - width;
    if (width_diff > 0) {
        start_x -= width_diff;
    }
    struct Room room;
    room.start_x = start_x;
    room.start_y = start_y;
    room.end_x = end_x;
    room.end_y = end_y;
    if (room_is_valid(room)) {
        rooms.push_back(room);
    }
    else {
        dig_room(index);
    }
}

int room_is_valid(struct Room room) {
    int width = room.end_x - room.start_x;
    int height = room.end_y - room.start_y;
    if (height < MIN_ROOM_HEIGHT || width < MIN_ROOM_WIDTH) {
        return 0;
    }
    for (int i = 0; i < rooms.size(); i++) {
        struct Room current_room = rooms[i];
        int start_x = current_room.start_x - 1;
        int start_y = current_room.start_y - 1;
        int end_x = current_room.end_x + 1;
        int end_y = current_room.end_y + 1;
        if ((room.start_x >= start_x  && room.start_x <= end_x) ||
                (room.end_x >= start_x && room.end_x <= end_x)) {
            if ((room.start_y >= start_y && room.start_y <= end_y) ||
                    (room.end_y >= start_y && room.end_y <= end_y)) {
                return 0;
            }
        }
    }
    return 1;
}

void add_rooms_to_board() {
    Board_Cell cell;
    cell.type = TYPE_ROOM;
    cell.hardness = ROOM;
    cell.monster = NULL;
    cell.object = NULL;
    cell.has_player = 0;
    for(int i = 0; i < rooms.size(); i++) {
        struct Room room = rooms[i];
        for (int y = room.start_y; y <= room.end_y; y++) {
            for(int x = room.start_x; x <= room.end_x; x++) {
                cell.x = x;
                cell.y = y;
                board[y][x] = cell;
            }
        }
    }
}

void dig_cooridors() {
    for (int i = 0; i < rooms.size(); i++) {
        int next_index = i + 1;
        if (next_index == rooms.size()) {
            next_index = 0;
        }
        connect_rooms_at_indexes(i, next_index);
    }
}

void connect_rooms_at_indexes(int index1, int index2) {
    struct Room room1 = rooms[index1];
    struct Room room2 = rooms[index2];
    int start_x = ((room1.end_x - room1.start_x) / 2) + room1.start_x;
    int end_x = ((room2.end_x - room2.start_x) / 2) + room2.start_x;
    int start_y = ((room1.end_y - room1.start_y) / 2) + room1.start_y;
    int end_y = ((room2.end_y - room2.start_y) / 2) + room2.start_y;
    int x_incrementer = 1;
    int y_incrementer = 1;
    if (start_x > end_x) {
        x_incrementer = -1;
    }
    if (start_y > end_y) {
        y_incrementer = -1;
    }
    int cur_x = start_x;
    int cur_y = start_y;
    while(1) {
        int random_num = random_int(0, RAND_MAX);
        int move_y = random_num % 2 == 0;
        if (strcmp(board[cur_y][cur_x].type.c_str(),  TYPE_ROCK.c_str()) != 0) {
            if (cur_y != end_y) {
                cur_y += y_incrementer;
            }
            else if(cur_x != end_x) {
                cur_x += x_incrementer;
            }
            else if(cur_y == end_y && cur_x == end_x) {
                break;
            }
            continue;
        }
        Board_Cell corridor_cell;
        corridor_cell.type = TYPE_CORRIDOR;
        corridor_cell.hardness = CORRIDOR;
        corridor_cell.monster = NULL;
        corridor_cell.object = NULL;
        corridor_cell.has_player = 0;
        corridor_cell.x = cur_x;
        corridor_cell.y = cur_y;
        board[cur_y][cur_x] = corridor_cell;
        if ((cur_y != end_y && move_y) || (cur_x == end_x)) {
            cur_y += y_incrementer;
        }
        else if ((cur_x != end_x && !move_y) || (cur_y == end_y)) {
            cur_x += x_incrementer;
        }
        else {
            break;
        }
    }
}

struct Available_Coords get_non_tunneling_available_coords_for(struct Coordinate coord) {
    int x = coord.x;
    int y = coord.y;
    struct Available_Coords available_coords;
    struct Coordinate new_coord;
    int size = 0;
    available_coords.length = 0;
    available_coords.coords = (struct Coordinate *) malloc(sizeof(struct Coordinate) * 8);
    if (board[y - 1][x].hardness == 0) {
        new_coord.y = y - 1;
        new_coord.x = x;
        available_coords.coords[size] = new_coord;
        size++;
    }
    if (board[y - 1][x - 1].hardness == 0) {
        new_coord.y = y - 1;
        new_coord.x = x - 1;
        available_coords.coords[size] = new_coord;
        size++;
    }
    if(board[y - 1][x + 1].hardness == 0) {
        new_coord.y = y - 1;
        new_coord.x = x + 1;
        available_coords.coords[size] = new_coord;
        size ++;
    }
    if(board[y + 1][x].hardness == 0) {
        new_coord.y = y + 1;
        new_coord.x = x;
        available_coords.coords[size] = new_coord;
        size ++;
    }
    if(board[y + 1][x - 1].hardness == 0) {
        new_coord.y = y + 1;
        new_coord.x = x - 1;
        available_coords.coords[size] = new_coord;
        size ++;
    }
    if(board[y + 1][x + 1].hardness == 0) {
        new_coord.y = y + 1;
        new_coord.x = x + 1;
        available_coords.coords[size] = new_coord;
        size++;
    }
    if(board[y][x - 1].hardness == 0) {
        new_coord.y = y;
        new_coord.x = x - 1;
        available_coords.coords[size] = new_coord;
        size ++;
    }
    if (board[y][x + 1].hardness == 0) {
        new_coord.y = y;
        new_coord.x = x + 1;
        available_coords.coords[size] = new_coord;
        size++;
    }
    available_coords.length = size;
    return available_coords;
}

struct Coordinate get_random_new_non_tunneling_location(struct Coordinate coord) {
    struct Coordinate new_coord;
    struct Available_Coords coords = get_non_tunneling_available_coords_for(coord);
    int new_coord_index = random_int(0, coords.length - 1);
    struct Coordinate temp_coord = coords.coords[new_coord_index];
    new_coord.x = temp_coord.x;
    new_coord.y = temp_coord.y;
    return new_coord;
}

struct Coordinate get_random_new_tunneling_location(struct Coordinate coord) {
    struct Coordinate new_coord;
    new_coord.x = coord.x;
    new_coord.y = coord.y;
    int max_x = coord.x + 1;
    if (coord.x + 1 >= WIDTH - 1) {
        max_x = coord.x;
    }
    int min_x = coord. x - 1;
    if (min_x <= 1) {
        min_x = coord.x;
    }
    int min_y = coord.y - 1;
    if (min_y <= 1) {
        min_y = coord.y;
    }
    int max_y = coord.y + 1;
    if (max_y >= WIDTH - 1) {
        max_y = coord.y;
    }
    while(1) {
        new_coord.x = random_int(min_x, max_x);
        new_coord.y = random_int(min_y, max_y);
        if (coord.x == new_coord.x && coord.y == new_coord.y) {
            continue;
        }
        if (board[new_coord.y][new_coord.x].hardness != IMMUTABLE_ROCK) {
            break;
        }
    }
    return new_coord;
}


void move_player() {
    int found_monster = 0;
    struct Coordinate new_coord;
    struct Available_Coords coords = get_non_tunneling_available_coords_for(player);
    for (int i = 0; i < coords.length; i++) {
        struct Coordinate current_coord = coords.coords[i];
        if (board[current_coord.y][current_coord.x].monster) {
            found_monster = 1;
            new_coord = current_coord;
            break;
        }
    }
    if (!found_monster) {
        new_coord = get_random_new_non_tunneling_location(player);
    }
    if (new_coord.x != player.x || new_coord.y != player.y) {
        kill_player_or_monster_at(new_coord);
    }
    player.x = new_coord.x;
    player.y = new_coord.y;
}

vector<Board_Cell> get_surrounding_cells(struct Coordinate c) {
    vector<Board_Cell> cells;
    cells.push_back(board[c.y + 1][c.x]);
    cells.push_back(board[c.y + 1][c.x - 1]);
    cells.push_back(board[c.y + 1][c.x + 1]);
    cells.push_back(board[c.y - 1][c.x]);
    cells.push_back(board[c.y - 1][c.x + 1]);
    cells.push_back(board[c.y - 1][c.x - 1]);
    cells.push_back(board[c.y][c.x + 1]);
    cells.push_back(board[c.y][c.x - 1]);
    return cells;
}

Board_Cell get_cell_on_tunneling_path(struct Coordinate c) {
    vector<Board_Cell> cells = get_surrounding_cells(c);
    Board_Cell cell = board[c.y][c.x];
    for (int i = 0; i < cells.size(); i++) {
        Board_Cell current_cell = cells[i];
        if (current_cell.tunneling_distance < cell.tunneling_distance) {
            cell = current_cell;
        }
    }
    return cell;
}


Board_Cell get_cell_on_non_tunneling_path(struct Coordinate c) {
    vector<Board_Cell> cells = get_surrounding_cells(c);
    Board_Cell cell = board[c.y][c.x];
    int min = cell.non_tunneling_distance;
    for (int i = 0; i < cells.size(); i++) {
        Board_Cell my_cell = cells[i];
        if (my_cell.non_tunneling_distance < min) {
            cell = my_cell;
            min = my_cell.non_tunneling_distance;
        }
    }
    return cell;
}

struct Room get_room_player_is_in() {
    struct Room room;
    room.start_x = 0;
    room.end_x = 0;
    room.start_y = 0;
    room.end_y = 0;
    for (int i = 0; i < rooms.size(); i++) {
        struct Room current_room = rooms[i];
        if (current_room.start_x <= player.x && player.x <= current_room.end_x) {
            if (current_room.start_y <= player.y && player.y <= current_room.end_y) {
                room = current_room;
                break;
            }
        }
    }
    return room;
}

bool monster_is_in_same_room_as_player(Monster * m) {
    int x = m->x;
    int y = m->y;
    struct Room room = get_room_player_is_in();
    if (room.start_x <= x && x <= room.end_x) {
        if (room.start_y <= y && y <= room.end_y) {
            return true;
        }
    }
    return false;
}

int should_do_erratic_behavior() {
    return random_int(0, 1);
}

int monster_knows_last_player_location(Monster * m) {
    int last_x = m->last_known_player_x;
    int last_y = m->last_known_player_y;
    return last_x != 0 && last_y != 0;
}

struct Coordinate get_straight_path_to(Monster * m, struct Coordinate coord) {
    int x = m->x;
    int y = m->y;
    struct Coordinate new_coord;
    if (x == coord.x) {
        new_coord.x = x;
    }
    else if (x < coord.x) {
        new_coord.x = x + 1;
    }
    else {
        new_coord.x = x - 1;
    }

    if (y == coord.y) {
        new_coord.y = y;
    }
    else if (y < coord.y) {
        new_coord.y = y + 1;
    }
    else {
        new_coord.y = y - 1;
    }

    return new_coord;
}

void kill_monster(Monster * m) {
    int index = -1;
    for (int i = 0; i < monsters.size(); i++) {
        Monster * monster = monsters[i];
        if (monster->x == m->x && monster->y == m->y) {
            index = i;
            break;
        }
    }
    int x = m->x;
    int y = m->y;
    board[y][x].monster = NULL;
    if (index >= 0) {
        monsters.erase(monsters.begin() + index);
    }
    delete m;
}

void kill_player_or_monster_at(struct Coordinate coord) {
    Monster * m = board[coord.y][coord.x].monster;
    if (m) {
        kill_monster(m);
    }
    if (player.x == coord.x && player.y == coord.y) {
        PLAYER_IS_ALIVE = 0;
        add_message("The player was killed!\n");
    }
}

void move_monster(Monster * monster) {
    int monster_x = monster->x;
    int monster_y = monster->y;
    Board_Cell cell = board[monster_y][monster_x];
    struct Coordinate monster_coord;
    monster_coord.x = monster_x;
    monster_coord.y = monster_y;
    struct Coordinate new_coord;
    new_coord.x = monster_x;
    new_coord.y = monster_y;
    board[monster->y][monster->x].monster = NULL;
    int decimal_type = monster->getDecimalType();
    struct Coordinate last_known_player_location;
    last_known_player_location.x = monster->last_known_player_x;
    last_known_player_location.y = monster->last_known_player_y;
    switch(decimal_type) {
        case 0: // nothing
            if (monster_is_in_same_room_as_player(monster)) {
                new_coord = get_straight_path_to(monster, player);
            }
            else {
                new_coord = get_random_new_non_tunneling_location(monster_coord);
            }
            break;
        case 1: // intelligent
            if (monster_is_in_same_room_as_player(monster)) {
                monster->last_known_player_x = player.x;
                monster->last_known_player_y = player.y;
                new_coord = get_straight_path_to(monster, player);
            }
            else if(monster_knows_last_player_location(monster)) {
                new_coord = get_straight_path_to(monster, last_known_player_location);
                if (new_coord.x == last_known_player_location.x && new_coord.y == last_known_player_location.y) {
                    monster->resetPlayerLocation();
                }
            }
            else {
                new_coord = get_random_new_non_tunneling_location(monster_coord);
            }
            break;
        case 2: // telepathic
            new_coord = get_straight_path_to(monster, player);
            if (board[new_coord.y][new_coord.x].hardness > 0) {
                new_coord.x = monster_coord.x;
                new_coord.y = monster_coord.y;
            }
            break;
        case 3: // telepathic + intelligent
            cell = get_cell_on_non_tunneling_path(new_coord);
            new_coord.x = cell.x;
            new_coord.y = cell.y;
            break;
        case 4: // tunneling
            if (monster_is_in_same_room_as_player(monster)) {
                new_coord = get_straight_path_to(monster, player);
            }
            else {
                new_coord = get_random_new_tunneling_location(monster_coord);
            }
            cell = board[new_coord.y][new_coord.x];
            if (cell.hardness > 0) {
                board[cell.y][cell.x].hardness -= 85;
                if (board[cell.y][cell.x].hardness <= 0) {
                    board[cell.y][cell.x].hardness = 0;
                    board[cell.y][cell.x].type = TYPE_CORRIDOR;
                    set_non_tunneling_distance_to_player();
                }
                else {
                    new_coord.x = monster_x;
                    new_coord.y = monster_y;
                }
                set_tunneling_distance_to_player();
            }
            break;
        case 5: // tunneling + intelligent
            if (monster_is_in_same_room_as_player(monster)) {
                monster->last_known_player_x = player.x;
                monster->last_known_player_y = player.y;
                new_coord = get_straight_path_to(monster, player);
            }
            else if(monster_knows_last_player_location(monster)) {
                new_coord = get_straight_path_to(monster, last_known_player_location);
                if (new_coord.x == last_known_player_location.x && new_coord.y == last_known_player_location.y) {
                    monster->resetPlayerLocation();
                }
            }
            else {
                new_coord = get_random_new_tunneling_location(monster_coord);
                cell = board[new_coord.y][new_coord.x];
                if (cell.hardness > 0) {
                    board[cell.y][cell.x].hardness -= 85;
                    if (board[cell.y][cell.x].hardness <= 0) {
                        board[cell.y][cell.x].hardness = 0;
                        board[cell.y][cell.x].type = TYPE_CORRIDOR;
                        set_non_tunneling_distance_to_player();
                    }
                    else {
                        new_coord.x = monster_x;
                        new_coord.y = monster_y;
                    }
                    set_tunneling_distance_to_player();
                }
            }
            break;
        case 6: // tunneling + telepathic
            new_coord = get_straight_path_to(monster, player);
            cell = board[new_coord.y][new_coord.x];
            if (cell.hardness > 0) {
                board[cell.y][cell.x].hardness -= 85;
                if (board[cell.y][cell.x].hardness <= 0) {
                    board[cell.y][cell.x].hardness = 0;
                    board[cell.y][cell.x].type = TYPE_CORRIDOR;
                    set_non_tunneling_distance_to_player();
                }
                else {
                    new_coord.x = monster_x;
                    new_coord.y = monster_y;
                }
                set_tunneling_distance_to_player();
            }
            break;
        case 7: // tunneling + telepathic + intelligent
            cell = get_cell_on_tunneling_path(new_coord);
            new_coord.x = cell.x;
            new_coord.y = cell.y;
            if (cell.hardness > 0) {
                board[cell.y][cell.x].hardness -= 85;
                if (board[cell.y][cell.x].hardness <= 0) {
                    board[cell.y][cell.x].hardness = 0;
                    board[cell.y][cell.x].type = TYPE_CORRIDOR;
                    set_non_tunneling_distance_to_player();
                }
                else {
                    new_coord.x = monster_x;
                    new_coord.y = monster_y;
                }
                set_tunneling_distance_to_player();
            }
            break;
        case 8: // erratic
            if (should_do_erratic_behavior()) {
                new_coord = get_random_new_non_tunneling_location(monster_coord);
            }
            else {
                if (monster_is_in_same_room_as_player(monster)) {
                    new_coord = get_straight_path_to(monster, player);
                }
                else {
                    new_coord = get_random_new_non_tunneling_location(monster_coord);
                }
            }
            break;
        case 9: // erratic + intelligent
            if (should_do_erratic_behavior()) {
                new_coord = get_random_new_non_tunneling_location(monster_coord);
            }
            else {
                if (monster_is_in_same_room_as_player(monster)) {
                    monster->last_known_player_x = player.x;
                    monster->last_known_player_y = player.y;
                    new_coord = get_straight_path_to(monster, player);
                }
                else if(monster_knows_last_player_location(monster)) {
                    new_coord = get_straight_path_to(monster, last_known_player_location);
                    if (new_coord.x == last_known_player_location.x && new_coord.y == last_known_player_location.y) {
                        monster->resetPlayerLocation();
                    }
                }
                else {
                    new_coord = get_random_new_non_tunneling_location(monster_coord);
                }
            }
            break;
        case 10: // erratic + telepathic
            if (should_do_erratic_behavior()) {
                new_coord = get_random_new_non_tunneling_location(monster_coord);
            }
            else {
                new_coord = get_straight_path_to(monster, player);
                if (board[new_coord.y][new_coord.x].hardness != 0) {
                    new_coord.x = monster_x;
                    new_coord.y = monster_y;
                }
            }
            break;
        case 11: // erratic + intelligent + telepathic
            if (should_do_erratic_behavior()) {
                new_coord = get_random_new_non_tunneling_location(monster_coord);
            }
            else {
                new_coord = get_straight_path_to(monster, player);
                cell = board[new_coord.y][new_coord.x];
                if (cell.hardness > 0) {
                    board[cell.y][cell.x].hardness -= 85;
                    if (board[cell.y][cell.x].hardness <= 0) {
                        board[cell.y][cell.x].hardness = 0;
                        board[cell.y][cell.x].type = TYPE_CORRIDOR;
                        set_non_tunneling_distance_to_player();
                    }
                    else {
                        new_coord.x = monster_x;
                        new_coord.y = monster_y;
                    }
                    set_tunneling_distance_to_player();
                }
            }
            break;
        case 12: // erratic + tunneling
            if (should_do_erratic_behavior()) {
                new_coord = get_random_new_non_tunneling_location(monster_coord);
            }
            else {
                if (monster_is_in_same_room_as_player(monster)) {
                    new_coord = get_straight_path_to(monster, player);
                }
                else {
                    new_coord = get_random_new_tunneling_location(monster_coord);
                }
                cell = board[new_coord.y][new_coord.x];
                if (cell.hardness > 0) {
                    board[cell.y][cell.x].hardness -= 85;
                    if (board[cell.y][cell.x].hardness <= 0) {
                        board[cell.y][cell.x].hardness = 0;
                        board[cell.y][cell.x].type = TYPE_CORRIDOR;
                        set_non_tunneling_distance_to_player();
                    }
                    else {
                        new_coord.x = monster_x;
                        new_coord.y = monster_y;
                    }
                    set_tunneling_distance_to_player();
                }
            }
            break;
        case 13: // erratic + tunneling + intelligent
            if (should_do_erratic_behavior()) {
                new_coord = get_random_new_non_tunneling_location(monster_coord);
            }
            else {
                if (monster_is_in_same_room_as_player(monster)) {
                    monster->last_known_player_x = player.x;
                    monster->last_known_player_y = player.y;
                    new_coord = get_straight_path_to(monster, player);
                }
                else if(monster_knows_last_player_location(monster)) {
                    new_coord = get_straight_path_to(monster, last_known_player_location);
                    if (new_coord.x == last_known_player_location.x && new_coord.y == last_known_player_location.y) {
                        monster->resetPlayerLocation();
                    }
                }
                else {
                    new_coord = get_random_new_tunneling_location(monster_coord);
                    cell = board[new_coord.y][new_coord.x];
                    if (cell.hardness > 0) {
                        board[cell.y][cell.x].hardness -= 85;
                        if (board[cell.y][cell.x].hardness <= 0) {
                            board[cell.y][cell.x].hardness = 0;
                            board[cell.y][cell.x].type = TYPE_CORRIDOR;
                            set_non_tunneling_distance_to_player();
                        }
                        else {
                            new_coord.x = monster_x;
                            new_coord.y = monster_y;
                        }
                        set_tunneling_distance_to_player();
                    }
                }
            }
            break;
        case 14: // erratic + tunneling + telepathic
            if (should_do_erratic_behavior()) {
                new_coord = get_random_new_non_tunneling_location(monster_coord);
            }
            else {
                new_coord = get_straight_path_to(monster, player);
                cell = board[new_coord.y][new_coord.x];
                if (cell.hardness > 0) {
                    board[cell.y][cell.x].hardness -= 85;
                    if (board[cell.y][cell.x].hardness <= 0) {
                        board[cell.y][cell.x].hardness = 0;
                        board[cell.y][cell.x].type = TYPE_CORRIDOR;
                        set_non_tunneling_distance_to_player();
                    }
                    else {
                        new_coord.x = monster_x;
                        new_coord.y = monster_y;
                    }
                    set_tunneling_distance_to_player();
                }
            }
            break;
        case 15: // erratic + tunneling + telepathic + intelligent
            if (should_do_erratic_behavior()) {
                new_coord = get_random_new_non_tunneling_location(monster_coord);
            }
            else {
                cell = get_cell_on_tunneling_path(new_coord);
                new_coord.x = cell.x;
                new_coord.y = cell.y;
                if (cell.hardness > 0) {
                    board[cell.y][cell.x].hardness -= 85;
                    if (board[cell.y][cell.x].hardness <= 0) {
                        board[cell.y][cell.x].hardness = 0;
                        board[cell.y][cell.x].type = TYPE_CORRIDOR;
                        set_non_tunneling_distance_to_player();
                    }
                    else {
                        new_coord.x = monster_x;
                        new_coord.y = monster_y;
                    }
                    set_tunneling_distance_to_player();
                }
            }
            break;
        default:
            printf("Invalid decimal type, %d\n", decimal_type);
            break;
    }
    if (new_coord.x != monster_x || new_coord.y != monster_y) {
        kill_player_or_monster_at(new_coord);
    }
    board[monster->y][monster->x].monster = NULL;
    monster->x = new_coord.x;
    monster->y = new_coord.y;
    board[new_coord.y][new_coord.x].monster = monster;
}
