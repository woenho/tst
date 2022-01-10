#include "amiaction.h"


ATP_STAT amiLogin(PATP_DATA atp_data)
{
	if (ami_socket)
		return stat_suspend;

	// 일단 로그인 프로세스가 시작되었다는 것을 알리기 위해서라도 객체만 생성한다
	uint32_t	buf_len = 4096;
	ami_socket = new TST_SOCKET(buf_len, buf_len);
	struct	sockaddr_in s_info;
	AMI_LOGIN& login = *(PAMI_LOGIN)&atp_data->s;

	int sd = server.tcp_connect(login.Host, login.Port, &s_info);
	if (sd < 1) {
		// 연결 실패
		printf("--- tcp socket connect failed(%d:%s)\n", errno, strerror(errno));
		delete ami_socket;
		ami_socket = NULL;
		return stat_suspend;
	}

	struct timeval stTimeVal;
	fd_set rset;

	stTimeVal.tv_sec = 5;
	stTimeVal.tv_usec = 0;
	FD_ZERO(&rset);
	FD_SET(sd, &rset);
	
	// 5초간 입력을 기다린다
	int rc = select(sd+1, &rset, 0, 0, &stTimeVal);
	if (!rc) {
		printf("연결시간 5초 만료!!!!\n");
		delete ami_socket;
		ami_socket = NULL;
		return stat_suspend;
	}
	if (rc<0) {
		printf("연결된 소켓 오류\n");
		delete ami_socket;
		ami_socket = NULL;
		return stat_suspend;
	}

	ami_socket->sd = sd;
	ami_socket->type = sock_client;
	memcpy(&ami_socket->client, &s_info, sizeof(s_info));
	// 이 소켓은 AMI 소켓이고 이 소켓에 대이타가 들어오면 ami_event() 를 호출하라
	ami_socket->func = ami_event;
	ami_socket->user_data = AMI_MANAGE::alloc();

	AMI_MANAGE& manage = *(PAMI_MANAGE)ami_socket->user_data->s;	
	TST_DATA& rdata = *ami_socket->recv;
	TST_DATA& sdata = *ami_socket->send;
	int connected = 0;
	int retry = 100;

	while (connected < 2) {
		ioctl(sd, SIOCINQ, &rdata.checked_len);
		if (rdata.checked_len) {
			rdata.com_len += read(sd, rdata.s + rdata.com_len, rdata.checked_len);
			TRACE("recv:\n%s", rdata.s);
		}

		if (rdata.com_len < 22) {
			if (!--retry) {
				printf("연결된 소켓은 Asterisk AMI 가 아니다!(retry over)\n");
				delete ami_socket;
				ami_socket = NULL;
				return stat_suspend;
			}
			usleep(10000);
			continue;
		}

		if (!connected && strncmp(rdata.s, "Asterisk Call Manager/", 22)) {
			printf("연결된 소켓은 Asterisk AMI 가 아니다!(no matching Asterisk Call Manager/)\n");
			delete ami_socket;
			ami_socket = NULL;
			return stat_suspend;
		}
		if (!connected && !strncmp(rdata.s, "Asterisk Call Manager/", 22)) {

			sdata.req_len = sprintf(sdata.s,
				"Action: Login\n"
				"Username: %s\n"
				"Secret: %s\n"
				"ActionID: %d\n\n"
				, login.Username
				, login.Secret
				, ++manage.actionid
			);
			sdata.com_len = write(sd, sdata.s, sdata.req_len);
			if (sdata.com_len != sdata.req_len) {
				printf("연결된 AMI소켓에 쓰기 실패\n");
				delete ami_socket;
				ami_socket = NULL;
				return stat_suspend;
			}
			connected = 1;
			retry = 100;
			rdata.reset_data();
			continue;
		}

		// 연결되었는지 확인
		if (rdata.s[rdata.com_len - 2] == '\r' && rdata.s[rdata.com_len - 1] == '\n') {
			AMI_EVENTS events;
			strncpy(events.event, rdata.s, rdata.com_len);
			rdata.reset_data();

			parse_amievent(events);


			if (!events.rec_count) {
				printf("연결된 소켓은 Asterisk AMI 가 아니다!(no parse_amievent)\n");
				delete ami_socket;
				ami_socket = NULL;
				return stat_suspend;
			}

			if(!strncmp(events.key[0], "Response", 8)) {
				if (!strncmp(events.value[0], "Success", 7)) {
					connected = 2;
				}
				else {
					printf("AMI login failed:%s\n", get_amivalue(events,"Message"));
					delete ami_socket;
					ami_socket = NULL;
					return stat_suspend;
				}
			}
		}
	}

	server.addsocket(ami_socket);

	// epoll에 등록
	struct epoll_event ev;
	memset(&ev, 0x00, sizeof(ev));
	ev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLONESHOT;
	ev.data.fd = sd;
	epoll_ctl(server.m_epfd, EPOLL_CTL_ADD, ev.data.fd, &ev);

	// 혹시 벌써 들어온 문자가 있다면, 이벤트 타야한다... 이벤트 확인시키자.
	ioctl(sd, SIOCINQ, &ami_socket->recv->checked_len);
	TRACE("--- ami in data check(%d)\n", ami_socket->recv->checked_len);

	return stat_suspend;
}


PAMI_RESPONSE amiDeviceStatus(const char* device)
{
	PAMI_RESPONSE resp = new AMI_RESPONSE;



	return resp;
}


