/**
 * comp2017 - assignment 2
 * Thomas Ryan
 * trya5786
 */\

//Use testscript using ./testscript.sh

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

#include "ymirdb.h"

int compare_str(const void* a, const void* b) {
	/*
	Comparator for strings
	*/
    return strcmp(* (char * const *) a, * (char * const *) b);
}

//bubbleSort function appropriated from https://www.geeksforgeeks.org/c-program-bubble-sort-linked-list/
void bubbleSort(element* head) {
    int swapped;
    element* ptr1;
    element* lptr = NULL;
	int temp;
  
    /* Checking for empty list */
    if (head == NULL)
        return;
  
    do {
        swapped = 0;
        ptr1 = head;
  
        while (ptr1->next != lptr)
        {
            if (ptr1->value > ptr1->next->value)
            { 
                temp = ptr1->value;
				ptr1->value = ptr1->next->value;
				ptr1->next->value = temp;
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    }
    while (swapped);
}

element* element_init(int type, int value, entry* entry) {
	//Initialises elements
	element* new = (element*)malloc(sizeof(element));
	if (new == NULL) {
		return NULL;
	}
	new->type = type;
	if (!type)
		new->value = value;
	else
		new->entry = entry;
	new->next = NULL;
	new->prev = NULL;
	return new;
}

entry* entry_init(char* key) {
	//Initialises entries
	if (strlen(key) > MAX_KEY) {
		return NULL;
	}
	entry* new = (entry*)malloc(sizeof(entry));
	if (new == NULL) {
		return NULL;
	}
	strcpy(new->key, key);
	new->next = NULL;
	new->prev = NULL;
	new->head = NULL;
	new->length = 0;
	new->is_simple = 1;

	new->forward_max = 5;
	new->forward_size = 0;
	new->forward = (entry**)calloc(new->forward_max, sizeof(entry*));

	new->backward_max = 5;
	new->backward_size = 0;
	//Can just do with malloc, dont need to initalise things as zero
	new->backward = (entry**)calloc(new->backward_max, sizeof(entry*));
	return new;
}

snapshot* snapshot_init(int id) {
	//Initialises snapshot
	snapshot* new = (snapshot*)malloc(sizeof(snapshot));
	if (new == NULL) {
		return NULL;
	}
	new->id = id;
	new->entries = NULL;
	new->next = NULL;
	new->prev = NULL;
	return new;
}

void delete_snapshots(snapshot* head) {
	//Snapshots from the head onwards will be freed
	snapshot* current = head;
	if (current->prev != NULL) {
		current->prev->next = NULL;
	}
	while (current->next != NULL) {
		current = current->next;
		if (current->prev->entries != NULL)
			delete_all_entrys(current->prev->entries);
		free(current->prev);
	}
	if (current->entries != NULL)
		delete_all_entrys(current->entries);
	// printf("%d\n", current->id);
	free(current);
}

void delete_snapshot(snapshot* snap) {
	//Removes the snapshot and preserves the linked list.
	//Since snapshots have an inaccessible head, the snapshot deleted by the user will never be the head.
	snap->prev->next = snap->next;
	if (snap->next != NULL) {
		snap->next->prev = snap->prev;
	}
	delete_all_entrys(snap->entries);
	free(snap);

}


void delete_entry(entry* ptr) {
	/*
	Removes the heap memory associated with this entry and all elements within it,
	the linked list is preserved.
	The head of the entry will never be the an interactable entry as a dummy entry is
	initialised at the start of the program.
	*/
	// printf("%s\n", ptr->key);
	if (ptr->prev != NULL) {
		ptr->prev->next = ptr->next;
	}
	if (ptr->next != NULL)
		ptr->next->prev = ptr->prev;
	if (ptr->head != NULL) {
		delete_all_elements(ptr, ptr->head);
	}
	free(ptr->backward);
	free(ptr->forward);
	free(ptr);
}

void delete_all_entrys(entry* ptr) {
	//All elements that are apart of this linked list will be destroyed.
	//Iterate to the start of the list.
	while (true) {
		while (ptr->prev != NULL) {
			ptr = ptr->prev;
		}
		//Go to the next and free the previous entry

		while (ptr->next != NULL) {
			ptr = ptr->next;
			// printf("%p : %s\n", ptr->prev, ptr->prev->key);
			delete_entry(ptr->prev); 
		}
		if (ptr->prev != NULL) {
			ptr = ptr->prev;
			delete_entry(ptr->next);	
			continue;
		}
		else {
			break;
		}
	}
	//Loop breaks at last element so now free the last element.
	delete_entry(ptr);
}

void delete_element(entry* entry, element* ptr){
	//Deletes an element, ensuring that the linked list is preserved.
	
	//If this element is the head
	if (entry->head == ptr) {
		//If the last element in this entry
		if (ptr->next == NULL) {
			entry->head = NULL;
			free(ptr);
			update_length(entry, entry->head);	
			return;
		}
		//Otherwise
		entry->head = ptr->next;
		ptr->next->prev = NULL;
		free(ptr);
		update_length(entry, entry->head);
		update_type(entry);
	} 
	else {
		//If this element is in the middle or at the end
		ptr->prev->next = ptr->next;
		if (ptr->next != NULL)
			ptr->next->prev = ptr->prev;
		free(ptr);
		update_length(entry, entry->head);
		update_type(entry);
	}
}

void delete_all_elements(entry* entry, element* ptr) {
	//All elements that are apart of this linked list will be destroyed.
	//Iterate to the start of the list.
	while (ptr->prev != NULL) {
		ptr = ptr->prev;
	}
	//Go to the next and free the previous element
	while (ptr->next != NULL) {
		ptr = ptr->next;
		delete_element(entry, ptr->prev);
	}
	//Loop breaks at last element so now free the last element.
	delete_element(entry, ptr);
}

int type_id(char* element) {
	//Returns 1 if number, 2 if key, 
	//0 if error or bad input.
	//Figures out whether the given element is number or letter

	//If first char is a letter then it can be assumed it is a key.
	if (isalpha(*element)) {
		return 2;
	}
	else {
		//If at any point there is not a digit in the element it is an error
		for (int i = 0; i < strlen(element); i++) {
			if (isdigit(*(element + i)) == 0 && (unsigned char)*element != '-') {
				return 0;
			}
		}
		return 1;
	}
}

int check_key_valid(char* key) {
	//Key is valid if it is at most 15 characters long
	//and starts with an alphabetical char.
	//Returns 0 if invalid and 1 if valid
	if (key == NULL) {
		return 0;
	}
	if (strlen(key) > 15) {
		return 0;
	} else if (isalpha(*key) == 0) {
		return 0;
	}
	int i = 0;
	while(true) {
		if (*(key + i) == '\0') {
			break;
		}
		if (isalnum(*(key + i)) == 0){
			return 0;
		}
		i ++;
	}
	return 1;
}

entry* check_key_exists(entry* head, char* key) {
	//Checks if key exists. 
	//Returns the address of where the key exists if it does,
	//Otherwise returns NULL.
	entry* curr_entry = head;
	while (true) {
		// printf("%s\n", curr_entry->key);
		if (strcmp(key, curr_entry->key) == 0){
			return curr_entry;
		}
		if (curr_entry->next == NULL) {
			break;
		}
		curr_entry = curr_entry->next;
	}
	return NULL;
}

void update_length(entry* entry, element* head) {
	long unsigned int length = 0;
	element* current = head;
	if (current != NULL) {
		while (current->next != NULL) {
			length++;
			current = current->next;
		}
		length++;
	}
	entry->length = length;
}


//ADD GENERAL, TRY TO FIGURE OUT ENTRY**
element* check_element_valid(entry* head, char* key, char* current_element) {
	//Checks if the element is valid. Returns the element* if it is,
	//if it is not then NULL is returned.
	int curr_type = type_id(current_element);
	int value;
	entry* entry;
	if (curr_type == 1) {
		value = atoi(current_element);
		return element_init(0, value, NULL);
	} else if (curr_type == 2) {
		if (check_key_valid(current_element) == 0) {
			printf("no such key\n\n");
			return NULL;
		} else {
			//Check that the element is not its own entry.
			if (strcmp(current_element, key) == 0){
				printf("not permitted\n\n");
				return NULL;
			}
			//If it is not, then check that the entry actually exists.
			entry = check_key_exists(head, current_element);
			if (entry == NULL) {
				printf("no such key\n\n");
				return NULL;
			}
			//Entry exists and is not its own entry so the element is valid
			return element_init(1, -1, entry);
		}
	} else {
		printf("invalid Command\n\n");
		return NULL;
	}
}

/*
For creating a snapshot:
In order to copy a new state, first new entires are copied,
then the elements for those entries, then the forwards and
backwards for those entries are updated.
*/

entry* copy_entries(entry* to_copy_head) {
	entry* current = to_copy_head;
	entry* previous = NULL;
	entry* new_head = NULL;
	while (current != NULL) {
		entry* new = entry_init(current->key);
		if (previous == NULL) {
			previous = new;
			new_head = new;
		} else {
			new->prev = previous;
			previous->next = new;
			previous = new;
		}
		current = current->next;
	}

	// printf("new: %p, old: %p", &(new_head), &(to_copy_head));
	return new_head;
}

element* copy_elements(entry* copy_to_head, element* to_copy_head) {
	/*
	Copying all elements from to_copy_head into entrys from the copy_to_head
	entry linked list. copy_to_head is to ensure that entry values that are 
	assigned to elements are from the copy and not from the original.
	*/
	element* current = to_copy_head;
	element* previous = NULL;
	element* new_head = NULL;
	while (current != NULL) {
		entry* to_copy = NULL;
		if (current->type == 1){
			to_copy = check_key_exists(copy_to_head, current->entry->key);
		}
		element* new = element_init(current->type, current->value, to_copy);
		
		if (previous == NULL) {
			previous = new;
			new_head = new;
		} else {
			new->prev = previous;
			previous->next = new;
			previous = new;
		}
		current = current->next;
	}
	return new_head;
}

void update_all_fwd_and_bck(entry* head) {
	//Call this function to update all forwards and backwards references
	entry* current = head;
	while (current != NULL) {
		// printf("Updating forward of %s\n", current->key);
		update_fwd(current);
		current = current->next;
	}
	//Free back references in preperation for new back references.
	current = head;
	while (current != NULL) {
		free(current->backward);
		current->backward_size = 0;
		current->backward = (entry**)malloc(sizeof(entry*)*current->backward_max);
		current = current->next;
	}
	current = head;
	while (current != NULL) {
		update_bck(current);
		current = current->next;
	}
}

void add_fwd(entry* e, entry* fwd_ref) {
	/*
	Adds a forwards reference ensuring the array is large enough to fit
	all elements.
	*/
	//Check if fwd ref already exists
	for (int i = 0; i < e->forward_size; i++) {
		if (e->forward[i] == fwd_ref) {
			return;
		}
	}
	if (e->forward_size == e->forward_max) {
		e->forward_max = e->forward_max*2;
		e->forward = realloc(e->forward, sizeof(entry*)*e->forward_max);
	}
	// printf("adding %s to %s\n", fwd_ref->key, e->key);
	e->forward[e->forward_size] = fwd_ref;
	e->forward_size++;
}

void update_fwd(entry* e) {
	/*
	This function updates the forward references for an entry
	*/
	//Free the old forward entries.
	free(e->forward);
	e->forward_size = 0;
	e->forward = (entry**)malloc(sizeof(entry*)*e->forward_max);
	element* current = e->head;
	//This loop checks this entrys own elements
	while (current != NULL) {
		if (current->type == 1) {
			recursive_fwd_check(e, current->entry);
			add_fwd(e, current->entry);
		}
		current = current->next;
	}
}

void recursive_fwd_check(entry* original_e, entry* e_getting_checked) {
	/*
	This function goes through and checks the entry element that is passed
	into it for its own forwards references.
	*/
	element* current = e_getting_checked->head;
	while (current != NULL) {
		if (current->type == 1) {
			recursive_fwd_check(original_e, current->entry);
			add_fwd(original_e, current->entry);
		}
		current = current->next;
	}
}

void add_bck(entry* e, entry* bck_ref) {
	//Check if there is already a backwards refernece, if so, do not add new.
	for (int i = 0; i < e->backward_size; i++) {
		if (e->backward[i] == bck_ref) {
			return;
		}
	}
	if (e->backward_size == e->backward_max) {
		e->backward_max = e->backward_max*2;
		e->backward = realloc(e->backward, sizeof(entry*)*e->backward_max);
	}
	// printf("adding %s to %s\n", fwd_ref->key, e->key);
	e->backward[e->backward_size] = bck_ref;
	e->backward_size++;
}

void update_bck(entry* e) {
	entry* current = e;
	while (current != NULL) {
		/*
		Add this entry as a back reference to all the entries it is
		forward dependent on
		*/
		for (int i = 0; i < current->forward_size; i++) {
			add_bck(current->forward[i], current);
		}
		current = current->next;
	}
}

void update_type(entry* entry) {
	//Updates the type of an entry
	element* current = entry->head;
	entry->is_simple = 1;
	while (current != NULL) {
		if (current->type == 1) {
			entry->is_simple = 0;
		}
		current = current->next;
	}
}

void cmd_set(entry* head, char* arguments) {
	//Create entry which will hold the element values.
	//Checks key is valid
	char* key = strtok(arguments, " \n");
	if (!check_key_valid(key)) {
		printf("key is not permitted\n\n");
		return;
	}
	//Check elements are valid, then create and link them if they are.
	element* element_head = check_element_valid(head, key, strtok(NULL, " \n"));
	if (element_head == NULL) {
		return;
	}
	char* current_string;
	element* current_element;
	element* previous_element;
	while (true) {
		//If strtok returns null then all the elements have been checked
		current_string = strtok(NULL, " \n");
		if (current_string == NULL) {
			break;
		}
		//Element exists so check if it is valid
		current_element = check_element_valid(head, key, current_string);
		//return if element is invalid
		if (current_element == NULL) {
			delete_all_elements(NULL, element_head);
			return;
		}
		if (element_head-> next == NULL) {
			element_head->next = current_element;
			current_element->prev = element_head;
			previous_element = current_element;
			continue;
		} else {
			previous_element->next = current_element;
			current_element->prev = previous_element;
			previous_element = current_element;
			continue;
		}
	}
	//Now all elements are valid and linked together
	
	//Check if key already exists
	entry* keyaddress = check_key_exists(head, key);
	if (keyaddress != NULL) {
		/*
		Key exists and elements are all valid, 
		remove the old elements from memory and,
		the new elements are linked to the entry.
		*/
		delete_all_elements(keyaddress, keyaddress->head);
		keyaddress->head = element_head;
		update_all_fwd_and_bck(head);
		update_length(keyaddress, element_head);
		printf("ok\n\n");
		return;
	} else {
		//Key does not exist so it is created
		entry* current_entry = head;
		//Iterate to the end of the linked list
		while (current_entry->next != NULL) {
			current_entry = current_entry->next;
		}
		current_entry->next = entry_init(key);
		current_entry->next->prev = current_entry;
		current_entry->next->head = element_head;
		update_all_fwd_and_bck(head);
		update_length(current_entry->next, element_head);
		//New entry is created at the end of the entry list, then its elements are linked.
	}
	printf("ok\n\n");
}

void cmd_get(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	element* element = entry->head;
	printf("[");
	while (true) {
		if (element == NULL) {
			break;
		}
		if (element->type == 0) {
			printf("%d", element->value);
		} else if (element->type == 1) {
			printf("%s", element->entry->key);
		}
		if (element->next == NULL){
			break;
		} else {
			printf(" ");
			element = element->next;
		}
	}
	printf("]\n");
}

void list_redirect(entry* head, char* argument, snapshot* shead) {
	/*
	This is a helper function to redirect the LIST ... command to 
	its respective function
	*/
	//Makes the command case insensitive
	char* initialptr = argument;
	while (*argument) {
		*argument = toupper((unsigned char) *argument);
		argument ++;
	}
	argument = initialptr;

	if (strcmp("KEYS", argument) == 0) {
		cmd_list_keys(head);
	}

	else if (strcmp("ENTRIES", argument) == 0) {
		cmd_list_entries(head);
	}

	else if (strcmp("SNAPSHOTS", argument) == 0) {
		cmd_list_snapshots(shead);
		printf("\n");
	}

	else {
		printf("Unknown input\n\n");
	}
}

void cmd_list_keys(entry* head) {
	if (head->next == NULL) {
		printf("no keys\n\n");
		return;
	}
	entry* tail = head;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	//Tail is now at the end of the linked list.
	while (strcmp(tail->key, "1") != 0) {
		printf("%s\n", tail->key);
		tail = tail->prev;
	}
	printf("\n");
	return;
}

void cmd_list_entries(entry* head){
	if (head->next == NULL) {
		printf("no entries\n\n");
		return;
	}
	entry* tail = head;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	//Tail is now at the end of the linked list.
	while (strcmp(tail->key, "1") != 0) {
		printf("%s ", tail->key);
		cmd_get(head, tail->key);
		tail = tail->prev;
	}
	printf("\n");
	return;
}


void cmd_list_snapshots(snapshot* head){
	if (head->next == NULL) {
		printf("no snapshots\n");
		return;
	}
	//Iterate to the end of the linked list
	snapshot* current = head->next;
	while (current->next != NULL) {
		current = current->next;
	}
	while (current->id != 0) {
		printf("%d\n", current->id);
		current = current->prev;
	}
}

void cmd_delete(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	if (entry->backward_size > 0) {
		printf("not permitted\n");
		return;
	}
	delete_entry(entry);
	update_all_fwd_and_bck(head);
	printf("ok\n");
}

void cmd_purge(entry* head, snapshot* snap_head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry != NULL) {
		if (entry->backward_size > 0) {
			printf("not permitted\n");
			return;
		}
	}
	/*
	Check if in any snapshot this entry has a backward reference
	if so, purge is not permitted
	*/
	snapshot* current = snap_head;
	struct entry* snap_entry;
	while (current != NULL) {
		if (current->entries != NULL) {
			snap_entry = check_key_exists(current->entries, key);
			if (snap_entry == NULL) {
				current = current->next;
				continue;
			} else if (snap_entry->backward_size > 0) {
				printf("not permitted\n");
				return;
			} 
		}
		current = current->next;
	}
	/*
	Purge is permitted, so delete entry from all snapshots and
	update their fwd and bck refs
	*/
	current = snap_head;
	snap_entry = NULL;
	while (current != NULL) {
		if (current->entries != NULL) {
			snap_entry = check_key_exists(current->entries, key);
			if (snap_entry == NULL) {
				current = current->next;
				continue;
			} else {
				delete_entry(snap_entry);
				update_all_fwd_and_bck(current->entries);
			} 
		}
		current = current->next;
	}
	//Delete entry from current working environment
	if (entry != NULL) {
		delete_entry(entry);
		update_all_fwd_and_bck(head);
	}
	printf("ok\n");
}

void cmd_push(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	element* element_head = check_element_valid(head, key, strtok(NULL, " \n"));
	if (element_head == NULL) {
		return;
	}
	char* current_string;
	element* current_element;
	while (true) {
		//If strtok returns null then all the elements have been checked.
		current_string = strtok(NULL, " \n");
		if (current_string == NULL) {
			break;
		}
		//Element exists so check if it is valid
		current_element = check_element_valid(head, key, current_string);
		//return if element is invalid
		if (current_element == NULL) {
			delete_all_elements(NULL, element_head);
			return;
		}
		element_head->prev = current_element;
		current_element->next = element_head;
		element_head = current_element;
		continue;
	}
	//All elements are valid, now attach them to the start of the entry.
	element* final_element = element_head;
	while (final_element->next != NULL) {
		final_element = final_element->next;
	}
	if (entry->head != NULL)
		entry->head->prev = final_element;
	final_element->next = entry->head;
	entry->head = element_head;
	update_all_fwd_and_bck(head);
	update_length(entry, entry->head);
	printf("ok\n");
}

void cmd_append(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	element* element_head = check_element_valid(head, key, strtok(NULL, " \n"));
	if (element_head == NULL) {
		return;
	}
	char* current_string;
	element* current_element;
	element* previous_element;
	while (true) {
		//If strtok returns null then all the elements have been checked.
		current_string = strtok(NULL, " \n");
		if (current_string == NULL) {
			break;
		}
		//Element exists so check if it is valid
		current_element = check_element_valid(head, key, current_string);
		//return if element is invalid
		if (current_element == NULL) {
			delete_all_elements(NULL, element_head);
			return;
		}
		if (element_head-> next == NULL) {
			element_head->next = current_element;
			current_element->prev = element_head;
			previous_element = current_element;
			continue;
		} else {
			previous_element->next = current_element;
			current_element->prev = previous_element;
			previous_element = current_element;
			continue;
		}
	}
	//All elements are valid, now attach them to the end of the entry.
	element* tail = entry->head;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = element_head;
	element_head->prev = tail;
	update_all_fwd_and_bck(head);
	update_length(entry, entry->head);
	printf("ok\n");
}

void cmd_pick(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	long unsigned int index = atoi(strtok(NULL, " \n"));
	if (index > entry->length || index <= 0) {
		printf("index out of range\n");
		return;
	}
	element* element = entry->head;
	for (int i = 0; i < index-1; i++) {
		element = element->next;
	}
	if (element->type == 0) {
		printf("%d\n", element->value);
	} else if (element->type == 1) {
		printf("%s\n", element->entry->key);
	}
}

void cmd_pluck(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	long unsigned int index = atoi(strtok(NULL, " \n"));
	if (index > entry->length || index <= 0) {
		printf("index out of range\n");
		return;
	}
	element* element = entry->head;
	for (int i = 0; i < index-1; i++) {
		element = element->next;
	}
	if (element->type == 0) {
		printf("%d\n", element->value);
		delete_element(entry, element);
		update_length(entry, entry->head);
	} else if (element->type == 1) {
		printf("%s\n", element->entry->key);
		delete_element(entry, element);
		update_all_fwd_and_bck(head);
		update_length(entry, entry->head);
	}
}

void cmd_pop(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	if (entry->length == 0) {
		printf("nil\n");
		return;
	}
	element* element = entry->head;
	if (element->type == 0) {
		printf("%d\n", element->value);
		delete_element(entry, element);
		update_length(entry, entry->head);
	} else if (element->type == 1) {
		printf("%s\n", element->entry->key);
		delete_element(entry, element);
		update_all_fwd_and_bck(head);
		update_length(entry, entry->head);
	}
}


int cmd_max(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return INT_MIN;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return INT_MIN;
	}
	element* element = entry->head;
	int largest = INT_MIN;
	while (true) {
		if (element->type == 1) {
			int val = cmd_max(head, element->entry->key);
			if (largest < val) {
				largest = val;
			}
		} else if (largest < element->value) {
				largest = element->value;
		}
		if (element->next == NULL) {
			break;
		}
		element = element->next;
	}
	return largest;
}

