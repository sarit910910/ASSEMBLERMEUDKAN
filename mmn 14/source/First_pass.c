#include "Global_def.h"
#include "Global_fun.h"
#include "Symbols.h"
#include "Opcodes_table.h"
#include "second_pass.h"
#include "Arrays.h"
#include "Errors.h"
#include "First_pass.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

int first_pass_main(struct macro_linked_list *list, int *error_exist, struct file_status *file, FILE *file_am) {
    file->line = 0;
    int found_EOF = FALSE, i;
    char **ptp;
    char *buffer, *copy;
    DC_IC *locations;
    date_structures *structures;
    errors_status *errors;
    LabelNode *labelList;

    /* Allocate memory for the label list */
    labelList = (LabelNode *)malloc(sizeof(LabelNode));
    if (labelList == NULL) {
        print_internal_error(memory_failed);
        return INTERNAL_ERROR;
    }

    /* Initialize the symbols_node pointer as NULL */
    struct symbols_node *fictive = NULL;

    /* Allocate memory for the DC_IC structure */
    locations = (DC_IC *)malloc(sizeof(DC_IC));
    if (locations == NULL) {
        print_internal_error(memory_failed);
        return INTERNAL_ERROR;
    }

    /* Initialize DC and IC counters */
    locations->DC = 0;
    locations->IC = 0;

    /* Allocate memory for the date_structures structure */
    structures = (date_structures *)malloc(sizeof(date_structures));
    if (structures == NULL) {
        print_internal_error(memory_failed);
        free(locations);
        return INTERNAL_ERROR;
    }

    /* Initialize the macro list and symbols list */
    structures->macro_list = list;
    structures->symbols_list = create_symbols_linked_list();
    if (structures->symbols_list == NULL) {
        print_internal_error(memory_failed);
        free(locations);
        free(structures);
        return INTERNAL_ERROR;
    }

    /* Initialize the data_array and instructions_array as NULL */
    structures->data_array = NULL;
    structures->instructions_array = NULL;

    /* Allocate memory for the errors_status structure */
    errors = (errors_status *)malloc(sizeof(errors_status));
    if (errors == NULL) {
        print_internal_error(memory_failed);
        free(locations);
        free_symbols_list(structures->symbols_list);
        free(structures);
        return INTERNAL_ERROR;
    }

    /* Set initial error states */
    errors->external_error_exist = *error_exist;
    errors->internal_error_exist = FALSE;

    /* Allocate memory for the buffer */
    buffer = (char *)malloc((MAX_LINE_LENGTH) * sizeof(char));
    if (buffer == NULL) {
        free(locations);
        free_symbols_list(structures->symbols_list);
        free(structures);
        free(errors);
        print_internal_error(memory_failed);
        return INTERNAL_ERROR;
    }

    /* Main loop to process the file until EOF is found */
    while (!found_EOF) {
        file->line++;
        buffer = (char *)malloc((MAX_LINE_LENGTH) * sizeof(char));
        if (buffer == NULL) {
            free(locations);
            free_symbols_list(structures->symbols_list);
            free(structures);
            free(errors);
            print_internal_error(memory_failed);
            return INTERNAL_ERROR;
        }

        /* Check for EOF and fill the buffer */
        found_EOF = fill_and_check(buffer, file_am);
        if (feof(file_am)) {
            found_EOF = TRUE;
        }

        /* Process the buffer content */
        copy = buffer;
        ptp = &copy;
        fictive = send_to_function(FALSE, ptp, file, errors, locations, structures);
        free(fictive);

        /* Check for internal errors */
        if (errors->internal_error_exist == TRUE) {
            free(locations);
            free_symbols_list(structures->symbols_list);
            free(structures->data_array);
            freeLabelList(structures->label_list);
            free(structures->instructions_array);
            free(structures);
            free(errors);
            return INTERNAL_ERROR;
        }
        free(buffer);
    }

    /* End of the first pass processing */
    end_of_pass_1(locations, structures, file, file_am);

    /* DEBUG: Print symbols list and instruction array */
    print_list(structures->symbols_list); /* DEBUG */
    for (i = 0; i < locations->IC; i++) { /* DEBUG */
        printBinary15Bit(structures->instructions_array[i].data); /* DEBUG */
    }

    /* Call the second pass */
    printf("---------------------------");
    print_list(structures->symbols_list); /* DEBUG */
    printLabelList(structures->label_list); /* DEBUG */
    second_pass(structures, locations);

    /* Free allocated memory */
    free(buffer);
    free(locations);
    free_symbols_list(structures->symbols_list);
    free(structures->data_array);
    free(structures->instructions_array);
    free(structures);
    free(errors);

    /* Return appropriate status based on internal error flag */
    if (errors->internal_error_exist == TRUE) {
        return INTERNAL_ERROR;
    }
    return NO_ERROR;
}



