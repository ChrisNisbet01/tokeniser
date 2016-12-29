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
typedef void (* fsm_state)(fsm_class * const fsm, fsm_event const * const event);

/* Although it doesn't contain anything useful, and fsm_event 
 * structure is required in each child class event so that the 
 * macros below can work, and the fsm_state type definition has 
 * a proper type. 
 */
struct fsm_event
{
    int dummy;
};

/* Finite State Machine base class */
struct fsm_class
{
    fsm_state current_state; /* the current state */
};

/* "inlined" methods of Fsm class */
#define Fsm_dispatch(me_, e_) ((me_)->current_state)((me_), (e_))
#define Fsm_init(me_, e_) Fsm_dispatch((me_), (e_))

#define Fsm_state_transition(me_, new_state) ((me_)->current_state = (new_state))
#define Fsm_constructor(me_, init_) Fsm_state_transition((me_), (init_))
#define Fsm_current_state(me_) (me_)->current_state

#endif /* __FSM_CLASS_H__ */
