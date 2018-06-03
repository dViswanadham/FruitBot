// fruit_bot.c
// Assignment 3, COMP1511 18s1: Fruit Bot
//
// Dheeraj Viswanadham (z5204820)
// Started: 16/05/18 | Last edited: 3/06/18
//
// Version 1.0.0: Assignment released.
// Version 1.0.1: minor bug in main fixed
// 
// Acknowledgment: Some of the code was edited after looking at the week 11
// lab exercises so as to improve the overall readability of my code and they 
// were much more simpler to implement than my previous solutions.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "fruit_bot.h"

#define TRUE 1
#define FALSE 0

#define BUY 1
#define SELL 2
#define ANY 3

// Directions:
#define EAST 1
#define WEST -1
#define LEAST 3

// Decisions:
#define TRAVERSE_PURCHASER 1
#define PURCHASE 2
#define TRAVERSE_RETAILER 3
#define UNLOAD 4
#define TRAVERSE_CHARGE 5
#define RECHARGING 6
#define UNLOAD_AT_DUMP 7
#define TRAVERSE_DUMP 8
#define ABJURE_PURCHASE 9

// Other:
#define DISTANCE 100000
#define COST 100000
#define QUARTER 25.0
#define MAX 100

void print_player_name(void);
void print_move(struct bot *b);
void run_unit_tests(void);

// ADD PROTOTYPES FOR YOUR FUNCTIONS HERE
int within(char *variable, char *variable_arr[], int array_length);
int can_reach_location_with_top_up(struct bot *b, struct location *location);
int max_income(struct bot *b, struct location *buyer, struct location *seller);
int depot_price(struct bot *b, struct location *vendor);
int max_amount(int accessible, int needed);
int largest_amount_for_bot(struct bot *b, struct location *location);
int free_room(struct bot *b);
int move_charge(struct bot *b, struct location *location);
int fruit_buyers(struct bot *b, char *fruit, struct location *loc[]);
int fruit_sellers(struct bot *b, char *fruit, struct location *loc[]);
int electricity_depots(struct bot *b, struct location *loc[]);
int curr_location_decision(struct location *location);
int displacement(struct location *loc_a, struct location *loc_b, int route);
int minimum_displacement(struct location *loc_a, struct location *loc_b);
int move_direction(struct location *loc_a, struct location *loc_b);
int same_location(struct location *loc_a, struct location *loc_b);
int battery_lvl(struct bot *b);
int can_reach(struct bot *b, struct location *location);
int can_reach_with_curr_battery(int curr_charge, struct location *start, 
                                struct location *destination);
int fruit_location(struct bot *b, char *fruit, int decision, 
                   struct location *loc[]);
double current_battery_quota(struct bot *b);

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
	printf("BattleBot.Bravo");
}

// print_move - should print a single line indicating
//              the move your bot wishes to make
//
// This line should contain only the word Move, Sell or Buy
// followed by a single integer

