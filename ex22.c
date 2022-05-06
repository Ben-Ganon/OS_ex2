//Ben Ganon 318731007

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "stdio.h"
#include "sys/types.h"
#include <sys/stat.h>
#include "dirent.h"
#include "errno.h"
#include "math.h"

#define LINE_LEN 150


int runStudent(char* root, char* dirPath, int inputFd, int outputFd){
    return 0;
}

int main(int argc, char* argv[]) {
    char* path = argv[1];
    char root[LINE_LEN] = {};
    char input[LINE_LEN] = {};
    char output[LINE_LEN] = {};
    int  fdConfig = open(path, O_RDONLY);
    if(fdConfig < 0) {
        perror("Not a valid config file");
        exit(-1);
    }
    int readLine = read(fdConfig, root, LINE_LEN);
    if (readLine < 0) {
        perror("Error in ex22");
        close(fdConfig);
        exit(-1);
    }
    readLine = read(fdConfig, input, LINE_LEN);
    if (readLine < 0) {
        perror("Error in ex22");
        close(fdConfig);
        exit(-1);
    }
    readLine = read(fdConfig, output, LINE_LEN);
    if (readLine < 0) {
        perror("Error in ex22");
        close(fdConfig);
        exit(-1);
    }
    DIR* rootDir = opendir(root);
    if(rootDir == NULL) {
        perror("Not a valid directory");
        exit(-1);
    }

    struct dirent* currStd;
    int inputFd = open(input, O_RDONLY);
    int outputFd = open(output, O_RDONLY);
    if(inputFd < 0 || outputFd < 0) {
        perror("Error in ex22");
        closedir(rootDir);
        close(fdConfig);
        exit(-1);
    }
    while((currStd = readdir(rootDir))!=NULL) {
        if(currStd->d_type != DT_DIR)
            continue;

        int status = runStudent(root, currStd->d_name, inputFd, outputFd);
    }


    close(fdConfig);
}