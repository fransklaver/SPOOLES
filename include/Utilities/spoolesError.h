#ifndef _SPOOLES_ERROR_
#define _SPOOLES_ERROR_

/*
 * The type of the exit function
 */
typedef void (*ExitFunction)(void);

/*
 * Register a custom exit function
 *
 * This may help in preventing spooles from exiting your program when an error
 * occurs. This may mean using setjmp/longjmp, exceptions or just plain
 * breakpoints inside that function.
 *
 * If NULL is passed, the default fail-behavior is restored, which is to plainly call exit(-1);
 *
 * Return the previously used exit function.
 */
ExitFunction registerExitFunction(ExitFunction func);

/*
 * Abandond the current operation and call the registered exit function.
 */
void spoolesFatal(void);

#endif