struct symbols_node* send_to_function(int is_label, char **ptp, struct file_status *file, 
                                      errors_status *errors, DC_IC *locations, 
                                      date_structures *structures) {
    struct symbols_node* node = NULL;
    int index_of_action;
    int word_length;
    char *first_word = next_word(ptp);
    
    /* Notice: ptp has been moved to the next word */
    if (first_word == NULL) {
        errors->internal_error_exist = TRUE;
        return NULL;
    }

    word_length = strlen(first_word);

    /* Handle the case where the first word is EOF */
    if (word_length < 1) {
        return NULL;
    }

    /* Handle the case where the line is an ".entry" directive */
    if (!strcmp(first_word, ".entry")) {
        free(first_word);
        /* Skip entry directives */
        return NULL;
    }
    /* Handle the case where the first word is a label */
    else if (!is_label && first_word[word_length - 1] == ':') {
        /* Send the label name */
        /* Remove the last character (':') by overwriting it */
        first_word[word_length - 1] = '\0';
        node = label(first_word, ptp, file, errors, locations, structures);
    }
    /* Handle the case where the first word is an ".extern" directive */
    else if (!strcmp(first_word, ".extern")) {
        node = extern_line(ptp, file, errors, locations, structures);
    }
    /* Handle the case where the first word is a ".data" directive */
    else if (!strcmp(first_word, ".data")) {
        node = data(ptp, file, errors, locations, structures);
    }
    /* Handle the case where the first word is a ".string" directive */
    else if (!strcmp(first_word, ".string")) {
        node = string(ptp, file, errors, locations, structures);
    }
    /* Handle the case where the first word is an action */
    else if ((index_of_action = index_action(first_word)) != FICTIVE) {
        /* If the action is valid, send the action index */
        node = action(index_of_action, ptp, file, errors, locations, structures);
    }
    /* Handle unrecognized lines */
    else {
        /* Print a general error message for unrecognized lines */
        print_external_error(unrocognize_line, file);
        errors->external_error_exist = TRUE;
    }

    free(first_word);
    return node;
}



struct symbols_node* label(char *label, char **ptp, struct file_status *file, 
                           errors_status *errors, DC_IC *locations, 
                           date_structures *structures) {
    struct symbols_node *node;
    int result;
    int type = REGULAR; /* Default type is REGULAR */
    char *first_word = next_word(ptp);
    char *st = ".entry";
    
    /* Check if the first word is ".entry" */
    result = strcmp(first_word, st);
    if (result == 0) {
        /* If the first word is ".entry", mark the node as an entry */
        node->is_entry = TRUE;
    }
    
    /* Check if the label is legal */
    if (!legal_word(label, FALSE, structures->macro_list, file)) {
        errors->external_error_exist = TRUE;
        return NULL;
    }

    /* Skip spaces; if no word is found after skipping, report an empty label error */
    if (skip_spaces(ptp) == FALSE) {
        print_external_error(empty_label, file);
        errors->external_error_exist = TRUE;
        return NULL;
    }

    /* Process the rest of the line with send_to_function */
    node = send_to_function(TRUE, ptp, file, errors, locations, structures);
    if (node == NULL) {
        /* If node is NULL, it might be due to define, entry, extern, or an error */
        return NULL;
    }

    /* Check if the node should be marked as EXTERN */
    if (node->update_attribute == EXTERNAL) {
        type = EXTERN;
    }

    /* Check if the label already exists in the symbols list */
    if (appear_in_symbols(structures->symbols_list, label, type, file, errors)) {
        free(node);
        return NULL;
    }

    /* Duplicate the label name and assign it to the node */
    node->name = my_strdup(label);
    if (node->name == NULL) {
        free(node);
        errors->internal_error_exist = TRUE;
        return NULL;
    }

    /* Insert the new symbol node into the symbols list */
    insert_new_symbol(structures->symbols_list, node);
    return NULL;
}


