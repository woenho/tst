
#include "amiaction.h"
#include "processevents.h"


int g_exit = 0;

tstpool server;						// 기본적인 tcp epoll 관리쓰레드 풀 클래스 객체
PTST_SOCKET ami_socket = NULL;		// ami 연결용 TST_SOCKET 구조체로 new 로 직접 생성하여 server.addsocket(ami_socket)으로 추가등록하고 epoll 도 직접 등록한다.
map<const char*, void*> g_process;	// ami event 중 처리하고자 하는 이벹에 대한 함수포인터를 등록해 둔다

void* rm_ami_socket(tst::TST_USER_T* puser) {
	
	if (!puser || puser->s_len != (sizeof(TST_USER) + sizeof(AMI_MANAGE)))
		return NULL;

	PAMI_MANAGE ami_manage = (PAMI_MANAGE)puser->s;
	ami_manage->destroy();
	return NULL;
}

const char* get_amivalue(AMI_EVENTS& events, const char* key)
{
	int i;
	for (i = 0; i < events.rec_count; i++) {
		if (!strcmp(events.key[i], key))
			return events.value[i];
	}
	return "";
}

int parse_amievent(AMI_EVENTS& events) {
	// parsing
	char* p = events.event;
	char* start = events.event;
	bool isHeader = true;

	for (events.rec_count = 0; *p; p++) {
		if (isHeader) {
			if (*p == ':') {
				*p = '\0';
				events.key[events.rec_count] = start;
				isHeader = false;
				start = NULL;
			}
		} else {
			if (!start && *p != ' ')
				start = p;
			if (*p == '\r' && *(p+1) == '\n') {
				*p++ = '\0';
				*p++ = '\0';
				if (start) {
					events.value[events.rec_count++] = start;
				}
				isHeader = true;
				start = p;
			}
		}
	}
	return events.rec_count;
}

PAMI_RESPONSE AMI_MANAGE::ami_sync(char* action)
{
	PAMI_RESPONSE pResponse = new AMI_RESPONSE;

	if (!ami_socket || ami_socket->sd < 1) {
		pResponse->result = -99;
		strcpy(pResponse->msg, "AMI서버에 연결되지 않음");
		return pResponse;
	}

	TST_DATA& sdata = *ami_socket->send;
	ami_lock();
	resp_lock();

	sdata.req_len = sprintf(sdata.s,
		"%s"
		"ActionID: %d\n"
		"\n"
		, action
		, ++actionid
	);
	TRACE("ami action sync ---------\n%s", sdata.s);
	struct timespec waittime;
	clock_gettime(CLOCK_REALTIME, &waittime);	// NTP영향받아야 함
	waittime.tv_sec += 5;
	waittime.tv_nsec = 0;
	
	pResponse->mode = action_requst;
	pResp = pResponse;
	write(ami_socket->sd, sdata.s, sdata.req_len);
	int rc = pthread_cond_timedwait(&condResp, &mutexResp, &waittime);
	if (!rc) {
		// 정상처리
	}
	else {
		if (rc == ETIMEDOUT) {
			//printf("ami action setvar error... 서버가 응답을 하지 않음\n");
			pResponse->result = -8;
			strcpy(pResponse->msg, "ami action error... 서버가 응답을 하지 않음");
		} else {
			pResponse->result = errno;
			snprintf(pResponse->msg, sizeof(pResponse->msg)-1, "ami action error... %s",strerror(errno));
		}
	}

	resp_unlock();
	ami_unlock();

	return pResponse;
}

void AMI_MANAGE::ami_async(char* action) {
	if (!ami_socket || ami_socket->sd < 1)
		return;

	ami_lock();
	TST_DATA& sdata = *ami_socket->send;
	sdata.req_len = sprintf(sdata.s,
		"%s"
		"ActionID: %d\n"
		"\n"
		, action
		, ++actionid
	);
	TRACE("ami action async ---------\n%s", sdata.s);
	write(ami_socket->sd, sdata.s, sdata.req_len);
	ami_unlock();
}

