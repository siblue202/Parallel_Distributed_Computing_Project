#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define SMALL_BUF 100
#define LISTEN_Q 128 // JGH 
#define FILE_NUM 4 // JGH

void* request_handler(void* arg);

pthread_mutex_t lock[FILE_NUM];
pthread_mutex_t queue;
pthread_cond_t cond_use[128];
pthread_cond_t cond_notuse[128];

char msg_list[128][100];
pthread_t t_id[128];
int sock_list[128];
pthread_mutex_t thread_lock[128];

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_addr, clnt_addr;
	int clnt_addr_size;
	char buf[BUF_SIZE];
	int threads_num;	

	if(argc!=3) { // 실행파일 경로/port번호 입력 2개 받기.
		printf("Usage : %s <port> <num of threads>\n", argv[0]);
		exit(1);
	}

	threads_num = atoi(argv[2]);

	serv_sock=socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성

	// 서버 주소정보 초기화
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET; // 주소체계 정하기 af : address family. 
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY); // INADDR_ANY는 자동으로 이 컴퓨터에 존재하는 랜카드 중 사용가능한 랜카드의 IP주소를 사용하라는 의미
	serv_addr.sin_port = htons(atoi(argv[1])); // htons <-> ntohs

	// 서버 주소 정보를 기반으로 주소할당
	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) {
		printf("bind() error\n");
		return -1;
	}

	// 서버소켓(리스닝 소켓)이 됨.
	// 연결요청 대기큐가 생성되며, 이 함수호출 이후로부터 클라이언트 연결요청이 가능함. 
	if(listen(serv_sock, LISTEN_Q)== -1) {
		printf("listen() error\n");
		return -1;
	}

	// 파일 수는 4로 가정하고 파일에 대한 lock init
	for (int i = 0; i< FILE_NUM; i++) {
		pthread_mutex_init(&lock[i], NULL);
	}

	for (int i = 0; i< 128; i++) {
		sock_list[i] = -1;
	}

	pthread_mutex_init(&queue, NULL);
	
	int thread_id[threads_num];
	for (int i=0; i< threads_num; i++) {
		thread_id[i] = i;
		pthread_mutex_init(&thread_lock[i], NULL);
		pthread_cond_init(&cond_use[i], NULL);	
		pthread_cond_init(&cond_notuse[i], NULL);
		pthread_create(&t_id[i], NULL, request_handler, &thread_id[i]);	// 스레드 생성 및 실행
		pthread_detach(t_id[i]);	// 종료된 스레드의 리소스 소멸
	}

	
	int index = 0;
	// 요청 및 응답
	while(1)
	{
		clnt_addr_size=sizeof(clnt_addr);
		char msg[100];
		if (index >= 1024) index = 0;
		// 클라이언트 연결요청 수락
		// 클라이언트와의 송수신을 위해 새로운 소켓 생성
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		if (clnt_sock < 0) {
			printf("Error : accept\n");
			return -1;
		}
		else {
			printf("Connection Request : %s:%d\n", 
			inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port)); // inet_ntoa : 네트워크 바이트 순서를 dotted-10진수로 변경 
			recv(clnt_sock, msg, sizeof(msg), 0);

			// thread[id]가 실행중인지 체크 
			pthread_mutex_lock(&thread_lock[index]);
			while(sock_list[index] != -1)
				pthread_cond_wait(&cond_notuse[index], &thread_lock[index]);
			strncpy(msg_list[index], msg, sizeof(msg));
			sock_list[index] = clnt_sock;
			pthread_cond_signal(&cond_use[index]);
			pthread_mutex_unlock(&thread_lock[index]);

			index++; 
			index = index % threads_num;
		}
	}

	close(serv_sock);	// 서버소켓 연결종료
	return 0;
}

void* request_handler(void *arg)
{
	int id=*((int*)arg);
	char req_line[SMALL_BUF];
	FILE* clnt_write;
	FILE* fp;
	struct stat sb;
	char msg[100];
	char buf[BUF_SIZE];
	
	char method[10];
	char file[15];
	char protocol[30];

	while(1) {
		pthread_mutex_lock(&thread_lock[id]);
		while (sock_list[id] == -1) {
			pthread_cond_wait(&cond_use[id], &thread_lock[id]);
		}
		
		strncpy(msg, msg_list[id], sizeof(msg));
		clnt_write=fdopen(dup(sock_list[id]), "w");

		if (sscanf(msg, "%[^ ] %[^ ] %[^ ]", method, file, protocol) != 3) {
			char protocol[] = "400 Bad Request\r\n";
			fputs(protocol, clnt_write);
			fflush(clnt_write);

			char end_msg[]="quit";
			send(sock_list[id], end_msg, sizeof(end_msg), 0);

			fclose(clnt_write);
			close(sock_list[id]);
			sock_list[id] = -1;
			pthread_cond_signal(&cond_notuse[id]);
			pthread_mutex_unlock(&thread_lock[id]);
			
			continue;
		}

		if (stat(file, &sb) < 0 ) {
			char protocol[] = "404 Not Found\r\n";
			fputs(protocol, clnt_write);
			fflush(clnt_write);

			char end_msg[]="quit";
			send(sock_list[id], end_msg, sizeof(end_msg), 0);
			
			fclose(clnt_write);
			sock_list[id] = -1;
			pthread_cond_signal(&cond_notuse[id]);
			pthread_mutex_unlock(&thread_lock[id]);
			
			continue;
		}

		fp = fopen(file, "r");

		char protocol[]="HTTP/1.0 200 OK\r\n";
		fputs(protocol, clnt_write);
		fflush(clnt_write);
		// 요청 데이터 전송
		while(fgets(buf, BUF_SIZE, fp) != NULL) {
			send(sock_list[id], buf, BUF_SIZE, 0);
		}
		char end_msg[]="quit";
		send(sock_list[id], end_msg, sizeof(end_msg), 0);


		// while(fgets(buf, BUF_SIZE, fp)!=NULL) 
		// {
		// 	printf("debug 17\n");
		// 	fputs(buf, fp);
		// 	fflush(fp);
		// }

		fflush(fp);
		fclose(fp);
		fclose(clnt_write);
		close(sock_list[id]);
		sock_list[id] = -1;
		pthread_cond_signal(&cond_notuse[id]);
		pthread_mutex_unlock(&thread_lock[id]);
	}
}
