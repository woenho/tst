
#ifndef _HTTP_H_
#define _HTTP_H_

#include "tst.h"
#include "AsyncThreadPool.h"

using namespace std;
using namespace tst;

typedef struct http_header {
	const char* name;  /* HTTP header name */
	const char* value; /* HTTP header value */
}HTTP_HEADER;

#define HTTP_MAX_HEADERS	30

typedef struct http_request_info {
	const char* request_method;		/* "GET", "POST", etc */
	char* request_uri;				/* URL-decoded URI (absolute or relative,* as in the request) */
	const char* query_string;		/* URL part after '?', not including '?', or NULL */
	const char* http_version;		/* E.g. "1.0", "1.1" */
	char* body_string;				/* post body or NULL */
	uint32_t content_length;		/* Length (in bytes) of the request body, can be -1 if no length was given. */
	int num_headers;				/* Number of HTTP headers */
	HTTP_HEADER http_headers[HTTP_MAX_HEADERS]; /* Allocate maximum headers */

	static PTST_USER alloc() {
		uint32_t s_len = sizeof(TST_USER) + sizeof(struct http_request_info);
		PTST_USER puser = (PTST_USER)calloc(s_len, 1);
		puser->s_len = s_len;
		return puser;
	}
}REQUEST_INFO, * PREQUEST_INFO;

typedef TST_STAT httpproc(PTST_SOCKET psocket);

extern map<const char*, void*> g_route;

TST_STAT http_dtmf(PTST_SOCKET psocket);
TST_STAT http_transfer(PTST_SOCKET psocket);

#endif
