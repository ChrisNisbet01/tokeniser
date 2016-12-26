#ifndef __TOKENISER_H__
#define __TOKENISER_H__

#include <stdbool.h>
#include <stddef.h>

typedef enum tokeniser_result_t
{
    tokeniser_result_continue, /* Tokeniser able to accept the next character. */
    tokeniser_result_ok, /* A complete line has been scanned up to EOF or the line ending ('\n'). */
    tokeniser_result_already_done, /* The tokeniser was called after the tokeniser has completed tokenising a line. */
    tokeniser_result_incomplete_token, /* EOF or EOL was hit before the current (quoted) token was completed. */
    tokeniser_result_error /* Some other error. */
} tokeniser_result_t;

#define TOKENISER_EOF (-1) /* Feed this to the tokeniser to indicate that no more characters will be fed to it. */

typedef struct tokeniser_st tokeniser_st;
typedef int (* getc_cb)(void * const user_context);
typedef bool (* new_token_cb)(char const * const token, /* The token. */
                              size_t const start_index, /* The starting index of the token in the supplied characters. */
                              size_t const end_index, /* The ending index of the token in the supplied characters. */
                              char const quote_char, /* If the token was quoted, this character will indicate the quote character (else is '\0'). */
                              void * const user_arg); /* The user arg supplied to tokeniser_feed. */

/*  
 * Create a now tokeniser. 
 * Returns: A new tokeniser. 
*/
tokeniser_st * tokeniser_alloc(void);

/*  
 * Prepare the tokeniser for a new line. 
 */
void tokeniser_init(tokeniser_st * const tokeniser);

/*  
 * Frees the tokeniser. 
*/ 
void tokeniser_free(tokeniser_st * const tokeniser);

/*  
 * Feed a character into the tokeniser. The user_callback will 
 * be called when a new token is found. 
 * @tokeniser: The tokeniser context returned from 
 * tokeniser_alloc. 
 * @next_char: The next character for the tokeniser to process. 
 * @user_callback: The callback to call with each new token 
 * discovered. 
 * @user_arg: Passed to the user_callback. 
 * Return value: Indicates the current status of the tokeniser.
*/ 
tokeniser_result_t tokeniser_feed(tokeniser_st * const tokeniser, 
                                  int const next_char, 
                                  new_token_cb const user_callback, 
                                  void * const user_arg);


#endif /* __TOKENISER_H__ */
