#ifndef __HSM_H__
#define __HSM_H__

/* HSM, state, and event types */
typedef struct hsm_hsm_s hsm_hsm_t;
typedef struct hsm_state_s hsm_state_t;

typedef struct {
    int dummy; /* Placeholder. */
} hsm_event_t;

typedef hsm_state_t * hsm_event_handler_fun_t(hsm_hsm_t * const hsm, hsm_event_t const * const event);

void hsm_constructor(hsm_hsm_t * const hsm, hsm_state_t const * const initial_state);
void hsm_initialise(hsm_hsm_t * const hsm, hsm_event_t const * const event);
void hsm_dispatch_event(hsm_hsm_t * const hsm, hsm_event_t const * const event);
hsm_state_t const * hsm_current_state(hsm_hsm_t const * const hsm);

/* The Base HSM 'class' */
struct hsm_hsm_s {
	hsm_state_t const * current_state;
};

struct hsm_state_s
{
    /* The state's event handler. */
    hsm_event_handler_fun_t * const event_handler;
    /* s.parent != NULL indicates that s is a sub-state. */
    hsm_state_t const * const parent;
}; 

/* Helper macro to define the state structure and declare the 
 * handler function. 
 */
#define HSM_DEFINE_STATE(STATE, PARENT) \
    hsm_state_t const * STATE##_state_handler(hsm_hsm_t * const hsm, hsm_event_t const * const event); \
    hsm_state_t const STATE = { .event_handler = STATE##_state_handler, .parent = PARENT}


#endif /* __HSM_H__ */
