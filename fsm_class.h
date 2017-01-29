#ifndef __FSM_CLASS_H__
#define __FSM_CLASS_H__

/* The user should include one of these in the child event 
 * class. 
 */ 
typedef struct fsm_event fsm_event;

/* The user should include one of these in the child 
 * FSM 'class'. 
 */
typedef struct fsm_class fsm_class;

/* The application is expected to define a 
 * struct event_handler_st and pass a reference to an instance 
 * of one to Fsm_constructor() before the FSM is first used. 
 * Done this way as the FSM class doesn't need to know the 
 * internals of the event_handler_st, just that there is one. 
 */
typedef struct fsm_event_handlers_st fsm_event_handlers_st;

/* The user should define functions matching these types for 
 * each desired state. Note that exit and entry handlers are 
 * optional but the transition handler is not. The transition 
 * handler is called whenever the state machine transitions to 
 * the state specified in fsm_state_transition().
 */
typedef void (* fsm_event_handler)(fsm_class * const fsm, fsm_event const * const event_fsm);
typedef void (* fsm_entry_handler)(fsm_class * const fsm);
typedef void (* fsm_exit_handler)(fsm_class * const fsm);
typedef void (* fsm_transition_handler)(fsm_event_handlers_st * const event_handlers);

/* The application is expected to define one of these for each 
 * state. The DEFINE_STATE macro below can be used to simplify 
 * this process. 
 */
typedef struct
{
    fsm_entry_handler entry_handler;
    fsm_exit_handler exit_handler;
    fsm_transition_handler transition_handler;
    char const * name;
} fsm_state_config;

typedef struct
{
    fsm_state_config const * config;
    fsm_event_handlers_st * event_handlers;
} fsm_state; 

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
    fsm_state current_state;
};

void fsm_state_transition(fsm_class * const fsm, fsm_state_config const * const new_state);

/* Helper macro to create a state definition. */
#define DEFINE_STATE(STATE, ENTRY, EXIT, TRANSITION) \
    fsm_state_config STATE  = { \
        .entry_handler = ENTRY, \
        .exit_handler = EXIT, \
        .transition_handler = TRANSITION, \
        .name = #STATE \
    }


/* "inlined" methods of FSM class */
#define Fsm_constructor(fsm, handlers) do \
            { \
                (fsm)->current_state.config = NULL; \
                (fsm)->current_state.event_handlers = (handlers); \
            } \
            while (0)

#define Fsm_state_name(state) (state)->config->name
#define Fsm_current_state(fsm) &(fsm)->current_state
#define Fsm_current_state_name(fsm) Fsm_state_name(Fsm_current_state((fsm)))

#endif /* __FSM_CLASS_H__ */
