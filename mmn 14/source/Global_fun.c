#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "Errors.h"
#include "Global_fun.h"
#include "Reserved_words.h"
#include "NewMacro.h"
#include "Global_def.h"
#include "Symbols.h"

char * next_word(char **ptp) {
    int i = 0;
    char *first_word = (char *)malloc((MAX_LINE_LENGTH) * sizeof(char));
    if (first_word == NULL) {
        print_internal_error(memory_failed);
        return NULL; /* Memory allocation failed */
    }
    while (i < MAX_LINE_LENGTH && !isspace(**ptp) && isprint(**ptp) && (**ptp) != EOF) {
        first_word[i] = **ptp; /* Copy the current character */
        (*ptp)++; /* Move to the next character */
        i++; 
    }
    first_word[i] = '\0'; /* Null-terminate the string */
    return first_word;
    /* `ptp` now points to the character after the extracted word */
}


int legal_word(char *str, int is_define, struct macro_linked_list *list, struct file_status *file) {

    if (!isalpha(*str)) { /* Check if the first character is alphabetic */
        print_external_error(illegal_name, file);
        return FALSE;
    }
    if (reserved_words(str)) { /* Check if the word is a reserved word */
        print_external_error(reserved_word, file);
        return FALSE;
    }	
    if (!is_define) {
        while (*str) {
            if (!isdigit(*str) && !isalpha(*str)) { /* Ensure all characters are alphanumeric */
                print_external_error(illegal_name, file);
                return FALSE;
            }
            str++;
        }
    }
      if (macro_name_appeared(list, str)) { /* Check if the macro name appeared */ 
          print_external_error(defined_macro, file);
          return FALSE;
      }

    return TRUE;
}


int skip_spaces(char **ptp) {
    while (**ptp != '\0') {
        if (!isspace(**ptp)) { /* Found a non-whitespace character */
            return TRUE; 
        }
        (*ptp)++; /* Move to the next character */
    }
    return FALSE; 
}

char *my_strdup(char *src) {
    int length = strlen(src) + 1; /* Include space for the null terminator */
    char *dst = (char *)malloc(length); /* Allocate memory for the new string */
    if (dst == NULL) {
        print_internal_error(memory_failed); /* Handle memory allocation failure */
        return NULL;
    }
    strcpy(dst, src); /* Copy the source string to the new memory */
    return dst; /* Return the duplicated string */
}


int valid_end(char *line, int index, struct file_status *file, int *error_exists) {
    char *ptr = line + index; /* Move to the given index */
    while (*ptr != '\0') {
        if (!isspace(*ptr)) { 
            *error_exists = TRUE;
            print_external_error(extra_text, file);
            return FALSE; /* Found non-whitespace */
        }
        ptr++;
    }
    return TRUE; /* Only whitespace after index */
}

char * next_param(char ** ptp){
	int i =0;
    char * param = (char *)malloc((MAX_LINE_LENGTH) * sizeof(char));
    if (param == NULL){
	        print_internal_error(memory_failed);
        return NULL; 
    }
    while(i<MAX_LINE_LENGTH && isprint(**ptp) &&!isspace(**ptp)&& (**ptp) != ','  && (**ptp) != EOF){
      param[i] = **ptp;
      (*ptp)++;
      i++; 
    }
    param[i] = '\0';
    return param;
}

void free_strings(int num_strings, ...) {
    int i = 0;
    va_list args;
    va_start(args, num_strings);

    for (; i < num_strings; ++i) {
        char *str = va_arg(args, char*);
        free(str);
    }

    va_end(args);
}

int figure_addressing_methods(char *opernad, struct symbols_linked_list *list) {
    /* Index indicating the addressing methods: 
     * IMMEDIATE 0 
     * DIRECT 1 
     * INDIRECT 2
     * REGISTER 3 
     * NO_OPPERAND 4 */
    if (opernad == NULL) {
        return NO_OPPERAND;
    }
    /* Skip '#' for immediate addressing */
    if (*opernad == '#') {
        opernad++;
    }
    /* Check if it's a number */
    if (isdigit(*opernad) || *opernad == '+' || *opernad == '-') {
        return IMMEDIATE;
    }
    /* Check for indirect addressing */
    if (opernad[0] == '*') { 
        return INDIRECT;
    }
    /* Check if it's a register */
    if (is_register(opernad)) {
        return REGISTER;
    }
    /* Otherwise, it's a label (direct addressing) */
    return DIRECT;
}


int fill_and_check(char *buffer, FILE *file) {
    char c;
    int index = 0;
    c = fgetc(file);
    /* Skip leading whitespace */
    while (isspace(c) != 0) {
        c = fgetc(file);
    }
    /* Fill the buffer with characters until newline, EOF, or max length */
    for (; index < MAX_LINE_LENGTH && c != '\n' && c != EOF; index++) {
        buffer[index] = c;
        c = fgetc(file);
    }
    /* Add newline and null terminator to the buffer */
    buffer[index++] = '\n';
    buffer[index] = '\0';
    /* Return TRUE if EOF is reached, otherwise FALSE */
    if (c == EOF) {
        return TRUE;
    }
    return FALSE;
}

int* decimalToBinary(int n, int bits) {
    int *binaryNum = (int *)malloc(bits * sizeof(int));
    /* Initialize the binary array with 0s */
    for (int i = 0; i < bits; i++) {
        binaryNum[i] = 0; 
    }
    /* Convert negative numbers using two's complement */
    if (n < 0) {
        n = (1 << bits) + n;
    }
    int i = bits - 1;
    /* Convert decimal to binary */
    while (n > 0 && i >= 0) {
        binaryNum[i] = n % 2;
        n = n / 2;
        i--;
    }
    return binaryNum;
}

void remove_spaces(char *str) {
    int i, j = 0;
    int len = strlen(str);
    /* Remove all spaces from the string */
    for (i = 0; i < len; i++) {
        if (str[i] != ' ') { 
            str[j++] = str[i]; 
        }
    }
    str[j] = '\0'; /* Null-terminate the modified string */
}


int compare_strings_n(const char *str1, const char *str2, size_t n) {
    // Compare up to n characters between str1 and str2
    for (size_t i = 0; i < n; i++) {
        if (str1[i] != str2[i]) {
            return 0; // Strings are different
        }
        // Check if either string ends before reaching n characters
        if (str1[i] == '\0' || str2[i] == '\0') {
            break;
        }
    }
    return 1; // Strings are equal up to n characters
}