void print_move(struct bot *b) {
	struct location *current = b->location;
	char *array_fruit[MAX_FRUIT_TYPES];
	int counter = 0;
	int traversed_world = FALSE;
	
	while(current != NULL && !traversed_world) {
		if (!within(current->fruit, array_fruit, counter) 
		    && strcmp("Electricity", current->fruit) != 0 
		    && strcmp("Anything", current->fruit) != 0 
		    && strcmp("Nothing", current->fruit) != 0 
		    && strcmp("other", current->fruit) != 0) {
			
			array_fruit[counter] = current->fruit;
			counter = (counter + 1);
		}
		
		current = (current->east);
		
		if (same_location(current, b->location)) {
			
			traversed_world = TRUE;
		}
	}
	
	struct location *prime_retailer = NULL; // Selling fruits
	struct location *prime_purchaser = NULL; // Buying fruits
	int var_x = 0;
	int economic_choice = 0;
	
	while(var_x < counter) {
		struct location *fruit_retailers[MAX_LOCATIONS];
		struct location *fruit_purchasers[MAX_LOCATIONS];
		struct location *economic_retailer = NULL;
		struct location *economic_purchaser = NULL;
		
		int fruit_amount_retailers = fruit_sellers(b, array_fruit[var_x], 
		                             fruit_retailers);
		int fruit_amount_purchasers = fruit_buyers(b, array_fruit[var_x], 
		                              fruit_purchasers);
		int max_income_possible = 0;
		int var_y = 0;
		
		while(var_y < fruit_amount_retailers) {
			int var_z = 0;
			
			while(var_z < fruit_amount_purchasers) {
				if (max_income_possible < max_income(b, fruit_purchasers[var_z],
				    fruit_retailers[var_y])) {
					
					max_income_possible = max_income(b, fruit_purchasers[var_z],
					fruit_retailers[var_y]);
					economic_retailer = fruit_retailers[var_y];
					economic_purchaser = fruit_purchasers[var_z];
				}
				
				var_z = (var_z + 1);
			}
			
			var_y = (var_y + 1);
		}
		
		if (max_income_possible >= economic_choice) {
			
			prime_purchaser = economic_purchaser;
			prime_retailer = economic_retailer;
		}
		
		var_x = (var_x + 1);
	}
	
	int decision_lvl = 0;
	
	if (prime_retailer != NULL && prime_purchaser != NULL) {
		// Insert strategy
	
	} else {
		// It is better to buy electricity otherwise there will be a loss 
		// at this point
		decision_lvl = -1;
	}
	
	// Do not buy anything in last two turns (only sell)
	int deny_purchase = FALSE;
	
	if (b->turns_left <= 2) {
		
		deny_purchase = TRUE;
	}
	
	// Check the distance to nearest electricity and move there (within the same 
	// turn if possible)
	if (decision_lvl != -1) {
		if (current_battery_quota(b) < QUARTER) { 
			// Should recharge if less than a quarter of electricity is left
			if (strcmp("Electricity", b->location->fruit) == 0 
			    && b->location->quantity > 0) {
				
				decision_lvl = RECHARGING;
				
			} else {
				
				decision_lvl = TRAVERSE_CHARGE;
			}
			
		// Sell the fruit we have on board to the strategically best buyer
		} else if (b->fruit != NULL && strcmp(prime_purchaser->name, 
		           b->location->name) == 0) {
			
			decision_lvl = UNLOAD;
		
		// There is fruit however, we aren't near the strategically best buyer
		} else if (b->fruit != NULL) {
			if (can_reach_location_with_top_up(b, prime_purchaser)) {
				
				decision_lvl = TRAVERSE_PURCHASER;
				
			} else {
				
				decision_lvl = TRAVERSE_CHARGE;
			}
		
		// There is no fruit on board and we aren't near the strategically 
		// best seller
		} else if (b->fruit == NULL && strcmp(prime_retailer->name, 
		           b->location->name) != 0) {
			if (can_reach_location_with_top_up(b, prime_retailer)) {
				
				decision_lvl = TRAVERSE_RETAILER;
			
			} else {
				
				decision_lvl = TRAVERSE_CHARGE;
			}
		
		// There is no fruit on board and we are near the strategically 
		// best seller
		} else if (b->fruit == NULL && strcmp(prime_retailer->name, 
		           b->location->name) == 0 && !deny_purchase) {
			
			decision_lvl = PURCHASE;
		
		} else if (deny_purchase) {
			
			decision_lvl = ABJURE_PURCHASE;
		}
	
	} else {
		// We have fruit, but no strategically best buyer and we are AT 
		// the buyer of "anything" so sell at "anything"
		if (b->fruit != NULL && strcmp("Anything", b->location->fruit) == 0) {
			
			decision_lvl = UNLOAD_AT_DUMP;
		
		// We have fruit, but no strategically best buyer however we are NEAR 
		// the buyer of "anything" so go to "anything"
		} else if (b->fruit != NULL && strcmp("Anything", 
		           b->location->fruit) != 0) {
			
			decision_lvl = TRAVERSE_DUMP;
		
		// We have no strategically best buyer/seller but we are AT an 
		// electricity depot so it's best to buy some electricity
		} else if (b->fruit == NULL && strcmp("Electricity", 
		           b->location->fruit) == 0 && !battery_lvl(b)) {
			
			decision_lvl = RECHARGING;
		
		// We have no strategically best buyer/seller and bot is not at an 
		// electricity depot however we are NEAR one so move the bot there
		} else if (b->fruit == NULL && strcmp("Electricity", 
		           b->location->fruit) != 0 && !battery_lvl(b)) {
			
			decision_lvl = TRAVERSE_CHARGE;
		
		} else {
			
			decision_lvl = ABJURE_PURCHASE;
		}
	}
	
	// We have fruit on board that we can sell
	if (decision_lvl == TRAVERSE_PURCHASER) {
		int traverse = minimum_displacement(b->location, prime_purchaser);
		int traverse_direction = move_direction(b->location, prime_purchaser);
		
		printf("Move %d\n", traverse * traverse_direction);
	
	} else if (decision_lvl == UNLOAD) {
		
		printf("Sell %d\n", max_amount(prime_purchaser->quantity, b->fruit_kg));
	
	} else if (decision_lvl == TRAVERSE_RETAILER) {
		int traverse = minimum_displacement(b->location, prime_retailer);
		int traverse_direction = move_direction(b->location, prime_retailer);
		
		printf("Move %d\n", traverse * traverse_direction);
	
	} else if (decision_lvl == PURCHASE) {
		
		printf("Buy %d\n", max_amount(largest_amount_for_bot(b, b->location), 
		b->maximum_fruit_kg));
	
	} else if (decision_lvl == UNLOAD_AT_DUMP) {
		
		printf("Sell %d\n", b->fruit_kg);
	
	} else if (decision_lvl == TRAVERSE_DUMP) {
		struct location *fruit_dump[MAX_LOCATIONS];
		struct location *closest_fruit_dump = NULL;
		
		int count_dumps = fruit_buyers(b, "Anything", fruit_dump);
		int closest_displacement = DISTANCE;
		int var_a = 0;
		
		while(var_a < count_dumps) {
			if (minimum_displacement(b->location, fruit_dump[var_a]) 
			    <= closest_displacement) {
				
				closest_displacement = minimum_displacement(b->location, 
				fruit_dump[var_a]);
				closest_fruit_dump = fruit_dump[var_a];
			}
			
			var_a = (var_a + 1);
		}
		
		int traverse = minimum_displacement(b->location, closest_fruit_dump);
		int traverse_direction = move_direction(b->location, 
		                         closest_fruit_dump);
		
		printf("Move %d\n", traverse * traverse_direction);
	
	// Go to strategically best electricity depot
	} else if (decision_lvl == TRAVERSE_CHARGE) {
		struct location *elec_depot[MAX_LOCATIONS];
		struct location *profitable_depot = NULL;
		struct location *nearest_depot = NULL;
		
		int num_of_depots = electricity_depots(b, elec_depot);
		int nearest_depot_price = COST;
		int nearest_depot_displacement = DISTANCE;
		int var_a = 0;
		
		while(var_a < num_of_depots) {
			if (depot_price(b, elec_depot[var_a]) <= nearest_depot_price) {
				
				nearest_depot_price = depot_price(b, elec_depot[var_a]);
				profitable_depot = elec_depot[var_a];
			}
			
			if (minimum_displacement(b->location, elec_depot[var_a]) 
			    <= nearest_depot_displacement) {
				
				nearest_depot_displacement = minimum_displacement(b->location, 
				elec_depot[var_a]);
				nearest_depot = elec_depot[var_a];
			}
			
			var_a = (var_a + 1);
		}
		
		int traverse = 0;
		int traverse_direction = 1;
		
		if (can_reach(b, profitable_depot)) {
			
			traverse = minimum_displacement(b->location, profitable_depot);
			traverse_direction = move_direction(b->location, profitable_depot);
			
		} else {
			
			traverse = minimum_displacement(b->location, nearest_depot);
			traverse_direction = move_direction(b->location, nearest_depot);
		}
		
		printf("Move %d\n", traverse * traverse_direction);
	
	} else if (decision_lvl == RECHARGING) {
		
		printf("Buy %d\n", max_amount(largest_amount_for_bot(b, b->location), 
		(b->battery_capacity - b->battery_level)));
	
	} else if (decision_lvl == ABJURE_PURCHASE) {
		
		printf("Move 1\n");
	
	} else {
		
		printf("NO ACTION - STAGE %d\n", decision_lvl);
	}
}

