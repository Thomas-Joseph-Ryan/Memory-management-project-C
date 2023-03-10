#ifndef YMIRDB_H
#define YMIRDB_H

#define MAX_KEY 16
#define MAX_LINE 1024

#include <stddef.h>
#include <sys/types.h>

enum item_type {
    INTEGER=0,
    ENTRY=1
};

typedef struct element element;
typedef struct entry entry;
typedef struct snapshot snapshot;

struct element {
  enum item_type type;
  element* prev;
  element* next;
  union {
    int value;
    struct entry *entry;
  };
};

struct entry {
  char key[MAX_KEY];
  size_t is_simple;
  element * head;
  size_t length;

  entry* next;
  entry* prev;

  size_t forward_size;
  size_t forward_max;
  entry** forward;
    
  size_t backward_size; 
  size_t backward_max; 
  entry** backward; // these entries depend on this
};

struct snapshot {
  int id;
  entry* entries;
  snapshot* next;
  snapshot* prev;
};


const char* HELP =
	"BYE   clear database and exit\n"
	"HELP  display this help message\n"
	"\n"
	"LIST KEYS       displays all keys in current state\n"
	"LIST ENTRIES    displays all entries in current state\n"
	"LIST SNAPSHOTS  displays all snapshots in the database\n"
	"\n"
	"GET <key>    displays entry values\n"
	"DEL <key>    deletes entry from current state\n"
	"PURGE <key>  deletes entry from current state and snapshots\n"
	"\n"
	"SET <key> <value ...>     sets entry values\n"
	"PUSH <key> <value ...>    pushes values to the front\n"
	"APPEND <key> <value ...>  appends values to the back\n"
	"\n"
	"PICK <key> <index>   displays value at index\n"
	"PLUCK <key> <index>  displays and removes value at index\n"
	"POP <key>            displays and removes the front value\n"
	"\n"
	"DROP <id>      deletes snapshot\n"
	"ROLLBACK <id>  restores to snapshot and deletes newer snapshots\n"
	"CHECKOUT <id>  replaces current state with a copy of snapshot\n"
	"SNAPSHOT       saves the current state as a snapshot\n"
	"\n"
	"MIN <key>  displays minimum value\n"
	"MAX <key>  displays maximum value\n"
	"SUM <key>  displays sum of values\n"
	"LEN <key>  displays number of values\n"
	"\n"
	"REV <key>   reverses order of values (simple entry only)\n"
	"UNIQ <key>  removes repeated adjacent values (simple entry only)\n"
	"SORT <key>  sorts values in ascending order (simple entry only)\n"
	"\n"
	"FORWARD <key> lists all the forward references of this key\n"
	"BACKWARD <key> lists all the backward references of this key\n"
	"TYPE <key> displays if the entry of this key is simple or general\n";


element* element_init(int type, int value, entry* entry);
entry* entry_init(char* key);
snapshot* snapshot_init();
void update_backwards(entry* dependant, entry* entry);
void delete_entry(entry* ptr);
void delete_all_entrys(entry* ptr);
void delete_element(entry* entry, element* ptr);
void delete_all_elements(entry* entry, element* ptr);
int type_id(char* element);
int check_key_valid(char* key);
entry* check_key_exists(entry* head, char* key);
element* check_element_valid(entry* head, char* key, char* current_element);
void cmd_set(entry* head, char* arguments);
void cmd_get(entry* head, char* arguments);
void cmd_list_keys();
void cmd_list_entries();
void cmd_list_snapshots();
void cmd_bye();
void cmd_help();
void update_length(entry* entry, element* head);
void add_backwards(entry* dependant, entry* entry);
void delete_backwards(entry* dependant, entry* entry);
void update_type(entry* entry);
void add_forwards(entry* o_entry, entry* entry);
void delete_forwards(entry* o_entry, entry* entry);
void add_forwards_wrapper(entry* o_entry, entry* r_entry);
void update_all_fwd_and_bck(entry* head);
void add_fwd(entry* e, entry* fwd_ref);
void update_fwd(entry* e);
void recursive_fwd_check(entry* original_e, entry* e_getting_checked);
void update_bck(entry* e);

#endif
