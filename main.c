#include "tokeniser.h"
#include "tokens.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define UNUSED(arg) (void)(arg)


static void print_tokens(tokens_st const * const tokens)
{
    unsigned int index;
    size_t const count = tokens_count(tokens);

    printf("\r\nshow %d tokens\n", count);
    for (index = 0; index < count; index++)
    {
        printf("token %u: %s\n", index, tokens_get_token(tokens, index));
    }
}

typedef struct tokeniser_context_st
{
    char const * line;
    tokens_st * tokens;
} tokeniser_context_st;

char * partial_strdup(char const * const string, size_t const len)
{
    char * const partial = malloc(len + 1);

    if (partial == NULL)
    {
        goto done;
    }

    memcpy(partial, string, len);
    partial[len] = '\0';

done:
    return partial;
}

static bool new_token(char const * const token, 
                      size_t const start_index, 
                      size_t const end_index, 
                      char const quote_char,
                      void * const user_arg)
{
    UNUSED(start_index);
    UNUSED(end_index);
    UNUSED(quote_char);
    tokeniser_context_st * const tokeniser_context = user_arg;
    char * const partial = partial_strdup(&tokeniser_context->line[start_index], end_index - start_index);


    printf("\r\ngot token from these chars %s\n", partial);
    tokens_add_token(tokeniser_context->tokens, token);
    free(partial);

    return true;
}

static void do_tokenise_test(char const * const string)
{
    tokeniser_st * const tokeniser = tokeniser_alloc();
    char const * pch = string;
    tokeniser_result_t tokeniser_result;
    tokeniser_context_st tokeniser_context;

    tokeniser_context.tokens = tokens_alloc();
    tokeniser_context.line = string;

    while (*pch)
    {
        tokeniser_result = tokeniser_feed(tokeniser, *pch, new_token, &tokeniser_context);
        pch++;
        if (tokeniser_result == tokeniser_result_ok)
        {
            if (*pch != '\0')
            {
                printf("\r\ngot OK result before the end of the input line");
            }
            break;
        }
        else if (tokeniser_result != tokeniser_result_continue)
        {
            printf("\r\nhad error tokenising %s at position %zu", string, (size_t)(pch - string));
            goto done;
        }
    }
    if (tokeniser_result != tokeniser_result_ok)
    {
        /* Indicate that we have no more tokens to feed in. */
        tokeniser_result = tokeniser_feed(tokeniser, TOKENISER_EOF, new_token, &tokeniser_context);
    }

    switch (tokeniser_result)
    {
        case tokeniser_result_ok:
            print_tokens(tokeniser_context.tokens);
            break;
        case tokeniser_result_incomplete_token:
            printf("got incomplete token\n");
            print_tokens(tokeniser_context.tokens);
            break;
        case tokeniser_result_error:
            printf("got error\n");
            break;
        case tokeniser_result_continue: /* Shouldn't happen. */
            break;
        case tokeniser_result_already_done: /* Shouldn't happen unless we've done something dumb. */
            break;
    }

done:
    tokens_free(tokeniser_context.tokens);
    tokeniser_free(tokeniser);
}

int main(int const argc, char * const * const argv)
{
    UNUSED(argc);
    UNUSED(argv);

    do_tokenise_test("test | > < abc' | \"|\" def 'ghi \"|\" 123\" 456 \"789 \"double quoted\" \'single quoted\' \"double quoted embedded single quote \'\" \'single quoted embedded double quote \"\'");
    do_tokenise_test("test \"double quotedincomplete");

    return 0;
}
