#include <ctype.h>
#include "definitions.h"
#include "tokens.h"

char token_buffer[TOTAL_LEN];

int total_words_in_row(const char *line)
{
    char *word;
    int count;

    count = 0;

    while (TRUE)
    {
        word = get_word_number(line, count); /* Get word at index 'count' */

        if (*word == '\0') /* Check for end of line */
        {
            break;
        }

        count++;
    }

    return count; /* Return total number of words */
}

char *get_word_number(const char *line, int search_index)
{
    char current_char;
    int start_index;
    int end_index;
    int copy_index;
    int token_index;
    int is_whitespace;
    int is_comma;
    int is_colon;
    int is_end;
    int is_delimiter;

    token_buffer[0] = '\0'; /* Initialize buffer */

    start_index = 0;
    end_index = 0;
    token_index = 0;

    while (TRUE)
    {
        if (search_index < token_index) /* Check if index is past current token */
        {
            break;
        }

        while (TRUE)
        {
            current_char = line[start_index]; /* Get current character */

            if (current_char == ';')
            {
                current_char = '\0'; /* End of comment */
            }

            if (!isspace(current_char)) /* Check for non-whitespace */
            {
                break;
            }

            start_index++;
        }

        end_index = start_index;

        while (TRUE)
        {
            current_char = line[end_index]; /* Get character for token end */

            if (current_char == ';')
            {
                current_char = '\0'; /* End of comment */
            }

            is_whitespace = isspace(current_char);
            is_comma = current_char == ',';
            is_colon = current_char == ':';
            is_end = current_char == '\0';

            if (is_whitespace || is_comma || is_colon || is_end)
            {
                is_delimiter = is_comma || is_colon;
                if (start_index == end_index && is_delimiter)
                {
                    end_index++;
                }
                break;
            }

            end_index++;
        }

        if (search_index == token_index)
        {
            copy_index = 0;
            while (start_index < end_index)
            {
                current_char = line[start_index];

                if (current_char == ';')
                {
                    current_char = '\0'; /* End of comment */
                }

                token_buffer[copy_index] = current_char;
                copy_index++;

                start_index++;
            }

            token_buffer[copy_index] = '\0'; /* Null-terminate buffer */
        }

        start_index = end_index;

        token_index++;
    }

    return token_buffer; /* Return the extracted token */
}
