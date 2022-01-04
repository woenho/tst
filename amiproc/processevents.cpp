#include "amiaction.h"

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

ATP_STAT event_dialend(AMI_EVENTS& events)
{
	ATP_STAT next = stat_suspend;

	int i, len = 0;
	char msg[2048];
	len += sprintf(msg + len, "--- %s(atp threadno:%d)...\n", __func__, events.nThreadNo);
	for (i = 0; i < events.rec_count; i++) {
		len += sprintf(msg + len, "%s%s: %s\n", i ? "    " : "", events.key[i], events.value[i]);
	}
	printf("%s\n", msg);


	if (!strcmp(get_amivalue(events,"DialStatus"),"ANSWER")) {

		AMI_MANAGE& manage = *(PAMI_MANAGE)ami_socket->user_data->s;
		char* buf = (char*)malloc(1024);
		snprintf(buf, 1023,
			"Action: Setvar\n"
			"Channel: %s\n"
			"Variable: %s\n"
			"Value: %s\n"
			, get_amivalue(events, "Channel")
			, "woenho_test"
			, "Have a good day!"
			);
		PAMI_RESPONSE resp = manage.ami_sync(buf);
		free(buf);
		if (resp->result) {
			// error
			printf("ami action setvar error... result:%d:%s:\n", resp->result, resp->msg);
		} else {
			printf("ami action setvar success... result:%d:%s:\n", resp->result, resp->msg);
		}

		delete(resp);
	}

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
	printf("%s\n", msg);

	return next;
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
			void* trans = it->second;
			eventproc* func = (eventproc*)trans;
			next = func(events);
			break;
		}
	}

	return next;
}
