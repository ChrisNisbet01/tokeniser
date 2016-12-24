#include "tokens.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct token_st token_st;
struct token_st
{
    char * token;
};

struct tokens_st
{
    size_t current_token_index;
    char const * current_token;
    size_t count;
    size_t token_array_size;
    token_st * token_array;
};

static bool tokens_ensure_space_for_new_token(tokens_st * const tokens)
{
    bool has_space;
    token_st * new_token_array;
    size_t new_token_array_size;

    if (tokens->count < tokens->token_array_size)
    {
        has_space = true;
        goto done;
    }
    /* Make space for another token. */
    new_token_array_size = tokens->token_array_size + 1;
    new_token_array = calloc(new_token_array_size, sizeof *new_token_array);
    if (new_token_array == NULL)
    {
        has_space = false;
        goto done;
    }
    memcpy(new_token_array, tokens->token_array, tokens->token_array_size * sizeof *new_token_array);
    free(tokens->token_array);
    tokens->token_array = new_token_array;
    tokens->token_array_size = new_token_array_size;
    has_space = true;

done:
    return has_space;
}

tokens_st * tokens_alloc(void)
{
    tokens_st * const tokens = calloc(1, sizeof *tokens);

    return tokens;
}

void tokens_free(tokens_st * const tokens)
{
    if (tokens != NULL)
    {
        if (tokens->token_array != NULL)
        {
            size_t index;

            for (index = 0; index < tokens->count; index++)
            {
                free(tokens->token_array[index].token);
            }
            free(tokens->token_array);
        }
        free(tokens);
    }
}

bool tokens_add_token(tokens_st * const tokens, char const * const token)
{
    bool token_added;

    if (!tokens_ensure_space_for_new_token(tokens))
    {
        token_added = false;
        goto done;
    }

    tokens->token_array[tokens->count].token = strdup(token);
    if (tokens->token_array[tokens->count].token == NULL)
    {
        token_added = false;
        goto done;
    }

    tokens->count++;
    token_added = true;

done:
    return token_added;
}

size_t tokens_count(tokens_st const * const tokens)
{
    size_t count;

    if (tokens == NULL)
    {
        count = 0;
    }
    else
    {
        count = tokens->count;
    }

    return count;
}

char const * tokens_get_token(tokens_st const * const tokens, size_t const index)
{
    char const * token;

    if (tokens == NULL)
    {
        token = NULL;
        goto done;
    }
    if (index >= tokens->count)
    {
        token = NULL;
        goto done;
    }

    token = tokens->token_array[index].token;

done:
    return token;
}


