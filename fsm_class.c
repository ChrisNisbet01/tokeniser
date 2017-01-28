#include "fsm_class.h"

#include <stdlib.h>

void fsm_state_transition(fsm_class * const fsm, fsm_state * const new_state)
{
    /* Execute the exit action for the current state. Note that the 
     * current state may be NULL for the first transition. Note that 
     * the entry and exit actions are optional, so may be NULL. 
     */
    if (fsm->current_state != NULL && fsm->current_state->exit != NULL)
    {
        fsm->current_state->exit(fsm);
    }

    /* Change the current state. */
    fsm->current_state = new_state;
    fsm->current_state->transition_handler(fsm);

    /* Execute the entry action for the new state. */
    if (fsm->current_state != NULL && fsm->current_state->entry != NULL)
    {
        fsm->current_state->entry(fsm);
    }
}
