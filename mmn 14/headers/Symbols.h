#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stdio.h>
#include <stdlib.h>
#include "First_pass.h"
#include "Global_def.h"


/**
 * @brief Represents a node in the symbols linked list.
 *
 * This structure holds information about a symbol, including its name, value,
 * location, update attribute, size, and line number. It also includes a flag
 * to indicate if the symbol is an entry and a pointer to the next node in the list.
 *
 * @member name The name of the symbol.
 * @member value The value associated with the symbol.
 * @member location Indicates the data/code location of the symbol.
 * @member update_attribute Attribute indicating if the symbol is external or relocatable.
 * @member size The size of the symbol.
 * @member line The line number where the symbol is defined.
 * @member is_entry Flag indicating if the symbol is an entry.
 * @member next Pointer to the next symbol node in the list.
 */
struct symbols_node {
	char *name;
	int value;
	int location;
	int update_attribute;
	int size;
	int line;
	int is_entry;
	struct symbols_node *next;
};

/**
 * @brief Represents a linked list of symbol nodes.
 *
 * This structure defines a linked list that holds symbol nodes, providing
 * a way to traverse and manage a collection of symbols.
 *
 * @member head Pointer to the first node in the linked list.
 */
struct symbols_linked_list {
	struct symbols_node *head;
	struct symbols_node *tail;
};






/**
 * @brief Initializes and allocates memory for a new linked list.
 *
 * @return Pointer to the newly created linked list.
 */

struct symbols_linked_list* create_symbols_linked_list();


/**
 * @brief Adds a new symbol node to the symbols linked list.
 *
 * This function appends a new symbol node to the end of the given symbols linked list.
 *
 * @param list Pointer to the symbols linked list where the node will be added.
 * @param new_symbol_node Pointer to the symbol node that will be inserted.
 */

void insert_new_symbol(struct symbols_linked_list *list, struct symbols_node * new_symbols_node);

/**
 * @brief Verify the presence of a symbol in the symbols linked list and handle related actions.
 *
 * This function examines whether a specified symbol is present in the symbols linked list and
 * executes actions according to the parameters provided.
 *
 * @param list Pointer to the symbols linked list to search within.
 * @param symbol_name Pointer to the name of the symbol being checked.
 * @param type An integer indicating the type of the symbol.
 * @param file Pointer to the file status structure for reporting.
 * @param errors Pointer to the errors status structure for recording errors.
 *
 * @return Returns an integer indicating the outcome of the check and actions:
 *         - TRUE if errors are detected in the label or definition.
 *         - FALSE if the symbol name is valid according to the conditions.
 */

int appear_in_symbols(struct symbols_linked_list *list, char * symbol_name,int type,struct file_status * file , errors_status *errors);
/**
 * @brief Set a symbol as an entry point in the symbols linked list.
 * 
 * This function designates a specified symbol as an entry point within the linked list of symbols.
 * 
 * @param list Pointer to the symbols linked list.
 * @param name The name of the symbol to be set as an entry point.
 * @param file Pointer to the file status structure for reporting.
 * 
 * @return Returns TRUE if the entry point is successfully assigned, or FALSE if the operation fails.
 */

int set_entry(struct symbols_linked_list * list,char * name,struct file_status * file);


/**
 * @brief Appends a new node to the end of the linked list.
 *
 * This function adds a new node with specified properties to the end of the linked list.
 *
 * @param list Pointer to the linked list to which the new node will be added.
 * @param name Pointer to a string representing the name of the symbol.
 * @param value The value associated with this symbol.
 * @param location Indicates whether the symbol is for code or data.
 * @param update_attribute Specifies if the symbol is relocatable or external.
 * @param size The amount of memory this symbol occupies.
 * @param line The line number where the symbol is declared.
 * 
 */

 int create_insert_symbol(struct symbols_linked_list * list,char *name, int value,int location,int update_attribute,long size,int line, int is_entry);

/**
 * @brief Releases the memory used by the linked list and its nodes.
 *
 * This function deallocates all memory associated with the linked list and its nodes.
 *
 * @param list Pointer to the linked list to be deallocated.
 * 
 * @return void
 */

void free_symbols_list(struct symbols_linked_list *list);

/**
 * @brief Marks a symbol as an entry point in the symbols linked list.
 * 
 * This function updates the linked list to set a symbol as an entry point based on its name.
 * 
 * @param list Pointer to the symbols linked list.
 * @param name The name of the symbol to be marked.
 * @param file Pointer to the file status structure.
 * 
 * @return Returns TRUE if the entry point was successfully set, otherwise returns FALSE.
 */

int set_entry(struct symbols_linked_list * list,char * name,struct file_status * file);

void print_list(struct symbols_linked_list *list);/*DEBUG*/



#endif /* SYMBOLS_H */
