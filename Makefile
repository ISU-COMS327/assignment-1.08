CC = gcc
CXX = g++
CFLAGS = -g -Wall -Werror -ggdb
OBJ = monster monster_wrapper generate_dungeon
OBJS = $(addsuffix .o,$(OBJ))

all:
	make compile

compile:
	$(CXX) -c monster.cpp -o monster.o
	$(CXX) -c monster_wrapper.cpp -o monster_wrapper.o
	$(CXX) $(CFLAGS) -c priority_queue.cpp -o priority_queue.o
	$(CXX) $(CFLAGS) -c monster_template.cpp -o monster_template.o
	$(CXX) $(CFLAGS) -c monster_description_parser.cpp -o monster_description_parser.o
	$(CXX) $(CFLAGS) -c generate_dungeon.cpp -o generate_dungeon.o -Ipriority_queue.o -Ilncurses -Imonster_description_parser.o
	$(CXX) generate_dungeon.o monster_wrapper.o monster.o monster_template.o monster_description_parser.o priority_queue.o -lncurses -o generate_dungeon

.PHONY: clean
clean:
	@rm -rf $(OBJS) monsterexe generate_dungeon *.o *.dSYM
	@echo "Directory cleaned."
