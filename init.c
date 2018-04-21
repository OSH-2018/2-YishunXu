#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

extern char **environ ;

int main()
{
    /* 输入的命令行 */
    char cmd[256];
    char **env ;
    env = environ ;

    /* 命令行拆解成的各部分，以空指针结尾 */
    //这是一个字符指针数组!
    char *args[128];
    while (1) {
        /* 提示符 */
        printf("$ ");
        fflush(stdin);
        fgets(cmd, 256, stdin);
        /* 清理结尾的换行符 */
        int i;
        for (i = 0; cmd[i] != '\n'; i++);
        //
        cmd[i] = '\0';
        /* 拆解命令行 */
        //必要的步骤！
        args[0] = cmd;
        for (i = 0; *args[i]; i++)
            for (args[i + 1] = args[i] + 1; *args[i + 1]; args[i + 1]++)
                //此处将for 改为while
                //待测试
                if (*args[i + 1] == ' ') {
                    *args[i + 1] = '\0';
                    args[i + 1]++;
                    //此处有break语句！！
                    break;
                    //此处若为while的话，break语句仅仅是退出此时的while循环，不能到达第一次for循环
                }
        // 之后均无命令
        args[i] = NULL;

        /* 没有输入命令 */
        if (!args[0])
            continue;

        /* 内建命令 */
        if (strcmp(args[0], "cd") == 0) {
            if (args[1])
                chdir(args[1]);
            continue;
        }
        if (strcmp(args[0], "pwd") == 0) {
            char wd[5000];//wd[4090]
            puts(getcwd(wd, 5000));
            continue;
        }
        if (strcmp(args[0], "exit") == 0)
            return 0;

        //char *env_p;
        if (strcmp(args[0], "env") == 0)
        {
            while (*env)
            {
                //env_p = getenv(*env);
                printf("%s\n", *env);
                env++;
            }
            env = environ ;
            continue;
            //printf("\n");
        }

        if (strcmp(args[0], "export") == 0)
        {
            //printf("%s",args[1]);
            putenv(args[1]);
            env = environ ;
            continue;
        }


        /*管道命令 若有 “|” 则执行*/
        /*用于记录管道之间的代码*/
        char *args_1[128];
        for(i = 0;args[i];i++)
        {
            args_1[i] = args[i];
        }
        /* 清理 |
        for (i = 0; args_1[i] != NULL; i++);
            if(strcmp(args_1[i], "|") == 0)
                args_1[i] = NULL ;*/
        /* 拆解命令行 */
        args_1[0] = args[0];
        //Flag 为管道的数目
        int Flag = 0 ;
        //命令的数量 Flag + 1;
        int pi = 5 ;
        //管道的位置
        int pos[5] = { 9,9,9,9,9 };
        for (i = 0; args_1[i] ; i++)
            if ( (strcmp(args[i], "|") == 0))
            {
                //说明有管道命令
                //第 Flag 个管道的位置为 i + 1 ;
                pos[Flag] = i ;
                Flag++;
                //此处漏了一个指针！
                args_1[i] = NULL; //里面的内容!!
                //args_1[i+1]++;
                //此处有break语句！！
                //break;
            }
            //此处有问题　Ｆｌａｇ并不等于管道的数目！！！
            /*for (args_1[i+1] = args_1[i] + 1; args_1[i+1]; args_1[i+1]++)

                if ( (strcmp(args[i+1], "|") == 0)) {
                    //说明有管道命令
                    //第 Flag 个管道的位置为 i + 1 ;
                    pos[Flag] = i + 1 ;
                    Flag ++ ;
                    //此处漏了一个指针！
                    *args_1[i+1] = '\0' ; //里面的内容!!

                    args_1[i+1]++;
                    //此处有break语句！！
                    break;
                }
*/
        args_1[i] = NULL;
        int pipefd[2][2] ;
        int result = 2 ,result_1 = 2 ,result_2 =2;


        //printf(" %d  ", 5 );

        if(Flag)
            //进入管道命令
        {
            //    char buff[1024];
            //创建一个字程序，子程id号为pid的值
            char * res[pi][pi] ;
            int r ;
            //将args_1数组中的各个管道符之间的命令分开
            for (i = 0; i < Flag ;  ++i)
            {
                if(i == 0)
                {
                    /*(strcmp(args[j], "|") != 0) && */
                    int j,k;
                    for ( j = 0;    (args_1[j] !=  NULL);    ++j)
                    {
                         res[0][j]=  args_1[j];
                    }
                    res[0][j] = NULL ;

                    for ( k = pos[i] + 1 ,r = 0; args_1[k] ; ++k ,r++)
                    {
                        res[1][r] = args_1[k];
                    }
                    res[1][r] = NULL ;
                }
                else
                {
                    for (int q = pos[i] + 1 , r = 0 ; args_1[q] ; ++q , r++)
                    {
                        res[i+1][r] = args_1[q];
                    }
                    res[i+1][r] = NULL ;
                }
            }

      //      char *argv_pid1[2] ;
       //     char *argv_pid2[2] ;

            i = 0 ;
            if(Flag == 1)
            {
                result = pipe(pipefd[0]);
                //创建管道失败
                if(result == -1)
                {
                    perror("pipe error:");
                    exit(EXIT_FAILURE);
                }

                pid_t pid_1 = fork();
                //创建子程序失败
                if((pid_1 == -1) )
                {
                    perror("fork  error:");
                    exit(EXIT_FAILURE);
                }

                //printf("%s   %s   ",res[0][0],res[0][1]);

                if (pid_1 == 0)
                    //成功创建子程序
                {
                    /* 子进程 */
                    //fclose(stdin);
                    close(pipefd[0][0]);
                    dup2(pipefd[0][1],1);
                    //dup2(0 , pipefd[1]);
                    close(pipefd[0][1]);
                    //fprintf(stderr ,"%s   %s   \n",res[0][0],res[0][1]);
                   // execvp("ls", res[i]);
                    execvp(res[i][0], res[i]);
                    /* execvp 执行失败则直接返回-1, 失败原因存于errno 中*/
                    if(execvp(res[i][0], res[i]) == -1)
                        return 255;
                }

                pid_t pid_2 = fork();
                if((pid_2 == -1) )
                {
                    perror("fork  error:");
                    exit(EXIT_FAILURE);
                }
                if (pid_2 == 0)
                    //成功创建子程序
                {
                    /* 子进程 */
                    //fclose(stdout);
                    close(pipefd[0][1]);
                    dup2(pipefd[0][0],0);
                    close(pipefd[0][0]);
                    /*res[i+1]*/
                    /*res[i+1][0]*/
                    //fprintf(stderr ,"%s   %s   \n",res[1][0],res[1][1]);
                    //execvp("wc",res[i+1] );
                    execvp(res[i+1][0],res[i+1] );
                    /* execvp 执行失败则直接返回-1, 失败原因存于errno 中*/
                    if(execvp(res[i+1][0], res[i+1]) == -1) ;
                    return 255;
                }
                    /* 父进程 */
                close(pipefd[0][0]);
                close(pipefd[0][1]);

                wait(NULL);/* code */
                wait(NULL);/* code */

            }
            else if (Flag == 2)
            {
                result_1 = pipe(pipefd[0]);
                result_2 = pipe(pipefd[1]);
                //创建管道失败
                if((result_1 == -1)||(result_2 == -1))
                {
                    perror("pipe error:");
                    exit(EXIT_FAILURE);
                }

                pid_t pid_1 = fork();
                if (pid_1 == 0)
                    //成功创建子程序
                {
                    /* 子进程 */
                    //fclose(stdin);
                    close(pipefd[0][0]);
                    close(pipefd[1][0]);
                    close(pipefd[1][1]);
                    dup2(pipefd[0][1],1);
                    //dup2(0 , pipefd[1]);
                    close(pipefd[0][1]);
                    //fprintf(stderr ,"%s   %s   \n",res[0][0],res[0][1]);
                    // execvp("ls", res[i]);
                    execvp(res[i][0], res[i]);
                    /* execvp 执行失败则直接返回-1, 失败原因存于errno 中*/
                    if(execvp(res[i][0], res[i]) == -1)
                        return 255;
                }

                pid_t pid_2 = fork();
                if((pid_2 == -1) )
                {
                    perror("fork  error:");
                    exit(EXIT_FAILURE);
                }
                if (pid_2 == 0)
                    //成功创建子程序
                {
                    /* 子进程 */
                    //fclose(stdout);
                    close(pipefd[0][1]);
                    close(pipefd[1][0]);
                    dup2(pipefd[0][0],0);
                    dup2(pipefd[1][1],1);
                    close(pipefd[0][0]);
                    close(pipefd[1][1]);
                    /*res[i+1]*/
                    /*res[i+1][0]*/
                    //fprintf(stderr ,"%s   %s   \n",res[1][0],res[1][1]);
                    //execvp("wc",res[i+1] );
                    execvp(res[i+1][0],res[i+1] );
                    /* execvp 执行失败则直接返回-1, 失败原因存于errno 中*/
                    if(execvp(res[i+1][0], res[i+1]) == -1) ;
                    return 255;
                }
                pid_t pid_3 = fork();
                if((pid_3 == -1) )
                {
                    perror("fork  error:");
                    exit(EXIT_FAILURE);
                }
                if (pid_3 == 0)
                    //成功创建子程序
                {
                    /* 子进程 */
                    //fclose(stdout);
                    close(pipefd[0][0]);
                    close(pipefd[0][1]);
                    close(pipefd[1][1]);
                    dup2(pipefd[1][0],0);
                    close(pipefd[1][0]);
                    /*res[i+1]*/
                    /*res[i+1][0]*/
                    //fprintf(stderr ,"%s   %s   \n",res[1][0],res[1][1]);
                    //execvp("wc",res[i+1] );
                    execvp(res[i+2][0],res[i+2] );
                    /* execvp 执行失败则直接返回-1, 失败原因存于errno 中*/
                    if(execvp(res[i+1][0], res[i+1]) == -1) ;
                    return 255;
                }

                /* 父进程 */
                close(pipefd[0][0]);
                close(pipefd[0][1]);
                close(pipefd[1][0]);
                close(pipefd[1][1]);

                wait(NULL);/* code */
                wait(NULL);/* code */
                wait(NULL);/* code */
            }

            continue;
        }
        else
        {
            /* 外部命令 */
            pid_t exterpid = fork();
            if(exterpid == 0)
            {//子进程
                execvp(args[0],args);
                return 255;

            }
            wait(NULL) ;

        }




    }
}


