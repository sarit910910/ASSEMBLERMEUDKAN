#include "NewMacro.h"
#include "Errors.h"
#include "Reserved_words.h"
#include "Global_fun.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_DATA_LINES 10//DEBUG

/* Function to create a new macro list*/
MacroList* create_macro_list() {
    MacroList *list = (MacroList *)malloc(sizeof(MacroList));
    if (!list) {
        print_internal_error(memory_failed);
        return NULL;
    }
    list->head = list->tail = NULL;
    return list;
}

int insert_new_macro(MacroList *list, const char *name, const char *content, struct file_status *file) {
    if (strlen(name) > MAX_MACRO_NAME_LENGTH) {  
        print_external_error( illegal_name_macro,file);
        return RESERVED_WORD_ERROR;
    }

    if (is_reserved_word(name)) {
        print_external_error( illegal_name_macro,file);
        return RESERVED_WORD_ERROR;
    }

    if (macro_name_appeared(list, name)) {
        print_external_error( double_name_macro,file);
        return RESERVED_WORD_ERROR;
    }

    MacroNode *new_macro = (MacroNode *)malloc(sizeof(MacroNode));
    if (!new_macro) {
        print_internal_error(memory_failed);
        return memory_failed;
    }
    new_macro->name = strdup(name);
    new_macro->content = strdup(content);
    if (!new_macro->name || !new_macro->content) {
        print_internal_error(memory_failed);
        free(new_macro->name);
        free(new_macro->content);
        free(new_macro);
        return memory_failed;
    }
    new_macro->next = NULL;
    if (!list->head) {
        list->head = list->tail = new_macro;
    } else {
        list->tail->next = new_macro;
        list->tail = new_macro;
    }
    return 0;
}



/* Function to call a macro and replace it with its content*/
void call_macro(const char *name, FILE *output_file, MacroList *list, struct file_status *file) {
    MacroNode *current = list->head;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            fprintf(output_file, "%s", current->content);
            return;
        }
        current = current->next;
    }
        print_external_error( macro_name_missing,file);
}


/*Function to free the memory used by the macro list*/
void free_macro_list(MacroList *list) {
    MacroNode *current = list->head;
    while (current) {
        MacroNode *next = current->next;
        free(current->name);
        free(current->content);
        free(current);
        current = next;
    }
    free(list);
}

/* Function to determine the type of a line*/
LineType determine_line_type(const char *line, MacroList *list, char **macro_name) {
    if (line[0] == ';') {
        return COMMENT_LINE;  
    }
    /*Checks if the line starts with data to move it to the end of the file*/
    if (strncmp(line, ".data", 5) == 0) {
        return DATA_LINE;
    } 

    if (strncmp(line, "macr", 4) == 0) {
        *macro_name = strdup(line + 5);
        char *newline_pos = strchr(*macro_name, '\n');
        if (newline_pos != NULL) {
            *newline_pos = '\0';
        }
        char *extra_chars = strchr(*macro_name, ' ');
        if (extra_chars != NULL && strlen(extra_chars) > 1) {
            return RESERVED_WORD_ERROR;
        }
        return MACRO_DEF;
    }

    if (strncmp(line, "endmacr", 7) == 0) {
        if (strlen(line) > 7 && line[7] != '\n' && line[7] != '\0' && strchr(line + 7, ' ') != NULL) {
            return RESERVED_WORD_ERROR;
        }
        return MACRO_END_DEF;
    }

    MacroNode *current = list->head;
    while (current) {
        if (strncmp(line, current->name, strlen(current->name)) == 0) {
            *macro_name = strdup(current->name);
            return MACRO_CALL;
        }
        current = current->next;
    }

    return ANY_OTHER_LINE_TYPE;
}

