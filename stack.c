#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void stack_init(stackT*, char*);
void stack_destroy(stackT*);
int stack_empty(stackT*);
int stack_full(stackT*);
void stack_push(stackT*, int);
void stack_swap(stackT*);
int stack_pop(stackT*);

void stack_init (stackT *stackP, char *name) {
   stackP->top = NULL;
   strcpy(stackP->name, name);
}

void stack_destroy (stackT *stackP) {
    stackNodeT* node = stackP->top;
    while (node) {
        stackNodeT* nextNode = node->next;
        free(node);
        node = nextNode;
    }
    stackP->top = NULL;
}

int stack_empty (stackT *stackP) {
    return stackP->top == NULL;
}

void stack_push (stackT *stackP, stackElementT element) {
    stackNodeT* newNode = (stackNodeT*)malloc(sizeof(stackNodeT));
    newNode->value = element;
    newNode->next = stackP->top;
    stackP->top = newNode;
}

int stack_pop (stackT *stackP) {
    if (stack_empty(stackP)) {
        fprintf(stderr, "%s is empty.\n", stackP->name);
        exit(1);
    }
    stackNodeT* oldTop = stackP->top;

    int retVal = stackP->top->value;
    stackP->top = stackP->top->next;
    free(oldTop);
    return retVal;
}

void stack_swap (stackT *stackP) {
    stackElementT temp, temp2;
    temp = stack_pop(stackP);
    temp2 = stack_pop(stackP);
    stack_push(stackP, temp);
    stack_push(stackP, temp2);
}

stackElementT stack_peek (stackT *stackP) {
    return stackP->top->value;
}