int cmd_min(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return INT_MAX;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return INT_MAX;
	}
	element* element = entry->head;
	int smallest = INT_MAX;
	while (true) {
		if (element->type == 1) {
			int val = cmd_min(head, element->entry->key);
			if (smallest > val) {
				smallest = val;
			}
		} else if (smallest > element->value) {
				smallest = element->value;
		}
		if (element->next == NULL) {
			break;
		}
		element = element->next;
	}
	return smallest;
}

int cmd_sum(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return INT_MIN;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return INT_MIN;
	}
	element* element = entry->head;
	int sum = 0;
	while (true) {
		if (element->type == 1) {
			int val = cmd_sum(head, element->entry->key);
			sum += val;
		} else {
				sum += element->value;
		}
		if (element->next == NULL) {
			break;
		}
		element = element->next;
	}
	return sum;
}

int cmd_len(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return INT_MIN;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return INT_MIN;
	}
	element* element = entry->head;
	int sum = 0;
	while (true) {
		if (element->type == 1) {
			int val = cmd_len(head, element->entry->key);
			sum += val;
		} else {
				sum ++;
		}
		if (element->next == NULL) {
			break;
		}
		element = element->next;
	}
	return sum;
}

void cmd_rev(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	element* temp = NULL;
	element* current = entry->head;
	while (current != NULL) {
    	temp = current->prev;
    	current->prev = current->next;
    	current->next = temp;             
    	current = current->prev;
    }     
      
    /*
	Before changing head, check for the cases like empty
    list and list with only one node 
	*/
    if(temp != NULL ) {
        entry->head = temp->prev;
		printf("ok\n");
	} else {
		printf("ok\n");
	}
}

