#include "tokeniser_states.h"

#include <ctype.h>
#include <stdio.h>

//#define TOKENISER_STATE_DEBUG

#if defined(TOKENISER_STATE_DEBUG)
#define STATE_PRINTF(fmt, ...) do {printf(fmt, ##__VA_ARGS__);} while(0)
#else
#define STATE_PRINTF(fmt, ...) do {} while(0)
#endif

static void tokeniser_state_entry(fsm_class * const fsm);
static void tokeniser_state_exit(fsm_class * const fsm);

static void tokeniser_state_init_transition(fsm_event_handlers_st * const event_handlers);
static void tokeniser_state_no_token_transition(fsm_event_handlers_st * const event_handlers);
static void tokeniser_state_done_transition(fsm_event_handlers_st * const event_handlers);
static void tokeniser_state_regular_token_transition(fsm_event_handlers_st * const event_handlers);
static void tokeniser_state_single_quoted_token_transition(fsm_event_handlers_st * const event_handlers);
static void tokeniser_state_double_quoted_token_transition(fsm_event_handlers_st * const event_handlers);
static void tokeniser_state_single_quoted_regular_token_transition(fsm_event_handlers_st * const event_handlers);
static void tokeniser_state_double_quoted_regular_token_transition(fsm_event_handlers_st * const event_handlers);

static const DEFINE_STATE(tokeniser_state_init, tokeniser_state_entry, tokeniser_state_exit, tokeniser_state_init_transition);
static const DEFINE_STATE(tokeniser_state_no_token, tokeniser_state_entry, tokeniser_state_exit, tokeniser_state_no_token_transition);
static const DEFINE_STATE(tokeniser_state_done, tokeniser_state_entry, tokeniser_state_exit, tokeniser_state_done_transition);
static const DEFINE_STATE(tokeniser_state_regular_token, tokeniser_state_entry, tokeniser_state_exit, tokeniser_state_regular_token_transition);
static const DEFINE_STATE(tokeniser_state_single_quoted_token, tokeniser_state_entry, tokeniser_state_exit, tokeniser_state_single_quoted_token_transition);
static const DEFINE_STATE(tokeniser_state_double_quoted_token, tokeniser_state_entry, tokeniser_state_exit, tokeniser_state_double_quoted_token_transition);
static const DEFINE_STATE(tokeniser_state_single_quoted_regular_token, tokeniser_state_entry, tokeniser_state_exit, tokeniser_state_single_quoted_regular_token_transition);
static const DEFINE_STATE(tokeniser_state_double_quoted_regular_token, tokeniser_state_entry, tokeniser_state_exit, tokeniser_state_double_quoted_regular_token_transition);

static void default_init_event_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    UNUSED(fsm);
    UNUSED(event_fsm);
    STATE_PRINTF("%s\n", __FUNCTION__);
}

static void default_nul_event_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    UNUSED(fsm);
    UNUSED(event_fsm);
    STATE_PRINTF("%s\n", __FUNCTION__);
}

static void default_space_event_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    UNUSED(fsm);
    UNUSED(event_fsm);
    STATE_PRINTF("%s\n", __FUNCTION__);
}

static void default_single_quote_event_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    UNUSED(fsm);
    UNUSED(event_fsm);
    STATE_PRINTF("%s\n", __FUNCTION__);
}

static void default_double_quote_event_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    UNUSED(fsm);
    UNUSED(event_fsm);
    STATE_PRINTF("%s\n", __FUNCTION__);
}

static void default_regular_char_event_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    UNUSED(fsm);
    UNUSED(event_fsm);
    STATE_PRINTF("%s\n", __FUNCTION__);
}

static void default_event_handlers_set(fsm_event_handlers_st * const event_handlers)
{
    event_handlers->init = default_init_event_handler;
    event_handlers->nul = default_nul_event_handler;
    event_handlers->space = default_space_event_handler;
    event_handlers->single_quote = default_single_quote_event_handler;
    event_handlers->double_quote = default_double_quote_event_handler;
    event_handlers->regular_char = default_regular_char_event_handler;
}

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

static void tokeniser_state_entry(fsm_class * const fsm)
{
    UNUSED(fsm);

    STATE_PRINTF("enter %s\n", Fsm_current_state_name(fsm));
}

static void tokeniser_state_exit(fsm_class * const fsm)
{
    UNUSED(fsm);

    STATE_PRINTF("leave %s\n", Fsm_current_state_name(fsm));
}

static void tokeniser_state_init_init_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Initial state. Transition to the first 
     * working state when processing the 'init' event.
     */
    UNUSED(event_fsm);

    STATE_PRINTF("%s\n", __FUNCTION__);
    fsm_state_transition(fsm, &tokeniser_state_no_token);
}

