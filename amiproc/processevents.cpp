#include "amiaction.h"

ATP_STAT event_hangup(AMI_EVENTS& events)
{
	ATP_STAT next = stat_suspend;

	printf("--- %s()전화가 끊어짐...\n", __func__);
	int i;
	for (i = 0; i < events.rec_count; i++) {
		printf("%s%s: %s\n", i ? "    " : "", events.key[i], events.value[i]);
	}
	printf("\n");

	return next;
}


ATP_STAT event_dialbegin(AMI_EVENTS& events)
{
	ATP_STAT next = stat_suspend;

	printf("--- %s()...\n", __func__);
	int i;
	for (i = 0; i < events.rec_count; i++) {
		printf("%s%s: %s\n", i ? "    " : "", events.key[i], events.value[i]);
	}
	printf("\n");

	return next;
}

ATP_STAT event_dialend(AMI_EVENTS& events)
{
	ATP_STAT next = stat_suspend;

	printf("--- %s()...\n", __func__);
	int i;
	for (i = 0; i < events.rec_count; i++) {
		printf("%s%s: %s\n", i ? "    " : "", events.key[i], events.value[i]);
	}
	printf("\n");

	return next;
}


ATP_STAT process_events(PATP_DATA atp_data)
{
	ATP_STAT next = stat_suspend;

	AMI_EVENTS& events = *(PAMI_EVENTS)&atp_data->s;

#if 0
	// printf("--- %s()\n", __func__);
	int i;
	for (i = 0; i < events.rec_count; i++) {
		printf("%s%s: %s\n", i ? "    " : "", events.key[i], events.value[i]);
	}
	printf("\n");
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
