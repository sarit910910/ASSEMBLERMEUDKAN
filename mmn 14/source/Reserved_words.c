#include "Reserved_words.h"
#include "Global_def.h"
#include <string.h>

/* Array of all reserved words in the language */
char strings[RESERVED_WORD_NUM][10] = {
    /* Directive names */
    ".data",
    ".string",
    ".entry",
    ".extern",
    /* Macro definitions */
    "macr",
    "endmacr",
    /* 16 instructions in the assembly language */
    "mov",
    "cmp",
    "add",
    "sub",
    "lea",
    "clr",
    "not",
    "inc",
    "dec",
    "jmp",
    "bne",
    "red", 
    "prn",
    "jsr", 
    "rts",
    "stop",
    /* Processor's 8 registers */
    "r0",
    "r1",
    "r2",
    "r3",
    "r4",
    "r5",
    "r6",
    "r7"
};

/* Checks if a word exists in the reserved words array */
int reserved_words(char *str) {
    int i;
    /* Iterate over the reserved words to find a match */
    for (i = 0; i < RESERVED_WORD_NUM; i++) {
        if (strcmp(strings[i], str) == 0)
            return TRUE; /* The word is a reserved word */
    }
    return FALSE; /* The word is not a reserved word */
}

/* Checks if a given string is a register name */
int is_register(char *str) {
    /* Registers start from index 22 in the strings array */
    int register_index = 22;
    int i;
    /* Compare the string against the register names */
    for (i = register_index; i < RESERVED_WORD_NUM; i++) {
        if (strcmp(strings[i], str) == 0)
            return TRUE; /* The string is a register name */
    }
    return FALSE; /* The string is not a register name */
}
