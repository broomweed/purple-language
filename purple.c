#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "stack.h"
#include <errno.h>
#include <time.h>

typedef struct {
    int location;
    char* name;
} label;

void parse_line (char*, int);
void parse_command (char*, int);
void parse_label (char*, int);
void parse_function (int, int);
void quit();
char *str_trim (char*);
int str_prefix (char*, char*);
int str_parseint (char*, int*);
char *str_dup (char*);
label labels[1024];
label functions[1024];

int labelnum = 0;
int functionnum = 0;
char input[1024];
char *readLine;

int i;
int argcount = 0;
int inpcount = 0;
stackT stack;
stackT auxstack;
stackT callstack;
FILE *fp = NULL;

int main (int argc, char **argv) {
    stack_init(&stack, "Stack");
    stack_init(&callstack, "Call stack");
    stack_init(&auxstack, "Auxiliary stack");

    srand(time(NULL));
    if (argc == 2) {
        if (!strcmp(argv[1], "--about")){
            printf("PURPLE (PURPoseless LanguagE) Interpreter v. 0.1\n");
            printf("Compiled on %s at %s\n", __DATE__, __TIME__);
            exit(0);
        } else {
            fp = fopen(argv[1], "r+");
            if (fp == NULL) {
                fprintf(stderr, "error opening file at: %s\n", argv[1]);
                quit();
            }
        }
    } else {
        printf("Usage: %s [--about | <file-name>]\n", argv[0]);
        exit(1);
    }

    int line = 1;
    int startspecial = 0;

    readLine = malloc(1024*sizeof(char));

    while (fgets(readLine, 1024, fp)) {
        readLine = str_trim(readLine);
        if (readLine[0] == ':') {
            for (i=0; readLine[i]; i++) readLine[i] = tolower(readLine[i]);
            readLine[i] = '\0';
            labels[labelnum].location = ftell(fp);
            labels[labelnum].name = malloc(sizeof(char)*(strlen(readLine)));
            strncpy(labels[labelnum].name, readLine+1, strlen(readLine));
            labels[labelnum].name = str_trim(labels[labelnum].name);
            if (!strcmp(labels[labelnum].name, "start")) startspecial = 1;
            labelnum++;
        }
        if (readLine[0] == '~') {
            for (i=0; readLine[i]; i++) readLine[i] = tolower(readLine[i]);
            readLine[i] = '\0';
            functions[functionnum].location = ftell(fp);
            functions[functionnum].name = malloc(sizeof(char)*(strlen(readLine)));
            strcpy(functions[functionnum].name, readLine+1);
            functions[functionnum].name = str_trim(functions[functionnum].name);
            functionnum++;
        }
        line++;
    }

    fseek(fp, 0, SEEK_SET);
    line = 1;

    if (startspecial) parse_label("start", 0);

    while (fgets(readLine, 1000, fp)) {
        parse_line(readLine, line);
        line++;
    }

    printf("\nReached end without finding `END`!\n");

    free(readLine);

    quit();
}

