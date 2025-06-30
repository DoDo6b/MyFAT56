#ifndef STRUCTURES_H
#define STRUCTURES_H


#include <stdint.h>

#define FS_SIGN "VFS512"
#define FS_SIGN_SIZE 6 // < 8 иначе выравние нужно пересчитать
#define FS_FILENAME_MAX_SIZE 32
#define FS_FAT_CAPACITY 512
#define FS_SUPERBLOCK_SIZE 512

#define FLAG_DELETED 0
#define FLAG_ACTIVE 1


typedef struct{
    char sign[FS_SIGN_SIZE];
    uint64_t FATOffset;
    uint64_t dataOffset;
    uint8_t reserved[FS_SUPERBLOCK_SIZE - 24];  // размер супер блока - размер хеддера(размер структуры без этой строки) P.S. Надо избавиться от такого костыля
} SuperBlock;


typedef struct{
    char filename[FS_FILENAME_MAX_SIZE];
    uint8_t flags;
    uint64_t offset;
    uint64_t size;
}FileHeader;


#endif