// Testing Strategy:
//
// My testing strategy was to utilise my knowledge of fundamental econometrics 
// and with the help of the week 11 lab exercises, i was able to draw out a plan 
// on how to approach the assignment. I divided my "steps" into smaller
// functions and had various "forks" and decisions that my bot would take
// depending on the size and environment of the world, which I tested by using 
// the provided referee and making my own worlds of different sizes and types of
// locations to ensure my bot could adapt to any simulation that it could 
// encounter in the actual tournament. I first got all my buyer and seller 
// locations of the fruit I wanted to sell (and double-checked that there was 
// in fact a profitable location that I could sell them to). I then determined 
// the most strategic steps that the bot should take depending on it's current 
// location within the tournament world in order to ensure that it would have
// the maximum possible profit (given no external interference and a separate
// strategy if I was in a multi-bot world). I then tested that my decisions 
// worked by having printf debugging statements throughout my code to determine
// which actions my bot took within the test world and the referee program.

void run_unit_tests(void) {
	// PUT YOUR UNIT TESTS HERE
	// This is a difficult assignment to write unit tests for,
	// but make sure you describe your testing strategy above.
}

// OWN FUNCTIONS START HERE:

// Determines whether a fruit is within the buyer array
int within(char *variable, char *variable_arr[], int array_length) {
	int var_y = 0;
	
	while(var_y < array_length) {
		if (strcmp(variable, variable_arr[var_y]) == 0) {
			
			return TRUE;
		}
		
		var_y = (var_y + 1);
	}
	
	return FALSE;
}

