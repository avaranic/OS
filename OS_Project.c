#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
//git add .
//git commit -m "message"
//git push

int openFile(char* filename)
{
    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(file == -1)
    {
        printf("Error locating file \n");
        exit(EXIT_FAILURE);
    }
    return file;
}
void exploreDirectory(const char *basePath, int file)
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
                int len = snprintf(buffer, sizeof(buffer), "Entrance: %s, Last modification: %s, Size: %lld bytes\n", path, modTime, (long long)statbuf.st_size);
                printf("Entrance: %s, Last modification: %s, Size: %lld bytes,User:%u\n", path, modTime, (long long)statbuf.st_size, statbuf.st_uid);

                write(file, buffer, len);
            }

            if (S_ISDIR(statbuf.st_mode))
            {
                exploreDirectory(path, file);
            }
        }
    }

    closedir(dir);
}

void ScrollThroughFolders(int argc, char* argv[], int file)
{
    for(int i = 1; i < argc; i++)
    {
        exploreDirectory(argv[i],file);
        write(file,"\n",1);
    }
}

int main(int argc, char* argv[])
{
    if(argc < 2 || argc > 11)
    {
        printf("Error \n");
        exit(EXIT_FAILURE);
    }
    int file = openFile("snapshot.txt");
    ScrollThroughFolders(argc, argv, file);
    return 0;
}