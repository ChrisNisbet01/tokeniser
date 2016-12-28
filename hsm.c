#include "hsm.h"

#include <stddef.h>

static void _enter_state(hsm_hsm_t * const hsm, hsm_state_t const * const to)
{
    hsm->current_state = to;
}

static hsm_state_t *_local_dispatch(hsm_hsm_t * const hsm, hsm_state_t const * const state, hsm_event_t const * const event)
{
	if (state == NULL)
		return NULL;

	return state->event_handler(hsm, event);
}

void hsm_state_transition(hsm_hsm_t * const hsm, hsm_state_t const * const to)
{
    _enter_state(hsm, to);
}

void hsm_dispatch_event(hsm_hsm_t * const hsm, hsm_event_t const * const event)
{
    hsm_state_t const * next_state;

    /* Call all handlers in the list until one of them handles the 
     * event, or we run out of handlers. When a handler handles an 
     * event it should return NULL. 
     */
    do
    {
        next_state = _local_dispatch(hsm, hsm->current_state, event);
    }
    while (next_state != NULL);
}

void hsm_initialise(hsm_hsm_t * const hsm, hsm_event_t const * const event)
{
    hsm_dispatch_event(hsm, event);
}

void hsm_constructor(hsm_hsm_t * const hsm, hsm_state_t const * const initial_state)
{
    hsm_state_transition(hsm, initial_state);
}

hsm_state_t const * hsm_current_state(hsm_hsm_t const * const hsm)
{
	return hsm->current_state;
}


