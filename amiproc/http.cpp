#include "amiaction.h"

// 일단 기동되는 tcp 서버는 HTTP 프로토콜을 지원하도록 한다
// 외부에서 amiprocess에 연결하여 요청하는 것은 기본적으로 http GET 메소드를 지원한다

TST_STAT http(PTST_SOCKET psocket)
{
	// AMI 소켓은 psocket->func 에 ami_event 를 등록해 두었다
	if (psocket->func) {
		TST_STAT next = psocket->func(psocket);
		if (next != tst_run)
			return next;
	}

	// 기본적으로 서버은 수신버퍼가 무조건 있어야한다. 필요없지만  혹시나 해서 검증
	if (!psocket->recv) {
		return tst_disconnect;
	}

	TST_DATA& rdata = *psocket->recv;	// 수신버퍼
	TST_DATA& sdata = *psocket->send;	// 발신버퍼

	// http parse
	if (psocket && psocket->events & EPOLLIN) {
		if (!rdata.checked_len)
			return tst_suspend;

		uint remain = rdata.s_len - rdata.result_len;
		if (remain > rdata.checked_len)
			remain = rdata.checked_len;

		do {
			// 한 바이트씩 읽어야 메시지 종료(\r\n\r\n)를 정확히 검증할 수 있다.
			// 다음 메시지는 담에 읽자. POST 라면 Contents-Length 가 들어오아 아니면 chunk.... 미지원
			rdata.result_len += read(psocket->sd, rdata.s + rdata.result_len, 1);
		} while (--remain
			&& (rdata.result_len < 4
				|| rdata.s[rdata.result_len - 4] != '\r'
				|| rdata.s[rdata.result_len - 3] != '\n'
				|| rdata.s[rdata.result_len - 2] != '\r'
				|| rdata.s[rdata.result_len - 1] != '\n'
				)
			);

		if (rdata.result_len < 4
			|| rdata.s[rdata.result_len - 4] != '\r'
			|| rdata.s[rdata.result_len - 3] != '\n'
			|| rdata.s[rdata.result_len - 2] != '\r'
			|| rdata.s[rdata.result_len - 1] != '\n') {
			return tst_suspend;
		}


		if (rdata.result_len >= rdata.s_len) {
			// 413 Payload Too Large
			// http header 가 넘 크다... 기본은 4K까지만 지원한다
			sdata.result_len = sprintf(sdata.s,
				"HTTP / 1.0 413 Payload Too Large\r\n"
				"Content - Type: text / html\r\n"
				"Connection : close\r\n\r\n");
			write(psocket->sd, sdata.s, sdata.result_len);

			// tst_send: req_len을 설정하면 work_thread 가 보내준다
			// tst_send: req_len을 설정하지 않으면 메시지 다보낸 후에 EPOLLOUT 으로 사용자함수 호출되며 checked_len = 0 이다
			// http 프로토콜상 메시지를 보냈으면 연결을 종료한다
			return tst_disconnect;
		}

		// 자 이제 한 메시지를 받았다 파싱해야한다

		rdata.s[rdata.result_len] = '\0';
		TRACE("http protocol....\n%s", rdata.s);




		if (!memcmp(rdata.s, "GET ", 4)) {
			rdata.reset_data();
#if 0
			psocket->send->result_len = sprintf(psocket->send->s,
				"HTTP/1.1 500 Internal Server Error 아직 http parser 구현안했음....\r\n"
				"Content-Type: text/html\r\n"
				"Connection: close\r\n\r\n");
#else
			if (!memcmp(rdata.s + 4, "/favicon.ico ", 13)) {
				psocket->send->result_len = sprintf(psocket->send->s,
					"HTTP/1.1 404 Not found\r\n"
					"Content-Type: text/html\r\n"
					"Connection: close\r\n\r\n"
				);
			}
			else {
				rdata.result_len = sprintf(rdata.s, "<HTML><HEAD></HEAD><BODY>아직 파서를 구현하지 않았음...</BODY></HTML>");

				psocket->send->result_len = sprintf(psocket->send->s,
					"HTTP/1.1 200 Ok\r\n"
					"Content-Type: text/html;charset=utf-8\r\n"
					"Content-Length: %d\r\n"
					"Connection: close\r\n\r\n"
					"%s"
					, rdata.result_len
					, rdata.s
				);
				rdata.reset_data();
			}

#endif
			write(psocket->sd, psocket->send->s, psocket->send->result_len);
			clock_gettime(CLOCK_REALTIME, &psocket->send->trans_time);
		}

	}
	else if (psocket && psocket->events & EPOLLOUT) {
		// TST_DATA& data = *psocket->send;
		if (!sdata.checked_len) {
			printf("try disconnect http protocol....\n");
			return tst_disconnect;
		}
		else {
			// tst_send 리턴하면 무한룹 탈 수 있다.
			//return tst_send;
		}
	}

	// tst_send: req_len을 설정하면 work_thread 가 보내준다
	// tst_send: req_len을 설정하지 않으면 메시지 다보낸 후에 EPOLLOUT 으로 사용자함수 호출되며 checked_len = 0 이다
	// http 프로토콜상 메시지를 보냈으면 연결을 종료한다
	// 소켓을 클로즈해도 tcp 가 send buffer 에 있는 나머지 자료 다 보내고 닫아준다.
	return tst_disconnect;
}



