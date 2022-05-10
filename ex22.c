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


char *getFileType(char *input, int back) {
    char *ptr = input;
    ptr = ptr + strlen(input) - back;
    return ptr;
}

int findOutFile(char *dirPath) {
    DIR *curr = opendir(dirPath);
    if (curr == NULL) {
        perror("Not a valid directory");
        exit(-1);
    }
    struct dirent *currEntry;
    while ((currEntry = readdir(curr)) != NULL) {
        if (strcmp(getFileType(currEntry->d_name, 4), ".out") == 0)
            return 1;
    }
    closedir(curr);
    return 0;
}

int runStudent(char *root, char *dirPath, int inputFd, int errorFd, char*outPutComp, char* compDir) {
    int stat;
    char tempPath[150];
    strcpy(tempPath, root);
    strcat(tempPath, "/");
    char *stuPath = strcat(tempPath, dirPath);
    DIR *stuDir = opendir(stuPath);
    int exitCode = 0;
    int oldIn = dup(0);
    int oldOut = dup(1);
    int save_err = dup(2);


    if (stuDir == NULL) {
        perror("Not a valid directory");
        exit(-1);
    }
    struct dirent *currEntry;
    while ((currEntry = readdir(stuDir)) != NULL) {
        if (currEntry->d_type != DT_REG)
            continue;
        if (strcmp(getFileType(currEntry->d_name, 2), ".c") == 0 ||
            strcmp(getFileType(currEntry->d_name, 2), ".C") == 0) {

            chdir(stuPath);
            char tempCwd[LINE_LEN];
            getcwd(tempCwd, LINE_LEN);
            exitCode++;
            dup2(errorFd, 2);
            char *args[] = {"gcc", "-w", currEntry->d_name, "-o a.out", NULL};
            pid_t pid = fork();
            if (pid == 0) {
                int retCode = execvp(args[0], args);
                if (retCode < 0) {
                    perror("Error execvp1");
                    exit(-1);
                }
            } else {
                wait(&stat);
                if(stat == 0) {
                    exitCode++;
                    int outputFd = open("out.txt", O_CREAT| S_IWUSR, 0666);
                    dup2(inputFd, 0);
                    dup2(outputFd, 1);
                    char *argRunStudent[] = {"./a.out", NULL};
                    pid = fork();
                    if(pid == 0) {
                        int retCode = execvp(argRunStudent[0], argRunStudent);
                        if (retCode < 0) {
                            perror("Error execvp2");
                            exit(-1);
                        }
                    } else {
                        wait(&stat);
                        close(outputFd);
                        char* argRunComp[] = {compDir, outPutComp, "out.txt", NULL};
                        pid = fork();
                        if(pid ==0) {
                            int retCode = execvp(argRunComp[0], argRunComp);
                            if (retCode < 0) {
                                perror("Error execvp3");
                                exit(-1);
                            }
                        } else {
                            wait(&stat);
                            remove("out.txt");
                            if(stat == 1) {
                                exitCode += 3;
                            } else if (stat == 2) {
                                exitCode++;
                            }else if (stat ==3) {
                                exitCode += 2;
                            } else {
                                continue;
                            }
                        }
                    }

                }

            }
        }
    }
    dup2(oldIn, 0);
    dup2(oldOut, 1);
    dup2(save_err, 2);
    closedir(stuDir);
    return exitCode;
}


int main(int argc, char *argv[]) {
    char *path = argv[1];
    char fileRead[LINE_LEN * 3] = {};
    char *root;
    char *input;
    char *output;
    int fdConfig = open(path, O_RDONLY);
    if (fdConfig < 0) {
        perror("Not a valid config file");
        exit(-1);
    }
    int readNum = read(fdConfig, fileRead, LINE_LEN * 3);
    if (readNum < 0) {
        perror("Error in read");
        close(fdConfig);
        exit(-1);
    }
    root = strtok(fileRead, "\n");
    input = strtok(NULL, "\n");
    output = strtok(NULL, "\n");
    char rootNew[LINE_LEN];
    DIR *rootDir = opendir(root);
    if (rootDir == NULL) {
        perror("Not a valid directory");
        exit(-1);
    }

    char compDir[LINE_LEN];
    getcwd(compDir, LINE_LEN);
    strcat(compDir, "/comp.out");

    struct dirent *currStd;
    int inputFd = open(input, O_RDONLY);
    int outputFd = open(output, O_RDONLY);
    if (inputFd < 0 || outputFd < 0) {
        perror("Error in open");
        closedir(rootDir);
        close(fdConfig);
        exit(-1);
    }
    chdir(root);
    int errorFd = open("errors.txt", O_CREAT| S_IWUSR, 0666);
    while ((currStd = readdir(rootDir)) != NULL) {
        if (currStd->d_type != DT_DIR || currStd->d_name[0] =='.')
            continue;
        int status = runStudent(root, currStd->d_name, inputFd, errorFd, output, compDir);
        printf("%d, %s\n", status, currStd->d_name);
    }
    close(errorFd);
    close(fdConfig);
}