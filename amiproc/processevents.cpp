/*
* amiproc.cpp 에 있는 ami_event() 함수가 입력되는 이벤트메시지를 분리하여 AMI_EVENTS 구조체에 넣는다
* 이 파일은 g_process 에 등록된 이벤트 처리 함수들을 모아 놓는다
* 즉 실제 이벤트를 처리하는 함수들은 여기에 모아놓는다
*/
#include "amiaction.h"
#include "http.h"
#include "processevents.h"


ATP_STAT event_hangup(AMI_EVENTS& events)
{
	ATP_STAT next = stat_suspend;

	logging_events(events);


	return next;
}


ATP_STAT event_dialbegin(AMI_EVENTS& events)
{
	ATP_STAT next = stat_suspend;

	logging_events(events);


	return next;
}

ATP_STAT event_varset(AMI_EVENTS& events)
{
	ATP_STAT next = stat_suspend;

	logging_events(events);

	return next;
}

ATP_STAT event_userevent(AMI_EVENTS& events)
{
	ATP_STAT next = stat_suspend;

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


