#include "tokeniser.h"
#include "fsm.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define UNUSED(arg) (void)(arg)

struct tokeniser_st
{
    Fsm fsm;

    new_token_cb user_callback;
    void * user_arg;
    char * current_token;
    size_t char_count;
    size_t token_start; /* The position where we started reading a token. */
    tokeniser_result_t result;
    char expected_close_quote;
};

typedef struct tokeniser_event_st
{
    Event _super;
    int current_char;
} tokeniser_event_st;

static void tokeniser_state_init(tokeniser_st * const tokeniser, Event const * const event_fsm);
static void tokeniser_state_no_token(tokeniser_st * const tokeniser, Event const * const event_fsm);
static void tokeniser_state_done(tokeniser_st * const tokeniser, Event const * const event_fsm);
static void tokeniser_state_regular_token(tokeniser_st * const tokeniser, Event const * const event_fsm);

static void str_extend(char * * const str, char const new_char)
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

static void current_token_free(tokeniser_st * const tokeniser)
{
    free(tokeniser->current_token);
    tokeniser->current_token = NULL;
}

static void current_token_init(tokeniser_st * const tokeniser, char const first_char)
{
    current_token_free(tokeniser);
    tokeniser->token_start = tokeniser->char_count;
    if (first_char != '\0')
    {
        str_extend(&tokeniser->current_token, first_char);
    }
}

static void tokeniser_result_set(tokeniser_st * const tokeniser, tokeniser_result_t const result)
{
    tokeniser->result = result;
}

static void tokeniser_state_init(tokeniser_st * const tokeniser, Event const * const event_fsm)
{
    UNUSED(event_fsm);

    printf("Initialising tokeniser\n");

    Fsm_state_transition(&tokeniser->fsm, tokeniser_state_no_token);
}

static void tokeniser_state_done(tokeniser_st * const tokeniser, Event const * const event_fsm)
{
    UNUSED(tokeniser);
    UNUSED(event_fsm);

    printf("Not handling any char in 'done' state\n");
    tokeniser->result = tokeniser_result_already_done;
}

static void tokeniser_state_quoted_token(tokeniser_st * const tokeniser, Event const * const event_fsm)
{
    tokeniser_event_st * const event = (tokeniser_event_st *)event_fsm;

    if (event->current_char == '\r')
    {
        /* Ignore. */
    }
    else if (event->current_char == tokeniser->expected_close_quote)
    {
        if (tokeniser->user_callback != NULL)
        {
            tokeniser->user_callback(tokeniser->current_token,
                                     tokeniser->token_start, 
                                     tokeniser->char_count + 1, /* Include the closing quote. */
                                     tokeniser->user_arg);
        }
        current_token_free(tokeniser); 
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_no_token);
    }
    else if (event->current_char == TOKENISER_EOF || event->current_char == '\n')
    {
        if (tokeniser->user_callback != NULL)
        {
            tokeniser->user_callback(tokeniser->current_token,
                                     tokeniser->token_start, 
                                     tokeniser->char_count, tokeniser->user_arg);
        }
        current_token_free(tokeniser);
        tokeniser_result_set(tokeniser, tokeniser_result_incomplete_token);
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_done);
    }
    else 
    {
        str_extend(&tokeniser->current_token, event->current_char);
    }
}

static void tokeniser_state_quoted_regular_token(tokeniser_st * const tokeniser, Event const * const event_fsm)
{
    tokeniser_event_st * const event = (tokeniser_event_st *)event_fsm;

    if (event->current_char == '\r')
    {
        /* Ignore. */
    }
    else if (event->current_char == tokeniser->expected_close_quote)
    {
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_regular_token);
    }
    else if (event->current_char == TOKENISER_EOF || event->current_char == '\n')
    {
        if (tokeniser->user_callback != NULL)
        {
            tokeniser->user_callback(tokeniser->current_token,
                                     tokeniser->token_start, 
                                     tokeniser->char_count,
                                     tokeniser->user_arg);
        }
        current_token_free(tokeniser);
        tokeniser_result_set(tokeniser, tokeniser_result_incomplete_token);
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_done);
    }
    else
    {
        str_extend(&tokeniser->current_token, event->current_char);
    }
}

static void tokeniser_state_regular_token(tokeniser_st * const tokeniser, Event const * const event_fsm)
{
    tokeniser_event_st * const event = (tokeniser_event_st *)event_fsm;

    if (event->current_char == '\r')
    {
        /* Ignore. */
    }
    else if (event->current_char == TOKENISER_EOF || event->current_char == '\n')
    {
        if (tokeniser->user_callback != NULL)
        {
            tokeniser->user_callback(tokeniser->current_token,
                                     tokeniser->token_start, 
                                     tokeniser->char_count,
                                     tokeniser->user_arg);
        }
        current_token_free(tokeniser);
        tokeniser_result_set(tokeniser, tokeniser_result_ok);
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_done);
    }
    else if (event->current_char == '\"' || event->current_char == '\'')
    {
        tokeniser->expected_close_quote = event->current_char;
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_quoted_regular_token);
    }
    else if (isspace(event->current_char))
    {
        if (tokeniser->user_callback != NULL)
        {
            tokeniser->user_callback(tokeniser->current_token,
                                     tokeniser->token_start, 
                                     tokeniser->char_count,
                                     tokeniser->user_arg);
        }
        current_token_free(tokeniser);
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_no_token);
    }
    else
    {
        str_extend(&tokeniser->current_token, event->current_char);
    }
}

static void tokeniser_state_no_token(tokeniser_st * const tokeniser, Event const * const event_fsm)
{
    tokeniser_event_st * const event = (tokeniser_event_st *)event_fsm;

    if (event->current_char == TOKENISER_EOF || event->current_char == '\n')
    {
        tokeniser_result_set(tokeniser, tokeniser_result_ok);
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_done);
    }
    else if (event->current_char == '\r' || isspace(event->current_char))
    {
        /* Ignore. */
    }
    else if (event->current_char == '\"' || event->current_char == '\'')
    {
        tokeniser->expected_close_quote = event->current_char;
        current_token_init(tokeniser, '\0');
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_quoted_token);
    }
    else
    {
        current_token_init(tokeniser, (char)event->current_char);
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_regular_token);
    }
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

    Fsm_constructor(&tokeniser->fsm, tokeniser_state_init);
    Fsm_init(&tokeniser->fsm, NULL);
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

static void tokeniser_dispatch(tokeniser_st * const tokeniser, tokeniser_event_st const * const tokeniser_event)
{
    Fsm_dispatch(&tokeniser->fsm, tokeniser_event);
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

    tokeniser->char_count++; /* Update the number of characters we've processed. */

    result = tokeniser->result;

done:
    return result;
}
