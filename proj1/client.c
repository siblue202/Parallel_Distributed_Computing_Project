#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

// https://mangkyu.tistory.com/48
// https://recipes4dev.tistory.com/153
// https://novice-programmer-story.tistory.com/40

#define SERV_URL "127.0.0.1"
#define BUF_SIZE 1024
#define SMALL_BUF 100

void* request_handler(void* arg);

char file[128][32];
int request_num;
pthread_t tid[128];
pthread_mutex_t lock;
int file_num;
struct sockaddr_in serv_addr;

int main(int argc, char** argv) {    
    FILE *fp;
    char filename[16];
    int threads_num;
    int port;

    if (argc != 5) {
        printf("Usage : %s <port> <num of threads> <num of requests> <filename>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);
    threads_num = atoi(argv[2]);
    request_num = atoi(argv[3]);
    strcpy(filename, argv[4]);
    fp = fopen(filename, "r");

    int index = 0;
    while (fscanf(fp, "%s", file[index]) != EOF) { // 요청할 파일들 읽어오기
        index++;
    }
    file_num = index;
    fclose(fp);

    // 서버 주소정보 초기화
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET; // 주소체계 정하기 af : address family. 
	// serv_addr.sin_addr.s_addr=inet_addr(SERV_URL); // INADDR_ANY는 자동으로 이 컴퓨터에 존재하는 랜카드 중 사용가능한 랜카드의 IP주소를 사용하라는 의미
	int rtnValue = inet_pton(AF_INET, SERV_URL, &serv_addr.sin_addr.s_addr);
    if (rtnValue == 0) printf("invalid addr string\n");
    else
        if(rtnValue <0) printf("inet_pton() fails\n");
    serv_addr.sin_port = htons(port); // htons <-> ntohs

    int thread_id[threads_num];
    pthread_mutex_init(&lock, NULL);
    for (int i=0; i<threads_num; i++) {
        thread_id[i] = i;
        pthread_create(&tid[i], NULL, request_handler, &thread_id[i]);
        // pthread_detach(tid[i]);
    }
    
    int status;
    for (index = 0; index < threads_num; index++) {
        pthread_join(tid[index], (void **)&status);
    }

    pthread_exit(NULL);
    
    return 0;
}

void *request_handler(void *arg) {
    int id = *((int*)arg);
    char msg[SMALL_BUF];
    char buf[BUF_SIZE];
    int clnt_sock;
    clock_t start, end;

    FILE * readfp;

    unsigned long size = 0;
    
    for (int i=0; i< request_num; i++) {
        srand(time(NULL));
        start = clock();
        if ((clnt_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // TCP 소켓 생성 
            printf("[thread %d]Error: Unable to open socket in server.\n", id);
            exit(1);
        }
        

        if ((connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) == -1) {
            perror("error in connect()");
            printf("[thread %d]Error: connection to server failed in client\n", id);
            exit(1);
        }

        // pthread_mutex_lock(&lock);
        int num = rand()%file_num;
        // int num = 0; // for test
        readfp = fdopen(clnt_sock, "r");

        sprintf(msg, "GET %s HTTP/1.0", file[num]);
        send(clnt_sock, msg, sizeof(msg), 0);
        // while(1) {
        //     if (fgets(buf, sizeof(buf), readfp)==NULL)
        //         break;
        //     printf("debug 3\n");
        //     // fputs(buf, stdout);
        //     printf("%s", buf);
        //     printf("debug 4\n");
        //     fflush(stdout);
        //     printf("debug 5\n");
        // }

        while(1) {
            unsigned long tmp = recv(clnt_sock, buf, sizeof(buf), 0);
            if (strstr(buf, "quit") != NULL) {
                printf("[thread %d]Iter%d connection completed\n", id, i);
                memset(buf, 0, sizeof(buf));
                break;
            } 
            printf("%s", buf);
            size += tmp;
        }

        fclose(readfp);

        close(clnt_sock);
        end = clock();
        printf("[thread %d]Iter%d exec time : %lf\n", id, i, (double)(end-start)/CLOCKS_PER_SEC);
        
        // pthread_mutex_unlock(&lock);
        int sleep_time = rand()%10000; // 1초 미만의 랜덤한 request interval
        usleep(sleep_time);
    }

    printf("[thread %d]Total received bytes : %lu\n", id, size);
    exit(1);
}