void cmd_uniq(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	element* current = entry->head;
	while (true) {
		if (current->next == NULL) {
			break;
		}
		if (current->type == 0 && current->next->type == 0) {
			if (current->value == current->next->value) {
				delete_element(entry, current->next);
				continue;
			}
		}
		current = current->next;
	}
	printf("ok\n");
}

void cmd_sort(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	bubbleSort(entry->head);
	printf("ok\n");
}

void cmd_backward(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	if (entry->backward_size == 0) {
		printf("nil\n");
		return;
	}
	char* keys[entry->backward_size];

	for (int i = 0; i < entry->backward_size; i++) {
		keys[i] = entry->backward[i]->key;
	}
	qsort(keys, entry->backward_size, sizeof(char*), compare_str);
	for (int i = 0; i < entry->backward_size; i++) {
		printf("%s", keys[i]);
		if (i + 1 != entry->backward_size) {
			printf(", ");
		}
	}
	printf("\n");
}

void cmd_forwards(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	if (entry->forward_size == 0) {
		printf("nil\n");
		return;
	}
	char* keys[entry->forward_size];

	for (int i = 0; i < entry->forward_size; i++) {
		keys[i] = entry->forward[i]->key;
	}
	qsort(keys, entry->forward_size, sizeof(char*), compare_str);
	for (int i = 0; i < entry->forward_size; i++) {
		printf("%s", keys[i]);
		if (i + 1 != entry->forward_size) {
			printf(", ");
		}
	}
	printf("\n");
}

