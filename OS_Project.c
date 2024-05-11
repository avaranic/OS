#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
// git add .
// git commit -m "message"
// git push

typedef struct
{
    long long unsigned int inode;
    char snap[1024];
} snapshot;

FILE *OpenFiles(char *filename, char *type)
{
    FILE *fin = fopen(filename, type);
    if (fin == NULL)
    {
        printf("Error finding the file \n");
        exit(EXIT_FAILURE);
    }
    return fin;
}

int openFile(char *filename)
{
    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == -1)
    {
        printf("Error locating file \n");
        exit(EXIT_FAILURE);
    }
    return file;
}

int checkPermisions(char *filename, int pipe_fd_write)
{
    if (access(filename, F_OK) == -1)
    {
        printf("File %s doesn't exist!\n", filename);
    }
    printf("\n\n %s \n\n", filename);
    if (access(filename, R_OK) == -1 || access(filename, W_OK) == -1 || access(filename, X_OK) == -1)
    {
        printf("File %s have missing permissions! We will start Analising for malware\n", filename);
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork error!");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            execl("verify_for_malicios.sh", "verify_for_malicios.sh", filename, NULL);
            perror("exec error");
            exit(EXIT_FAILURE);
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
            printf("waitpid status is : %d\n", status);
            char message[1024];
            if (WIFEXITED(status))
            {
                int exit_status = WEXITSTATUS(status);
                printf("Child process exited with status %d\n", exit_status);
                if (exit_status == 0)
                {
                    snprintf(message, sizeof(message), "%s is SAFE", filename);
                    printf("File %s is safe", filename);
                    return 0;
                }
                else if (exit_status == 1)
                {
                    snprintf(message, sizeof(message), "%s is DANGEROUS", filename);
                    printf("File %s is dangeros", filename);
                    return 1;
                }
                else
                {
                    snprintf(message, sizeof(message), "%s has an UNKNOWN status", filename);
                    printf("Unknown exit status");
                    return -1;
                }
            }
            else
            {
                snprintf(message, sizeof(message), "%s exited abnormally", filename);
            }
            write(pipe_fd_write, message, strlen(message));
            return -1;
        }
    }
    else
    {
        printf("This  %s file has all permissions\n", filename);
        return 2;
    }
    return 3;
}
void exploreDirectory(const char *basePath, int file, snapshot *current, int *lencur, const char *safeDir)
{
    // printf("Exploring directory: %s\n", basePath);
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    if (!dir)
    {
        printf("\nCannot open directory: %s\n\n", basePath);
        return;
    }

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }
    while ((dp = readdir(dir)) != NULL)
    {
        // printf("\n\nFile: %s\n\n\n", dp->d_name);
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", basePath, dp->d_name);

            struct stat statbuf;
            if (lstat(path, &statbuf) == 0)
            {
                char modTime[20];
                strftime(modTime, 20, "%Y-%m-%d %H:%M:%S", localtime(&statbuf.st_mtime));
                char buffer[2048];

                int danger = checkPermisions(path, pipe_fd[1]);
                if (danger == -1)
                {
                    // File status communicated via pipe, continue to next iteration
                    continue;
                }
                else if (danger == 1)
                {
                    // DIR*dir=opendir(safeDir);
                    // if(dir==NULL){
                    //     printf("error openning safe directory\n");
                    //     exit(-1);
                    // }
                    char command[1024];
                    sprintf(command, "mv %s %s", path, safeDir);
                    int result = system(command);
                    // int result=rename(path,safeDir);
                    printf("path %s  safeDir: %s\n", path, safeDir);
                    if (result == 0)
                    {
                        printf("File moved succesfully.\n");
                    }
                    else
                    {
                        printf("Error moving File. result: %d'\n", result);
                        perror("error");
                    }
                }

                int len = snprintf(buffer, sizeof(buffer), "%llu Entrance: %s, Last Modification: %s, Size: %lld bytes, inode = %llu\n", statbuf.st_ino, path, modTime, (long long)statbuf.st_size, statbuf.st_ino);
                // printf("%llu Entrance: %s, Last Modification: %s, Size: %lld bytes,User:%u\n", statbuf.st_ino, path, modTime, (long long)statbuf.st_size, statbuf.st_uid);

                current[*lencur].inode = statbuf.st_ino;
                strcpy(current[*lencur].snap, buffer);
                (*lencur)++;
                write(file, buffer, len);
            }

            if (S_ISDIR(statbuf.st_mode))
            {
                exploreDirectory(path, file, current, lencur, safeDir);
            }
        }
    }
    close(pipe_fd[1]);
    close(pipe_fd[0]);
    closedir(dir);
}

