/*
* http 수신에있어서 코어모듈이다. 수정에 신중함이 필요하다
* 소켓을 통하여 들어온 메시지를 HTTP 프로토콜에 따라 '\r\n\r\n' 까지 입력을 받아들이고 이를 http 프로토콜에 따라 파싱한다
* 파싱된 URI 정보가 g_route 에 등록된 라우트패스가 있는 경우 해당 라우팅 함수를 호출해 준다 (routes.cpp)
* 기본적으로 파일 및 디렉토리의 http 파일브라우징은 기본적으로 지원하지않는다. 업무특성상 필요하지 않기 때문이다
* HTTP 메소드 중 GET, POST 만 지원할 예정이다
*/
#include "amiaction.h"
#include "http.h"

map<const char*, void*> g_route;

char* urlDecodeNewString(const char* str) {
	char eStr[] = "00"; /* for a hex code */
	int len = strlen(str);
	char* dStr = (char*)malloc(len + 1);

	bzero(dStr, len + 1);

	int i, j; /* the counter for the string */

	for (i = 0, j = 0; i < len; ++i, ++j) {

		if (str[i] == '%') {
			if (str[i + 1] == 0 || str[i + 2] == 0) {
				break;
			}

			if (isxdigit(str[i + 1]) && isxdigit(str[i + 2])) {

				/* combine the next to numbers into one */
				eStr[0] = str[i + 1];
				eStr[1] = str[i + 2];

				/* convert it to decimal */
				long int x = strtol(eStr, NULL, 16);

				dStr[j] = x;
				i++;
				i++;
			}
		}
		else {
			dStr[j] = str[i];
		}
	}

	return dStr;
}

char* urlDecodeRewite(char* str)
{
	char eStr[] = "00"; /* for a hex code */
	int len = strlen(str);
	char* dStr = str;

	int i, j; /* the counter for the string */

	for (i = 0, j = 0; i < len; ++i, ++j) {

		if (str[i] == '%') {
			if (str[i + 1] == 0 || str[i + 2] == 0) {
				break;
			}

			if (isxdigit(str[i + 1]) && isxdigit(str[i + 2])) {

				/* combine the next to numbers into one */
				eStr[0] = str[i + 1];
				eStr[1] = str[i + 2];

				/* convert it to decimal */
				long int x = strtol(eStr, NULL, 16);

				dStr[j] = x;
				i++;
				i++;
			}
		}
		else {
			if (j < i)
				dStr[j] = str[i];
		}
	}
	dStr[j] = '\0';
	return dStr;
}

// 일단 기동되는 tcp 서버는 HTTP 프로토콜을 지원하도록 한다
// 외부에서 amiprocess에 연결하여 요청하는 것은 기본적으로 http GET 메소드를 지원한다

