#include <stdio.h> 		/* printf() 함수를 위한 헤더 파일 */
#include <unistd.h>
#include <fcntl.h> 		/* O_CREAT, O_EXEC 매크로를 위한 헤더 파일 */
#include <pthread.h>
#include <semaphore.h> 		/* sem_open(), sem_destroy(), sem_wait() 등 함수를 위한 헤더 파일 */
#include <stdbool.h>

sem_t *sem; 			/* 세마포어를 위한 전역 변수 */
static int cnt = 7; 		/* 세마포어에서 사용할 임계 구역 변수 */
static bool isRun = true;

void p() 			/* 세마포어의 P 연산 */
{
    sem_wait(sem);
    cnt--;
}

void v() 			/* 세마포어의 V 연산 */
{
    sem_post(sem);
    cnt++;
}

void *pthreadV(void *arg) 	/* V 연산을 수행하기 위한 함수를 작성한다. */
{
    while(isRun) {
        usleep(110000); 	/* 7 이상이면 100마이크로초 동안 대기한다. */
        v();
        printf("increase : %d\n", cnt) ;
        fflush(stdout);
    }

    return NULL;
}

void *pthreadP(void *arg) 	/* P 연산을 수행하기 위한 함수를 작성한다. */
{
    while(isRun) { // 감소시키니는 것이 더 빨라야 0이 되어 종료할수있다.
        p(); 			/* 세마포어가 0이 되면 블록된다. */
        printf("decrease : %d\n", cnt);
        fflush(stdout);
        if(cnt == 0) isRun = false;
        usleep(100000); 		/* 100마이크로초 간 기다린다. */
    }

    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t ptV, ptP; 	/* 스레드를 위한 자료형 */
    const char* name = "posix_sem";
    unsigned int value = cnt; 	/* 세마포어의 값 */

    /* 세마포어 열기 */
    sem = sem_open(name, O_CREAT, S_IRUSR | S_IWUSR, value);
    pthread_create(&ptV, NULL, pthreadV, NULL); 	/* 스레드 생성 */
    pthread_create(&ptP, NULL, pthreadP, NULL);

    pthread_join(ptV, NULL); 	/* 스레드가 종료될 때까지 대기 */
    pthread_join(ptP, NULL);

    /* 다 쓴 세마포어 닫고 정리 */
    sem_close(sem);
//    printf("sem_destroy() : %d\n", sem_destroy(sem));

    /* 세마포어 삭제 */
    sem_unlink(name);

    return 0;
}
