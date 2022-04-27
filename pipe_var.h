#include <stdio.h>
#include <string.h> // strtok ...
#include <stdlib.h>
#include <unistd.h>   // execvp
#include <sys/wait.h> // wait

typedef struct _pnode
{
    int pos;
    struct _pnode *next;
} Pnode;

typedef struct _plist // 스택
{
    struct _pnode *top;
} Plist;

void PLinit(Plist *plist)
{
    plist->top = NULL;
}

void PLinsert(Plist *plist, int pos)
{
    Pnode *p = (Pnode *)malloc(sizeof(Pnode));
    // 값 초기화
    p->next = NULL;
    p->pos = pos;

    if (plist->top == NULL) // 값이 하나도 안 들어있는 경우.
    {
        plist->top = p; // top에 삽입.
    }
    else
    {
        p->next = plist->top;
        plist->top = p;
    }
}

int PLremove(Plist *plist)
{
    if (plist->top == NULL) // top가 null이면
    {
        return 0;
    }

    Pnode *delnode = plist->top; // 삭제할 front 노드

    plist->top = plist->top->next; // 포인터 이동
    free(delnode);                 // 노드 삭제

    return 1; // 아직 값 남음.
}

void PLremoveAll(Plist *plist)
{
    while (PLremove(plist))
        ;
}

void rec_pipe(Pnode *p, char **args, int outer)
{
    if (p == NULL)
    {
        exit(EXIT_SUCCESS); // 파이프에 넣을 인자가 없으면 종료.
    }
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("[error]");
    }
    pid_t c = fork();

    if (c > 0) // 부모
    {
        close(fd[1]); // 쓰기용 파이프 닫기
        if (outer != 0)
        {
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            wait(NULL);
        }
        else
        {
            //    free(p); // 더미노드이기 때문

            int n;
            char buffer[1024];
            waitpid(c);

            n = read(fd[0], buffer, sizeof(buffer)); // 읽은게 있으면
            if (n)
            {
                printf("%s", buffer);
            }
            else
            {
                printf("no strings\n");
            }
            close(fd[0]);
        }
    }
    else if (c == 0) // 자식

    {
        char **pos = args + p->pos; // 명령 위치 찾기.
        printf("pos : %s\n", pos[0]);

        close(fd[0]);               // 읽기용 파이프 닫기
        dup2(fd[1], STDOUT_FILENO); // 쓰기용 파이프 다른곳으로 연결
        close(fd[1]);               // 쓰기용 파이프 닫기

        rec_pipe(p->next, args, outer + 1); // 자식 수행. key = NULL이면 아무것도 안함.

        if (!execvp(pos[0], pos))
        {
            exit(EXIT_SUCCESS);
        }
        else
        {
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        perror("[error]");
        exit(EXIT_FAILURE);
    }
}

Plist my_p;

void clear_plist()
{
    PLremoveAll(&my_p);
}

Pnode *give_dummy(Plist *p)
{
    Pnode *dummy = (Pnode *)malloc(sizeof(Pnode));
    dummy->next = p->top;
    return dummy;
}