void cmd_type(entry* head, char* arguments) {
	char* key = strtok(arguments, " \n");
	if (check_key_valid(key) == 0) {
		printf("Invalid input\n");
		return;
	}
	entry* entry = check_key_exists(head, key);
	if (entry == NULL) {
		printf("no such key\n");
		return;
	}
	update_type(entry);
	if (entry->is_simple) {
		printf("simple\n");
	} else {
		printf("general\n");
	}
}

void cmd_snapshot(snapshot* snap_head, entry* ent_head, int id) {
	//Create new snapshot id
	snapshot* current = snap_head;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = snapshot_init(id);
	current->next->prev = current;
	snapshot* new_snap = current->next;
	//Copy all current memory somewhere else in program.

	//Copy each entries attributes ie keys, length..
	new_snap->entries = copy_entries(ent_head);
	
	//Copy new elements from each entry to new entry.
	entry* current_copy_from = ent_head;
	entry* current_copy_to = new_snap->entries;
	while (current_copy_from != NULL) {
		current_copy_to->head = copy_elements(new_snap->entries, current_copy_from->head);
		update_length(current_copy_to, current_copy_to->head);
		update_type(current_copy_to);
		current_copy_from = current_copy_from->next;
		current_copy_to = current_copy_to->next;
	}

	update_all_fwd_and_bck(new_snap->entries);

	printf("saved as snapshot %d\n", new_snap->id);
}