// AMI 에 연결된 소켓에서 데이타가 들어오면 무조건 이 사용자함수(ami_event())가 호출된다
TST_STAT ami_event(PTST_SOCKET psocket) {

	if (!psocket || !psocket->recv) {
		return tst_disconnect;
	}

	// AMI_MANAGE::alloc() 에서 ami_base 를 설정한다
	if (!psocket->user_data || psocket->user_data->type != ami_base) { 
		
		// 이건 잘못 들어온거다 상위의 http 함수에서 처리해라
		return tst_run;
	}

	// 이하는 ami_message 만 이리 온다고 가정하고 함수 작성한다.
	TST_DATA& rdata = *psocket->recv;
	// TST_DATA& sdata = *psocket->send;
	TST_USER& udata = *(PTST_USER)psocket->user_data;
	AMI_MANAGE& manage = *(PAMI_MANAGE)udata.s;

	if (psocket->events & EPOLLIN) {
		uint remain = rdata.s_len - rdata.result_len -1; // -1은 마지막문자로 널스트링을 넣기 위하여
		if (remain > rdata.checked_len)
			remain = rdata.checked_len;

		clock_gettime(CLOCK_REALTIME, &rdata.trans_time);

		// login 외의 모든 메시지는 "\r\n" 이 두 개로 구분된다
		do {
			rdata.result_len += read(psocket->sd, rdata.s + rdata.result_len, 1);
		} while (--remain && (
			rdata.result_len < 4
			|| rdata.s[rdata.result_len - 4] != '\r'
			|| rdata.s[rdata.result_len - 3] != '\n'
			|| rdata.s[rdata.result_len - 2] != '\r'
			|| rdata.s[rdata.result_len - 1] != '\n'
			)
			);


		if (rdata.s[rdata.result_len - 4] != '\r'
			|| rdata.s[rdata.result_len - 3] != '\n'
			|| rdata.s[rdata.result_len - 2] != '\r'
			|| rdata.s[rdata.result_len - 1] != '\n') {
			return tst_suspend;
		}

		rdata.s[rdata.result_len] = '\0';
		// TRACE("%s", rdata.s);

		// event or response data processing
		if (manage.pResp && !strncmp(rdata.s, "Response:", 9)) {
			if (manage.resp_waitcount && manage.pResp->mode == action_requst) {
				manage.resp_lock();
				AMI_RESPONSE& resp = *manage.pResp;

				strncpy(resp.responses.event, rdata.s, rdata.result_len);
				parse_amievent(resp.responses);
				rdata.reset_data();


				uint32_t curr_id = atoi(get_amivalue(resp.responses, "ActionID")); // response의 action id

				if (curr_id < manage.actionid) {
					// 이전 액션의 결과면 패스... 다음 것을 기다려야 한다
					manage.resp_unlock();
					return tst_suspend;
				}
				else if (curr_id > manage.actionid) {
					// 이후 액션의 결과면 오류 처리
					resp.mode = action_response;
					resp.result = -2;
					sprintf(resp.msg, "action id is past...");
					bzero(&resp.responses, sizeof(resp.responses));
				}
				else {
					resp.mode = action_response;
					if (!strncmp(resp.responses.value[0], "Success", 7)) {
						// 응답 성공
						resp.result = 0;
					}
					else {
						// 응답실패 
						resp.result = -1;
					}
					// set "Message: Authentication accepted" to manage.msg
					strncpy(resp.msg, get_amivalue(resp.responses, "Message"), sizeof(resp.msg) - 1);
				}
				manage.pResp = NULL;
				// 싱크로 동작하는 ami sction 요청이 있다. 통보해 주자
				pthread_cond_signal(&manage.condResp);
				manage.resp_unlock();
			}
			return tst_suspend;
		}

		PATP_DATA atpdata = atp_alloc(sizeof(AMI_EVENTS));
		AMI_EVENTS& events = *(PAMI_EVENTS)&atpdata->s;
		strncpy(events.event, rdata.s, rdata.result_len);
		parse_amievent(events);
		rdata.reset_data();

#if 0
		int i, len = 0;
		char msg[2048];
		len += sprintf(msg + len, "--- %s(atp threadno:%d)...\n", __func__, events.nThreadNo);
		for (i = 0; i < events.rec_count; i++) {
			len += sprintf(msg + len, "%s%s: %s\n", i ? "    " : "", events.key[i], events.value[i]);
		}
		printf("%s\n", msg);
#endif


		// 이벤트 처리
		atpdata->func = process_events;
		atp_addQueue(atpdata);
		TRACE("REQUEST ami events(%s)...\n", events.value[0]);


	} else if (psocket->events & EPOLLOUT) {
		printf("---send remain(%d).....\n", psocket->send->checked_len);

	} else {
		if (psocket->events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
			// 연결이 끊어졌다.
			printf("---흐미 끊어졌다.....\n");
			ami_socket = NULL;
		}

	}

	return tst_suspend;
}

