#pragma once 

#include "definitions.h" 

/* Structure for a macro */
struct Macro
{
    char name[TOTAL_LEN]; /* Macro name */
    char **lines; /* Lines of the macro */
    int counter_line; /* Current line count */
    int max_line_limit; /* Maximum line count */
};

/* Initialize a Macro */
void Macro_init(
    struct Macro* macroPtr,
    const char* name);

 /* Free memory used by a Macro */
void Macro_free(struct Macro *macroPtr);

/* Append a line to a Macro */
void Macro_append(
    struct Macro* macroPtr,
    const char* line);

/* Structure for a list of macros */
struct MacrosList
{
    struct Macro **macros; /* Array of macro pointers */
    int macro_count; /* Number of macros in the list */
    int macro_limit; /* Limit of macros in the list */
};

/* Initialize a MacrosList */
void MacrosList_init(struct MacrosList *collection);

/* Free memory used by a MacrosList */
void MacrosList_free(struct MacrosList *collection);

/* Find a Macro by name in the MacrosList */
struct Macro *MacrosList_find(
    struct MacrosList *collection,
    const char *name);

/* Register a new Macro in the MacrosList */
struct Macro *MacrosList_register(
    struct MacrosList *collection,
    const char *name);
