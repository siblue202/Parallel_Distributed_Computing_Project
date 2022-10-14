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
pthread_mutex_t lock;

int main(int argc, char** argv) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr;
    pthread_t tid;
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
    while (fscanf(fp, "%s", file[i]) != EOF) { // 요청할 파일들 읽어오기
        index++;
    }

    clnt_sock = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성 

    // 서버 주소정보 초기화
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET; // 주소체계 정하기 af : address family. 
	serv_addr.sin_addr.s_addr=htonl(SERV_URL); // INADDR_ANY는 자동으로 이 컴퓨터에 존재하는 랜카드 중 사용가능한 랜카드의 IP주소를 사용하라는 의미
	serv_addr.sin_port = htons(atoi(argv[1])); // htons <-> ntohs

    if (connect(clnt_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Error: connection to clinet failed in server\n");
        exit(1);
    }

    pthread_mutex_init(&lock, NULL);
    for (int i=0; i<threads_num; i++) {
        pthread_create(&tid, NULL, request_handler, &clnt_sock);
        pthread_detach(tid);
    }

    while (1) {
        
    }

    fclose(fp);
    close(clnt_sock);

    return 0;
}

void *request_handler(void *arg) {
    int clnt_sock = *((int*)arg);
    char msg[SMALL_BUF];
    char buf[BUF_SIZE];
    unsigned int count;

    FILE * readfp;
    
    for (int i=0; i< request_num; i++) {
        pthread_mutex_lock(&lock);
        // int num = rand()%128;
        int num = 0; // for test
        readfp = fdopen(clnt_sock, "r");

        sprintf(msg, "GET %s HTTP/1.0", file[num]);
        send(clnt_sock, msg, sizeof(msg), 0);

        while(1) {
            if (fgets(buf, sizeof(buf), readfp)==NULL)
                break;
            fputs(buf, stdout);
            fflush(stdout);
        }

        fclose(readfp);
        pthread_mutex_unlock(&lock);
        
        int sleep_time = rand()%1000; // 1초 미만의 랜덤한 request interval
        sleep(sleep_time);
    }

}