struct symbols_node* extern_line(char **ptp, struct file_status *file, 
                                 errors_status *errors, DC_IC *locations, 
                                 date_structures *structures) {
    char *label;

    /* Skip spaces; if no word is found after skipping, report an empty extern error */
    if (skip_spaces(ptp) == FALSE) {
        print_external_error(empty_extern, file);
        errors->external_error_exist = TRUE;
        return NULL;
    }

    /* Get the label after the ".extern" directive */
    label = next_word(ptp);
    if (label == NULL) {
        errors->internal_error_exist = TRUE;
        return NULL;
    }

    /* Validate the label and check for valid end of line */
    if (!valid_end(*ptp, 0, file, &(errors->external_error_exist)) || 
        !legal_word(label, FALSE, structures->macro_list, file)) {
        free(label);
        errors->external_error_exist = TRUE;
        return NULL;
    }

    /* Check if the label already exists in the symbols list as EXTERN */
    if (appear_in_symbols(structures->symbols_list, label, EXTERN, file, errors)) {
        free(label);
        return NULL;
    }

    /* Create and insert the symbol with EXTERN attribute */
    /* The value is set to 0 because it's extern; location and size are fictive */
    if (create_insert_symbol(structures->symbols_list, label, 0, FICTIVE, EXTERNAL, 
                             FICTIVE, file->line, FALSE) == INTERNAL_ERROR) {
        free(label);
        errors->internal_error_exist = TRUE;
    }

    /* Return NULL because externs are never associated with a label in the current context */
    return NULL;
}


struct symbols_node* data(char **ptp, struct file_status *file, 
                          errors_status *errors, DC_IC *locations, 
                          date_structures *structures) {
    struct symbols_node *node = (struct symbols_node*)malloc(sizeof(struct symbols_node));
    char *param, *param2, *check_param; /* Pointers for parameter processing */
    long num = 0;
    int end = FALSE;

    /* Check for memory allocation failure */
    if (node == NULL) {
        print_internal_error(memory_failed);
        errors->internal_error_exist = TRUE;
        return NULL;
    }

    /* Initialize node attributes */
    node->value = locations->DC;
    node->location = DATA;
    node->update_attribute = RELOCATEABLE;
    node->line = file->line;

    /* Process input parameters */
    while (!end) {
        /* Skip spaces; if nothing is found, report a missing parameter error */
        if (skip_spaces(ptp) == FALSE) {
            print_external_error(missing_parameter, file);
            errors->external_error_exist = TRUE;
            free(node);
            return NULL;
        }

        /* Retrieve the next parameter */
        param = next_param(ptp);
        if (param == NULL) {
            free(node);
            print_internal_error(memory_failed);
            errors->internal_error_exist = TRUE;
            return NULL;
        }

        /* Handle parameters that start with '#' */
        param2 = param;
        if (param2[0] == '#') {
            param2++; /* Skip the '#' character */
        }

        /* Convert the string to a long integer */
        num = strtol(param2, &check_param, 10);

        /* Validate the conversion result */
        if (param2 == check_param || *check_param != '\0') {
            print_external_error(no_such_define, file);
            errors->external_error_exist = TRUE;
            free(node);
            free(param);
            return NULL;
        }

        /* Check if the number is within the valid range */
        if (num > MAXIMUM_VALUE || num < MINIMUM_VALUE) {
            print_external_error(too_big_number, file);
            errors->external_error_exist = TRUE;
            free(node);
            free(param);
            return NULL;
        }

        /* Insert the validated data */
        insert_data(num, file, errors, locations, structures);

        /* Check for internal errors during insertion */
        if (errors->internal_error_exist == TRUE) {
            free(node);
            free(param);
            return NULL;
        }

        /* Check if we reached the end of the input */
        if (skip_spaces(ptp) == FALSE) {
            end = TRUE;
        } else {
            /* If not at the end, check for a comma separator */
            if (**ptp == ',') {
                (*ptp)++; /* Skip the comma */
            } else {
                /* If a comma is missing, report a missing comma error */
                print_external_error(missing_comma, file);
                errors->external_error_exist = TRUE;
                free(node);
                free(param);
                return NULL;
            }
        }
    }

    /* Calculate the size of the data segment processed */
    node->size = (locations->DC) - (node->value);
    free(param);
    return node;
}


