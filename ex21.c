//Ben Ganon 318731007

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "stdio.h"
#include "sys/types.h"
#include "dirent.h"
#include "errno.h"

int main(int argc, char* argv[]) {
    char* path1 = argv[1];
    char* path2 = argv[2];
    int fd1 = open(path1, O_RDONLY);
    int fd2 = open(path2, O_RDONLY);
    if(fd1 < 0 || fd2 < 0) {
        perror("error opening files");
        close(fd1);
        close(fd2);
        exit(1);
    }

}