snapshot* check_id(snapshot* head, int id) {
	//Return NULL if error
	//returns address if it exists
	if (id == 0) {
		return NULL;
	}
	snapshot* current = head;
	while (current != NULL) {
		if (current->id == id) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}

entry* cmd_checkout(entry* ent_head, snapshot* head, char* arguments) {
	/*
	Copies all entrys and elements from snapshot to current working environment
	*/
	char* arg = strtok(arguments, " \n");
	int id;
	if (type_id(arg) == 1) {
		id = atoi(arg);
	} else {
		printf("no such snapshot\n");
		return ent_head;
	}
	snapshot* snap = check_id(head, id);
	if (snap == NULL) {
		printf("no such snapshot\n");
		return ent_head;
	}
	delete_all_entrys(ent_head);

	//COPY TO WORKING DIRECTORY

	entry* copy_to_head = copy_entries(snap->entries);
	
	//Copy new elements from each entry to new entry.
	entry* current_copy_from = snap->entries;
	entry* current_copy_to = copy_to_head;
	while (current_copy_from != NULL) {
		current_copy_to->head = copy_elements(copy_to_head, current_copy_from->head);
		update_length(current_copy_to, current_copy_to->head);
		update_type(current_copy_to);
		current_copy_from = current_copy_from->next;
		current_copy_to = current_copy_to->next;
	}

	update_all_fwd_and_bck(copy_to_head);
	printf("ok\n");
	return copy_to_head;
}

void cmd_drop(snapshot* head, char* arguments) {
	char* arg = strtok(arguments, " \n");
	int id;
	if (type_id(arg) == 1) {
		id = atoi(arg);
	} else {
		printf("no such snapshot\n");
		return;
	}
	snapshot* snap = check_id(head, id);
	if (snap == NULL) {
		printf("no such snapshot\n");
		return;
	}
	delete_snapshot(snap);
	printf("ok\n");
}

entry* cmd_rollback(entry* ent_head, snapshot* head, char* arguments) {
	char* arg = strtok(arguments, " \n");
	int id;
	if (type_id(arg) == 1) {
		id = atoi(arg);
	} else {
		printf("no such snapshot\n");
		return ent_head;
	}
	snapshot* snap = check_id(head, id);
	if (snap == NULL) {
		printf("no such snapshot\n");
		return ent_head;
	}
	delete_snapshots(snap->next);
	return cmd_checkout(ent_head, head, arguments);
}



void cmd_bye() {
	printf("bye\n");
}

void cmd_help() {
	printf("%s\n", HELP);
}

int main(void) {

	entry* ent_head = entry_init("1");
	snapshot* snap_head = snapshot_init(0);
	int id = 1;
	char line[MAX_LINE];

	while (true) {
		printf("> ");

		//Takes input
		if (NULL == fgets(line, MAX_LINE, stdin)) {
			printf("\n");
			delete_all_entrys(ent_head);
			delete_snapshots(snap_head);
			cmd_bye();
			return 0;
		} else if (*line == '\n') {
			continue;
		}

		char* cmd = strtok(line, " \n");
		char* arguments = strtok(NULL, "\n");

		//Makes the command case insensitive
		char* initialptr = cmd;
		while (*cmd) {
			*cmd = toupper((unsigned char) *cmd);
			cmd ++;
		}
		cmd = initialptr;

		//printf("Command: %s, Arguments: %s", cmd, arguments);

		if (strcmp("SET", cmd) == 0) {
			cmd_set(ent_head, arguments);
			continue;
		}

		else if (strcmp("HELP", cmd) == 0){
			cmd_help();
			continue;
		}

		else if (strcmp("GET", cmd) == 0) {
			cmd_get(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("BYE", cmd) == 0) {
			delete_all_entrys(ent_head);
			delete_snapshots(snap_head);
			cmd_bye();
			return 0;
		}

		else if (strcmp("LIST", cmd) == 0) {
			list_redirect(ent_head, arguments, snap_head);
			continue;
		}

		else if (strcmp("DEL", cmd) == 0) {
			cmd_delete(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("PUSH", cmd) == 0) {
			cmd_push(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("APPEND", cmd) == 0) {
			cmd_append(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("PICK", cmd) == 0) {
			cmd_pick(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("PLUCK", cmd) == 0) {
			cmd_pluck(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("POP", cmd) == 0) {
			cmd_pop(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("MAX", cmd) == 0) {
			int val = cmd_max(ent_head, arguments);
			if (val == INT_MIN) {
				printf("\n");
			} else {
				printf("%d\n\n", val);
			}
		}
		
		else if (strcmp("MIN", cmd) == 0) {
			int val = cmd_min(ent_head, arguments);
			if (val == INT_MAX) {
				printf("\n");
				continue;
			} else {
				printf("%d\n\n", val);
				continue;
			}
		}
		
		else if (strcmp("SUM", cmd) == 0) {
			int val = cmd_sum(ent_head, arguments);
			if (val == INT_MIN) {
				printf("\n");
				continue;
			} else {
				printf("%d\n\n", val);
				continue;
			}
		}

		else if (strcmp("LEN", cmd) == 0) {
			int val = cmd_len(ent_head, arguments);
			if (val == INT_MIN) {
				printf("\n");
				continue;
			} else {
				printf("%d\n\n", val);
				continue;
			}
		}

		else if (strcmp("REV", cmd) == 0) {
			cmd_rev(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("UNIQ", cmd) == 0) {
			cmd_uniq(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("SORT", cmd) == 0) {
			cmd_sort(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("BACKWARD", cmd) == 0) {
			cmd_backward(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("FORWARD", cmd) == 0) {
			cmd_forwards(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("TYPE", cmd) == 0) {
			cmd_type(ent_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("PURGE", cmd) == 0) {
			cmd_purge(ent_head, snap_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("SNAPSHOT", cmd) == 0) {
			cmd_snapshot(snap_head, ent_head, id);
			id++;
			printf("\n");
			continue;
		}

		else if (strcmp("CHECKOUT", cmd) == 0) {
			ent_head = cmd_checkout(ent_head, snap_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("DROP", cmd) == 0) {
			cmd_drop(snap_head, arguments);
			printf("\n");
			continue;
		}

		else if (strcmp("ROLLBACK", cmd) == 0) {
			ent_head = cmd_rollback(ent_head, snap_head, arguments);
			printf("\n");
			continue;
		}

		else {
			printf("Unknown input\n\n");
		}
  	}
	return 0;
}