struct symbols_node* string(char **ptp, struct file_status *file, 
                            errors_status *errors, DC_IC *locations, 
                            date_structures *structures) {
    struct symbols_node *node = (struct symbols_node*)malloc(sizeof(struct symbols_node));

    /* Check for memory allocation failure */
    if (node == NULL) {
        print_internal_error(memory_failed);
        errors->internal_error_exist = TRUE;
        return NULL;
    }

    /* Initialize node attributes */
    node->value = locations->DC;
    node->location = DATA;
    node->update_attribute = RELOCATEABLE;
    node->line = file->line;

    /* Check for missing string parameter */
    if (skip_spaces(ptp) == FALSE) {
        print_external_error(missing_parameter, file);
        errors->external_error_exist = TRUE;
        free(node);
        return NULL;
    }

    /* Check for the beginning of the string (must start with a quotation mark) */
    if (**ptp != '"') {
        print_external_error(string_with_invalid_begin, file);
        errors->external_error_exist = TRUE;
        free(node);
        return NULL;
    }

    /* Skip the opening quotation mark */
    (*ptp)++;

    /* Process characters within the string */
    while (**ptp != '\n' && (**ptp != '"' || !isspace((*ptp)[1]))) {
        /* Check for invalid characters in the string */
        if (!isprint(**ptp)) {
            print_external_error(invalid_char_in_string, file);
            errors->external_error_exist = TRUE;
            free(node);
            return NULL;
        }

        /* Debug output for IC before insertion */
        printf("IC DATA: BEFORE %d\n", locations->DC); /* DEBUG */

        /* Insert the character into data */
        insert_data((int)(**ptp), file, errors, locations, structures);

        /* Debug output for IC after insertion */
        printf("IC STRING: AFTER %d\n", locations->DC); /* DEBUG */

        /* Move to the next character */
        (*ptp)++;
    }

    /* Check for the end of the string (must end with a quotation mark) */
    if (**ptp != '"') {
        print_external_error(missing_ending_char, file);
        errors->external_error_exist = TRUE;
        free(node);
        return NULL;
    }

    /* Skip the closing quotation mark */
    (*ptp)++;

    /* Ensure no extra text follows the closing quotation mark */
    if (!valid_end(*ptp, 0, file, &(errors->external_error_exist))) {
        print_external_error(missing_ending_char, file);
        free(node);
        return NULL;
    }

    /* Set the size of the string data */
    node->size = locations->DC - node->value;
    return node;
}



void insert_data(int value, struct file_status *file, errors_status *errors, DC_IC *locations, date_structures *structures) {
    int *dataBinary;
    int index_code, index_arr;
    /* Reallocate memory for data_array to accommodate the new data entry */
    structures->data_array = (bit_field *)realloc(structures->data_array, 
                          (locations->DC + 1) * sizeof(bit_field));
    if (structures->data_array == NULL) {
        print_internal_error(memory_failed);
        errors->internal_error_exist = TRUE;
        return; /* Exit function on memory allocation failure */
    } else {
        /* Store the new value in the data_array and increment DC */
        structures->data_array[locations->DC].data = value;
        locations->DC = locations->DC + 1;
    }
    /* Convert the value to a 15-bit binary representation */
    dataBinary = decimalToBinary(value, 15);
    /* Reallocate memory for instructions_array to accommodate the new instruction */
    structures->instructions_array = (bit_field *)realloc(structures->instructions_array, 
                          (locations->IC + 1) * sizeof(bit_field));
    if (structures->instructions_array == NULL) {
        print_internal_error(memory_failed);
        errors->internal_error_exist = TRUE;
        free(dataBinary); /* Free the binary representation array */
        return; /* Exit function on memory allocation failure */
    }
    /* Set the bits in the instructions_array */
    index_code = 14;
    for (index_arr = 0; index_arr < 15; index_arr++) {
        set_bit(&(structures->instructions_array[locations->IC]), index_code, dataBinary[index_arr]);
        index_code--;
    }
    /* Increment IC to account for the new instruction data */
    locations->IC++;
    /* Free the temporary binary representation array */
    free(dataBinary);
}




struct symbols_node* action(int index_of_action, char **ptp, struct file_status *file, errors_status *errors, DC_IC *locations, date_structures *structures) {
    /* This variable holds the number of words this line will take in the instructions array */
    int L, i, destination_operand_method, source_operand_method, index_data, index_binary_num, number, number2;
    char *destination_operand, *source_operand;
    int *binaryNum, *binaryNum2;
    char *operand, *opernd2; /* For encoding the destination operand */
    struct symbols_node* node = (struct symbols_node*)malloc(sizeof(struct symbols_node));

    /* Check if memory allocation was successful */
    if (node == NULL) {
        print_internal_error(memory_failed);
        errors->internal_error_exist = TRUE;
        return NULL;
    }

