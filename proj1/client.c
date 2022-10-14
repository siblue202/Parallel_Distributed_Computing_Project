#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>

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
int port;

int main(int argc, char** argv) {
    int serv_sock, clnt_sock;
    
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

    int thread_id[threads_num];
    pthread_mutex_init(&lock, NULL);
    for (int i=0; i<threads_num; i++) {
        thread_id[i] = i;
        pthread_create(&tid[i], NULL, request_handler, &thread_id[i]);
        pthread_detach(tid[i]);
    }
    
    while (1) {
        
    }

    fclose(fp);
    close(clnt_sock);

    return 0;
}

void *request_handler(void *arg) {
    int id = *((int*)arg);
    char msg[SMALL_BUF];
    char buf[BUF_SIZE];
    int clnt_sock;
    struct sockaddr_in serv_addr;

    FILE * readfp;
    
    for (int i=0; i< request_num; i++) {

        clnt_sock = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성 

        // 서버 주소정보 초기화
	    memset(&serv_addr, 0, sizeof(serv_addr));
	    serv_addr.sin_family=AF_INET; // 주소체계 정하기 af : address family. 
	    serv_addr.sin_addr.s_addr=inet_addr(SERV_URL); // INADDR_ANY는 자동으로 이 컴퓨터에 존재하는 랜카드 중 사용가능한 랜카드의 IP주소를 사용하라는 의미
	    serv_addr.sin_port = htons(atoi(port)); // htons <-> ntohs

        if (connect(clnt_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("Error: connection to clinet failed in server\n");
            exit(1);
        }

        // pthread_mutex_lock(&lock);
        // int num = rand()%128;
        int num = 0; // for test
        readfp = fdopen(clnt_sock, "r");
        printf("debug 1\n");

        sprintf(msg, "GET %s HTTP/1.0", file[num]);
        send(clnt_sock, msg, sizeof(msg), 0);
        printf("debug 2\n");
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
            recv(clnt_sock, buf, sizeof(buf), 0);
            if (strstr(buf, "quit") != NULL) {
                printf("connection completed\n");
                memset(buf, 0, sizeof(buf));
                break;
            } 
            printf("%s", buf);
        }

        printf("debug 6\n");
        fclose(readfp);
        // pthread_mutex_unlock(&lock);
        printf("debug 7\n");
        int sleep_time = rand()%10000; // 1초 미만의 랜덤한 request interval
        printf("debug 8\n");
        usleep(sleep_time);
        printf("debug 9\n");
    }

}