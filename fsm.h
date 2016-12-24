typedef int Signal;
typedef struct Event Event;
typedef struct Fsm Fsm;
typedef void (*State)(Fsm * const fsm, Event const * const event);

/* Event base class */
struct Event
{
   Signal __dummy__; /* Not used, but allows for something default to be added. */
};

/* Finite State Machine base class */
struct Fsm
{
   State current_state; /* the current state */
};

/* "inlined" methods of Fsm class */
#define Fsm_dispatch(me_, e_) ((me_)->current_state)((Fsm *)(me_), (Event *)(e_))
#define Fsm_init(me_, e_) Fsm_dispatch((me_), (e_))

#define Fsm_state_transition(me_, new_state) ((me_)->current_state = (State)(new_state))
#define Fsm_constructor(me_, init_) Fsm_state_transition((me_), (init_))
#define Fsm_current_state(me_) (me_)->current_state

