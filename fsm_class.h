#ifndef __FSM_CLASS_H__
#define __FSM_CLASS_H__

/* The user should include one of these in the child event 
 * class. 
 */ 
typedef struct fsm_event fsm_event;

/* The user should include one of these in the child 'class'. */
typedef struct fsm_class fsm_class;

/* The user should define functions matching this type for 
 * each desired state. 
 */
typedef void (* fsm_state_handler)(fsm_class * const fsm, fsm_event const * const event);

typedef struct
{
    void (* entry)(fsm_class * const fsm);
    void (* exit)(fsm_class * const fsm);
    fsm_state_handler handler;
    char const * name;
} fsm_state;

#define DEFINE_STATE(STATE, ENTRY, EXIT, HANDLER) \
    fsm_state const STATE  = { \
        .entry = ENTRY, \
        .exit = EXIT, \
        .handler = HANDLER, \
        .name = #STATE \
    }

/* Although it doesn't contain anything useful, and fsm_event 
 * structure is required in each derived event class fsm_state 
 * handler type definition has a proper event type. 
 */
struct fsm_event
{
    int dummy;
};

/* Finite State Machine base class */
struct fsm_class
{
    fsm_state const * current_state; /* the current state */
};

/* "inlined" methods of FSM class */
#define Fsm_dispatch(fsm, e_) ((fsm)->current_state->handler)((fsm), (e_))
#define Fsm_init(fsm, e_) Fsm_dispatch((fsm), (e_))

#define Fsm_state_transition(fsm, new_state) fsm_state_transition((fsm), &(new_state))

#define Fsm_constructor(fsm, init_) do \
            { \
                (fsm)->current_state = NULL; \
                Fsm_state_transition((fsm), (init_)); \
            } \
            while (0)

#define Fsm_current_state(fsm) (fsm)->current_state

#endif /* __FSM_CLASS_H__ */