void parse_line (char *line, int linenum) {
    char *command, *temp, numstring[32];
    int arguments[64];
    int stringmode = 0, numindex = 0,
        numtopush = 0, argindex = -1;

    line = str_trim(line);
    if(str_prefix(line, "#")) return;
    command = malloc(strlen(line)*sizeof(char)+1);

    for (i = 0; !isspace(line[i]) && line[i] != '\0'; i++) command[i] = line[i];
    command[i] = '\0';
    for (i=0; command[i]; i++) command[i] = tolower(command[i]);

    if(str_prefix(command, ":") || str_prefix(command, "~")) {
        free(command);
        return;
    }

    temp = str_dup(&line[strlen(command)]);
    temp = str_trim(temp);

    if(!strcmp(command, "goto")) {
        parse_label(temp, linenum);
        free(temp);
        free(command);
        return;
    }

    if(!strcmp(command, "zbr")) {
        if(!stack_peek(&stack)) {
            parse_label(temp, linenum);
        }
        free(temp);
        free(command);
        return;
    }

    if(!strcmp(command, "nzbr")) {
        if(stack_peek(&stack)) {
            parse_label(temp, linenum);
        }
        free(temp);
        free(command);
        return;
    }

    if(!strcmp(command, "ebr")) {
        if (stack_empty(&stack)) {
            parse_label(temp, linenum);
        }
        free(temp);
        free(command);
        return;
    }

    if(!strcmp(command, "nebr")) {
        if (!stack_empty(&stack)) {
            parse_label(temp, linenum);
        }
        free(temp);
        free(command);
        return;
    }

    if(strcmp(temp, "")) temp = strcat(temp, ",");

    for (i = 0; temp[i]; i++) {
        if (temp[i] == '"') {
            stringmode = !stringmode;
        } else if (stringmode) {
            if (temp[i] != '\0') {
                argindex++;
                arguments[argindex] = temp[i];
            }
        } else {
            /* number mode */
            if (temp[i] == ',') {
                if (numindex != 0) { //i.e. it's not ""
                    /* if numstring IS "", most likely it's a comma after a string entry,
                       which we don't want to push 0 for */
                    numstring[numindex] = '\0';
                    if (str_parseint(numstring, &numtopush)) {
                        argindex++;
                        arguments[argindex] = numtopush;
                    } else {
                        fprintf(stderr, "\ninvalid integer: `%s` at line %d", numstring, linenum);
                    }
                    numindex = 0;
                }
            } else if (isdigit(temp[i]) || temp[i] == '-') {
                numstring[numindex] = temp[i];
                numindex++;
            } else if (!isspace(temp[i])) {
                fprintf(stderr, "\nunexpected character: `%c` at line %d", temp[i], linenum);
                numindex = 0;
            }
        }
    }

    if (stringmode) {
        fprintf(stderr, "\nunclosed quote at line %d", linenum);
        free(temp);
        free(command);
        return;
    } else {
        numstring[numindex] = '\0';
        if (str_parseint(numstring, &numtopush)) {
            argindex++;
            arguments[argindex] = numtopush;
        } else {
            fprintf(stderr, "\ninvalid integer: `%s` at line %d", numstring, linenum);
        }
    }

    /* now push the array onto the stack, in reverse order */
    for (i = argindex-1; i >= 0; i--) {
        stack_push(&stack, arguments[i]);
    }

    if (argindex > 0) {
        argcount = argindex;
    }

    /* printf("\nline %d: parsing command `%s`.", linenum, command); */

    parse_command(command, linenum);

    free(command);
    free(temp);
}

