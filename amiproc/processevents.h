
#ifndef _PROCESS_EVENTS_H_
#define _PROCESS_EVENTS_H_

#include "amiaction.h"

ATP_STAT process_events(PATP_DATA atp_data);

// 이벤트를 처리할 함수들 목록
ATP_STAT event_hangup(AMI_EVENTS& events);
ATP_STAT event_dialbegin(AMI_EVENTS& events);
ATP_STAT event_dialend(AMI_EVENTS& events);


#endif
