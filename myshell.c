#include "cqueue.h"
#include "pipe_var.h"

#define MAX_LINE 80
#define BUF_SIZE 100

int history(char *arg)
{
    if (arg[0] == '!')
    {
        if (CQEmpty(&my_q))
        {
            printf("No commands in history\n");
        }
        if (arg[1] == '!') // all history 모드이면
        {
            CQPrint(&my_q, 10); // 전부 출력
        }
        else
        {
            char *num_str = arg + 1; // ! 다음 문자부터 포함.
            int num = atoi(num_str);
            if (num > 0)
            {
                CQPrint(&my_q, num);
            }
            else
            {
                fprintf(stderr, "[error] : No such commands in history\n");
            }
        }
        return 7; // 4 + 2 + 1. history + continue + 큐에 안넣음을 의미.
    }

    return 0;
}

int bye(char *arg)
{
    if (!strncmp(arg, "exit", BUF_SIZE)) // 문자열이 exit 이면
    {
        clear_queue();
        return 0; // is_running = 0
    }
    return 1; // is_running = 1
}

int mycd(char *arg, char *dest)
{
    if (!strcmp(arg, "cd"))
    {
        int success = chdir(dest);
        if (success == -1)
        {
            perror("[error]");
        }

        return 2; // 플래그 값으로 2 의미. continue 의미.
    }
    return 0;
}

int is_parent = 1;

void clear_func()
{
    clear_queue();
    //    clear_plist();
}

int main()
{
    CQinit(&my_q);
    PLinit(&my_p);

    atexit(clear_func);
    // 히스토리 및 큐 정보 삭제

    char *args[MAX_LINE / 2 + 1];

    char buffer[BUF_SIZE];  // 입력 버퍼
    char cop_buf[BUF_SIZE]; // history 위한 버퍼. buffer은 토큰화하면서 값이 깨져서 따로 저장해둔다.
    char cur_cwd[BUF_SIZE]; // 현재 경로.

    char *in_toc; // fgets을 통해 읽은 내용 토큰 단위로 쪼갤 때 사용되는 변수.

    int count = 0; // 몇번째 매개변수인지 기억하기 위함

    pid_t c;

    int is_running = 1;
    int is_pipe = 0;

    while (is_running)
    {
        // 초기화 코드
        PLremoveAll(&my_p);
        count = 0;
        is_pipe = 0;

        getcwd(cur_cwd, sizeof(cur_cwd)); // 현재 경로
        printf("%s: myc > ", cur_cwd);    // 쉘 표시

        fgets(buffer, 100, stdin);           // fgets을 이용하여 문자열 입력
        buffer[strcspn(buffer, "\r\n")] = 0; // strcspn으로 개행문자 찾고, 해당 문자를 \0로 처리 (개행 삭제 위해)
        // https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
        strncpy(cop_buf, buffer, BUF_SIZE);

        in_toc = strtok(buffer, " "); // 공백 기준으로 토큰 나누기

        while (in_toc && count < MAX_LINE / 2 + 1) // 토큰 내용 존재하고(NULL 아니고), 아직 공간 있으면
        {
            args[count++] = in_toc; // 토큰 args에 저장.
            in_toc = strtok(NULL, " ");
        }
        args[count] = NULL; // 마지막 위치(원래 비어 있는 공간)는 NULL로 채운다.

        if (count > 0) // NULL만 있는거 아니면
        {

            int is_concurrent = 0; // concurrent 모드인지?

            int cond = 0; // continue, 큐에 기록 여부 등 점검.

            pid_t conc;

            if (!strcmp(args[count - 1], "&")) // concurrent 모드인지 체크.
            {                                  // count = 1 인 경우, &와 NULL 밖에 없으므로 concurrent 모드라고 볼 수 없음.
                if (count == 1)
                {
                    fprintf(stderr, "[error] : syntax error near unexpected token '&'\n");
                    cond = 2;
                }
                else
                {
                    is_concurrent = 1; // concurrent 모드가 된다!

                    conc = fork();
                    // 자식 프로세스를 fork 한다.

                    if (conc == 0) // 자식 프로세스이면,
                    {
                        setpgid(0, 0); // 해당 프로세스를 현재 프로세스와 분리하여 실행한다.
                        args[count - 1] = NULL;
                        count -= 1;
                    }
                    else
                    {             // 부모 프로세스거나 에러나면
                        cond = 2; // 현재 명령은 큐에 기록하고, continue 수행.
                    }
                }
            }

            /* 조건 검사 구간 */

            /* cond 플래그
            2 : continue
            1 : 큐에 현재 명령 기록 안함.
            */
            if (!is_concurrent || (is_concurrent && conc == 0) )
            {
                cond |= history(args[0]); // history 조건 검사

                if (count > 1) // cd 조건 검사.
                {
                    cond |= mycd(args[0], args[1]);
                }

                is_running = bye(args[0]); // 종료 조건 검사
                if (!is_running)
                {
                    cond |= 3; // 명령 기록 안하고, continue -> 그냥 종료된다.
                }
            }

            /* 조건 검사 종료 */

            /* 조건에 따른 동작 */

            if ((cond & 1) != 1) // 큐에 기록
            {
                CQInsert(&my_q, cop_buf);
            }

            if ((cond & 2) == 2) // continue
            {
                if (is_concurrent && conc == 0) // concurrent 모드이면
                {
                    exit(EXIT_SUCCESS); // 그냥 종료
                }
                continue;
            }
            /* 동작 종료 */

            int cur_pos = 0;
            int l_pipe_pos = 0; // pipe 기호 위치

            while (l_pipe_pos < count)
            {
                if (!strcmp(args[l_pipe_pos], "|"))
                {
                    is_pipe = 1;
                    args[l_pipe_pos] = NULL;
                    PLinsert(&my_p, cur_pos);
                    cur_pos = l_pipe_pos + 1;
                }
                l_pipe_pos++;
            }
            if (is_pipe)
            {
                PLinsert(&my_p, cur_pos);

                Pnode *cur = my_p.front;
                // while(cur)
                // {
                //     printf("%s\n", (args + cur->pos)[0]);
                //     cur = cur->next;
                // }

                // 만약 맨 마지막에 파이프를 썼으면, 삽입할 필요 없음.
                pid_t c = fork();

                // if (c > 0)
                // {
                //     waitpid(c, NULL, 0);
                // }
                if (c == 0)
                {
                    setpgid(0, 0);
                    rec_pipe(&my_p, args);
                    exit(EXIT_SUCCESS);
                }
            }
            else
            {
                c = fork(); // 자식 만들기.
                if (c > 0)  // 자식 기다리기
                {
                    wait(NULL);
                }
                else if (c == 0)
                {
                    int result = execvp(args[0], args); // 해당 프로그램 실행.
                    if (result == -1)
                    {
                        perror("[error]");
                        exit(EXIT_FAILURE);
                        // 이거 없으면 종료 안됨!
                        //무조건 자식 프로세스에서 exit 해야함!
                    }
                    exit(EXIT_SUCCESS);
                }
            }

            if (is_concurrent)
            { // concurrent mode로 실행됬다면
                exit(EXIT_SUCCESS);
            }
        }
    }

    printf("bye!\n");
    exit(EXIT_SUCCESS);
}