void parse_command (char *command, int linenum) {
    char *temp;
    char writeme[1024];
    //int val = 0;

    temp = malloc(strlen(command)*sizeof(char) + 1);
    strcpy(temp, command);
    if (temp) {
        /* trim whitespace */
        temp = str_trim(temp);
        for (i=0; i < functionnum; i++) {
            if (!strcmp(functions[i].name, command)) {
                parse_function(i, linenum);
                free(temp);
                return;
            }
        }
        if(strcmp(temp, "end") == 0) {
            quit();
        } else if (strcmp(temp, "pop") == 0) {
            stack_pop(&stack);
        } else if (!strcmp(temp, "dup")) {
            stackElementT tempElement = stack_pop(&stack);
            stack_push(&stack, tempElement);
            stack_push(&stack, tempElement);
        } else if (strcmp(temp, "add") == 0) {
            stack_push(&stack, stack_pop(&stack) + stack_pop(&stack));
        } else if (!strcmp(temp, "push")) {
            /* push is basically a noop now since the argument handling does it instead */
        } else if (!strcmp(temp, "print")) {
            putchar(stack_peek(&stack));
        } else if (!strcmp(temp, "iprint")) {
            printf("%d", stack_peek(&stack));
        } else if (!strcmp(temp, "line")) {
            printf("\n");
        } else if (!strcmp(temp, "swap")) {
            stack_swap(&stack);
        } else if (!strcmp(temp, "zsk")) {
            if (stack_peek(&stack) == 0) {
                fgets(writeme, 1024, fp);
            }
        } else if (!strcmp(temp, "nzsk")) {
            if (stack_peek(&stack) != 0) {
                fgets(writeme, 1024, fp);
            }
        } else if (!strcmp(temp, "esk")) {
            if (stack_empty(&stack)) {
                fgets(writeme, 1024, fp);
            }
        } else if (!strcmp(temp, "nesk")) {
            if (!stack_empty(&stack)) {
                fgets(writeme, 1024, fp);
            }
        } else if (!strcmp(temp, "return")) {
            fseek(fp, stack_pop(&callstack), SEEK_SET);
        } else if (!strcmp(temp, "give")) {
            stack_push(&auxstack, stack_pop(&stack));
        } else if (!strcmp(temp, "take")) {
            stack_push(&stack, stack_pop(&auxstack));
        } else if (!strcmp(temp, "argc")) {
            stack_push(&stack, argcount);
        } else if (!strcmp(temp, "neg")) {
            stack_push(&stack, -1*stack_pop(&stack));
        } else if (!strcmp(temp, "input")) {
            fgets(writeme, 1024, stdin);
            inpcount = strlen(writeme)-1;
            for (i = inpcount; i >= 0; i--) {
                if (writeme[i] != '\n') {
                    stack_push(&stack, writeme[i]);
                }
            }
        } else if (!strcmp(temp, "inpc")) {
            stack_push(&stack, inpcount);
        } else if (!strcmp(temp, "rand")) {
            stack_push(&stack, rand() % stack_pop(&stack));
        } else if (!strcmp(temp, "")) {
            // DON'T DO ANYTHING, because it's an empty command
        } else if (!strcmp(temp, "stack")) {
            printf("<- ");
            stackNodeT *node = stack.top;
            while (node) {
                printf("%d ", node->value);
                node = node->next;
            }
            printf("]\n");
        } else {
            fprintf(stderr, "\nunrecognized command: `%s` at line %d.", temp, linenum);
        }
        /*if (strcmp(temp, "stack")) {
            printf("STACK : ");
            parse_command("stack");
        }*/
    }

    free(temp);
}

void parse_label(char *label, int linenum) {
    for (i=0; i<labelnum; i++) {
        if (!strcmp(labels[i].name, label)) {
            fseek(fp, labels[i].location, SEEK_SET);
            return;
        }
    }
    fprintf(stderr, "\ninvalid label: `%s` at line %d.\n", label, linenum);
}

void parse_function(int id, int linenum) {
    stack_push(&callstack, ftell(fp));
    fseek(fp, functions[i].location, SEEK_SET);
}

void quit() {

    /* ok OS you can have your memory back now */
    fclose(fp);

    for (i = 0; i < labelnum; i++) {
        free(labels[i].name);
    }

    for (i = 0; i < functionnum; i++) {
        free(functions[i].name);
    }

    stack_destroy(&stack);
    stack_destroy(&auxstack);
    stack_destroy(&callstack);

    free(readLine);

    exit(0);
}

char *str_trim (char *string) {
    int i = 0;
    int j = strlen (string) - 1;
    int k = 0;

    while (isspace (string[i]) && string[i] != '\0')
        i++;

    while (j >= 0 && isspace (string[j]))
        j--;

    while (i <= j)
        string[k++] = string[i++];

    string[k] = '\0';

    return string;
}

int str_prefix(char *string, char *prefix) {
    return strncmp(prefix, string, strlen(prefix)) == 0;
}

int str_parseint(char *string, int *i) {
    char *ep;
    long l;

    l = strtol(string, &ep, 0);

    if (*ep != 0) {
        return 0;
    }

    *i = (int)l;
    return 1;
}

char *str_dup (char *str) {
    char *newstr = malloc ((strlen(str)+1) * sizeof(char));
    if (newstr) {
        strcpy(newstr, str);
    }
    return newstr;
}
