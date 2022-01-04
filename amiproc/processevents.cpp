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

#if 1
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
		PAMI_RESPONSE pResp = manage.ami_sync(buf);
		free(buf);
		if (pResp->result) {
			// error
			printf("ami action setvar error... result:%d:%s:\n", pResp->result, pResp->msg);
		} else {
			printf("ami action setvar success... result:%d:%s:\n", pResp->result, pResp->msg);
			len = 0;
			for (i = 0; i < pResp->responses.rec_count; i++) {
				len += sprintf(msg + len, "%s%s: %s\n", i ? "    " : "", pResp->responses.key[i], pResp->responses.value[i]);
			}
			printf("%s\n", msg);
		}

		delete(pResp);
		
		// ------------------------------------------------------------------

		buf = (char*)malloc(1024);
		snprintf(buf, 1023,
			"Action: Getvar\n"
			"Channel: \n"
			"Variable: DEVICE_STATE(PJSIP/%s)\n"
			, "800020003"
		);
		pResp = manage.ami_sync(buf);
		free(buf);
		if (pResp->result) {
			// error
			printf("ami action getvar error... result:%d:%s:\n", pResp->result, pResp->msg);
		}
		else {
			printf("ami action getvar success... result:%d:%s:\n", pResp->result, pResp->msg);
			len = 0;
			for (i = 0; i < pResp->responses.rec_count; i++) {
				len += sprintf(msg + len, "%s%s: %s\n", i ? "    " : "", pResp->responses.key[i], pResp->responses.value[i]);
			}
			printf("%s\n", msg);
		}

		delete(pResp);

		buf = (char*)malloc(1024);
		snprintf(buf, 1023,
			"Action: Getvar\n"
			"Channel: \n"
			"Variable: DEVICE_STATE(PJSIP/%s)\n"
			, "800020004"
		);
		pResp = manage.ami_sync(buf);
		free(buf);
		if (pResp->result) {
			// error
			printf("ami action getvar error... result:%d:%s:\n", pResp->result, pResp->msg);
		}
		else {
			printf("ami action getvar success... result:%d:%s:\n", pResp->result, pResp->msg);
			len = 0;
			for (i = 0; i < pResp->responses.rec_count; i++) {
				len += sprintf(msg + len, "%s%s: %s\n", i ? "    " : "", pResp->responses.key[i], pResp->responses.value[i]);
			}
			printf("%s\n", msg);
		}

		delete(pResp);


		buf = (char*)malloc(1024);
		snprintf(buf, 1023,
			"Action: Getvar\n"
			"Channel: \n"
			"Variable: DEVICE_STATE(PJSIP/%s)\n"
			, "800020005"
		);
		pResp = manage.ami_sync(buf);
		free(buf);
		if (pResp->result) {
			// error
			printf("ami action getvar error... result:%d:%s:\n", pResp->result, pResp->msg);
		}
		else {
			printf("ami action getvar success... result:%d:%s:\n", pResp->result, pResp->msg);
			len = 0;
			for (i = 0; i < pResp->responses.rec_count; i++) {
				len += sprintf(msg + len, "%s%s: %s\n", i ? "    " : "", pResp->responses.key[i], pResp->responses.value[i]);
			}
			printf("%s\n", msg);
		}

		delete(pResp);

	}
#endif

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
