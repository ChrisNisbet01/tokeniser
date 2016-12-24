#include "tokeniser.h"

#include <stdio.h>

#define STRING_VERSION 0
#define UNUSED(arg) (void)(arg)

#if STRING_VERSION == 0
static int my_getc(void * const arg)
{
    return fgetc((FILE *)arg);
}

#else
typedef struct my_getc_context_st
{
    char const * const current_char;
} my_getc_context_st;

static int my_getc(void * const arg)
{
    my_getc_context_st * const my_getc_context = arg;
    int current_char; 

    current_char = (unsigned char)*my_getc_context->current_char;     
    if (current_char != '\0')
    {
        my_getc_context->current_char++;
    }
    else
    {
        current_char = EOF;
    }

    return current_char;
}
#endif

static void print_tokens(tokens_st const * const tokens)
{
    unsigned int index;
    size_t const count = tokens_count(tokens);

    for (index = 0; index < count; index++)
    {
        printf("token %u: %s", index, tokens_get_token(tokens, index));
    }
}

int main(int const argc, char * const * const argv)
{
    UNUSED(argc);
    UNUSED(argv);
    tokeniser_st * const tokeniser = tokeniser_alloc();
    tokens_st * tokens;

#if STRING_VERSION
    my_getc_context_st my_getc_context;

    my_getc_context.current_char = "test";

    tokeniser_init(tokeniser, my_getc, &my_getc_context);
#else
    tokeniser_init(tokeniser, my_getc, stdin);
#endif

    switch (tokeniser_tokenise(tokeniser, &tokens))
    {
        case tokeniser_result_ok:
            print_tokens(tokens);
            tokens_free(tokens);
            break;
        case tokeniser_result_incomplete_token:
            printf("got incomplete token\n");
            break;
        case tokeniser_result_error:
            printf("got error\n");
            break;
    }

    tokeniser_free(tokeniser);

    return 0;
}