    /* Initialize the new node */
    node->value = locations->IC;
    node->location = CODE;
    node->update_attribute = RELOCATEABLE;
    node->line = file->line;

    /* Skip spaces and check if there are parameters */
    if (skip_spaces(ptp) == FALSE) {
        /* No parameters found */
        destination_operand = NULL;
        source_operand = NULL;
    } else {
        /* Parameters are present */
        source_operand = next_param(ptp);
        if (source_operand == NULL) {
            print_internal_error(memory_failed);
            errors->internal_error_exist = TRUE;
            free(node);
            return NULL;
        }

        if (skip_spaces(ptp) == FALSE) {
            /* Only one operand is present, treated as destination operand */
            destination_operand = source_operand;
            source_operand = NULL;
        } else {
            /* Multiple parameters present */
            if (**ptp != ',') {
                print_external_error(missing_comma, file);
                errors->external_error_exist = TRUE;
                free(node);
                free(source_operand);
                return NULL;
            }
            /* Comma found, move to the next parameter */
            (*ptp)++;
            if (skip_spaces(ptp) == FALSE) {
                /* No parameter found after the comma */
                print_external_error(missing_param_after_comma, file);
                errors->external_error_exist = TRUE;
                free(node);
                free(source_operand);
                return NULL;
            }
            destination_operand = next_param(ptp);
            if (destination_operand == NULL) {
                print_internal_error(memory_failed);
                errors->internal_error_exist = TRUE;
                free(node);
                free(source_operand);
                return NULL;
            }
        }

        /* Validate the end of the line */
        if (!valid_end(*ptp, 0, file, &(errors->external_error_exist))) {
            free(node);
            free_strings(2, destination_operand, source_operand);
            return NULL;
        }
    }

    /* Determine addressing methods for the operands */
    destination_operand_method = figure_addressing_methods(destination_operand, structures->symbols_list);
    source_operand_method = figure_addressing_methods(source_operand, structures->symbols_list);
    char *des = destination_operand;
    char *sur = source_operand;

    /* Calculate L and validate parameters */
    L = calculate_L_and_check(index_of_action, destination_operand_method, source_operand_method);
    if (L == FICTIVE) {
        /* Invalid parameters for this action */
        print_external_error(wrong_param_fun, file);
        errors->external_error_exist = TRUE;
        free(node);
        free_strings(2, destination_operand, source_operand);
        return NULL;
    }

    /* Allocate memory for the instruction array */
    structures->instructions_array = (bit_field *)realloc(structures->instructions_array, (locations->IC + L) * sizeof(bit_field));
    if (structures->instructions_array == NULL) {
        print_internal_error(memory_failed);
        errors->internal_error_exist = TRUE;
        free(node);
        free_strings(2, destination_operand, source_operand);
        return NULL;
    }

    /* Initialize the new instruction array entries to zero */
    for (i = 0; i < L; i++) {
        (structures->instructions_array)[locations->IC + i].data = 0;
    }

    /* Encode the first word in the array */
    code_opcode(index_of_action, locations, structures);
    code_operand_method(DESTINATION, destination_operand_method, locations, structures, structures->symbols_list, des);
    code_operand_method(SOURCE, source_operand_method, locations, structures, structures->symbols_list, sur);

    

