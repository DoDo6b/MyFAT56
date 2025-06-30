#include "commands.h"
#include "structures.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int cmd_mkfs(const char* image_path, unsigned long long size){
    FILE* fs = fopen(image_path, "wb");
    if(!fs){
        perror("fopen");
        return -1;
    }


    uint64_t FATOffset = sizeof(SuperBlock);
    uint64_t dataOffset = FATOffset + sizeof(FileHeader) * FS_FAT_CAPACITY;

    if(dataOffset >= size){
        fprintf(stderr, "ERROR: not enough space for reserved sectors (%llu)\n", dataOffset);
        fclose(fs);
        return -1;
    }


    SuperBlock sb;
    memcpy(sb.sign, FS_SIGN, FS_SIGN_SIZE);
    sb.FATOffset = FATOffset;
    sb.dataOffset = dataOffset;


    fseek(fs, 0, SEEK_SET);
    fwrite(&sb, sizeof(SuperBlock), 1, fs);

    FileHeader empty = {0};
    fseek(fs, FATOffset, SEEK_SET);
    for(int i=0; i<FS_FAT_CAPACITY; i++){
        fwrite(&empty, sizeof(FileHeader), 1, fs);
    }

    fseek(fs, size - 1, SEEK_SET);
    fputc('\0', fs);

    fclose(fs);
    printf("DONE: image %s is ready!(allocated %llu bytes)\n", image_path, size);
    return 0;
}


int cmd_cp(const char* image_path, const char* src, const char* dst){
    FILE* image = fopen(image_path, "rb+");
    if(!image){
        perror("fopen");
        return -1;
    }


    SuperBlock sb;
    fread(&sb, sizeof(SuperBlock), 1, image);
    if(memcmp(sb.sign, FS_SIGN, FS_SIGN_SIZE)!=0){
        fprintf(stderr, "ERROR: invalid filesystem image\n");
        fclose(image);
        return -1;
    }

    FileHeader FAT[FS_FAT_CAPACITY];
    fseek(image, sb.FATOffset, SEEK_SET);
    fread(FAT, sizeof(FileHeader), FS_FAT_CAPACITY, image);


    for(int i=0; i<FS_FAT_CAPACITY; i++){
        if(FAT[i].flags==FLAG_ACTIVE && strcmp(dst, FAT[i].filename)==0){
            fprintf(stderr, "ERROR: destination is already existing\n");
            fclose(image);
            return -1;
        }
    }


    FILE* source = fopen(src, "rb");
    if(!source){
        perror("fopen");
        return -1;
    }

    fseek(source, 0, SEEK_END);
    unsigned long long size = ftell(source);
    rewind(source);

    uint64_t offset = sb.dataOffset;
    for(int i=0; i<FS_FAT_CAPACITY; i++){
        uint64_t tale = FAT[i].offset + FAT[i].size;
        if(FAT[i].flags==FLAG_ACTIVE && tale > offset){
            offset = tale;
        }
    }

    fseek(image, 0, SEEK_END);
    if(ftell(image) < offset + size){
        fprintf(stderr, "ERROR: not enough memory\n");
        fclose(source);
        fclose(image);
        return 1;
    }

    int freeIndex = -1;
    for(int i=0; i<FS_FAT_CAPACITY; i++){
        if(FAT[i].flags==FLAG_DELETED){
            freeIndex = i;
            break;
        }
    }

    if(freeIndex==-1){
        fprintf(stderr, "ERROR: max amount of files has been reached\n");
        fclose(source);
        fclose(image);
        return 1;
    }

    
    char buffer[8192];
    unsigned long long read;
    fseek(image, offset, SEEK_SET);
    while((read = fread(buffer, 1, sizeof(buffer), source)) > 0){
        fwrite(buffer, 1, read, image);
    }

    FAT[freeIndex].flags = FLAG_ACTIVE;
    strncpy(FAT[freeIndex].filename, dst, FS_FILENAME_MAX_SIZE-1);
    FAT[freeIndex].filename[FS_FILENAME_MAX_SIZE-1] = '\0';
    FAT[freeIndex].offset = offset;
    FAT[freeIndex].size = size;


    fseek(image, 0, SEEK_SET);
    fwrite(&sb, sizeof(SuperBlock), 1, image);

    fseek(image, sb.FATOffset, SEEK_SET);
    fwrite(FAT, sizeof(FileHeader), FS_FAT_CAPACITY, image);
    
    fclose(source);
    fclose(image);

    printf("DONE: %s is coppied as %s\n", src, dst);
    return 0;
}

