#ifndef __TOKENISER_H__
#define __TOKENISER_H__

#include "tokens.h"

typedef enum tokeniser_result_t
{
    tokeniser_result_ok,
    tokeniser_result_incomplete_token,
    tokeniser_result_error
} tokeniser_result_t;

typedef struct tokeniser_st tokeniser_st;
typedef int (* getc_cb)(void * const user_context);

tokeniser_st * tokeniser_alloc(void);
void tokeniser_free(tokeniser_st * const tokeniser);

void tokeniser_init(tokeniser_st * const tokeniser, getc_cb const getc_fn, void * user_context);

tokeniser_result_t tokeniser_tokenise(tokeniser_st * const tokeniser, tokens_st * * const tokens);


#endif /* __TOKENISER_H__ */
