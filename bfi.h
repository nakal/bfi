
/*
	Include for the Brainf*ck-Interpreter.
*/

#ifndef BFI_H_INCLUDED__
#define BFI_H_INCLUDED__

#include <sys/types.h>

/*
	The Braif*ck-VM
*/
typedef struct {
	/*
	   The memory and the memory pointer.

	   The Brainf*ck-VM usually operates on a
	   30000 bytes large memory.
	 */
#ifndef MEM_SIZE
#define MEM_SIZE 30000
#endif
	char bfmemory[MEM_SIZE];
	unsigned int bfpointer;

	/*
	   The stack is modelling an optimisation for
	   quick finding of matching '['-parenthesis
	   when executing a ']'-command.

	   See also: stack push() and pop().

	   GROW_STACK defines in what steps the stack
	   is going to be allocated.
	 */
	off_t *bfstack;
	unsigned int bfstackpointer;
	unsigned int bfstacksize;
#ifndef GROW_STACK
#define GROW_STACK 256
#endif

	/*
	   Jump prediction is an O(1) optimisation for
	   recursive skip in case if the while-statement
	   evaluates negative and the loop body has been
	   executed at least 1 time.
	 */
	struct {
		off_t from; /* points to '[', if valid */
		off_t to; /* points to byte _after_ matching ']', if valid */
	} jump_prediction;
} bfvm;


extern int bfinit(bfvm **vm);
#ifdef BSD_OPTIMIZED
extern int bfexec(bfvm *vm, const char *scrmem, off_t scrlen);
#else
extern int bfexec(bfvm *vm, FILE *script);
#endif
extern int bfdone(bfvm **vm);

#endif

