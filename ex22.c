//Ben Ganon 318731007

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "stdio.h"
#include "sys/types.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include "dirent.h"
#include "errno.h"
#include "math.h"

#define LINE_LEN 150


char* getFileType(char* input) {
    char* ptr = input;
    ptr = ptr+strlen(input) -2;
    return ptr;
}

int runStudent(char* root, char* dirPath, int inputFd, int outputFd){
    int stat;
    char tempPath[150];
    strcpy(tempPath, root);
    strcat(tempPath,"/");
    char* stuPath = strcat(tempPath, dirPath);
    DIR* stuDir = opendir(stuPath);
    int errorcode = 0;
    if(stuDir == NULL) {
        perror("Not a valid directory");
        exit(-1);
    }
    struct dirent* currEntry;
    while((currEntry = readdir(stuDir)) != NULL) {
        if(currEntry->d_type != DT_REG)
            continue;
        if(strcmp(getFileType(currEntry->d_name), ".c") == 0 ||
            strcmp(getFileType(currEntry->d_name), ".C") == 0 ) {

            chdir(stuPath);
            char tempCwd[LINE_LEN];
            getcwd(tempCwd, LINE_LEN);
            errorcode++;
            int save_err = dup(2);
            int gccOut = open("/gccOut.txt", O_CREAT | O_RDWR | S_IROTH , 0);
            dup2(gccOut, 2);
            char* args[] = {"gcc", "-w", currEntry->d_name, "-o a.out", NULL};
            pid_t pid = fork();
            if(pid == 0) {
               int retCode = execvp(args[0], args);
               if(retCode < 0) {
                   perror("Error execvp");
                   exit(-1);
               }
            } else {
                wait(&stat);
            }
            close(gccOut);
        }
    }
    closedir(stuDir);
    return errorcode;
}



int main(int argc, char* argv[]) {
    char* path = argv[1];
    char fileRead[LINE_LEN * 3] = {};
    char* root;
    char* input;
    char* output;
    int  fdConfig = open(path, O_RDONLY);
    if(fdConfig < 0) {
        perror("Not a valid config file");
        exit(-1);
    }
    int readNum = read(fdConfig, fileRead, LINE_LEN * 3);
    if (readNum < 0) {
        perror("Error in read");
        close(fdConfig);
        exit(-1);
    }
    int rootNum, inputNum, outputNum=0;
    root = strtok(fileRead, "\n");
    input = strtok(NULL, "\n");
    output = strtok(NULL, "\n");
    char rootNew[LINE_LEN];
    DIR* rootDir = opendir(root);
    if(rootDir == NULL) {
        perror("Not a valid directory");
        exit(-1);
    }

    struct dirent* currStd;
    int inputFd = open(input, O_RDONLY);
    int outputFd = open(output, O_RDONLY);
    if(inputFd < 0 || outputFd < 0) {
        perror("Error in open");
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