TST_STAT my_disconnected(PTST_SOCKET psocket) {
	if (!psocket)
		return tst_suspend;

	if (psocket == ami_socket) {
		if (psocket->user_data->type == ami_base) {
			TRACE("--- 흐미 ami 끊어졌다.....\n");
			ami_socket = NULL;
		}
	}
	else {
		TRACE("--- disconnected client socket....%s(%s:%d) \n", __func__, inet_ntoa(psocket->client.sin_addr), ntohs(psocket->client.sin_port));
	}

	return tst_suspend;
}

ATP_STAT atpfunc(PATP_DATA atpdata)
{
	ATP_STAT next = stat_suspend;

	if (atpdata->func) {
		next = atpdata->func(atpdata);
	}
	return next;
}
int main(int argc, char* argv[])
{
	g_exit = 0;

	atp_create(3, atpfunc);

	// first는 test용
	//server.setEventNewConnected(first);

	server.setEventDisonnected(my_disconnected);

	if (!server.create(5, "0.0.0.0", 1234, http, 4096, 4096)) {
		printf("---쓰레드풀이 기동 되지 못했다....\n");
		return 0;
	}

	// -------------------------------------------------------------------------------------

	// 처리할 이벤트를 등록 한다....

	g_process.clear();
	g_process["Hangup"] = (void*)event_hangup;
	g_process["DialBegin"] = (void*)event_dialbegin;
	g_process["DialEnd"] = (void*)event_dialend;
	// g_process["VarSet"] = (void*)event_varset;

	map<const char*, void*>::iterator it;
	for (it = g_process.begin(); it != g_process.end(); it++) {
		printf(":%s: event func address -> %lX\n", it->first, ADDRESS(it->second));
	}

	// -------------------------------------------------------------------------------------

#if 0
	int i = 300;
	while (--i && server.m_main_run) {
#else
	while (server.m_main_run) {
#endif
		if (!ami_socket) {
			// 연결을 시도하는 작업 요청
			PATP_DATA atpdata = atp_alloc(sizeof(AMI_LOGIN));
			AMI_LOGIN& login = *(PAMI_LOGIN)&atpdata->s;
			strncpy(login.Host, "127.0.0.1", sizeof(login.Host));
			strncpy(login.Username, "call", sizeof(login.Username));
			strncpy(login.Secret, "call", sizeof(login.Secret));
			atpdata->func = amiLogin;
			atp_addQueue(atpdata);
			TRACE("REQUEST ami login...\n");
		}
		sleep(3);
	}



	printf("---쓰레드풀을 종료합니다.....\n");

	server.destroy();

	atp_destroy(gracefully);

	return 0;
}
