
/*
	A very simple Brainf*ck-Interpreter.

	Syntax: bfi scriptfile

	Have fun,
	Nakal
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/*
	BSD-optimised code is using mmap and pointer
	operations instead of walking through the
	file with a slower file-pointer.

	non-BSD code is only kept for portability.
*/
#ifdef BSD_OPTIMIZED
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif

#include "bfi.h"


int main(int argc, char *argv[]) {

#ifdef BSD_OPTIMIZED
	int scriptfd=-1;
	char *scrmem=NULL;
	off_t filelen=0;
#else
	FILE *script=NULL;
#endif
	int ret=0;
	bfvm *vm=NULL;

	switch (argc) {
	case 2:
#ifdef BSD_OPTIMIZED
		scriptfd=open(argv[1], O_RDONLY);
		if (scriptfd!=-1) break;
#else
		script=fopen(argv[1], "r");
		if (script!=NULL) break;
#endif
	default: /* FALL THROUGH ! */
		fprintf(stderr, "Syntax: bfi scriptfile\n");
		return -1;
	}

#ifdef BSD_OPTIMIZED
	filelen=lseek(scriptfd, 0, SEEK_END);
	if (filelen<0) {
		close(scriptfd);
		exit(22);
	}
	if (lseek(scriptfd, 0, SEEK_SET)<0) {
		close(scriptfd);
		exit(22);
	}

	if ((scrmem=mmap(NULL, filelen, PROT_READ, MAP_PRIVATE, scriptfd, 0))
		==MAP_FAILED) {
		close(scriptfd);
		exit(22);
	}
#endif

	ret=bfinit(&vm);

	if (ret==0) {
#ifdef BSD_OPTIMIZED
		ret=bfexec(vm, scrmem, filelen);
#else
		ret=bfexec(vm, script);
#endif

#ifdef BSD_OPTIMIZED
		munmap(scrmem, filelen);
		close(scriptfd);
#else
		fclose(script);
#endif

		ret=bfdone(&vm);
	}

	return ret;
}
