#ifndef __TOKENISER_STATES_H__
#define __TOKENISER_STATES_H__

#include "tokeniser_private.h"

void tokeniser_dispatch(tokeniser_st * const tokeniser, tokeniser_event_st const * const tokeniser_event);
void tokeniser_init_fsm(tokeniser_st * const tokeniser); 

#endif /* __TOKENISER_STATES_H__ */
