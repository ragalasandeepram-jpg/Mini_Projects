#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"comment_removal.h"
#include"file_inclusion.h"
#include"macro_replacement.h"
int main(int argc,char *argv[])
{
        FILE *i;
        FILE *o;
        FILE *temp1;
        FILE *temp2;
        if(argc<2)
        {
                printf("Wrong command line input\n");
                exit(0);
        }
        i=fopen(argv[1],"r");
        if(i==NULL)
        {
                printf("file not found");
                exit(0);
        }
        temp1=fopen("temp1.c","w");
        o=fopen("test.i","w");
        comment_r(i,temp1);
        fclose(temp1);
        printf("comment removal is successful\n");
        temp1=fopen("temp1.c","r");
        temp2=fopen("temp2.c","w");
        process_includes(temp1,temp2);
        printf("successful file inclusion\n");
        fclose(temp1);
        fclose(temp2);
        temp2=fopen("temp2.c","r");
        process_macros(temp2,o);
        fclose(temp2);
        fclose(o);
        fclose(i);
        printf("Macro_replacement successful\n");

}
Main.c
#ifndef FILE_INCLUSION_H
#define FILE_INCLUSION_H

#include <stdio.h>
#include <string.h>

void process_includes(FILE *input, FILE *output)
{
    char line[1000];
    char filename[150];
    char path[200];
    char *p;
    char *start;
    char *end;
    FILE *header;
    int c;

    while (fgets(line, sizeof(line), input) != NULL)
    {
        p = line;

        while (*p == ' ' || *p == '\t' ||
               *p == '\n' || *p == '\r')
            p++;

        if (strncmp(p, "#include", 8) == 0)
        {
            p += 8;

            while (*p == ' ' || *p == '\t')
                p++;

            if (*p == '<')
            {
                start = p + 1;
                end = strchr(start, '>');

                if (end != NULL)
                {
                    size_t len = (size_t)(end - start);

                    if (len < sizeof(filename))
                    {
                        memcpy(filename, start, len);
                        filename[len] = '\0';

                        if (strchr(filename, '/') == NULL)
                        {
                            strcpy(path, "/usr/include/");
                            strcat(path, filename);

                            header = fopen(path, "r");

                            if (header != NULL)
                            {
                                while ((c = fgetc(header)) != EOF)
                                    fputc(c, output);

                                fputc('\n', output);
                                fclose(header);
                                continue;
                            }
                        }
                    }
                }
            }
        }

        fputs(line, output);
    }
}

#endif
