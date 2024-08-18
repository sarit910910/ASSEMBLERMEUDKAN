#include "Errors.h"
#include "Symbols.h"
#include "Global_def.h"
#include "Global_fun.h"
#include "First_pass.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


void print_list(struct symbols_linked_list *list) {
    struct symbols_node *current = list->head;
    while (current != NULL) {
        /* Print the details of the current node */
        printf("Name: %s, Value: %d, Location: %d, Update Attribute: %d, Size: %d, Line: %d, Is Entry: %d\n",
               current->name, current->value, current->location, current->update_attribute,
               current->size, current->line, current->is_entry);
        current = current->next; /* Move to the next node */
    }
}

int appear_in_symbols(struct symbols_linked_list *list, char *symbol_name, int type, struct file_status *file, errors_status *errors) {
    struct symbols_node *current = list->head;
    while (current != NULL) {
        if (strcmp(current->name, symbol_name) == 0) {
            switch (type) {
                case REGULAR:
                    /* Mark an error for a duplicate label */
                    errors->external_error_exist = double_label;
                    print_external_error(double_label, file);
                    return TRUE; /* Same name */
                case EXTERN:
                    if (current->update_attribute != EXTERNAL) {
                        /* Mark an error for a duplicate label not marked as EXTERNAL */
                        errors->external_error_exist = double_label;
                        print_external_error(double_label, file);
                        return TRUE; /* Appears but not as extern, so it's a problem */
                    }
                    /* If the extern is already present, warn about the double entry */
                    print_warning(w_extern_double, file);
                    return TRUE;
            }
        }
        current = current->next; /* Move to the next node */
    }
    return FALSE; /* No conflict found */
}

void insert_new_symbol(struct symbols_linked_list *list, struct symbols_node *new_symbols_node) {
    new_symbols_node->next = NULL;
    if (list->head == NULL) {
        /* If the list is empty, set the new node as both head and tail */
        list->head = new_symbols_node;
        list->tail = new_symbols_node;
    } else {
        /* Append the new node at the end of the list */
        list->tail->next = new_symbols_node;
        list->tail = new_symbols_node;
    }
}

struct symbols_linked_list* create_symbols_linked_list() {
    /* Allocate memory for a new linked list */
    struct symbols_linked_list *newList = (struct symbols_linked_list *)malloc(sizeof(struct symbols_linked_list));
    if (newList == NULL) {
        print_internal_error(memory_failed);
        return NULL;
    }
    /* Initialize the head and tail to NULL */
    newList->head = NULL;
    newList->tail = NULL;
    return newList;
}

void free_symbols_list(struct symbols_linked_list *list) {
    struct symbols_node *current = list->head;
    while (current != NULL) {
        struct symbols_node *temp = current;
        current = current->next; /* Move to the next node */
        /* Free the memory allocated for the symbol name and the node */
        free(temp->name);
        free(temp);
    }
    free(list); /* Free the memory allocated for the linked list */
}

int create_insert_symbol(struct symbols_linked_list *list, char *name, int value, int location, int update_attribute, long size, int line, int is_entry) {
    /* Allocate memory for a new node */
    struct symbols_node *new_symbols_node = (struct symbols_node *)malloc(sizeof(struct symbols_node));
    if (new_symbols_node == NULL) {
        print_internal_error(memory_failed);
        return INTERNAL_ERROR; 
    }
    /* Copy the symbol name */
    new_symbols_node->name = my_strdup(name);
    if (name == NULL) {
        free(new_symbols_node);
        return INTERNAL_ERROR; 
    }
    /* Initialize the fields of the new node */
    new_symbols_node->value = value;
    new_symbols_node->location = location;
    new_symbols_node->update_attribute = update_attribute;
    new_symbols_node->size = size;
    new_symbols_node->line = line;
    new_symbols_node->is_entry = is_entry;
    new_symbols_node->next = NULL;
    /* Insert the new node into the list */
    insert_new_symbol(list, new_symbols_node);
    return NO_ERROR;
}

int set_entry(struct symbols_linked_list *list, char *name, struct file_status *file) {
    struct symbols_node *current = list->head;
    while (current != NULL) {
        /* Remove spaces from both the name and the current node's name */
        remove_spaces(name);
        remove_spaces(current->name);
        if (compare_strings_n(current->name, name, strlen(current->name))) {
            if (current->update_attribute == EXTERNAL) {
                /* Print an error if the symbol is already external */
                print_external_error(double_external_entry, file);
                return FALSE;
            } else {
                /* Mark the symbol as an entry */
                current->is_entry = TRUE;
                return TRUE;
            }
        }
        current = current->next; /* Move to the next node */
    }
    /* Print an error if the entry is not found */
    print_external_error(entry_un_realized, file);
    return FALSE; /* This entry was never realized */
}
