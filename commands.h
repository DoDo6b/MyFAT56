#ifndef COMMANDS_H
#define COMMANDS_H


int cmd_mkfs(const char* image_path, unsigned long long size);
int cmd_cp(const char* image_path, const char* src, const char* dst);
int cmd_ls(const char* image_path);
int cmd_rm(const char* image_path, const char* filename);
int cmd_cat(const char* image_path, const char* filename);


#endif