   /* An instruction word without operands, like STOP, has already been encoded... */
    if(L == 2 && source_operand == NULL)
{
    /* If L == 2 and source_operand is NULL, it means there's only one operand, and we'll encode it according to the destination method */

    /* Handling all addressing modes except mode 1 (immediate) */
    if(destination_operand_method == 0) /* Immediate addressing mode */
    {
        operand = destination_operand;
        operand++; /* Skip the '#' character */
        number = atoi(operand);
        binaryNum = decimalToBinary(number, 14 - START_BIT_DESTINATION_REGISTER + 1); /* Convert operand to binary array */

        index_data = 14;
        index_binary_num = 0;
        for(; index_binary_num < 14 - START_BIT_DESTINATION_REGISTER + 1; index_binary_num++)
        {
            set_bit(&(structures->instructions_array)[locations->IC+1], index_data, binaryNum[index_binary_num]);
            printf("Iteration imm %d: instruction value: %d, set bit %d to %d\n", index_binary_num, (structures->instructions_array)[locations->IC+1].data, index_data, binaryNum[index_binary_num]);
            index_data--;
        }

        /* Set the ARE bits */
        set_bit(&(structures->instructions_array)[locations->IC+1], 0, 0);
        set_bit(&(structures->instructions_array)[locations->IC+1], 1, 0);
        set_bit(&(structures->instructions_array)[locations->IC+1], 2, 1);
    }
    else if(destination_operand_method == 2 || destination_operand_method == 3) /* Indirect or direct register addressing mode */
    {
        operand = destination_operand;
        if(operand[0] == '*')
            operand++; /* Skip the '*' character */
        
        operand++; /* Skip the 'r' character */
        number = atoi(operand);
        binaryNum = decimalToBinary(number, 14 - START_BIT_DESTINATION_REGISTER + 1); /* Convert operand to binary array */

        index_data = 14;
        index_binary_num = 0;
        for(; index_binary_num < 14 - START_BIT_DESTINATION_REGISTER + 1; index_binary_num++)
        {
            set_bit(&(structures->instructions_array)[locations->IC+1], index_data, binaryNum[index_binary_num]);
            printf("Iteration %d: instruction value: %d, set bit %d to %d\n", index_binary_num, (structures->instructions_array)[locations->IC+1].data, index_data, binaryNum[index_binary_num]);
            index_data--;
        }

        /* Set the ARE bits */
        set_bit(&(structures->instructions_array)[locations->IC+1], 0, 0);
        set_bit(&(structures->instructions_array)[locations->IC+1], 1, 0);
        set_bit(&(structures->instructions_array)[locations->IC+1], 2, 1);
    }
    else if(destination_operand_method == 1) /* Direct addressing mode */
    {
        operand = destination_operand;
        addLabelNode(&structures->label_list, operand, locations->IC+1);
    }
}
else if(L == 2 && destination_operand_method != 4 && source_operand_method != 4)
{
    /* L == 2 and both operands are indirect or direct registers, encoding them in the same word */

    /* Encode destination operand */
    operand = destination_operand;
    if(operand[0] == '*')
        operand++; /* Skip the '*' character */

    operand++; /* Skip the 'r' character */
    number = atoi(operand);
    opernd2 = source_operand;

    if(opernd2[0] == '*')
        opernd2++; /* Skip the '*' character */

    opernd2++; /* Skip the 'r' character */
    number2 = atoi(opernd2);
    binaryNum = decimalToBinary(number, 3); /* Convert operand to binary array */
    binaryNum2 = decimalToBinary(number2, 3); /* Convert operand to binary array */

    /* Encoding destination */
    index_data = 5;
    index_binary_num = 0;
    for(; index_binary_num < 3; index_binary_num++)
    {
        set_bit(&(structures->instructions_array)[locations->IC+1], index_data, binaryNum[index_binary_num]);
        index_data--;
    }

    /* Set the ARE bits */
    set_bit(&(structures->instructions_array)[locations->IC+1], 0, 0);
    set_bit(&(structures->instructions_array)[locations->IC+1], 1, 0);
    set_bit(&(structures->instructions_array)[locations->IC+1], 2, 1);

    /* Encoding source */
    index_data = 8;
    index_binary_num = 0;
    for(; index_binary_num < 3; index_binary_num++)
    {
        set_bit(&(structures->instructions_array)[locations->IC+1], index_data, binaryNum2[index_binary_num]);
        index_data--;
    }

    /* Set the ARE bits */
    set_bit(&(structures->instructions_array)[locations->IC+1], 0, 0);
    set_bit(&(structures->instructions_array)[locations->IC+1], 1, 0);
    set_bit(&(structures->instructions_array)[locations->IC+1], 2, 1);
}    
    else 
{
    /* L == 3 indicates there are 2 operands, requiring the encoding of 2 additional lines */
    if (source_operand_method == 0) /* Immediate addressing */
    {
        operand = source_operand;
        operand++; /* Skip the hash symbol */
        number = atoi(operand);
        binaryNum = decimalToBinary(number, 14 - START_BIT_SOURCE_REGISTER + 1); /* Convert the operand to binary, sized appropriately */

        index_data = 14;
        index_binary_num = 0;
        for (; index_binary_num < 14 - START_BIT_SOURCE_REGISTER + 1; index_binary_num++) 
        {
            set_bit(&(structures->instructions_array)[locations->IC + 1], index_data, binaryNum[index_binary_num]);
            index_data--;
        }
        /* Set the ARE bits */
        set_bit(&(structures->instructions_array)[locations->IC + 1], 0, 0);
        set_bit(&(structures->instructions_array)[locations->IC + 1], 1, 0);
        set_bit(&(structures->instructions_array)[locations->IC + 1], 2, 1);
    }
    else if (source_operand_method == 2 || source_operand_method == 3) /* Indirect or direct register addressing */
    {
        operand = source_operand;
        if (operand[0] == '*')
        {
            operand++; /* Skip the asterisk */
        }
        operand++; /* Skip the 'r' character in both addressing modes */
        number = atoi(operand);
        binaryNum = decimalToBinary(number, 14 - START_BIT_SOURCE_REGISTER + 1); /* Convert the operand to binary */

        index_data = 14;
        index_binary_num = 0;
        for (; index_binary_num < 14 - START_BIT_SOURCE_REGISTER + 1; index_binary_num++) 
        {
            set_bit(&(structures->instructions_array)[locations->IC + 1], index_data, binaryNum[index_binary_num]);
            index_data--;
        }
        /* Set the ARE bits */
        set_bit(&(structures->instructions_array)[locations->IC + 1], 0, 0);
        set_bit(&(structures->instructions_array)[locations->IC + 1], 1, 0);
        set_bit(&(structures->instructions_array)[locations->IC + 1], 2, 1);
    }
    else if (source_operand_method == 1) /* Direct label addressing */
    {
        operand = source_operand;
        addLabelNode(&structures->label_list, operand, locations->IC + 1);
    }

    if (destination_operand_method == 0) /* Immediate addressing */
    {
        operand = destination_operand;
        operand++; /* Skip the hash symbol */
        number = atoi(operand);
        binaryNum = decimalToBinary(number, 14 - START_BIT_DESTINATION_REGISTER + 1); /* Convert the operand to binary */

        index_data = 14;
        index_binary_num = 0;
        for (; index_binary_num < 14 - START_BIT_DESTINATION_REGISTER + 1; index_binary_num++) 
        {
            set_bit(&(structures->instructions_array)[locations->IC + 2], index_data, binaryNum[index_binary_num]);
            index_data--;
        }
        printf("ic2 %d\n", locations->IC + 2); /* DEBUG */
        /* Set the ARE bits */
        set_bit(&(structures->instructions_array)[locations->IC + 2], 0, 0);
        set_bit(&(structures->instructions_array)[locations->IC + 2], 1, 0);
        set_bit(&(structures->instructions_array)[locations->IC + 2], 2, 1);
    }
    else if (destination_operand_method == 2 || destination_operand_method == 3) /* Indirect or direct register addressing */
    {
        operand = destination_operand;
        if (operand[0] == '*')
        {
            operand++; /* Skip the asterisk */
        }
        operand++; /* Skip the 'r' character in both addressing modes */
        number = atoi(operand);
        binaryNum = decimalToBinary(number, 14 - START_BIT_DESTINATION_REGISTER + 1); /* Convert the operand to binary */

        index_data = 14;
        index_binary_num = 0;
        for (; index_binary_num < 14 - START_BIT_DESTINATION_REGISTER + 1; index_binary_num++) 
        {
            set_bit(&(structures->instructions_array)[locations->IC + 2], index_data, binaryNum[index_binary_num]);
            index_data--;
        }
        /* Set the ARE bits */
        set_bit(&(structures->instructions_array)[locations->IC + 2], 0, 0);
        set_bit(&(structures->instructions_array)[locations->IC + 2], 1, 0);
        set_bit(&(structures->instructions_array)[locations->IC + 2], 2, 1);
    }
    else if (destination_operand_method == 1) /* Direct label addressing */
    {
        operand = destination_operand;
        addLabelNode(&structures->label_list, operand, locations->IC + 2);
    }
}

locations->IC = locations->IC + L;
node->size = L;
free_strings(2, destination_operand, source_operand);
free_strings(2, des, sur);
return node;
}


