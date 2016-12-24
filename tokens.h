#ifndef __TOKENS_H__
#define __TOKENS_H__

/* Copyright (C) Chris Nisbet - All Rights Reserved
 * Unauthorized copying of this file via any medium is strictly 
 * prohibited. Proprietary and confidential. Written by Chris 
 * Nisbet <nisbet@ihug.co.nz>, April 2016.
 */

#include <stddef.h>
#include <stdbool.h>

typedef struct tokens_st tokens_st;

tokens_st * tokens_alloc(void);
void tokens_free(tokens_st * const tokens);
bool tokens_add_token(tokens_st * const tokens, char const * const token);
size_t tokens_count(tokens_st const * const tokens);
char const * tokens_get_token(tokens_st const * const tokens, size_t const index);


#endif /* __TOKENS_H__ */
