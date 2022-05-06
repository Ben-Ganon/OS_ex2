//Ben Ganon 318731007

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "stdio.h"
#include "sys/types.h"
#include "dirent.h"
#include "errno.h"
#include "math.h"

#define SIZE 100

int main(int argc, char *argv[]) {
    char *path1 = argv[1];
    char *path2 = argv[2];
    char buff1[SIZE] = {};
    char buff2[SIZE] = {};
    int isCapDiff = 0;
    int fd1 = open(path1, O_RDONLY);
    if (fd1 < 0) {
        perror("Error in ex21");
        exit(-1);
    }
    int fd2 = open(path2, O_RDONLY);
    if (fd2 < 0) {
        perror("Error in ex21");
        exit(-1);
    }

    int readNum1 = read(fd1, buff1, SIZE);
    int readNum2 = read(fd2, buff2, SIZE);
    if(readNum2 < 0 || readNum1 < 0) {
        close(fd1);
        close(fd2);
        perror("Error in ex21");
        exit(-1);
    }
    char *endBuff1 = &buff1[readNum1];
    char *endBuff2 = &buff2[readNum2];
    char *p1 = buff1;
    char *p2 = buff2;

    while (1) {
        if(readNum1 == 0 && readNum2 == 0) {
            break;
        }
        while ((*p1 == ' ' || *p1 == '\n') && p1 != endBuff1 && readNum1) {
            p1++;
        }
        while ((*p2 == ' ' || *p2 == '\n') && p2 != endBuff2 && readNum2) {
            p2++;
        }
        if(p1 == endBuff1) {
            readNum1 = read(fd1, buff1, SIZE);
            continue;
        }
        if(p2 == endBuff2) {
            readNum2 = read(fd2, buff2, SIZE);
            continue;
        }
        if (*p1 == *p2) {
            p1++;
            p2++;
            continue;
        }
        if (abs(*p2 - *p1) != 32) {
            setenv("?", "2", 1);
            return 2;
        }
        if (abs(*p2 - *p1) == 32) {
            isCapDiff = 1;
            p1++;
            p2++;
            continue;
        }

    }

}