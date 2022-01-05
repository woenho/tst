#include "amiaction.h"
#include "http.h"

TST_STAT http_dtmf(PTST_SOCKET psocket)
{
	REQUEST_INFO& req = *(PREQUEST_INFO)psocket->user_data->s;
	// TST_DATA& rdata = *psocket->recv;	// 수신버퍼
	TST_DATA& sdata = *psocket->send;	// 발신버퍼

	sdata.com_len = sprintf(sdata.s,
		"HTTP/1.1 200 Ok\r\n"
		"Content-Type: text/html;charset=utf-8\r\n"
		"Content-Length: %u\r\n"
		"Connection: close\r\n\r\n"
		"%s"
		, (uint)strlen(req.request_method)
		, req.request_method
	);

	write(psocket->sd, sdata.s, sdata.com_len);
	clock_gettime(CLOCK_REALTIME, &sdata.trans_time);

	return tst_disconnect;
}

TST_STAT http_transfer(PTST_SOCKET psocket)
{
	REQUEST_INFO& req = *(PREQUEST_INFO)psocket->user_data->s;
	// TST_DATA& rdata = *psocket->recv;	// 수신버퍼
	TST_DATA& sdata = *psocket->send;	// 발신버퍼

	sdata.com_len = sprintf(sdata.s,
		"HTTP/1.1 200 Ok\r\n"
		"Content-Type: text/html;charset=utf-8\r\n"
		"Content-Length: %u\r\n"
		"Connection: close\r\n\r\n"
		"%s"
		, (uint)strlen(req.request_method)
		, req.request_method
	);

	write(psocket->sd, sdata.s, sdata.com_len);
	clock_gettime(CLOCK_REALTIME, &sdata.trans_time);

	return tst_disconnect;
}
