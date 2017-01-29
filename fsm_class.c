#include "fsm_class.h"

#include <stdlib.h>

void fsm_state_transition(fsm_class * const fsm, fsm_state_config const * const new_state)
{
    /* Execute the exit action for the current state. Transition to 
     * the new state by calling the transition handler of the new 
     * state. The transition handler of the new state is expected to
     * set up the event handlers for all possible events the FSM 
     * will handle. Then call the entry handler of the new state. 
     * Note that the current state may be NULL for the first 
     * transition. Note that the entry and exit actions are 
     * optional, so may be NULL. 
     */
    if (fsm->current_state.config != NULL && fsm->current_state.config->exit_handler != NULL)
    {
        fsm->current_state.config->exit_handler(fsm);
    }

    /* Change the current state. */
    fsm->current_state.config = new_state;
    fsm->current_state.config->transition_handler(fsm->current_state.event_handlers);

    /* Execute the entry action for the new state. */
    if (fsm->current_state.config->entry_handler != NULL)
    {
        fsm->current_state.config->entry_handler(fsm);
    }
}
