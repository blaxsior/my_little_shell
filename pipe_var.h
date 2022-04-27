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

typedef struct _plist // 리스트
{
    struct _pnode *front;
    struct _pnode *rear;
} Plist;

void PLinit(Plist *plist)
{
    plist->rear = NULL;
    plist->front = NULL;
}

void PLinsert(Plist *plist, int pos)
{
    Pnode *p = (Pnode *)malloc(sizeof(Pnode));
    // 값 초기화
    p->next = NULL;
    p->pos = pos;

    if (plist->front == NULL) // 값이 하나도 안 들어있는 경우.
    {
        plist->front = p;
    }
    else
    {
        plist->rear->next = p;
    }
    plist->rear = p; // top에 삽입
}

int PLremove(Plist *plist)
{
    if (plist->front == NULL)
    {
        plist->rear = NULL;
        return 0;
    }

    Pnode *delnode = plist->front; // 삭제할 front 노드

    plist->front = plist->front->next; // 포인터 이동
    free(delnode);                     // 노드 삭제

    return 1; // 아직 값 남음.
}

void PLremoveAll(Plist *plist)
{
    while (PLremove(plist))
        ;
}

int rec_pipe(Plist *p, char **args)
{
    int fd[2];
    int next = 0;

    Pnode *cur = p->front;

    while (cur->next) // 다음 것이 있다면
    {
        pipe(fd);

        pid_t c = fork();

        if (c > 0)
        {
            wait(NULL);
        }
        if (c == 0)
        {
            char **pos = args + cur->pos; // 현재 명령어 위치
            if (next != 0)
            {
                dup2(next, STDIN_FILENO);
                close(next);
            }

            if (fd[1] != 1)
            {
                dup2(fd[1], STDOUT_FILENO);

                close(fd[1]);
            }

            execvp(pos[0], pos);
            exit(EXIT_SUCCESS);
        }
        next = fd[0];
        cur = cur->next;
    }

    if (next != 0)
        dup2(next, 0);
    char **pos = args + cur->pos; // 현재 명령어 위치
    execvp(pos[0], pos);
    exit(EXIT_SUCCESS);
}

Plist my_p;

void clear_plist()
{
    PLremoveAll(&my_p);
}

Pnode *give_dummy(Plist *p)
{
    Pnode *dummy = (Pnode *)malloc(sizeof(Pnode));
    dummy->next = p->rear;
    return dummy;
}

// https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell
// 파이프의 구성은 위의 구조를 참고했으나, 오류가 존재...