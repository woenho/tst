#include "amiaction.h"
#include "http.h"

ATP_STAT event_hangup(AMI_EVENTS& events)
{
	ATP_STAT next = stat_suspend;

	int i, len = 0;
	char msg[2048];
	len += sprintf(msg + len, "--- %s(atp threadno:%d)...\n", __func__, events.nThreadNo);
	for (i = 0; i < events.rec_count; i++) {
		len += sprintf(msg + len, "%s%s: %s\n", i ? "    " : "", events.key[i], events.value[i]);
	}
	printf("%s\n", msg);

	return next;
}


ATP_STAT event_dialbegin(AMI_EVENTS& events)
{
	ATP_STAT next = stat_suspend;

	int i, len = 0;
	char msg[2048];
	len += sprintf(msg + len, "--- %s(atp threadno:%d)...\n", __func__, events.nThreadNo);
	for (i = 0; i < events.rec_count; i++) {
		len += sprintf(msg + len, "%s%s: %s\n", i ? "    " : "", events.key[i], events.value[i]);
	}
	printf("%s\n", msg);


	return next;
}

ATP_STAT event_varset(AMI_EVENTS& events)
{
	ATP_STAT next = stat_suspend;

	int i, len = 0;
	char msg[2048];
	len += sprintf(msg + len, "--- %s(atp threadno:%d)...\n", __func__, events.nThreadNo);
	for (i = 0; i < events.rec_count; i++) {
		len += sprintf(msg + len, "%s%s: %s\n", i ? "    " : "", events.key[i], events.value[i]);
	}
	TRACE("%s\n", msg);

	return next;
}

ATP_STAT event_userevent(AMI_EVENTS& events)
{
	ATP_STAT next = stat_suspend;

	int i, len = 0;
	char msg[2048];
	len += sprintf(msg + len, "--- %s(atp threadno:%d)...\n", __func__, events.nThreadNo);
	for (i = 0; i < events.rec_count; i++) {
		len += sprintf(msg + len, "%s%s: %s\n", i ? "    " : "", events.key[i], events.value[i]);
	}
	TRACE("%s\n", msg);

	const char* szEventName = NULL;
	const char* szChannel = NULL;
	const char* szUniqueid = NULL;
	const char* szExten = NULL;
	const char* szCallerIDNum = NULL;



	szEventName = get_amivalue(events, "UserEvent");
	szChannel = get_amivalue(events, "Channel");
	szUniqueid = get_amivalue(events, "Uniqueid");
	if (!strcmp(szEventName, "Hangup"))
		szExten = get_amivalue(events, "Callee"); // hangup push function 타면 exten 이 's' 로 변환한다, 그래서 body에 Callee 추가했다
	else
		szExten = get_amivalue(events, "Exten");
	szCallerIDNum = get_amivalue(events, "CallerIDNum");

	char* p;
	for (p = (char*)szUniqueid; *p; p++)
		if (*p == '.')
			*p = '0';

	try {

		if (!strcmp(szEventName, "DTMF_START")) {



		}
		else {
			throw util_exception(999, "Invalid UserEvent type. event name:%s, channel=%s", szEventName, szChannel);
		}

	}
	catch (util_exception& e) {
		if (e.code() != 999) {
			TRACE("m_code=%d, %s", e.code(), e.what());
		}
	}
	catch (...) {
		TRACE("errno=%d, %s", errno, strerror(errno));
	}

	return next;
}

ATP_STAT event_dialend(AMI_EVENTS& events)
{
	/*
		Event: DialEnd
		Privilege: call,all
		Channel: PJSIP/800020005-00000006
		ChannelState: 6
		ChannelStateDesc: Up
		CallerIDNum: 800020005
		CallerIDName: 유선20005
		ConnectedLineNum: 800020003
		ConnectedLineName: 더미유선02
		Language: en
		AccountCode:
		Context: DEVELOPMENT
		Exten: 800020003
		Priority: 6
		Uniqueid: 1636598043.6
		Linkedid: 1636598043.6
		DestChannel: PJSIP/800020003-00000007
		DestChannelState: 6
		DestChannelStateDesc: Up
		DestCallerIDNum: 800020003
		DestCallerIDName: 더미유선02
		DestConnectedLineNum: 800020005
		DestConnectedLineName: 유선20005
		DestLanguage: en
		DestAccountCode:
		DestContext: DEVELOPMENT
		DestExten:
		DestPriority: 1
		DestUniqueid: 1636598043.7
		DestLinkedid: 1636598043.6
		DialStatus: ANSWER
	*/

	const char* szExten = NULL;
	const char* szChannel = NULL;
#if !defined(USE_USEREVENT_CALLSTARTED)
	const char* szUniqueid = NULL;
#endif
	const char* szDestChannel = NULL;
	const char* szDestUniqueid = NULL;
	const char* szCallerIDNum = NULL;
	const char* szDialStatus = NULL;
	char* p;

	// char buf[10240] = { 0 };
	// int len, rc=0;


	try {

		szDialStatus = get_amivalue(events, "DialStatus");
		szCallerIDNum = get_amivalue(events, "CallerIDNum");

#if defined(USE_USEREVENT_CALLSTARTED)
		if (get_callinfo(szCallerIDNum, &ci)) {
			TRACE("memcached get not found, caller=%s\n", szCallerIDNum);
			throw util_exception(999, "memcached get not found");
		}
#endif
		int i, len = 0;
		char msg[2048];
		len += sprintf(msg + len, "--- %s(atp threadno:%d)...\n", __func__, events.nThreadNo);
		for (i = 0; i < events.rec_count; i++) {
			len += sprintf(msg + len, "%s%s: %s\n", i ? "    " : "", events.key[i], events.value[i]);
		}
		TRACE("%s", msg);


		if (!strcmp(szDialStatus, "ANSWER")) {
			// memcached에 다이알로그 등록
			szExten = get_amivalue(events, "Exten");
			szChannel = get_amivalue(events, "Channel");
			// 채널 및 exten  이 일치하는가?

			szDestChannel = get_amivalue(events, "DestChannel");
			szDestUniqueid = get_amivalue(events, "DestUniqueid");





		}
	}
	catch (exception& e) {
		TRACE("errno=%d, %s", errno, e.what());
	}
	catch (...) {
		TRACE("errno=%d, %s", errno, strerror(errno));
	}

	return stat_suspend;
}


ATP_STAT process_events(PATP_DATA atp_data)
{
	ATP_STAT next = stat_suspend;

	AMI_EVENTS& events = *(PAMI_EVENTS)&atp_data->s;
	events.nThreadNo = atp_data->threadNo;

#if 0
	int i, len = 0;
	char msg[2048];
	len += sprintf(msg + len, "--- %s(atp threadno:%d)...\n", __func__, events.nThreadNo);
	for (i = 0; i < events.rec_count; i++) {
		len += sprintf(msg + len, "%s%s: %s\n", i ? "    " : "", events.key[i], events.value[i]);
	}
	printf("%s\n", msg);
#endif

	// 등록된 이벤트를 처리한다.

	map<const char*, void*>::iterator it;
	for (it = g_process.begin(); it != g_process.end(); it++) {
		if (!strcmp(it->first, events.value[0])) {
			eventproc* func = (eventproc*)it->second;
			next = func(events);
			break;
		}
	}

	return next;
}
