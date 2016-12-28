#include "tokeniser_states.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void str_extend(char * * const str, char const new_char)
{
    char * existing_str = *str;
    char new_str[2];

    new_str[0] = new_char;
    new_str[1] = '\0';

    if (existing_str == NULL)
    {
        existing_str = malloc(2);
        if (existing_str == NULL)
        {
            goto done;
        }
        existing_str[0] = '\0';
    }
    else
    {
        /* Allow space for the NUL terminator and the new char. */
        existing_str = realloc(existing_str, strlen(existing_str) + 2);
        if (existing_str == NULL)
        {
            goto done;
        }
    }

    strcat(existing_str, new_str);

done:
    *str = existing_str;
}

void current_token_free(tokeniser_st * const tokeniser)
{
    free(tokeniser->current_token);
    tokeniser->current_token = NULL;
}

void current_token_init(tokeniser_st * const tokeniser, char const first_char)
{
    current_token_free(tokeniser);
    tokeniser->token_start = tokeniser->char_count;
    if (first_char != '\0')
    {
        str_extend(&tokeniser->current_token, first_char);
    }
}

void tokeniser_result_set(tokeniser_st * const tokeniser, tokeniser_result_t const result)
{
    tokeniser->result = result;
}

void tokeniser_free(tokeniser_st * const tokeniser)
{
    if (tokeniser == NULL)
    {
        goto done;
    }

    current_token_free(tokeniser);

    free(tokeniser);

done:
    return;
}

void tokeniser_init(tokeniser_st * const tokeniser)
{
    current_token_free(tokeniser);
    tokeniser->user_callback = NULL;
    tokeniser->user_arg = NULL;
    tokeniser->char_count = 0;

    tokeniser_init_fsm(tokeniser);
}

tokeniser_st * tokeniser_alloc(void)
{
    tokeniser_st * tokeniser = malloc(sizeof *tokeniser);

    if (tokeniser == NULL)
    {
        goto done;
    }

    tokeniser->current_token = NULL;
    tokeniser_init(tokeniser);

done:
    return tokeniser;
}

tokeniser_result_t tokeniser_feed(tokeniser_st * const tokeniser,
                                  int const next_char,
                                  new_token_cb const user_callback,
                                  void * const user_arg)
{
    tokeniser_result_t result;
    tokeniser_event_st tokeniser_event; 

    if (tokeniser == NULL)
    {
        result = tokeniser_result_error;
        goto done;
    }

    tokeniser->user_callback = user_callback;
    tokeniser->user_arg = user_arg;

    /* The default result will be continue unless an error is 
     * encountered or EOF or EOL is hit. 
     */
    tokeniser->result = tokeniser_result_continue;

    tokeniser_event.current_char = next_char;

    tokeniser_dispatch(tokeniser, &tokeniser_event);

    tokeniser->char_count++; /* Update the number of characters processed. */

    result = tokeniser->result;

done:
    return result;
}