void code_operand_method(int first_bit, int opernad_method, DC_IC *locations, date_structures *structures, struct symbols_linked_list *list, char *opernad) {
    /* Set ARE bits (Assume Relocatable Entry) */
    set_bit(&(structures->instructions_array)[locations->IC], 0, 0);
    set_bit(&(structures->instructions_array)[locations->IC], 1, 0);
    set_bit(&(structures->instructions_array)[locations->IC], 2, 1);

    if (opernad_method == IMMEDIATE /* || opernad_method == NO_OPPERAND */) {
        /* Set the operand method bits for immediate addressing */
        set_bit(&(structures->instructions_array)[locations->IC], first_bit, 0);
        set_bit(&(structures->instructions_array)[locations->IC], first_bit - 1, 0);
        set_bit(&(structures->instructions_array)[locations->IC], first_bit - 2, 0);
        set_bit(&(structures->instructions_array)[locations->IC], first_bit - 3, 1);
    } 
    else if (opernad_method == DIRECT) {
        /* Set the operand method bits for direct addressing */
        set_bit(&(structures->instructions_array)[locations->IC], first_bit, 0);
        set_bit(&(structures->instructions_array)[locations->IC], first_bit - 1, 0);
        set_bit(&(structures->instructions_array)[locations->IC], first_bit - 2, 1);
        set_bit(&(structures->instructions_array)[locations->IC], first_bit - 3, 0);
    } 
    else if (opernad_method == INDIRECT) {
        /* Set the operand method bits for indirect addressing */
        set_bit(&(structures->instructions_array)[locations->IC], first_bit, 0);
        set_bit(&(structures->instructions_array)[locations->IC], first_bit - 1, 1);
        set_bit(&(structures->instructions_array)[locations->IC], first_bit - 2, 0);
        set_bit(&(structures->instructions_array)[locations->IC], first_bit - 3, 0);
    } 
    else if (opernad_method == REGISTER) {
        /* Set the operand method bits for register addressing */
        set_bit(&(structures->instructions_array)[locations->IC], first_bit, 1);
        set_bit(&(structures->instructions_array)[locations->IC], first_bit - 1, 0);
        set_bit(&(structures->instructions_array)[locations->IC], first_bit - 2, 0);
        set_bit(&(structures->instructions_array)[locations->IC], first_bit - 3, 0);
    }
}

