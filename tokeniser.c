#include "tokeniser.h"
#include "fsm.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define UNUSED(arg) (void)(arg)

struct tokeniser_st
{
    Fsm fsm;

    getc_cb user_getc_fn;
    void * user_arg;
    char * current_token;
    tokens_st * tokens;
    bool done;
    tokeniser_result_t result;
};

typedef struct tokeniser_event_st
{
    Event _super;
    int current_char;
} tokeniser_event_st;

static void tokeniser_state_init(tokeniser_st * const tokeniser, Event const * const event_fsm);
static void tokeniser_state_no_token(tokeniser_st * const tokeniser, Event const * const event_fsm);
static void tokeniser_state_done(tokeniser_st * const tokeniser, Event const * const event_fsm);

static void tokeniser_done(tokeniser_st * const tokeniser, tokeniser_result_t const result)
{
    tokeniser->done = true;
    tokeniser->result = result;
}

static void tokeniser_state_init(tokeniser_st * const tokeniser, Event const * const event_fsm)
{
    UNUSED(event_fsm);

    printf("Initialising tokeniser");
    tokeniser->done = false;

    Fsm_state_transition(&tokeniser->fsm, tokeniser_state_no_token);
}

static void tokeniser_state_done(tokeniser_st * const tokeniser, Event const * const event_fsm)
{
    UNUSED(tokeniser);
    UNUSED(event_fsm);

    printf("Not handling any char in 'done' state");
}

static void tokeniser_state_no_token(tokeniser_st * const tokeniser, Event const * const event_fsm)
{
    tokeniser_event_st * const event = (tokeniser_event_st *)event_fsm;

    switch (event->current_char)
    {
        case '\r': /* Ignore. */
            break;
        case EOF:
        case '\n':
            tokeniser_done(tokeniser, tokeniser_result_ok);
            Fsm_state_transition(&tokeniser->fsm, tokeniser_state_done);
            break;
        default:
            printf("dealing with %c\n", event->current_char);
            break;
    }
}

void tokeniser_free(tokeniser_st * const tokeniser)
{
    if (tokeniser == NULL)
    {
        goto done;
    }

    tokens_free(tokeniser->tokens);
    free(tokeniser->current_token);

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

    if (tokeniser->user_getc_fn == NULL)
    {
        printf("Call init you dummy\n");
        result = tokeniser_result_init_not_called;
        goto done;
    }

    tokeniser->result = tokeniser_result_error;

    while (!tokeniser->done)
    {
        tokeniser_event_st tokeniser_event;

        tokeniser_event.current_char = tokeniser->user_getc_fn(tokeniser->user_arg);

        tokeniser_dispatch(tokeniser, &tokeniser_event);
    }

    result = tokeniser->result;

done:
    return result;
}
