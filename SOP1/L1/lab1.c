#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#define MAX_PATH 101

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

int main(int argc, char **argv)
{
        char path[MAX_PATH];
    DIR *dirp;
    struct dirent *dp;
    struct stat filestat;
    //FILE *s1;

        if (getcwd(path, MAX_PATH) == NULL)
                ERR("getcwd");

    if( argc < 2 )
        ERR("Usage: ./lab1 arg1 ...");

    if( strcmp("add", argv[1]) == 0 )
    {
        if( argc != 4 )
            ERR("wrong arg count");

        if (access(argv[2], F_OK) != 0)
        {
            char mkdir[MAX_PATH] = "mkdir ";
            strcat(mkdir, argv[2]);
            //fprintf(stderr, "%s\n", mkdir);
            if(system(mkdir) != 0)
                ERR("system mkdir");
        }

        if (chdir(argv[2]))
            ERR("chdir");

        if (NULL == (dirp = opendir(".")))
            ERR("opendir");

        // if ((s1 = fopen(argv[3], "w+")) == NULL)
        //     ERR("fopen");
        // fprintf(s1, "%s", "");
        // if (fclose(s1))
        //     ERR("fclose");

        char touch[MAX_PATH] = "touch ";
        strcat(touch, argv[3]);

        if(system(touch) != 0)
            ERR("system touch");

        if (closedir(dirp))
            ERR("closedir");
    }
    else if( strcmp("list", argv[1]) == 0 )
    {
        if( argc != 3 )
            ERR("wrong arg count");

        if (chdir(argv[2]))
        {
            fprintf(stderr, "No such author: %s\n", argv[2]);
            exit(EXIT_FAILURE);
        }


        if (NULL == (dirp = opendir(".")))
                    ERR("opendir");
            do {
                    errno = 0;
                    if ((dp = readdir(dirp)) != NULL)
            {
                if (lstat(dp->d_name, &filestat))
                                    ERR("lstat");

                if(dp->d_name[0] != '.')
                    printf("%s %lo\n", dp->d_name, filestat.st_mtime);

            }
        } while (dp != NULL);

        if (errno != 0)
            ERR("readdir");
        if (closedir(dirp))
            ERR("closedir");

        // char ls[MAX_PATH] = "ls ";
        // strcat(ls, argv[2]);
        // if(system(ls) != 0)
        //     ERR("system mkdir");
    }
    else if( strcmp("stats", argv[1]) == 0 )
    {
        if( argc != 2 )
            ERR("wrong arg count");

        if (NULL == (dirp = opendir(".")))
                    ERR("opendir");
            do {
                    errno = 0;
                    if ((dp = readdir(dirp)) != NULL)
            {
                if (lstat(dp->d_name, &filestat))
                                    ERR("lstat");

                char ls[MAX_PATH] = "ls ";
                strcat(ls, dp->d_name);
                strcat(ls, " | wc -l");
            // int wc;

            if (! S_ISDIR(filestat.st_mode) ) continue;

            if(dp->d_name[0] == '.') continue;

            fprintf(stderr, "%s: ", dp->d_name);

            if( (system(ls) != 0) )
                ERR("system ls");

            // printf("\n");

            }
        } while (dp != NULL);

        if (errno != 0)
            ERR("readdir");
        if (closedir(dirp))
            ERR("closedir");
    }
    else
    {
        ERR("wrong arg");
    }

        return EXIT_SUCCESS;
}