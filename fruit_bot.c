// fruit_bot.c
// Assignment 3, COMP1511 18s1: Fruit Bot
//
// Dheeraj Viswanadham (z5204820)
// Started: 16/05/18 | Last edited: 23/05/18
//
// Version 1.0.0: Assignment released.
// Version 1.0.1: minor bug in main fixed

#include <stdio.h>
#include <assert.h>

#include "fruit_bot.h"

void print_player_name(void);
void print_move(struct bot *b);
void run_unit_tests(void);

// ADD PROTOTYPES FOR YOUR FUNCTIONS HERE

// YOU SHOULD NOT NEED TO CHANGE THIS MAIN FUNCTION

int main(int argc, char *argv[]) {

    if (argc > 1) {
        // supply any command-line argument to run unit tests
        run_unit_tests();
        return 0;
    }

    struct bot *me = fruit_bot_input(stdin);
    if (me == NULL) {
        print_player_name();
    } else {
        print_move(me);
    }

    return 0;
}

void print_player_name(void) {
    printf("Survival of the Bots");
}

// print_move - should print a single line indicating
//              the move your bot wishes to make
//
// This line should contain only the word Move, Sell or Buy
// followed by a single integer

void print_move(struct bot *b) {
    // THE LINES BELOW IMPLEMENT A SIMPLE (USELESS) STRATEGY
    // REPLACE THEN WITH YOUR CODE

    int silly_choice = b->turns_left % 3;
    if (silly_choice == 0) {
        printf("Move 1\n");
    } else if (silly_choice == 1) {
        printf("Sell 1\n");
    } else {
        printf("Buy 1\n");
    }

}


// ADD A COMMENT HERE EXPLAINING YOUR OVERALL TESTING STRATEGY

void run_unit_tests(void) {
    // PUT YOUR UNIT TESTS HERE
    // This is a difficult assignment to write unit tests for,
    // but make sure you describe your testing strategy above.
}


// ADD YOUR FUNCTIONS HERE

