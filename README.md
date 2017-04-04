# Assignment 1.08: Loading Monsters and Objects

## Description

In this assignment, we built off last weeks assignment by having our object
and monster template classes generate instances of objects and monsters,
respectively. We now show objects in the dungeon. We use the `object_desc.txt`
and `monster_desc.txt` files to generate objects and monsters, respectively.
We randomly select templates from those files for each dungeon created.

I did a lot of things in this assignment. At first, it was very frustrating
because I was getting some very odd seg faults and overflow errors. Some of my
first code was causing errors.
However, I was able to track these issues down and solve them. There should
no longer be any overflow errors or seg faults.

Some major changes include:

* Brand new Makefile

* Changed file structure to put all source files in a `src` folder

* Created a util file that many classes used

* Changed `numeric_t` to become a `Numeric` object

* Swapped out old `priority_queue` file to become a `PriorityQueue` object

* Created an `Object` class

* Changed the way I was tracking monsters and objects. Instead of searching
through a list each time to access them, I now store them on cells.


## Usage

Same usage as always:
`make`
`./generate_dungeon`
