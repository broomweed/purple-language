typedef int stackElementT;

typedef struct stackTag {
    stackElementT value;
    struct stackTag *next;
} stackNodeT;

typedef struct {
   stackNodeT *top;
   char name[100];
} stackT;

extern void stack_init(stackT*, char*);
extern void stack_destroy(stackT*);
extern void stack_push(stackT*, stackElementT);
extern stackElementT stack_pop(stackT*);
extern int stack_empty(stackT*);
extern int stack_full(stackT*);
extern void stack_swap(stackT*);
extern stackElementT stack_peek(stackT*);
