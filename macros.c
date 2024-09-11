#include <stdlib.h>
#include <string.h>
#include "macros.h"

/* Initialize a Macro structure */
void Macro_init(
    struct Macro *macroPtr, 
    const char *name     
)
{
    strcpy(macroPtr->name, name); /* Set macro name */
    macroPtr->lines = NULL; /* No lines yet */
    macroPtr->counter_line = 0; /* Line count is zero */
    macroPtr->max_line_limit = 0; /* No limit set yet */
}

/* Free memory used by a Macro */
void Macro_free(struct Macro *macroPtr)
{
    int id;
    for (id = 0; id < macroPtr->counter_line; id++)
    {
        free(macroPtr->lines[id]); /* Free each line */
    }
    free(macroPtr->lines); /* Free the lines array */
}

/* Append a line to a Macro */
void Macro_append(
    struct Macro *macroPtr, 
    const char *line     
)
{
    char *copy_line;
    if (macroPtr->counter_line == macroPtr->max_line_limit)
    {
        macroPtr->max_line_limit += MEMORY_BLOCK_SIZE; /* Increase line limit */
        macroPtr->lines = realloc(
            macroPtr->lines,
            sizeof(*macroPtr->lines) * macroPtr->max_line_limit); /* Reallocate memory */
    }
    copy_line = malloc(strlen(line) + 1); /* Allocate memory for new line */
    strcpy(copy_line, line); /* Copy the line */
    macroPtr->lines[macroPtr->counter_line] = copy_line; /* Add line to array */
    macroPtr->counter_line++; /* Increment line count */
}

/* Initialize a list of Macros */
void MacrosList_init(struct MacrosList *collection)
{
    collection->macros = NULL; /* No macros yet */
    collection->macro_count = 0; /* Count is zero */
    collection->macro_limit = 0; /* Limit is zero */
}

/* Free memory used by a MacrosList */
void MacrosList_free(struct MacrosList *collection)
{
    int id;
    for (id = 0; id < collection->macro_count; id++)
    {
        Macro_free(collection->macros[id]); /* Free each Macro */
        free(collection->macros[id]); /* Free Macro pointer */
    }
    free(collection->macros); /* Free macros array */
}

/* Find a Macro by name in the list */
struct Macro *MacrosList_find(
    struct MacrosList *collection, 
    const char *name                    
)
{
    int id;
    struct Macro *macroPtr = NULL; /* Default to NULL */
    for (id = 0; id < collection->macro_count; id++)
    {
        if (strcmp(collection->macros[id]->name, name) == 0)
        {
            macroPtr = collection->macros[id]; /* Found the Macro */
            break;
        }
    }
    return macroPtr; /* Return the Macro or NULL */
}

/* Register a new Macro in the list */
struct Macro *MacrosList_register(
    struct MacrosList *collection, 
    const char *name                    
)
{
    int id;
    struct Macro *macroPtr;
    for (id = 0; id < collection->macro_count; id++)
    {
        if (strcmp(collection->macros[id]->name, name) == 0)
        {
            return NULL; /* Macro already exists */
        }
    }
    if (collection->macro_count == collection->macro_limit)
    {
        collection->macro_limit += MEMORY_BLOCK_SIZE; /* Increase limit */
        collection->macros = realloc(
            collection->macros,
            sizeof(*collection->macros) * collection->macro_limit); /* Reallocate memory */
    }
    macroPtr = malloc(sizeof(*macroPtr)); /* Allocate memory for new Macro */
    Macro_init(macroPtr, name); /* Initialize Macro */
    collection->macros[collection->macro_count] = macroPtr; /* Add Macro to list */
    collection->macro_count++; /* Increment count */
    return macroPtr; /* Return new Macro */
}
