#include "Opcodes_table.h"
#include "Global_def.h"
#include <string.h>
#define NUM_ACTIONS 16

instruction instructions[NUM_ACTIONS] = 
   { {"mov", {0,0,0,0},{1, 1, 1, 1,0}, {0, 1, 1, 1,0}},
       {"cmp", {0,0,0,1},{1, 1, 1, 1,0}, {1, 1, 1, 1,0}},
        {"add", {0,0,1,0},{1, 1, 1, 1,0}, {0, 1, 1, 1,0}},
        {"sub",{0,0,1,1} ,{1, 1, 1, 1,0}, {0, 1, 1, 1,0}},
        {"lea",{0,1,0,0}, {0, 1, 0, 0,0}, {0, 1, 1, 1,0}},
        {"clr",{0,1,0,1}, {0, 0, 0, 0,1}, {0, 1, 1, 1,0}},
        {"not",{0,1,1,0}, {0, 0, 0, 0,1}, {0, 1, 1, 1,0}},
        {"inc",{0,1,1,1}, {0, 0, 0, 0,1}, {0, 1, 1, 1,0}},
        {"dec",{1,0,0,0}, {0, 0, 0, 0,1}, {0, 1, 1, 1,0}},
        {"jmp",{1,0,0,1}, {0, 0, 0, 0,1}, {0, 1, 1, 0,0}},
        {"bne",{1,0,1,0}, {0, 0, 0, 0,1}, {0, 1, 1, 0,0}},
        {"red", {1,0,1,1},{0, 0, 0, 0,1}, {0, 1, 1, 1,0}},
        {"prn",{1,1,0,0}, {0, 0, 0, 0,1}, {1, 1, 1, 1,0}},
        {"jsr", {1,1,0,1},{0, 0, 0, 0,1}, {0, 1, 1, 0,0}},
        {"rts",{1,1,1,0}, {0, 0, 0, 0,1}, {0, 0, 0, 0,1}},
        {"stop",{1,1,1,1},{0, 0, 0, 0,1}, {0, 0, 0, 0,1}} };


void code_opcode(int index_action, DC_IC *locations, date_structures *structures) {
    int index_data = 14, index_opcode = 0;
    /* Encode 4 bits from bit 14 down to bit 11 */
    /* Opcode's first bit (index 0) goes to bit 14, last bit (index 3) goes to bit 11 */
    for (; index_opcode < 4; index_opcode++) {
        set_bit(&(structures->instructions_array)[locations->IC], index_data, instructions[index_action].opcode[index_opcode]);
        index_data--;
    }
}

int index_action(char *word) {
    int i = 0;
    /* Search for the action name in the instructions array */
    for (; i < NUM_ACTIONS; i++) {
        if (!strcmp(word, instructions[i].name)) {
            return i; /* Return the index if found */
        }
    }
    return FICTIVE; /* Return FICTIVE if action not found */
}

int calculate_L_and_check(int index_action, int destination_operand_method, int source_operand_method) {
    int L = 1; /* Start with 1 for the initial word */
    
    /* Check if operand methods are valid for the action */
    if (instructions[index_action].source_operand[source_operand_method] != 1 ||
        instructions[index_action].destination_operand[destination_operand_method] != 1) {
        return FICTIVE; /* Return FICTIVE if invalid */
    }

    /* If both operands use REGISTER or INDIRECT, they share the same word */
    if ((destination_operand_method == REGISTER || destination_operand_method == INDIRECT) &&
        (source_operand_method == REGISTER || source_operand_method == INDIRECT)) {
        return L + 1; /* Return 2 words total */
    }

    /* Calculate the total number of words */
    L += (calculate(destination_operand_method) + calculate(source_operand_method));
    return L;
}

/* Determine how many words the operand method adds */
int calculate(int operand_method) {
    int num = 0;
    
    /* These methods each add 1 word */
    if (operand_method == REGISTER || operand_method == IMMEDIATE || operand_method == DIRECT || operand_method == INDIRECT) {
        num++;
    }
    return num;
}
