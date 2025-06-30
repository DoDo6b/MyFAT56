#include "structures.h"
#include "commands.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char** argv){
    if(argc==1){
        fprintf(stderr, "Syntax error: usage: %s <command> <image path> <...>\n", argv[0]);
        return -1;
    }

    if(strcmp(argv[1], "mkfs")==0){
        if(argc!=4){
            fprintf(stderr, "Syntax error: usage: %s mkfs <image path> <size>\n", argv[0]);
            return -1;
        }
        return cmd_mkfs(argv[2], (uint64_t)strtol(argv[3], NULL, 10));
    }

    if(strcmp(argv[1], "cp")==0){
        if(argc!=5){
            fprintf(stderr, "Syntax error: usage: %s cp <image path> <source> <destination>\n", argv[0]);
            return -1;
        }
        return cmd_cp(argv[2], argv[3], argv[4]);
    }

    if(strcmp(argv[1], "ls")==0){
        if(argc!=3){
            fprintf(stderr, "Syntax error: usage: %s ls <image path>\n", argv[0]);
            return -1;
        }
        return cmd_ls(argv[2]);
    }

    if(strcmp(argv[1], "rm")==0){
        if(argc!=4){
            fprintf(stderr, "Syntax error: usage: %s rm <image path> <filename>\n", argv[0]);
            return -1;
        }
        return cmd_rm(argv[2], argv[3]);
    }

    if(strcmp(argv[1], "cat")==0){
        if(argc!=4){
            fprintf(stderr, "Syntax error: usage: %s cat <image path> <filename>\n", argv[0]);
            return -1;
        }
        return cmd_cat(argv[2], argv[3]);
    }

    fprintf(stderr, "Syntax error: command not found\n");
    return -1;
}