// Obtains the various locations of the fruits
int fruit_location(struct bot *b, char *fruit, int decision, 
	struct location *loc[]) {
	struct location *current = b->location;
	int traversed_world = FALSE;
	int var_y = 0;
	
	while(current != NULL && !traversed_world) {
		if (curr_location_decision(current) == decision || decision == ANY) {
			if (strcmp(current->fruit, fruit) == 0 && current->quantity > 0) {
				
				loc[var_y] = current;
				var_y = (var_y + 1);
			}
		}
		
		current = current->east;
		
		if (same_location(current, b->location)) {
			
			traversed_world = TRUE;
		}
	}
	
	return var_y;
}

// Gives the length of the array containing the various buyer locations
int fruit_buyers(struct bot *b, char *fruit, struct location *loc[]) {
	
	return fruit_location(b, fruit, BUY, loc);
}

// Gives the length of the array containing the various seller locations
int fruit_sellers(struct bot *b, char *fruit, struct location *loc[]) {
	
	return fruit_location(b, fruit, SELL, loc);
}

// Gives the length of the array containing the various electricity locations
int electricity_depots(struct bot *b, struct location *loc[]) {
	
	return fruit_sellers(b, "Electricity", loc);
}

// Determine whether the given location will buy or sell fruit/electricity
int curr_location_decision(struct location *location) {
	if (location->price < 0) {
		
		return SELL;
	
	} else if (location->price > 0) {
		
		return BUY;
	
	} else {
		
		return 0;
	}
}

// Determines the distance between any two given locations
int displacement(struct location *loc_a, struct location *loc_b, int route) {
	struct location *current = loc_a;
	int traversed_world = 0;
	int var_y = 0;
	
	while(current != NULL && !traversed_world) {
		if (same_location(current, loc_b)) {
			
			return var_y;
		
		} else {
			
			var_y = (var_y + 1);
		}
		
		if (route == WEST) {
			
			current = (current->west);
			
		} else if (route == EAST) {
			
			current = (current->east);
		}
		
		if (same_location(current, loc_a)) {
			
			traversed_world = TRUE;
		}
	}
	
	return var_y;
}

// Determines the smallest distance to a location
int minimum_displacement(struct location *loc_a, struct location *loc_b) {
	int displacement_e = displacement(loc_a, loc_b, EAST);
	int displacement_w = displacement(loc_a, loc_b, WEST);
	
	if (displacement_e <= displacement_w) {
		
		return displacement_e;
		
	} else {
		
		return displacement_w;
	}
}

// Determines the direction which the bot should take to travel the smallest 
// distance
int move_direction(struct location *loc_a, struct location *loc_b) {
	int displacement_e = displacement(loc_a, loc_b, EAST);
	int displacement_w = displacement(loc_a, loc_b, WEST);
	
	if (displacement_e <= displacement_w) {
		
		return EAST;
		
	} else {
		
		return WEST;
	}
}

// Determines whether two locations are in fact the same place or not
int same_location(struct location *loc_a, struct location *loc_b) {
	if (strcmp(loc_a->name, loc_b->name) == 0) {
		
		return TRUE;
	}
	
	return FALSE;
}

