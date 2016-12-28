#include "tokeniser_states.h"

#include <ctype.h>

static void tokeniser_state_init(fsm_class * const fsm, fsm_event const * const event_fsm);
static void tokeniser_state_no_token(fsm_class * const fsm, fsm_event const * const event_fsm);
static void tokeniser_state_done(fsm_class * const fsm, fsm_event const * const event_fsm);
static void tokeniser_state_regular_token(fsm_class * const fsm, fsm_event const * const event_fsm);

static void got_token(tokeniser_st * const tokeniser, size_t const end_index, char const quote_char)
{
    /* Called when a complete token has just been created. 
     * Notify the user if they are interested, and free the token 
     * just created. 
     */
    if (tokeniser->user_callback != NULL)
    {
        tokeniser->user_callback(tokeniser->current_token,
                                 tokeniser->token_start,
                                 end_index,
                                 quote_char,
                                 tokeniser->user_arg);
    }
    current_token_free(tokeniser);
}

static void tokeniser_state_init(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Initial state. Unconditionally transition to the first 
     * working state. No events handled in this state. 
     */
    UNUSED(event_fsm);

    Fsm_state_transition(fsm, tokeniser_state_no_token);
}

static void tokeniser_state_done(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* The input line has been tokenised. No events handled in 
     * this state. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    UNUSED(event_fsm);

    tokeniser->result = tokeniser_result_already_done;
}

static void tokeniser_state_quoted_token(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Inside a quoted ('\"' or '\''token. Append new chars to the 
     * current token until the closing quote character is received. 
     * If the line ends before the closing quote is received, set 
     * the result to incomplete token. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    tokeniser_event_st * const event = FSM_EVENT_TO_TOKENISER_EVENT(event_fsm);

    if (event->current_char == '\r')
    {
        /* Ignore. */
    }
    else if (event->current_char == tokeniser->expected_close_quote)
    {
        got_token(tokeniser,
                  tokeniser->char_count + 1, /* Include the closing quote in the end index. */
                  tokeniser->expected_close_quote);
        Fsm_state_transition(fsm, tokeniser_state_no_token);
    }
    else if (event->current_char == TOKENISER_EOF || event->current_char == '\0' || event->current_char == '\n')
    {
        got_token(tokeniser,
                  tokeniser->char_count,
                  '\0');
        Fsm_state_transition(fsm, tokeniser_state_done);
        tokeniser_result_set(tokeniser, tokeniser_result_incomplete_token);
    }
    else
    {
        str_extend(&tokeniser->current_token, event->current_char);
    }
}

static void tokeniser_state_quoted_regular_token(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Processing a regular token that contains embedded quotes 
     * (e.g. abc" def "ghi), and are within the quoted section of 
     * the token. Append new chars to the current token until the 
     * closing quote character is received. If the line ends before 
     * the closing quote is received, set the result to incomplete 
     * token. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    tokeniser_event_st * const event = FSM_EVENT_TO_TOKENISER_EVENT(event_fsm);

    if (event->current_char == '\r')
    {
        /* Ignore. */
    }
    else if (event->current_char == tokeniser->expected_close_quote)
    {
        /* Received the matching close quote character. */
        Fsm_state_transition(fsm, tokeniser_state_regular_token);
    }
    else if (event->current_char == TOKENISER_EOF || event->current_char == '\0' || event->current_char == '\n')
    {
        /* No more input. */
        got_token(tokeniser,
                  tokeniser->char_count,
                  '\0');
        Fsm_state_transition(fsm, tokeniser_state_done);
        tokeniser_result_set(tokeniser, tokeniser_result_incomplete_token);
    }
    else
    {
        /* Add the new char into the current token. */
        str_extend(&tokeniser->current_token, event->current_char);
    }
}

static void tokeniser_state_regular_token(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Processing a regular, unquoted token. Append new chars to the 
     * current token. If the current char is a space, this signals 
     * the end of the token. If the current char is a quote ('\"' or 
     * '\'' this signifies the start of a quoted regular token. 
     * If EOF or NUL or NEWLINE is received, this signifies the end 
     * of the input.  
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    tokeniser_event_st * const event = FSM_EVENT_TO_TOKENISER_EVENT(event_fsm);

    if (event->current_char == '\r')
    {
        /* Ignore. */
    }
    else if (event->current_char == TOKENISER_EOF || event->current_char == '\0' || event->current_char == '\n')
    {
        got_token(tokeniser,
                  tokeniser->char_count,
                  '\0');
        Fsm_state_transition(fsm, tokeniser_state_done);
        tokeniser_result_set(tokeniser, tokeniser_result_ok);
    }
    else if (event->current_char == '\"' || event->current_char == '\'')
    {
        tokeniser->expected_close_quote = event->current_char;
        Fsm_state_transition(fsm, tokeniser_state_quoted_regular_token);
    }
    else if (isspace(event->current_char))
    {
        got_token(tokeniser,
                  tokeniser->char_count,
                  '\0');
        current_token_free(tokeniser);
        Fsm_state_transition(fsm, tokeniser_state_no_token);
    }
    else
    {
        str_extend(&tokeniser->current_token, event->current_char);
    }
}

static void tokeniser_state_no_token(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Waiting for the start of a token. Ignore spaces. If the 
     * character is a quote this signifies the start of a quoted 
     * token. EOF, NUL and NEWLINE signal the end of input. Other 
     * regular characters signal the start of a token. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    tokeniser_event_st * const event = FSM_EVENT_TO_TOKENISER_EVENT(event_fsm);

    if (event->current_char == TOKENISER_EOF || event->current_char == '\0' || event->current_char == '\n')
    {
        tokeniser_result_set(tokeniser, tokeniser_result_ok);
        Fsm_state_transition(fsm, tokeniser_state_done);
    }
    else if (event->current_char == '\r' || isspace(event->current_char))
    {
        /* Ignore. */
    }
    else if (event->current_char == '\"' || event->current_char == '\'')
    {
        tokeniser->expected_close_quote = event->current_char;
        current_token_init(tokeniser, '\0');
        Fsm_state_transition(fsm, tokeniser_state_quoted_token);
    }
    else
    {
        current_token_init(tokeniser, (char)event->current_char);
        Fsm_state_transition(fsm, tokeniser_state_regular_token);
    }
}

void tokeniser_dispatch(tokeniser_st * const tokeniser, tokeniser_event_st const * const tokeniser_event)
{
    Fsm_dispatch(TOKENISER_TO_FSM(tokeniser), TOKENISER_EVENT_TO_FSM_EVENT(tokeniser_event));
}

void tokeniser_init_fsm(tokeniser_st * const tokeniser)
{
    fsm_class * const fsm = TOKENISER_TO_FSM(tokeniser);

    Fsm_constructor(fsm, tokeniser_state_init);
    Fsm_init(fsm, NULL);
}
