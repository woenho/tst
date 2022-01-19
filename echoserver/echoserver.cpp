
#include "tst.h"


using namespace std;
using namespace tst;

int g_exit = 0;

bool g_ami_connected = 0;

tstpool server;

TST_STAT echo(PTST_SOCKET psocket)
{
	if (psocket->func) {
		TST_STAT next = psocket->func(psocket);
		if (next != tst_run)
			return next;
	}

	if (psocket && psocket->events & EPOLLIN) {
		if (psocket->recv) {
			TST_DATA& data = *psocket->recv;
			uint remain = data.s_len - data.com_len;
			printf("recv data len(%d), remain buffer(%d)\n", data.checked_len, remain);
			if (data.checked_len > remain) {
				printf("Not enought remain buffer(%d/%d)\n", data.checked_len, remain);
				char a;
				while (data.checked_len--) {
					read(psocket->sd, &a, 1);
					printf("%c", a);
				}
				data.reset_data();
			} else {
#if 0
				while (data.checked_len--) {
					read(psocket->sd, data.s + data.result_len, 1);
					printf("%c", data.s[data.result_len]);
					data.result_len++;
				}
#else
				char* p = data.s + data.com_len;
				data.com_len += read(psocket->sd, data.s + data.com_len, data.checked_len);
				data.s[data.com_len] = '\0';
				printf("%s", p);
#if 0
				psocket->send->req_len = sprintf(psocket->send->s, "echo msg:%s:", p);
#else
				psocket->send->req_len = sprintf(psocket->send->s, "%s", p);
#endif
				psocket->send->com_len = write(psocket->sd, psocket->send->s, psocket->send->req_len);

#endif
				data.reset_data();
			}
		} else {
			printf("has not recv buffer\n");
			//
		}
	} else if (psocket && psocket->events & EPOLLOUT) {

		TST_DATA& data = *psocket->send;

		printf("remained write buffer(%d)\n", data.checked_len);

		int err = ioctl(psocket->sd, SIOCOUTQ, &data.checked_len);	

		if (err < 0 ) {
			return tst_disconnect;
		} else if (data.checked_len) {
			// 아직 보내지 못한 데이타 있다 
			return tst_send;
		} else {
			// 앞전에 보낸것 잘 보냈는지 확인
			// data send event stop
			data.reset_data();

			uint st = rand() % 1000000;
			usleep(st);
		}
	}

	return tst_suspend;
}

TST_STAT first(PTST_SOCKET psocket)
{
	// *** 이 함수는 메인 쓰레드가 호출하는 것으로 직접 write 하지 말자. 
	psocket->send->req_len = sprintf(psocket->send->s, "--- Wellcome to echo test server...\n");
	psocket->send->com_len = 0;
	printf("--- request first send rew_len=%d, result_len=%d :%s:\n", psocket->send->req_len, psocket->send->com_len, psocket->send->s);

	return tst_send; /// *** 반드시 tst_send 를 설정해야 워크쓰레드가 sned 에 설정된 메시지를 보내준다./
}



TST_STAT my_disconnected(PTST_SOCKET psocket) {
	if (!psocket)
		return tst_suspend;

	if (psocket->type == sock_client) {
		TRACE("--- 흐미 ami 끊어졌다.....\n");
		// 다시 연결하자
		// 타이머도 걸어야 한다
		// 타이머 걸린 동안 안 살아나면 다시 연결한다
		g_ami_connected = 0;

	} else {
		TRACE("--- disconnected client socket....%s() \n",__func__);
	}

	return tst_suspend;
}

int main(int argc, char* argv[])
{
	g_exit = 0;


	srand(1);

	// listen 모드의 서버소켓으로 동작한다.
	// 연결이 들어오면 받아주고 메시지가 오면 그대로 돌려주는 에코서버로 동작한다.
	// 기동된 다음 30초 후에 종료메시지를 카운트다운하며 보내주고 서버를 종료한다.

	server.setEventNewConnected(first);

	if (!server.create(5, "0.0.0.0", 20001, echo, 4096, 4096)) {
		printf("---쓰레드풀이 기동 되지 못했다....\n");
		return 0;
	}

	printf("--- now read test-------------------------\n");

	int i = 30;
	while (--i) {
		sleep(1);
	}

	printf("--- now write test-------------------------\n");

	g_exit = 1;

	i = 10;
	while (--i) {
		pthread_mutex_lock(&server.m_mutexConnect);
		map<int, PTST_SOCKET>::iterator it_connect;
		for (it_connect = server.m_connect.begin(); it_connect != server.m_connect.end(); it_connect++) {
			PTST_SOCKET socket = it_connect->second;

			socket->send->req_len = sprintf(socket->send->s, "--- good bye. about %d seconds...\n", i);
			socket->send->com_len = 0;

			printf("--- request send to sd=%d :%s:\n", socket->sd, socket->send->s);

			server.need_send(socket->sd);
		}
		pthread_mutex_unlock(&server.m_mutexConnect);

		sleep(1);
	}

	printf("---쓰레드풀을 종료합니다.....\n");

	server.destroy();


	return 0;
}
