#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
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
void exploreDirectory(const char *basePath, int file, snapshot *current, int *lencur)
{
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    if (!dir)
    {
        return;
    }

    while ((dp = readdir(dir)) != NULL)
    {
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
                int len = snprintf(buffer, sizeof(buffer), "%lu Entrance: %s, Last Modification: %s, Size: %lld bytes, inode = %lu\n", statbuf.st_ino, path, modTime, (long long)statbuf.st_size, statbuf.st_ino);
                printf("%lu Entrance: %s, Last Modification: %s, Size: %lld bytes,User:%u\n", statbuf.st_ino, path, modTime, (long long)statbuf.st_size, statbuf.st_uid);

                current[*lencur].inode = statbuf.st_ino;
                strcpy(current[*lencur].snap, buffer);
                (*lencur)++;
                write(file, buffer, len);
            }

            if (S_ISDIR(statbuf.st_mode))
            {
                exploreDirectory(path, file, current, lencur);
            }
        }
    }

    closedir(dir);
}

void ScrollThroughFolders(int argc, char *argv[], int file, snapshot *current, int *lencur)
{
    for (int i = 1; i < argc; i++)
    {
        exploreDirectory(argv[i], file, current, lencur);
        if (i != argc - 1)
            write(file, "\n", 1);
    }
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

    if (argc < 2 || argc > 11)
    {
        printf("Error not enough arguemnts.\n");
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