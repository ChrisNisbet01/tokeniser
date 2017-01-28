#ifndef __TOKENISER_PRIVATE_H__
#define __TOKENISER_PRIVATE_H__

#include "tokeniser.h"
#include "fsm_class.h"

#define UNUSED(arg) (void)(arg)

#if !defined(container_of)
#define container_of(ptr, type, member) \
                    (type *)((char *)(ptr) - (char *) &((type *)0)->member)
#endif

typedef enum event_code_t
{
    event_init,
    event_nul,
    event_space,
    event_single_quote,
    event_double_quote,
    event_regular_char
} event_code_t;

struct fsm_event_handlers_st
{
    fsm_state_handler init;
    fsm_state_handler nul;
    fsm_state_handler space;
    fsm_state_handler single_quote;
    fsm_state_handler double_quote;
    fsm_state_handler regular_char;
};

struct tokeniser_st
{
    fsm_class fsm; /* The base FSM 'class' */
    fsm_event_handlers_st event_handlers; /* The event handlers for this FSM. */

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
    fsm_event _super; /* The base event 'class'. */

    event_code_t code;
    int current_char; /* Tokeniser specific event information. */
} tokeniser_event_st;

/* Macros that remove the need for the FSM 'class' types to be 
 * the first field in the child 'classes' that use them. 
*/
#define TOKENISER_EVENT_TO_FSM_EVENT(event) (&event->_super)
#define FSM_EVENT_TO_TOKENISER_EVENT(event) container_of(event, tokeniser_event_st, _super)

#define TOKENISER_TO_FSM(tokeniser) (&tokeniser->fsm)
#define FSM_TO_TOKENISER(fsm) container_of(fsm, tokeniser_st, fsm)

void str_extend(char * * const str, char const new_char);
void current_token_free(tokeniser_st * const tokeniser);
void current_token_init(tokeniser_st * const tokeniser, char const first_char);
void tokeniser_result_set(tokeniser_st * const tokeniser, tokeniser_result_t const result);

#endif /* __TOKENISER_PRIVATE_H__ */