static void tokeniser_state_init_transition(fsm_event_handlers_st * const event_handlers)
{
    STATE_PRINTF("%s\n", __FUNCTION__);
    default_event_handlers_set(event_handlers);

    event_handlers->init = tokeniser_state_init_init_handler;
}

static void tokeniser_state_done_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* The input line has been tokenised. No events handled in 
     * this state. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    UNUSED(event_fsm);
    STATE_PRINTF("%s\n", __FUNCTION__);
    tokeniser->result = tokeniser_result_already_done;
}

static void tokeniser_state_done_transition(fsm_event_handlers_st * const event_handlers)
{
    STATE_PRINTF("%s\n", __FUNCTION__);
    default_event_handlers_set(event_handlers);

    event_handlers->nul = tokeniser_state_done_handler;
    event_handlers->space = tokeniser_state_done_handler;
    event_handlers->single_quote = tokeniser_state_done_handler;
    event_handlers->double_quote = tokeniser_state_done_handler;
    event_handlers->regular_char = tokeniser_state_done_handler;
}

static void tokeniser_state_quoted_token_nul_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Inside a quoted ('\"' or '\''token. Append new chars to the 
     * current token until the closing quote character is received. 
     * If the line ends before the closing quote is received, set 
     * the result to incomplete token. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    UNUSED(event_fsm); 
    STATE_PRINTF("%s\n", __FUNCTION__);

    got_token(tokeniser,
              tokeniser->char_count,
              '\0');
    tokeniser_result_set(tokeniser, tokeniser_result_incomplete_token);
    fsm_state_transition(fsm, &tokeniser_state_done);
}

static void tokeniser_state_quoted_token_quote_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Inside a quoted ('\"' or '\''token. Append new chars to the 
     * current token until the closing quote character is received. 
     * If the line ends before the closing quote is received, set 
     * the result to incomplete token. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    UNUSED(event_fsm); 
    STATE_PRINTF("%s\n", __FUNCTION__);

    got_token(tokeniser,
              tokeniser->char_count + 1, /* Include the closing quote in the end index. */
              tokeniser->expected_close_quote);
    fsm_state_transition(fsm, &tokeniser_state_no_token);
}

static void tokeniser_state_quoted_token_other_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Inside a quoted ('\"' or '\''token. Append new chars to the 
     * current token until the closing quote character is received. 
     * If the line ends before the closing quote is received, set 
     * the result to incomplete token. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    tokeniser_event_st * const event = FSM_EVENT_TO_TOKENISER_EVENT(event_fsm);
    STATE_PRINTF("%s\n", __FUNCTION__);

    str_extend(&tokeniser->current_token, event->current_char);
}

static void tokeniser_state_single_quoted_token_transition(fsm_event_handlers_st * const event_handlers)
{    
    STATE_PRINTF("%s\n", __FUNCTION__);

    default_event_handlers_set(event_handlers);

    event_handlers->nul = tokeniser_state_quoted_token_nul_handler;
    event_handlers->space = tokeniser_state_quoted_token_other_handler;
    event_handlers->single_quote = tokeniser_state_quoted_token_quote_handler;
    event_handlers->double_quote = tokeniser_state_quoted_token_other_handler;
    event_handlers->regular_char = tokeniser_state_quoted_token_other_handler;
}

static void tokeniser_state_double_quoted_token_transition(fsm_event_handlers_st * const event_handlers)
{
    STATE_PRINTF("%s\n", __FUNCTION__);
    default_event_handlers_set(event_handlers);

    event_handlers->nul = tokeniser_state_quoted_token_nul_handler;
    event_handlers->space = tokeniser_state_quoted_token_other_handler;
    event_handlers->single_quote = tokeniser_state_quoted_token_other_handler;
    event_handlers->double_quote = tokeniser_state_quoted_token_quote_handler;
    event_handlers->regular_char = tokeniser_state_quoted_token_other_handler;
}

static void tokeniser_state_quoted_regular_token_nul_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Processing a regular token that contains embedded quotes 
     * (e.g. abc" def "ghi), and are within the quoted section of 
     * the token. Append new chars to the current token until the 
     * closing quote character is received. If the line ends before 
     * the closing quote is received, set the result to incomplete 
     * token. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    UNUSED(event_fsm); 

    STATE_PRINTF("%s\n", __FUNCTION__);
    /* No more input. */
    got_token(tokeniser,
              tokeniser->char_count,
              '\0');
    tokeniser_result_set(tokeniser, tokeniser_result_incomplete_token);
    fsm_state_transition(fsm, &tokeniser_state_done);
}

static void tokeniser_state_quoted_regular_token_quote_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Processing a regular token that contains embedded quotes 
     * (e.g. abc" def "ghi), and are within the quoted section of 
     * the token. Append new chars to the current token until the 
     * closing quote character is received. If the line ends before 
     * the closing quote is received, set the result to incomplete 
     * token. 
     */
    UNUSED(event_fsm); 
    STATE_PRINTF("%s\n", __FUNCTION__);

    /* Received the matching close quote character. */
    fsm_state_transition(fsm, &tokeniser_state_regular_token);
}

