#include "desh.h"
#include <VFS/vfs.h>
#include <arch/x86/PIT.h>
#include <klibc/memory.h>
#include <arch/x86/power.h>
#include <drivers/hid/keyboard.h>
#include <drivers/io/pci.h>
#include <flanterm.h>
#include <font/font_renderer.h>
#include <kernel.h>
#include <types/string.h>
#include <vendor/printf.h>

/* Irresponsible import */
#include <drivers/audio/soundblaster16.h>

char typedstring[255];
char currentpath[255];

void DeshInit() {
    memset(typedstring, 0, 255);
    memset(currentpath, 0, 255);
    currentpath[0] = '/';
    printf("%s >", currentpath);
}

void parseCommand(char *command) {
    char **args = split(command, ' ');
    printf("\n");
    if (strcmp(args[0], "version") == 0) {
        printf("DEfault SHell v0.00000000000000001 alpha\n");
    } else if (strcmp(args[0], "reboot") == 0) {
        sys_reboot();
    } else if (strcmp(args[0], "clear") == 0 || strcmp(args[0], "cls") == 0) {
        flanterm_write(data.ft_ctx, "\e[2J\e[H", 7);
    } else if (strcmp(args[0], "echo") == 0) {
        for (size_t i = 1; args[i] != 0; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
    } else if (strcmp(args[0], "k") == 0) {
        printf("H 0 R N E T\n");
        list_PCI_devices();
        printf("Current uptime %u secs %u ms\n", pit_get_uptime_secs(),
               pit_get_uptime_milis());
    } else if (strcmp(args[0], "") == 0) {

    } else if (strcmp(args[0], "ls") == 0) {
        if (!vfs_is_dir(currentpath)) {
            printf("you are not in a directory???\n");
        } else {
            struct dir_report nodes = vfs_iterate_dir(currentpath);
            if (nodes.num_entries == 0){
                printf("empty");
            }
            for (size_t i = 0; i < nodes.num_entries; i++)
            {
                printf("%s ",nodes.entries[i].name);
            }
            printf("\n");
        }

    } else if (strcmp(args[0], "cd") == 0) {
        char path[255];
        if (args[1][0] == '/') {
            memcpy(path, args[1], 255);
        } else {
            if (currentpath[strlen(currentpath) - 1] == '/')
            {
                snprintf(path, 255, "%s%s", currentpath, args[1]);
            }else{
                snprintf(path, 255, "%s/%s", currentpath, args[1]);
            }
        }
        if (args[1] == 0 || !vfs_is_dir(path)) {
            printf("you need to provide a DIRECTORY to change to :P\n");
        }else{
            strcpy(currentpath, path);
            //struct node* file = vfs_inspect(path);
            //if(!(file->flags & FLAGS_LOADED)){
            //    vfs_load_contents(file,path);
            //}
        }

    } else if (strcmp(args[0], "cat") == 0) {
        char path[255];
        if (args[1][0] == '/') {
            memcpy(path, args[1], 255);
        } else {
            if (currentpath[strlen(currentpath) - 1] == '/')
            {
                snprintf(path, 255, "%s%s", currentpath, args[1]);
            }else{
                snprintf(path, 255, "%s/%s", currentpath, args[1]);
            }
        }
        if(vfs_inspect(path) == NULL){
            printf("desh: no such file or directory \"%s\"\n",path);
        }else if (vfs_is_dir(path)) {
            printf("\"%s\" is a directory, moron :P\n",path);
        } else {
            printf("%s\n", (char*)vfs_read(path,0,2000).data);
        }
    } else if(strcmp(args[0],"testsb") == 0){
        printf("Testing soundblaster16\n");
        testsb();
    } else {
        printf("No such command as \"%s\" sorry :P\n", args[0]);
    }
}

void DeshUpdate() {
    getstr(typedstring, 255);
    parseCommand(typedstring);

    printf("%s >", currentpath);
}