#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#include "kernel/fs.h"

char *getFileName(char *path)
{
    char *p;
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;
    return p;
}

// recursively find all the files in a dir with a given name
void find(char *path, char *target)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    char *fileName = getFileName(path);

    switch (st.type)
    {
    case T_FILE:
        if (strcmp(fileName, target) == 0)
        {
            printf("%s\n", path);
        }
        break;
    case T_DIR:
        if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0)
        {
            break;
        }
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("ls: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/'; // add a slash to the end of the path
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            {
                continue;
            }
            find(buf, target);
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(2, "Usage: find <path> <target>\n");
        exit(1);
    }

    if (strcmp(".", argv[1]) == 0 || strcmp("..", argv[1]) == 0)
    {
        int fd;
        char buf[30];
        strcpy(buf, argv[1]);
        int len = strlen(buf);
        buf[len++] = '/';
        if ((fd = open(argv[1], 0)) < 0)
        {
            fprintf(2, "main: cannot open %s\n", argv[1]);
            exit(1);
        }
        struct dirent de;
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            // avoid . and ..
            if (strcmp(".", de.name) == 0 || strcmp("..", de.name) == 0)
                continue;
            if (de.inum == 0)
                continue;
            memmove(buf + len, de.name, DIRSIZ);
            find(buf, argv[2]);
        }
        close(fd);
    }
    else
    {
        find(argv[1], argv[2]);
    }

    exit(0);
}