void end_of_pass_1(DC_IC *locations, date_structures *structures, struct file_status *file, FILE *file_am) {
    struct symbols_node *current = structures->symbols_list->head;
    
    /* Traverse the linked list of symbols */
    while (current != NULL) {
        /* If the symbol is in the data section and is relocatable, adjust its value */
        if (current->location == DATA && current->update_attribute == RELOCATEABLE) {
            current->value = current->value + 100 + locations->IC;
            printf("current->value %d\n", current->value); /* DEBUG */
        } 
        /* If the symbol is in the code section and is relocatable, adjust its value */
        else if (current->location == CODE && current->update_attribute == RELOCATEABLE) {
            current->value = current->value + 100;
        }
        current = current->next; /* Move to the next node */
    }
}
        
void entries(struct file_status * file,FILE * file_am, date_structures * structures ,DC_IC * locations){
    int i;
    file->line=0; 
    fseek(file_am, 0, SEEK_SET);
    struct symbols_node* fictive = NULL;
    int found_EOF=FALSE;
    char  *buffer2 ,*copy;
    char ** ptp;
    buffer2 = (char *)malloc((MAX_LINE_LENGTH) * sizeof(char));
    while (!found_EOF)
	{
        file->line++;
        buffer2 = (char *)malloc((MAX_LINE_LENGTH) * sizeof(char));
		found_EOF=fill_and_check(buffer2,file_am);
         if (feof(file_am)){
            found_EOF = TRUE;
         }
             copy = buffer2;
		     ptp = &copy;
             fictive=send_to_function_pass_two(ptp,file,structures);
             free(fictive);
    }
            print_list(structures->symbols_list);//DEBUG
    free(buffer2);
    free(structures);
}

struct symbols_node* send_to_function_pass_two(char ** ptp,struct file_status * file, date_structures * structures ){
    char * first_word = NULL;
			first_word =(char*)malloc((MAX_NAME_LENGTH)*sizeof(char));
            first_word = next_word(ptp);
    int x;
            if (!strcmp(first_word,".entry")){
				first_word = next_word(ptp);
                first_word=*ptp;
				x=set_entry(structures->symbols_list, first_word, file);
			}
            free(first_word);
            free(ptp);
}


int compare_strings(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        while (*str1 && !isalpha(*str1)) {
            str1++;
        }
        while (*str2 && !isalpha(*str2)) {
            str2++;
        }
        if (tolower(*str1) != tolower(*str2)) {
            return 0; 
        }
        if (*str1) str1++;
        if (*str2) str2++;
    }
    return *str1 == '\0' && *str2 == '\0';
}


void printBinary15Bit(int n) {
    int i,mask;
    for (i = 14; i >= 0; i--) { 
        mask = 1 << i;
        printf("%d", (n & mask) ? 1 : 0);
    }
    printf("\n");
}