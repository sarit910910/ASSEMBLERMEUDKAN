
#include <string.h>
#include "Arrays.h"
#include "Global_def.h"

void set_bit(bit_field *bitfield, int pos, int value)
{
    if (value)
        bitfield->data |= (1 << pos); /* Set the bit at position pos */
    else
        bitfield->data &= ~(1 << pos); /* Clear the bit at position pos */
}

LabelNode* createLabelNode(const char *label, int address) {
    LabelNode* newNode = (LabelNode*)malloc(sizeof(LabelNode));
    newNode->label = strdup(label);  
    newNode->address = address;
    newNode->next = NULL;
    return newNode;
}

void addLabelNode(LabelNode **head, const char *label, int address) {
    LabelNode* newNode = createLabelNode(label, address);
    newNode->next = *head;
    *head = newNode;
}

void freeLabelList(LabelNode *head) {
    LabelNode *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp->label);  
        free(temp);  
    }
}

void printLabelList(LabelNode *head) {
    LabelNode *current = head;
    while (current != NULL) {
        printf("Label: %s, Address: %d\n", current->label, current->address);
        current = current->next;
    }
}