void ScrollThroughFolders(int argc, char *argv[], int file, snapshot *current, int *lencur)
{
    int numChildren = argc - 2;
    char safeDir[30];
    strcpy(safeDir, argv[4]);
    for (int i = 5; i < argc; i++)
    {

        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork Error");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            printf("procesul Copil intri pe aici %d\n", i);
            exploreDirectory(argv[i], file, current, lencur, safeDir);
            exit(EXIT_SUCCESS);
        }
    }
    printf("procesul Parinte\n");
    for (int i = 0; i < numChildren; i++)
    {
        int status;
        pid_t child_pid = wait(&status); // Remove duplicate wait() call
        if (WIFEXITED(status))
        {
            printf("Procesul Copil %d s-a încheiat cu PID-ul %d și cu statusul de ieșire %d.\n", i - 4, child_pid, WEXITSTATUS(status));
        }
    }
    printf("All children have terminated.\n");
}

void loadSnapShot(snapshot *last, int *lenLast)
{

    long long unsigned int auxInode;
    char auxSnapShots[1024];
    FILE *fin = OpenFiles("snapshot.txt", "r");
    char buffer[1024];
    while (fgets(buffer, 1024, fin) != NULL)
    {
        sscanf(buffer, "%llu", &auxInode);
        sprintf(auxSnapShots, "buffer: %s", strchr(buffer, ' '));
        last[*lenLast].inode = auxInode;
        strcpy(last[*lenLast].snap, auxSnapShots);
        (*lenLast)++;
    }
    fclose(fin);
}

void PrintDataBase(snapshot *s, int len)
{
    for (int i = 0; i < len; i++)
    {
        printf("%llu %s \n", s[i].inode, s[i].snap);
    }
}
void selectionSort(snapshot *arr, int n)
{
    int i, j, min_idx;
    snapshot temp;
    for (i = 0; i < n - 1; i++)
    {
        min_idx = i;
        for (j = i + 1; j < n; j++)
            if (arr[j].inode < arr[min_idx].inode)
                min_idx = j;
        temp = arr[min_idx];
        arr[min_idx] = arr[i];
        arr[i] = temp;
    }
}
void Compare(snapshot *last, int lenLast, snapshot *current, int lenCurrent, snapshot *result, int *lenResult)
{
    int i = 0, j = 0, k = 0;
    while (i < lenLast && j < lenCurrent)
    {
        if (last[i].inode < current[j].inode)
        {
            result[k].inode = last[i].inode;
            strcpy(result[k].snap, last[i].snap);
            i++;
            k++;
        }
        else if (last[i].inode > current[j].inode)
        {
            result[k].inode = current[j].inode;
            strcpy(result[k].snap, current[j].snap);
            j++;
            k++;
        }
        else if (last[i].inode == current[j].inode)
        {
            if (strcmp(last[i].snap, current[j].snap) != 0)
            {
                result[k].inode = current[j].inode;
                strcpy(result[k].snap, current[j].snap);
                j++;
                k++;
            }
            else
            {
                i++;
                j++;
            }
        }
    }
    while (i < lenLast)
    {
        result[k].inode = last[i].inode;
        strcpy(result[k].snap, last[i].snap);
        i++;
        k++;
    }
    while (j < lenCurrent)
    {
        result[k].inode = current[j].inode;
        strcpy(result[k].snap, current[j].snap);
        j++;
        k++;
    }
    printf("K: %d\n", k);
    *lenResult = k;
}
int main(int argc, char *argv[])
{
    snapshot last[100], current[100];
    int lenLast;
    int lenCurrent;
    loadSnapShot(last, &lenLast);

    if (argc < 5 || argc > 14)
    {
        printf("Error not enough arguemnts.\n");
        printf("Command structure shoud be ./p -o snapshotName.txt -s isolatedDir testdir1 testdir2.....\n");
        exit(EXIT_FAILURE);
    }
    int file = openFile("snapshot.txt");
    ScrollThroughFolders(argc, argv, file, current, &lenCurrent);
    selectionSort(last, lenLast);
    selectionSort(current, lenCurrent);
    printf("LEN LAST: %d\n", lenLast);
    printf("LEN CURRENT: %d\n", lenCurrent);
    printf("Current DataBase: \n");
    PrintDataBase(current, lenCurrent);
    printf("Last DataBase: \n");
    PrintDataBase(last, lenLast);
    snapshot result[100];
    int lenResult;
    Compare(last, lenLast, current, lenCurrent, result, &lenResult);
    printf("Result DataBase: \n");
    PrintDataBase(result, lenResult);

    close(file);

    return 0;
}
