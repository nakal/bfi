
/*
	Brainf*ck-Interpreter core.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bfi.h"

static int push(bfvm *vm, off_t p);
static int pop(bfvm *vm, off_t *p);
#ifdef BSD_OPTIMIZED
static int recursive_skip(const char *scrmem, off_t *p, off_t scrlen);
#else
static int recursive_skip(FILE *script);
#endif

static int push(bfvm *vm, off_t p) {

	if (vm->bfstackpointer>=vm->bfstacksize) {
		off_t *newstack;

		if (vm->bfstacksize+GROW_STACK<vm->bfstacksize)
			return 12; /* stack overflow */

		newstack=realloc(vm->bfstack, vm->bfstacksize+GROW_STACK);

		if (newstack==NULL) return 13;
		vm->bfstacksize+=GROW_STACK;
		vm->bfstack=newstack;
	}

	vm->bfstack[vm->bfstackpointer++]=p;

	return 0;
}

static int pop(bfvm *vm, off_t *p) {

	if (vm->bfstackpointer==0) return 14;
	*p=vm->bfstack[--vm->bfstackpointer];

	return 0;
}

/*
	This function will be executed in case when
	a while-loop condition is false and the body
	has not been executed.

	Also see: jump_prediction.
*/
#ifdef BSD_OPTIMIZED
static int recursive_skip(const char *scrmem, off_t *p, off_t scrlen)
#else
static int recursive_skip(FILE *script)
#endif
{
	char c;

	do {
#ifdef BSD_OPTIMIZED
		if (*p>=scrlen) return 11;
		c=scrmem[(*p)++];

		if (c=='[') recursive_skip(scrmem, p, scrlen);
#else
		if (feof(script)) return 11;
		c=fgetc(script);
		if (ferror(script)) return 20;

		if (c=='[') recursive_skip(script);
#endif
	} while (c!=']');

	return 0;
}

/*
	Executes the initialised VM.

	scrmem points to the beginning of the code.
	scrlen tells us how many bytes are in the
	code region.
*/
#ifdef BSD_OPTIMIZED
int bfexec(bfvm *vm, const char *scrmem, off_t scrlen)
#else
int bfexec(bfvm *vm, FILE *script)
#endif
{
#ifdef BSD_OPTIMIZED
	off_t spos=0;

	while (spos<scrlen)
#else
	while (!feof(script))
#endif

	{

		char c;

#ifdef BSD_OPTIMIZED
		c=scrmem[spos++];
#else
		c=fgetc(script);
		if (ferror(script)) return 20;
#endif

		switch (c) {
		case '<':
			vm->bfpointer--;
			break;
		case '>':
			vm->bfpointer++;
			break;
		case '+':
			if (vm->bfpointer>=MEM_SIZE) return 10;
			vm->bfmemory[vm->bfpointer]++;
			break;
		case '-':
			if (vm->bfpointer>=MEM_SIZE) return 10;
			vm->bfmemory[vm->bfpointer]--;
			break;
		case '.':
			if (vm->bfpointer>=MEM_SIZE) return 10;
			putchar(vm->bfmemory[vm->bfpointer]);
			break;
		case ',':
			if (vm->bfpointer>=MEM_SIZE) return 10;
			vm->bfmemory[vm->bfpointer]=getchar();
			break;
		case '[':
			if (vm->bfpointer>=MEM_SIZE) return 10;

			if (vm->bfmemory[vm->bfpointer]==0) {
				int ret;

#ifdef BSD_OPTIMIZED
				if (vm->jump_prediction.from==spos-1) {
					spos=vm->jump_prediction.to;
					ret=0;
				} else
				ret=recursive_skip(scrmem, &spos, scrlen);
#else
				if (vm->jump_prediction.from==ftello(script)-1) {
					ret=fseeko(script, vm->jump_prediction.to,
						SEEK_SET);
				} else ret=recursive_skip(script);
#endif

				if (ret!=0) return ret;
			} else {

				int ret;
#ifdef BSD_OPTIMIZED
				ret=push(vm, spos-1);
#else
				ret=push(vm, ftello(script)-1);
#endif
				if (ret!=0) return ret;
			}
			break;
		case ']':
			{
				int ret;
#ifdef BSD_OPTIMIZED
				vm->jump_prediction.to=spos;
				ret=pop(vm, &spos);
				if (ret!=0) return ret;
				vm->jump_prediction.from=spos;
#else
				off_t fpos=0;

				vm->jump_prediction.to=ftello(script);
				ret=pop(vm, &fpos);
				if (ret!=0) return ret;
				if (fseeko(script, fpos, SEEK_SET)!=0)
					return 21;
				vm->jump_prediction.from=ftello(script);
#endif
			}
			break;

		default:
			break;
		}
	}

	return 0;
}

/*
	Initialisations for the VM.
*/
int bfinit(bfvm **vm) {

	*vm=malloc(sizeof(bfvm));
	if (*vm==NULL) return 13;

	memset((*vm)->bfmemory, 0, sizeof(MEM_SIZE));
	(*vm)->bfpointer=0;
	(*vm)->bfstack=NULL;
	(*vm)->bfstackpointer=0;
	(*vm)->bfstacksize=0;

	(*vm)->jump_prediction.from=0;
	(*vm)->jump_prediction.to=0;

	return 0;
}

/*
	VM destructor.
*/
int bfdone(bfvm **vm) {

	int ret=0;

	if ((*vm)->bfstack) {
		free((*vm)->bfstack);
		(*vm)->bfstack=NULL;

		if ((*vm)->bfstackpointer>0) ret=11;
	}

	free(*vm);
	*vm=NULL;

	return ret;
}