static void tokeniser_state_quoted_regular_token_other_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
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
    STATE_PRINTF("%s\n", __FUNCTION__);

    /* Add the new char into the current token. */
    str_extend(&tokeniser->current_token, event->current_char);
}

static void tokeniser_state_single_quoted_regular_token_transition(fsm_event_handlers_st * const event_handlers)
{
    STATE_PRINTF("%s\n", __FUNCTION__);

    default_event_handlers_set(event_handlers);

    event_handlers->nul = tokeniser_state_quoted_regular_token_nul_handler;
    event_handlers->space = tokeniser_state_quoted_regular_token_other_handler;
    event_handlers->single_quote = tokeniser_state_quoted_regular_token_quote_handler;
    event_handlers->double_quote = tokeniser_state_quoted_regular_token_other_handler;
    event_handlers->regular_char = tokeniser_state_quoted_regular_token_other_handler;
}

static void tokeniser_state_double_quoted_regular_token_transition(fsm_event_handlers_st * const event_handlers)
{
    STATE_PRINTF("%s\n", __FUNCTION__);

    default_event_handlers_set(event_handlers);

    event_handlers->nul = tokeniser_state_quoted_regular_token_nul_handler;
    event_handlers->space = tokeniser_state_quoted_regular_token_other_handler;
    event_handlers->single_quote = tokeniser_state_quoted_regular_token_other_handler;
    event_handlers->double_quote = tokeniser_state_quoted_regular_token_quote_handler;
    event_handlers->regular_char = tokeniser_state_quoted_regular_token_other_handler;
}

static void tokeniser_state_regular_token_nul_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Processing a regular, unquoted token. Append new chars to the 
     * current token. If the current char is a space, this signals 
     * the end of the token. If the current char is a quote ('\"' or 
     * '\'' this signifies the start of a quoted regular token. 
     * If EOF or NUL or NEWLINE is received, this signifies the end 
     * of the input.  
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    UNUSED(event_fsm); 
    STATE_PRINTF("%s\n", __FUNCTION__);

    got_token(tokeniser,
              tokeniser->char_count,
              '\0');
    tokeniser_result_set(tokeniser, tokeniser_result_ok);
    fsm_state_transition(fsm, &tokeniser_state_done);
}

static void tokeniser_state_regular_token_single_quote_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
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
    STATE_PRINTF("%s\n", __FUNCTION__);

    tokeniser->expected_close_quote = event->current_char;
    fsm_state_transition(fsm, &tokeniser_state_single_quoted_regular_token);
}

static void tokeniser_state_regular_token_double_quote_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
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
    STATE_PRINTF("%s\n", __FUNCTION__);

    tokeniser->expected_close_quote = event->current_char;
    fsm_state_transition(fsm, &tokeniser_state_double_quoted_regular_token);
}

static void tokeniser_state_regular_token_space_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Processing a regular, unquoted token. Append new chars to the 
     * current token. If the current char is a space, this signals 
     * the end of the token. If the current char is a quote ('\"' or 
     * '\'' this signifies the start of a quoted regular token. 
     * If EOF or NUL or NEWLINE is received, this signifies the end 
     * of the input.  
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    UNUSED(event_fsm); 
    STATE_PRINTF("%s\n", __FUNCTION__);

    got_token(tokeniser,
              tokeniser->char_count,
              '\0');
    current_token_free(tokeniser);
    fsm_state_transition(fsm, &tokeniser_state_no_token);
}

static void tokeniser_state_regular_token_regular_char_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
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
    STATE_PRINTF("%s\n", __FUNCTION__);

    str_extend(&tokeniser->current_token, event->current_char);
}

static void tokeniser_state_regular_token_transition(fsm_event_handlers_st * const event_handlers)
{
    STATE_PRINTF("%s\n", __FUNCTION__);

    default_event_handlers_set(event_handlers);

    event_handlers->nul = tokeniser_state_regular_token_nul_handler;
    event_handlers->space = tokeniser_state_regular_token_space_handler;
    event_handlers->single_quote = tokeniser_state_regular_token_single_quote_handler;
    event_handlers->double_quote = tokeniser_state_regular_token_double_quote_handler;
    event_handlers->regular_char = tokeniser_state_regular_token_regular_char_handler;
}

static void tokeniser_state_no_token_nul_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Waiting for the start of a token. Ignore spaces. If the 
     * character is a quote this signifies the start of a quoted 
     * token. EOF, NUL and NEWLINE signal the end of input. Other 
     * regular characters signal the start of a token. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    UNUSED(event_fsm); 
    STATE_PRINTF("%s\n", __FUNCTION__);

    tokeniser_result_set(tokeniser, tokeniser_result_ok);
    fsm_state_transition(fsm, &tokeniser_state_done);
}