int cmd_ls(const char* image_path){
    FILE* image = fopen(image_path, "rb");
    if(!image){
        perror("fopen");
        return -1;
    }


    SuperBlock sb;
    fread(&sb, sizeof(SuperBlock), 1, image);
    if(memcmp(sb.sign, FS_SIGN, FS_SIGN_SIZE)!=0){
        fprintf(stderr, "ERROR: invalid filesystem image");
        fclose(image);
        return -1;
    }

    FileHeader FAT[FS_FAT_CAPACITY];
    fseek(image, sb.FATOffset, SEEK_SET);
    fread(FAT, sizeof(FileHeader), FS_FAT_CAPACITY, image);


    for(int i=0; i<FS_FAT_CAPACITY; i++){
        if(FAT[i].flags==FLAG_ACTIVE){
            printf("filename: %s, size: %llu bytes\n", FAT[i].filename, FAT[i].size);
        }
    }
    return 0;
}

int cmd_rm(const char* image_path, const char* filename){
    FILE* image = fopen(image_path, "rb+");
    if(!image){
        perror("fopen");
        return -1;
    }


    SuperBlock sb;
    fread(&sb, sizeof(SuperBlock), 1, image);
    if(memcmp(sb.sign, FS_SIGN, FS_SIGN_SIZE)!=0){
        fprintf(stderr, "ERROR: invalid filesystem image");
        fclose(image);
        return -1;
    }

    FileHeader FAT[FS_FAT_CAPACITY];
    fseek(image, sb.FATOffset, SEEK_SET);
    fread(FAT, sizeof(FileHeader), FS_FAT_CAPACITY, image);


    for(int i=0; i<FS_FAT_CAPACITY; i++){
        if(FAT[i].flags==FLAG_ACTIVE && strcmp(filename, FAT[i].filename)==0){
            FAT[i].flags = FLAG_DELETED;
            fseek(image, sb.FATOffset, SEEK_SET);
            fwrite(FAT, sizeof(FileHeader), FS_FAT_CAPACITY, image);
            fclose(image);
            return 0;
        }
    }
    fprintf(stderr, "ERROR: file doesn't exist\n");
    fclose(image);
    return -1;
}

int cmd_cat(const char* image_path, const char* filename){
    FILE* image = fopen(image_path, "rb+");
    if(!image){
        perror("fopen");
        return -1;
    }


    SuperBlock sb;
    fread(&sb, sizeof(SuperBlock), 1, image);
    if(memcmp(sb.sign, FS_SIGN, FS_SIGN_SIZE)!=0){
        fprintf(stderr, "ERROR: invalid filesystem image\n");
        fclose(image);
        return -1;
    }

    FileHeader FAT[FS_FAT_CAPACITY];
    fseek(image, sb.FATOffset, SEEK_SET);
    fread(FAT, sizeof(FileHeader), FS_FAT_CAPACITY, image);


    for(int i=0; i<FS_FAT_CAPACITY; i++){
        if(FAT[i].flags==FLAG_ACTIVE && strcmp(filename, FAT[i].filename)==0){
            char* data = malloc(FAT[i].size);
            if(!data){
                fprintf(stderr, "ERROR: memory allocation failed\n");
                fclose(image);
                return 1;
            }

            fseek(image, FAT[i].offset, SEEK_SET);
            fread(data, 1, FAT[i].size, image);
            fwrite(data, 1, FAT[i].size, stdout);
            printf("\n");

            free(data);
            fclose(image);
            return 0;
        }
    }
    fprintf(stderr, "ERROR: file doesn't exist\n");
    fclose(image);
    return -1;
}