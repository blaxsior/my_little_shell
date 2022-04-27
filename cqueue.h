#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strtok ...

#define MAX_Q_LEN 10

// 원형 큐
// rear에서 삽입이 이루어진다.
typedef struct _cqueue
{
    char *queue[MAX_Q_LEN + 1];
    int rear;  // 뒤
    int front; // 앞
} CQueue;

// 원형 큐 내에서의 원소 움직임 묘사.
int cmove(int val)
{
    return val % (MAX_Q_LEN + 1);
}

void CQinit(CQueue *q)
{
    q->rear = 0;
    q->front = 0;
    for (int i = 0; i < MAX_Q_LEN + 1; i++)
    {
        q->queue[i] = NULL;
    }
}

int CQFull(CQueue *q)
{
    return cmove(q->rear + 1) == q->front; // rear + 1 == front 이면 꽉 찬 상태
}

int CQEmpty(CQueue *q)
{
    return (q->rear == q->front); // rear == front 이면 텅 빈 상태
}

// empty 상태이면, 아무것도 안한다.
void CQPop(CQueue *q)
{
    if (CQEmpty(q)) // 큐가 비어있다면
    {
        return; // 아무것도 안함.
    }

    q->front = cmove(q->front + 1); // q.front 이동.
    // 할당을 해제
    if (q->queue[q->front] != NULL)
    {
        free(q->queue[q->front]);
        q->queue[q->front] = NULL; // NULL 처리
    }

}

// full 상태이면, 가장 오래된 값을 삭제한다.
void CQInsert(CQueue *q, char *str)
{
    if (CQFull(q)) // 큐가 꽉 찬 경우
    {
        CQPop(q); // 내용물 제거
    }
    q->rear = cmove(q->rear + 1); // rear 이동

    // 문자열 삽입
    q->queue[q->rear] = (char *)malloc((strlen(str) + 1) * sizeof(char)); // 동적할당
    memcpy(q->queue[q->rear], str, strlen(str) * sizeof(char));           // str의 문자열을 복붙
    q->queue[q->rear][strlen(str)] = '\0';
    // 마지막 문자는 공백.
}

// rear - front
//  5 10
//  11 3
//  최대 10개
void CQPrint(CQueue *q, int len)
{
    int qlen = (q->rear - q->front + MAX_Q_LEN + 1) % (MAX_Q_LEN + 1); // 현재 큐 길이
    len = len % (MAX_Q_LEN + 1); // 값을 이상하게 높은 수치로 넣는 경우 대비.
    len = len < qlen ? len : qlen;

    for (int i = len - 1; i >= 0; i--)
    {
        int pos = cmove(q->rear - i + MAX_Q_LEN + 1); // 음수 안나오게 조정 
        if (q->queue[pos] != NULL)
        {
            printf("[%d] %s\n", i + 1, q->queue[pos]);
        }
    }
}

CQueue my_q;

void clear_queue()
{
    while(!CQEmpty(&my_q))
    {
        CQPop(&my_q);
    }
}

// int main()
// {
//     char buffer[100]; // 입력 버퍼
//     CQinit(&my_q);
//     atexit(clear_queue); // 나가면서 자원 반환.

//     while (1)
//     {
//         printf("myshell > ");

//         fgets(buffer, 100, stdin); // fgets을 이용하여 문자열 입력
//         buffer[strcspn(buffer, "\r\n")] = 0; // strcspn으로 개행문자 찾고, 해당 문자를 \0로 처리 (개행 삭제 위해)
//         if (strlen(buffer) > 0)
//         {
//             CQInsert(&my_q, buffer);
//         }

//         if (strcmp(buffer, "exit") == 0)
//         {
//             exit(EXIT_SUCCESS);
//         }

//         CQPrint(&my_q,10);
//     }
// }