static void tokeniser_state_no_token_space_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Waiting for the start of a token. Ignore spaces. If the 
     * character is a quote this signifies the start of a quoted 
     * token. EOF, NUL and NEWLINE signal the end of input. Other 
     * regular characters signal the start of a token. 
     */
    UNUSED(fsm);
    UNUSED(event_fsm);
    STATE_PRINTF("%s\n", __FUNCTION__);

}

static void tokeniser_state_no_token_single_quote_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Waiting for the start of a token. Ignore spaces. If the 
     * character is a quote this signifies the start of a quoted 
     * token. EOF, NUL and NEWLINE signal the end of input. Other 
     * regular characters signal the start of a token. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    tokeniser_event_st * const event = FSM_EVENT_TO_TOKENISER_EVENT(event_fsm);
    STATE_PRINTF("%s\n", __FUNCTION__);

    tokeniser->expected_close_quote = event->current_char;
    current_token_init(tokeniser, '\0');
    fsm_state_transition(fsm, &tokeniser_state_single_quoted_token);
}

static void tokeniser_state_no_token_double_quote_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Waiting for the start of a token. Ignore spaces. If the 
     * character is a quote this signifies the start of a quoted 
     * token. EOF, NUL and NEWLINE signal the end of input. Other 
     * regular characters signal the start of a token. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    tokeniser_event_st * const event = FSM_EVENT_TO_TOKENISER_EVENT(event_fsm);
    STATE_PRINTF("%s\n", __FUNCTION__);

    tokeniser->expected_close_quote = event->current_char;
    current_token_init(tokeniser, '\0');
    fsm_state_transition(fsm, &tokeniser_state_double_quoted_token);
}

static void tokeniser_state_no_token_regular_char_handler(fsm_class * const fsm, fsm_event const * const event_fsm)
{
    /* Waiting for the start of a token. Ignore spaces. If the 
     * character is a quote this signifies the start of a quoted 
     * token. EOF, NUL and NEWLINE signal the end of input. Other 
     * regular characters signal the start of a token. 
     */
    tokeniser_st * const tokeniser = FSM_TO_TOKENISER(fsm);
    tokeniser_event_st * const event = FSM_EVENT_TO_TOKENISER_EVENT(event_fsm);
    STATE_PRINTF("%s\n", __FUNCTION__);

    current_token_init(tokeniser, (char)event->current_char);
    fsm_state_transition(fsm, &tokeniser_state_regular_token);
}

static void tokeniser_state_no_token_transition(fsm_event_handlers_st * const event_handlers)
{
    STATE_PRINTF("%s\n", __FUNCTION__);

    default_event_handlers_set(event_handlers);

    event_handlers->nul = tokeniser_state_no_token_nul_handler;
    event_handlers->space = tokeniser_state_no_token_space_handler;
    event_handlers->single_quote = tokeniser_state_no_token_single_quote_handler;
    event_handlers->double_quote = tokeniser_state_no_token_double_quote_handler;
    event_handlers->regular_char = tokeniser_state_no_token_regular_char_handler;
}

void tokeniser_dispatch(tokeniser_st * const tokeniser, tokeniser_event_st const * const tokeniser_event)
{
    fsm_class * const fsm = TOKENISER_TO_FSM(tokeniser);
    fsm_event const * const event_fsm = TOKENISER_EVENT_TO_FSM_EVENT(tokeniser_event);

    STATE_PRINTF("%s code %d %c\n", __FUNCTION__, tokeniser_event->code, tokeniser_event->current_char);

    switch (tokeniser_event->code)
    {
        case event_init:
            Fsm_dispatch(fsm, init, event_fsm);
            break;
        case event_nul:
            Fsm_dispatch(fsm, nul, event_fsm);
            break;
        case event_space:
            Fsm_dispatch(fsm, space, event_fsm);
            break;
        case event_single_quote:
            Fsm_dispatch(fsm, single_quote, event_fsm);
            break;
        case event_double_quote:
            Fsm_dispatch(fsm, double_quote, event_fsm);
            break;
        case event_regular_char:
            Fsm_dispatch(fsm, regular_char, event_fsm);
            break;
    }
}

void tokeniser_init_fsm(tokeniser_st * const tokeniser)
{
    tokeniser_event_st event;
    fsm_class * const fsm = TOKENISER_TO_FSM(tokeniser);

    Fsm_constructor(fsm, &tokeniser->event_handlers);
    fsm_state_transition(fsm, &tokeniser_state_init);

    event.code = event_init;
    tokeniser_dispatch(tokeniser, &event);
}