TST_STAT http(PTST_SOCKET psocket)
{
	// AMI 소켓은 psocket->func 에 ami_event 를 등록해 두었다
	// 모든 ami 패킷은 ami_event 가 읽고 처리가 등록된 이벤트는 process_events 를 호출한다
	// process_events 는 등록된 이벤트 별 처리 함수를 호출한다
	if (psocket->func) {
		TST_STAT next = psocket->func(psocket);
		if (next != tst_run)
			return next;
	}

	// 기본적으로 서버는 수신버퍼가 무조건 있어야한다. 필요없지만  혹시나 해서 검증
	if (!psocket->recv) {
		return tst_disconnect;
	}

	TST_DATA& rdata = *psocket->recv;	// 수신버퍼
	TST_DATA& sdata = *psocket->send;	// 발신버퍼
	int remain;
	// http parse
	if (psocket && psocket->events & EPOLLIN) {
		clock_gettime(CLOCK_REALTIME, &rdata.trans_time);
		if (rdata.checked_len) {
			remain = rdata.s_len - rdata.req_pos - rdata.com_len;
			TRACE("remain=%d, checked_len=%d\n", remain, rdata.checked_len);
			if (remain > (int)rdata.checked_len)
				remain = rdata.checked_len;

			while (remain-- > 0 
				&& (rdata.req_pos + rdata.com_len) < rdata.s_len
				&& (rdata.com_len < 4
					|| rdata.s[rdata.req_pos + rdata.com_len - 4] != '\r'
					|| rdata.s[rdata.req_pos + rdata.com_len - 3] != '\n'
					|| rdata.s[rdata.req_pos + rdata.com_len - 2] != '\r'
					|| rdata.s[rdata.req_pos + rdata.com_len - 1] != '\n'
					)
				)
			{
				// 한 바이트씩 읽어야 메시지 종료(\r\n\r\n)를 정확히 검증할 수 있다.
				// 다음 메시지는 담에 읽자. POST 라면 Contents-Length 가 들어오아 아니면 chunk.... 미지원
				rdata.com_len += read(psocket->sd, rdata.s + rdata.req_pos + rdata.com_len, 1);
			} 

			if ((rdata.req_pos + rdata.com_len) >= rdata.s_len) {
				// http header 가 넘 크다... 기본은 4K까지만 지원한다
				sdata.com_len = sprintf(sdata.s, "HTTP/1.0 413 Payload Too Large\r\n\n");
				write(psocket->sd, sdata.s, sdata.com_len);

				// tst_send: req_len을 설정하면 work_thread 가 보내준다
				// tst_send: req_len을 설정하지 않으면 메시지 다보낸 후에 EPOLLOUT 으로 사용자함수 호출되며 checked_len = 0 이다
				// http 프로토콜상 메시지를 보냈으면 연결을 종료한다
				return tst_disconnect;
			}

			if (rdata.com_len < 4
				|| rdata.s[rdata.req_pos + rdata.com_len - 4] != '\r'
				|| rdata.s[rdata.req_pos + rdata.com_len - 3] != '\n'
				|| rdata.s[rdata.req_pos + rdata.com_len - 2] != '\r'
				|| rdata.s[rdata.req_pos + rdata.com_len - 1] != '\n') {
				return tst_suspend;
			}
		}

		// 자 이제 한 메시지를 받았다 파싱해야한다
		rdata.s[rdata.req_pos + rdata.com_len] = '\0';
		TRACE("http protocol....\n%s", rdata.s + rdata.req_pos);

		char* url = NULL;
		if (!psocket->user_data) {
			// user_data 가 설정되지 않았다면 http header를 수신했다
			psocket->user_data = REQUEST_INFO::alloc();
			REQUEST_INFO& req = *(PREQUEST_INFO)psocket->user_data->s;
			url = rdata.s;
			req.body_string = rdata.s + rdata.com_len;
			// parse
			char* src;
			for (src = rdata.s; *url; url++) {
				if (!req.request_method) {
					if (*url == ' ') {
						*url = '\0';
						req.request_method = src;
						src = url + 1;
					}
					continue;
				}
				if (!req.request_uri) {
					if (*url == ' ') {
						*url = '\0';
						req.request_uri = src;
						src = url + 1;
					}
					continue;
				}
				if (!req.http_version) {
					if (memcmp(src, "HTTP/", 5) || (memcmp(src + 5, "1.0", 3) && memcmp(src + 5, "1.1", 3))) {
						sdata.com_len = sprintf(sdata.s, "HTTP/1.0 505 HTTP Version Not Supported\r\n\r\n");
						write(psocket->sd, sdata.s, sdata.com_len);
						clock_gettime(CLOCK_REALTIME, &sdata.trans_time);
						return tst_disconnect;
					}
					req.http_version = src + 5;
					url = src + 8;
					*url = '\0';
					while (*url != '\n') url++;
					src = ++url;
					break;
				}
			}

			// header parse
			while (*url) {
				while (*url && *++url != '\r');
				if (*url == '\r') {
					*url++ = '\0';
					*url++ = '\0';
					req.http_headers[req.num_headers].name = src;
					while (*src && *++src != ':');
					if (*src == ':') {
						*src++ = '\0';
						*src++ = '\0';
						req.http_headers[req.num_headers].value = src;
					}
					src = url;
					req.num_headers++;
				}
			}

			// split uri
			for (url = src = req.request_uri; *url; url++) {
				while (*url && *url != '?') url++;
				if (*url == '?') {
					*url = '\0';
					req.query_string = url + 1;
				}
			}

			// 이거 살리면 keep-alive용 전문 로깅 많다 헐...
			// conft("%s(), uri=%s, querystr=%s:", __func__, req.request_uri, req.query_string);

			urlDecodeRewite(req.request_uri);
			urlDecodeRewite((char*)req.query_string);

			int i;
			for (i = 0; i < req.num_headers; i++) {
				if (!strcmp(req.http_headers[i].name, "Content-Length")) {
					int length = atoi(req.http_headers[i].value);
					if (length) {
						if (length > (int)(rdata.s_len - rdata.com_len)) {
							sdata.com_len = sprintf(sdata.s, "HTTP/1.0 413 Payload Too Large\r\n\r\n");
							write(psocket->sd, sdata.s, sdata.com_len);
							return tst_disconnect;
						}
						rdata.req_pos = rdata.com_len;
						rdata.com_len = 0;
						rdata.req_len = length;
						TRACE("body get auto rdata.req_len=%d\n", rdata.req_len);
						return tst_suspend;
					}
				}
			}
		} else {
			// post body 수신했다.
			REQUEST_INFO& req = *(PREQUEST_INFO)psocket->user_data->s;
			url = req.body_string;
			// body 수신 후 처리..... user defined func call()???



		}

		REQUEST_INFO& req = *(PREQUEST_INFO)psocket->user_data->s;

		if (!strcmp(req.request_method, "GET") || !strcmp(req.request_method, "POST")) {
			map<const char*, void*>::iterator it;
			for (it = g_route.begin(); it != g_route.end(); it++) {
				// printf(":%s: http func address -> %lX\n", it->first, ADDRESS(it->second));
				if (!strcmp(req.request_uri, it->first)) {
					httpproc* func = (httpproc*)it->second;
					TST_STAT next = func(psocket);
					if (next != tst_disconnect) {
						// keep alive 소켓이 연결을 끊지 않아서 이걸 삭제해 주어야 다음 메시지를 파싱처리한다.
						free(psocket->user_data);
						psocket->user_data = NULL;
						rdata.reset_data();
					}
					return next;
				}
			}

			if (!strcmp(req.request_uri, "/favicon.ico")) {
				sdata.com_len = sprintf(sdata.s, "HTTP/1.1 404 Not found\r\n\r\n");	// 404는 헤더를 첨부하지 않는다. 이후 모든 것은 body로 처리된다
			} else {

				TRACE("%s(), uri=%s, querystr=%s:", __func__, req.request_uri, req.query_string);

				rdata.com_len = sprintf(rdata.s, "<HTML><HEAD></HEAD><BODY>잘 수신했읍니다.<br>그러나 아직 이에 대한 처리를 구현하지 않았음...</BODY></HTML>");
				sdata.com_len = sprintf(sdata.s,
					"HTTP/1.1 501 Not Implemented\r\n"
					"Content-Type: text/html;charset=utf-8\r\n"
					"Content-Length: %d\r\n"
					"Connection: close\r\n\r\n"
					"%s"
					, rdata.com_len
					, rdata.s
				);
			}
		} else {
			TRACE("%s(), uri=%s, querystr=%s:", __func__, req.request_uri, req.query_string);

			rdata.com_len = sprintf(rdata.s, "<HTML><HEAD></HEAD><BODY>잘 수신했읍니다.<br>그러나 아직 지원하지않는 메소드입니다.</BODY></HTML>");
			sdata.com_len = sprintf(sdata.s,
				"HTTP/1.1 501 Not Implemented\r\n"
				"Content-Type: text/html;charset=utf-8\r\n"
				"Content-Length: %d\r\n"
				"Connection: close\r\n\r\n"
				"%s"
				, rdata.com_len
				, rdata.s
			);
		}
		write(psocket->sd, sdata.s, sdata.com_len);
		clock_gettime(CLOCK_REALTIME, &sdata.trans_time);
		rdata.reset_data();
	}
	else if (psocket && psocket->events & EPOLLOUT) {
		clock_gettime(CLOCK_REALTIME, &sdata.trans_time);
		if (!sdata.checked_len) {
			printf("try disconnect http protocol....\n");
			return tst_disconnect;
		}
		else {
			// 
			// tst_send 리턴하면 무한룹 탈 수 있다.
			// return tst_send;
		}
	}

	// tst_send: req_len을 설정하면 work_thread 가 보내준다
	// tst_send: req_len을 설정하지 않으면 메시지 다보낸 후에 EPOLLOUT 으로 사용자함수 호출되며 checked_len = 0 이다
	// http 프로토콜상 메시지를 보냈으면 연결을 종료한다
	// 소켓을 클로즈해도 tcp 가 send buffer 에 있는 나머지 자료 다 보내고 닫아준다.
	return tst_disconnect;
}