int pre_proccesor_main(int *error_exist, struct file_status* file, FILE *file_as, FILE *file_am, MacroList *list) {
    char line[MAX_LINE_LENGTH + 1];
    char macro_content[MAX_LINE_LENGTH + 1];
    char *macro_name = NULL;
    LineType line_type;
    int macro_open = 0;

    char *data_lines[MAX_DATA_LINES];
    int num_data_lines = 0;

    while (fgets(line, sizeof(line), file_as)) {
        file->line++;

        line_type = determine_line_type(line, list, &macro_name);

        if (line_type == DATA_LINE) {
            data_lines[num_data_lines] = strdup(line);
            if (!data_lines[num_data_lines]) {
                print_internal_error(memory_failed);
                *error_exist = 1;
                continue;
            }
            num_data_lines++;
            continue;
        }

        if (line_type == COMMENT_LINE) {
            continue;
        }

        switch (line_type) {
            case MACRO_DEF:
                if (line_type == RESERVED_WORD_ERROR) {
                print_external_error( illegal_name_macro,file);
                    *error_exist = 1;
                    continue;
                }
                if (strlen(macro_name) > MAX_MACRO_NAME_LENGTH) {
                print_external_error( illegal_name_macro,file);
                    *error_exist = 1;
                    continue;
                }
                if (macro_open) {
                print_external_error( illegal_name_macro,file);
                    *error_exist = 1;
                    continue;
                }
                macro_open = 1;
                macro_content[0] = '\0';

                while (fgets(line, sizeof(line), file_as) && strncmp(line, "endmacr", 7) != 0) {
                    file->line++;

                    if (determine_line_type(line, list, &macro_name) == COMMENT_LINE) {
                        continue;
                    }

                    if (strlen(line) > MAX_LINE_LENGTH) {
                    print_external_error( too_long_line,file);
                        *error_exist = 1;
                        continue;
                    }
                    strcat(macro_content, line);
                }

                if (strncmp(line, "endmacr", 7) == 0 && strlen(line) > 7 && line[7] != '\n' && line[7] != '\0') {
                    print_external_error( illegal_name_macro,file);
                    *error_exist = 1;
                    macro_open = 0;
                    continue;
                }

                if (!macro_open) {
                print_external_error( illegal_name_macro,file);
                    *error_exist = 1;
                    continue;
                }
                macro_open = 0;

                if (insert_new_macro(list, macro_name, macro_content, file) != 0) {
                    *error_exist = 1;
                }
                break;

            case MACRO_CALL:
                call_macro(macro_name, file_am, list, file);
                break;

            case MACRO_END_DEF:
                if (strncmp(line, "endmacr", 7) == 0 && strlen(line) > 7) {
                print_external_error( illegal_name_macro,file);
                    *error_exist = 1;
                    continue;
                }
                if (!macro_open) {
                print_external_error( illegal_name_macro,file);
                    *error_exist = 1;
                }
                macro_open = 0;
                break;

            default:
                if (line_type != COMMENT_LINE && strlen(line) <= MAX_LINE_LENGTH) {
                    fputs(line, file_am);  
                } else {
                print_external_error( illegal_name_macro,file);
                    *error_exist = 1;
                }
                break;
        }

        free(macro_name);
        macro_name = NULL;
    }

    if (macro_open) {
        print_external_error( illegal_name_macro,file);
        *error_exist = 1;
    }
    process_data_lines(data_lines, num_data_lines, file_am);

    return 0;
}




/* Function to create the output filename based on the input filename*/
void create_output_filename(char *input_filename, char *output_filename) {
    strcpy(output_filename, input_filename);
    char *dot = strrchr(output_filename, '.');
    if (dot != NULL) {
        strcpy(dot, ".am");
    } else {
        strcat(output_filename, ".am");
    }
}

/* Function to check if a macro name already appeared*/
int macro_name_appeared(MacroList *list, const char *str) {
    MacroNode *current = list->head;
    while (current) {
        if (strcmp(current->name, str) == 0) {
            return TRUE;
        }
        current = current->next;
    }
    return FALSE;
}

int is_reserved_word(const char *str) {
    int i;
    for (i = 0; i <RESERVED_WORD_NUM ; i++) {
        if (strcmp(strings[i], str) == 0)
            return TRUE; 
    }
    return FALSE; 
}


void process_data_lines(char *data_lines[], int num_data_lines, FILE *file_am) {
    for (int i = 0; i < num_data_lines; i++) {
        fputs(data_lines[i], file_am);
        free(data_lines[i]);
    }
}
