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

    getc_cb user_getc_fn;
    void * user_arg;
    char * current_token;
    tokens_st * tokens;
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
    if (first_char != '\0')
    {
        str_extend(&tokeniser->current_token, first_char);
    }
}

static void tokeniser_done(tokeniser_st * const tokeniser, tokeniser_result_t const result)
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
        tokens_add_token(tokeniser->tokens, tokeniser->current_token);
        current_token_free(tokeniser); 
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_no_token);
    }
    else if (event->current_char == EOF || event->current_char == '\n')
    {
        tokens_add_token(tokeniser->tokens, tokeniser->current_token);
        current_token_free(tokeniser);
        tokeniser_done(tokeniser, tokeniser_result_incomplete_token);
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
    else if (event->current_char == EOF || event->current_char == '\n')
    {
        tokens_add_token(tokeniser->tokens, tokeniser->current_token);
        current_token_free(tokeniser);
        tokeniser_done(tokeniser, tokeniser_result_incomplete_token);
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
    else if (event->current_char == EOF || event->current_char == '\n')
    {
        tokens_add_token(tokeniser->tokens, tokeniser->current_token);
        tokeniser_done(tokeniser, tokeniser_result_ok);
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_done);
    }
    else if (event->current_char == '\"' || event->current_char == '\'')
    {
        tokeniser->expected_close_quote = event->current_char;
        Fsm_state_transition(&tokeniser->fsm, tokeniser_state_quoted_regular_token);
    }
    else if (isspace(event->current_char))
    {
        tokens_add_token(tokeniser->tokens, tokeniser->current_token);
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

    if (event->current_char == EOF || event->current_char == '\n')
    {
        tokeniser_done(tokeniser, tokeniser_result_ok);
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

    tokens_free(tokeniser->tokens);
    current_token_free(tokeniser);

    free(tokeniser);

done:
    return;
}

tokeniser_st * tokeniser_alloc(void)
{
    tokeniser_st * tokeniser = malloc(sizeof *tokeniser);

    if (tokeniser == NULL)
    {
        goto done;
    }

    tokeniser->current_token = NULL;
    tokeniser->user_arg = NULL;
    tokeniser->user_getc_fn = NULL;

    tokeniser->tokens = tokens_alloc();

    if (tokeniser->tokens == NULL)
    {
        free(tokeniser);
        tokeniser = NULL;
    }

    Fsm_constructor(&tokeniser->fsm, tokeniser_state_init);
    Fsm_init(&tokeniser->fsm, NULL); 

done:
    return tokeniser;
}

void tokeniser_init(tokeniser_st * const tokeniser, getc_cb const user_getc_fn, void * user_arg)
{
    if (tokeniser == NULL)
    {
        goto done;
    }

    tokeniser->user_getc_fn = user_getc_fn;
    tokeniser->user_arg = user_arg;

done:
    return;
}

static void tokeniser_dispatch(tokeniser_st * const tokeniser, tokeniser_event_st const * const tokeniser_event)
{
    Fsm_dispatch(&tokeniser->fsm, tokeniser_event);
}

tokeniser_result_t tokeniser_tokenise(tokeniser_st * const tokeniser, tokens_st * * const tokens)
{
    UNUSED(tokeniser);
    tokeniser_result_t result;

    *tokens = NULL;
    if (tokeniser == NULL)
    {
        result = tokeniser_result_error;
        goto done;
    }

    if (tokeniser->user_getc_fn == NULL)
    {
        printf("Call init you dummy\n");
        result = tokeniser_result_init_not_called;
        goto done;
    }

    tokeniser->result = tokeniser_result_error;

    while (Fsm_current_state(&tokeniser->fsm) != (State)tokeniser_state_done)
    {
        tokeniser_event_st tokeniser_event;

        tokeniser_event.current_char = tokeniser->user_getc_fn(tokeniser->user_arg);

        tokeniser_dispatch(tokeniser, &tokeniser_event);
    }

    result = tokeniser->result;
    *tokens = tokeniser->tokens;
    tokeniser->tokens = NULL; 

done:

    return result;
}
