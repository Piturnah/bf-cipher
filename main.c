// Piturnah 15/02/2022

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define memCapacity 30000
char mem[memCapacity];

#define stackCapacity 1024
char* retStack[stackCapacity];
int retStackSize = 0;

#define charPool "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 "
int charPoolSize = strlen(charPool);

typedef enum
{
    ENCRYPT,
    DECRYPT,
} Mode;
Mode mode;

void usage(char *execName)
{
    printf("USAGE: %s <subcommand> <flags>\n", execName);
    printf("SUBCOMMANDS:\n");
    printf("    encrypt <file>\n");
    printf("    decrypt <file>\n");
    printf("FLAGS:\n");
    printf("    -k <key>\n");
}

// djb2 by Dan Bernstein
unsigned long
hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

int main(int argc, char *argv[])
{
    FILE *fptr;
    
    if (argc < 2) {
	fprintf(stderr, "ERROR: NO SUBCOMMAND PROVIDED\n");
	usage(argv[0]);
	return 1;
    }
    
    for (int i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-k") == 0) {
	    srand(hash(argv[++i]));
	}
	else if (strcmp(argv[i], "encrypt") == 0) {
	    mode = ENCRYPT;
	}
	else if (strcmp(argv[i], "decrypt") == 0) {
	    mode = DECRYPT;
	}
	else if ((fptr = fopen(argv[i], "r")) == NULL) {
	    fprintf(stderr, "ERROR: Could not open file %s\n", argv[i]);
	    usage(argv[0]);
	    return 1;
	}

    }

    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    
    char *program = malloc(fsize + 1);
    fread(program, fsize, 1, fptr);
    fclose(fptr);
    
    program[fsize] = 0;

    if (mode == DECRYPT) {
	int dataPtr = 0;
	
	while (*program != 0) {
	    int charOffset = rand();
	    
	    if (*program == charPool[charOffset % charPoolSize]) {
		mem[dataPtr]++;
	    }
	    else if (*program == charPool[(charOffset + 1) % charPoolSize]) {
		mem[dataPtr]--;
	    }
	    else if (*program == charPool[(charOffset + 2) % charPoolSize]) {
		dataPtr++;
	    }
	    else if (*program == charPool[(charOffset + 3) % charPoolSize]) {
		dataPtr--;
	    }
	    else if (*program == charPool[(charOffset + 4) % charPoolSize]) {
		printf("%c", mem[dataPtr]);
	    }
	    else if (*program == charPool[(charOffset + 5) % charPoolSize]) {
		mem[dataPtr] = getchar();
	    }
	    else if (*program == charPool[(charOffset + 6) % charPoolSize]) {
		if (mem[dataPtr] == 0) {
		    int stackSize = 1;
		    while (stackSize != 0) {
			switch(*program++)
			{
			case '[':
			    stackSize++;
			    break;
			case ']':
			    stackSize--;
			    break;
			}
		    }
		}
		else {
		    retStack[retStackSize++] = program - 1;
		}
	    }
	    else if (*program == charPool[(charOffset + 7) % charPoolSize]) {
		program = retStack[--retStackSize];
	    }
	    
	    program++;
	}
    }
    else if (mode == ENCRYPT) {
	char *outBuff = malloc(sizeof(char) * 30000);
	memset(outBuff, 0, sizeof(char) * 30000);
	int bufIndex = 0;
	int acc = 0;
	while (*program != 0) {
	    int charOffset;
	    
	    while (acc != *program) {
		charOffset = rand();
		
		if (acc > *program) {
		    outBuff[bufIndex++] = charPool[(charOffset + 1) % charPoolSize];
		    acc--;
		}
		else {
		    outBuff[bufIndex++] = charPool[charOffset % charPoolSize];
		    acc++;
		}
	    }
	    charOffset = rand();

	    outBuff[bufIndex++] = charPool[(charOffset + 4) % charPoolSize];
	    program++;
	}
	
	fptr = fopen("ciphertext.txt", "w");
	fputs(outBuff, fptr);

	fclose(fptr);
	printf("[INFO] Wrote output to ciphertext.txt\n");
	free(outBuff);
    }

    return 0;
}
