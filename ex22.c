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

void addEnv(char *argv) {
    char *tempPath = getenv("PATH");
    strcat(tempPath, ":");
    strcat(tempPath, argv);
    setenv("PATH", tempPath, 1);
}

char *getFileType(char *input, int back) {
    char *ptr = input;
    ptr = ptr + strlen(input) - back;
    return ptr;
}


int runStudent(char *root, char *dirPath, int inputFd, int errorFd, char *outPutComp, char *compDir) {
    int stat, errStat;

    char tempPath[150];
    strcpy(tempPath, root);
    strcat(tempPath, "/");
    char *stuPath = strcat(tempPath, dirPath);
    DIR *stuDir = opendir(dirPath);
    int exitCode = 0;
    int oldIn = dup(0);
    int oldOut = dup(1);
    int save_err = dup(2);


    if (stuDir == NULL) {
        perror("Not a valid directory");
        exit(-1);
    }
    chdir(dirPath);
    struct dirent *currEntry;
    while ((currEntry = readdir(stuDir)) != NULL) {
        if (currEntry->d_type != DT_REG)
            continue;
        if (strcmp(getFileType(currEntry->d_name, 2), ".c") == 0 ||
            strcmp(getFileType(currEntry->d_name, 2), ".C") == 0) {

            exitCode++;
            if(dup2(errorFd, 2) <0)
            {
                perror("Error in: dup2");
            }
            char *args[] = {"gcc", "-w", currEntry->d_name, "-o", "a.out", NULL};
            pid_t pid = fork();
            if (pid == 0) {
                int retCode = execvp(args[0], args);
                if (retCode < 0) {
                    perror("Error in: execvp");
                    exit(-1);
                }
            } else {
                wait(&stat);
                errStat = WEXITSTATUS(stat);
                if (errStat == 0) {
                    exitCode++;
                    int outputFd = open("out.txt", O_CREAT | O_RDWR, 0666);
                    dup2(inputFd, 0);
                    dup2(outputFd, 1);
                    char *argRunStudent[] = {"./a.out", NULL};
                    pid = fork();
                    if (pid == 0) {
                        int retCode = execvp(argRunStudent[0], argRunStudent);
                        if (retCode < 0) {
                            perror("Error in: execvp");
                            exit(-1);
                        }
                    } else {
                        wait(&stat);
                        close(outputFd);
                        char *argRunComp[] = {"comp.out", compDir, "out.txt", NULL};
                        pid = fork();
                        if (pid == 0) {
                            int retCode = execvp(argRunComp[0], argRunComp);
                            if (retCode < 0) {
                                perror("Error in: execvp");
                                exit(-1);
                            }
                        } else {
                            wait(&stat);
//                            remove("out.txt");
                            errStat = WEXITSTATUS(stat);
                            if (errStat == 1) {
                                exitCode += 3;
                            } else if (errStat == 2) {
                                exitCode++;
                            } else if (errStat == 3) {
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
    if(lseek(inputFd, 0, SEEK_SET) < 0)
    {
        perror("Error in: lseek");
        exit(-1);
    }
//    remove("a.out");
//    remove("out.txt");
    if(dup2(oldIn, 0) < 0 || dup2(oldOut, 1) < 0 || dup2(save_err, 2) < 0 || closedir(stuDir) < 0)
    {
        perror("Error in: dup2");
    }

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
        perror("Error in: read");
        close(fdConfig);
        exit(-1);
    }
    root = strtok(fileRead, "\n");
    input = strtok(NULL, "\n");
    output = strtok(NULL, "\n");
    DIR *rootDir = opendir(root);
    if (rootDir == NULL) {
        perror("Not a valid directory");
        exit(-1);
    }


    struct dirent *currStd;
    int inputFd = open(input, O_RDONLY);
    if (inputFd < 0) {
        perror("Input file not exist");
        closedir(rootDir);
        close(fdConfig);
        exit(-1);
    }

    int csvFd = open("results.csv", O_CREAT | O_APPEND | O_RDWR, 0666);
    if (csvFd < 0) {
        perror("Error in: open");
        closedir(rootDir);
        close(fdConfig);
        close(inputFd);
        exit(-1);
    }
    int errorFd = open("errors.txt", O_CREAT | O_RDWR | O_APPEND, 0666);
    if (errorFd < 0) {
        perror("Error in: open");
        closedir(rootDir);
        close(fdConfig);
        close(inputFd);
        exit(-1);
    }

    link(output, "corrOut.txt");

    char rootChdir[LINE_LEN];
    getcwd(rootChdir, LINE_LEN);
    addEnv(rootChdir);

    char compDir[LINE_LEN];
    strcpy(compDir, rootChdir);
    strcat(compDir, "/corrOut.txt");

    chdir(root);
    getcwd(rootChdir, LINE_LEN);


    while ((currStd = readdir(rootDir)) != NULL) {
        chdir(rootChdir);
        if (currStd->d_type != DT_DIR || currStd->d_name[0] == '.')
            continue;
        int status = runStudent(root, currStd->d_name, inputFd, errorFd,compDir,compDir);
        char csvStr[LINE_LEN];
        strcpy(csvStr, currStd->d_name);
        char *grade;
        switch (status) {
            case 0:
                grade = ",0,NO_C_FILE\n";
                break;
            case 1:
                grade = ",10,COMPILATION_ERROR\n";
                break;
            case 3:
                grade = ",50,WRONG\n";
                break;
            case 4:
                grade = ",75,SIMILAR\n";
                break;
            case 5:
                grade = ",100,EXCELLENT\n";
            default:
                break;
        }
        strcat(csvStr, grade);
        int err = write(csvFd, csvStr, strlen(csvStr));
        if(err < 0)
        {
            close(csvFd);
            closedir(rootDir);
            close(fdConfig);
            close(inputFd);
            exit(-1);
        }
        
    }
    close(csvFd);
    closedir(rootDir);
    close(errorFd);
    close(fdConfig);
}