// Checks whether the bot is fully charged with electricity or not
int battery_lvl(struct bot *b) {
	if (b->battery_level == b->battery_capacity) {
		
		return TRUE;
		
	} else {
		
		return FALSE;
	}
}

// Checks whether the bot can move to a location with it's current battery life
int can_reach(struct bot *b, struct location *location) {
	if (minimum_displacement(b->location, location) <= b->battery_level) {
		
		return TRUE;
		
	} else {
		
		return FALSE;
	}
}

int can_reach_with_curr_battery(int curr_charge, struct location *start, 
	struct location *destination) {
	if (minimum_displacement(start, destination) <= curr_charge) {
		
		return TRUE;
		
	} else {
		
		return FALSE;
	}
}

int can_reach_location_with_top_up(struct bot *b, struct location *location) {
	if (can_reach(b, location)) {
		struct location *elec_depot[MAX_LOCATIONS];
		struct location *profitable_depot = NULL;
		struct location *nearest_depot = NULL;
		
		int num_of_depots = electricity_depots(b, elec_depot);
		int nearest_depot_price = COST;
		int nearest_depot_displacement = DISTANCE;
		int var_a = 0;
		
		while(var_a < num_of_depots) {
			if (depot_price(b, elec_depot[var_a]) <= nearest_depot_price) {
				
				nearest_depot_price = depot_price(b, elec_depot[var_a]);
				profitable_depot = elec_depot[var_a];
			}
			
			if (minimum_displacement(b->location, elec_depot[var_a]) 
			    <= nearest_depot_displacement) {
				
				nearest_depot_displacement = minimum_displacement(b->location, 
				elec_depot[var_a]);
				nearest_depot = elec_depot[var_a];
			}
			
			var_a = (var_a + 1);
		}
		
		int future_charge_lvl = (b->battery_level - 
		                         minimum_displacement(b->location, location));
		
		if (can_reach_with_curr_battery(future_charge_lvl, location, 
		    profitable_depot)) {
			
			return TRUE;
		}
	}
	
	return FALSE;
}

// Determines the max profit the bot could take with it's current charge
int max_income(struct bot *b, struct location *buyer, struct location *seller) {
	int price = 0;
	int supply = max_amount(max_amount(seller->quantity, b->maximum_fruit_kg), 
	             buyer->quantity); 
	
	price += (abs(seller->price) * supply);
	
	int total_profit = (buyer->price * supply);
	int profit_after_costs = (total_profit - price);
	
	return profit_after_costs;
}

int depot_price(struct bot *b, struct location *vendor) {
	int price = 0;
	
	price += move_charge(b, vendor);
	
	int curr_battery = b->battery_level;
	int price_to_traverse = minimum_displacement(b->location, vendor);
	int future_battery_lvl = (curr_battery - price_to_traverse);
	int electricity_needed = (b->battery_capacity - future_battery_lvl);
	int supply = max_amount(largest_amount_for_bot(b, vendor), 
	             electricity_needed);
	
	price += (abs(vendor->price) * supply);
	
	return price;
}

int max_amount(int accessible, int needed) {
	int conclusion = needed;
	
	if (accessible < needed) {
		
		conclusion = accessible;
	}
	
	return conclusion;
}

int largest_amount_for_bot(struct bot *b, struct location *location) {
	int largest_amt_manageable = max_amount(location->quantity * 
	                             abs(location->price), b->cash);
	
	return (largest_amt_manageable / abs(location->price));
}

int free_room(struct bot *b) {
	
	return (b->maximum_fruit_kg - b->fruit_kg);
}

int move_charge(struct bot *b, struct location *location) {
	struct location *elec_depot[MAX_LOCATIONS];
	int distance_till_refuel = electricity_depots(b, elec_depot);
	int var_y = 0;
	int full_cost = 0;
	
	while(var_y < distance_till_refuel) {
		full_cost += abs((elec_depot[var_y])->price);
		
		var_y = (var_y + 1);
	}
	
	int mean_refuel_cost = (full_cost / var_y);
	
	return (mean_refuel_cost * 1 * minimum_displacement(b->location, location));
}

// Calculates the current battery percentage of the bot
double current_battery_quota(struct bot *b) {
	
	return (((b->battery_level * 1.0) / b->battery_capacity) * MAX);
}
