#ifndef __TOKENISER_PRIVATE_H__
#define __TOKENISER_PRIVATE_H__

#include "tokeniser.h"
#include "fsm_class.h"

#define UNUSED(arg) (void)(arg)

#if !defined(container_of)
#define container_of(ptr, type, member) \
                    (type *)((char *)(ptr) - (char *) &((type *)0)->member)
#endif

struct tokeniser_st
{
    fsm_class fsm; /* The base FSM 'class' */

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