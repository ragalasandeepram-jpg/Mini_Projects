#include<stdlib.h>
void comment_r(FILE *i,FILE *o)
{
        int p;
        int n;
        int nn;
        while((p=fgetc(i))!=EOF)
        {
                if(p=='/')
                {
                        n=fgetc(i);
                        if(n=='/')
                        {
                                while((p=fgetc(i))!='\n')
                                {
                                }
                                if(p=='\n')
                                {
                                        fputc('\n',o);
                                }
                        }
                        else if(n=='*')
                        {
                                int prev=0;
                                while((p=fgetc(i))!=EOF)
                                {
                                        if(prev=='*' && p=='/')
                                        {
                                                break;
                                        }
                                        prev=p;
                                }
                        }
                        else
                        {
                                fputc(p,o);
                                if(n!=EOF)
                                {
                                        fputc(n,o);
                                }
                        }
                }
                else if(p=='"')
                {
                        n=fgetc(i);
                        if(n=='/')
                        {
                                fputc(p,o);
                                fputc(n,o);
                        }
                        else
                        {
                                fputc(p,o);
                                fputc(n,o);
                        }

                }
                else
                {
                        fputc(p,o);
                }
        }
}
