```
/*
* programming. woenho@daum.net
* on. 2021-12-20
* for. anybody
* using. free
* pay. comment to me
*/

```

주요기능:

1. 지정된 수 만큼의 워크 쓰레드를 생성하도록 하는 API 제공 (실행할 외부함수를 인자로 받아야함)
2. 명시적으로 쓰레드풀을 종료 할 수 있는 api를 제공 (큐 처리 후 종료 | 큐 버리고 종료)
3. 프로세스의 쓰레드 요청 내역을 큐에 넣는 api를 제공
4. 쓰레드 풀을 관리하는 관리 쓰레드가 하나 있고 지정된 수 만큼 워크 쓰레드를 확보한다
5. 프로세스 종료 시 관리 쓰레드는 자동으로 워크 쓰레드의 실행 상태를 확인하고 쓰레드를 종료 시킨다
6. 관리쓰레드는 작업요청 프로세스와 비동기적으로 동작한다.
7. 각 워크쓰레드는 작업완료 시 스스로 잠들어야 한다
8. 메인쓰레드는 작업요청 큐를 감시하다가 새로운 작업요청이 있을 시 잠든 워크쓰레드 중 하나를 깨워 업무를 맡긴다
9. 잠든 쓰레드가 없을 시 잠든 쓰레드가 있는지 주기적으로 폴링하여 작업을 맡길 수 있어야 한다
10. 작업의뢰 큐의 크기와 현재 동작하는 쓰레드 수에 대한 정보를 제공한다
11. 워크쓰레드는 사용자 함수 호출시 인자로 범용적 구조의 데이타 구조체를 넘겨준다
12. 각 워크쓰레드에는 고유의 쓰레드 번호를 부여한다
13. 워크쓰레드는 뮤텍스를 통해 관리 쓰레드와 동기화 한다
14. pthread_cond_wait() 를 사용하지 않고 pthread_cond_timedwait() 를 사용하여 누가 깨워주지 않아도 스스로 일정 시간이 지나면 할 일이 있는 지 스스로 확인한다.
15. 각 쓰레드별로 작업수행 시간의 합을 저장한다 (통계)
16. 타스크 처리 시간 중 가장 오래걸린 시간을 별도 저장한다(통계)
17. 현재 수행되고 있는 잡의 시작 시각을 기록한다, 마지막에 수행된 잡의 종료시각도 기록해 둔다

이해:
사용자 프로그램과 온전히 독립적으로 동작할 수 있는 완전체 모습의 pthread pool 관리라이브러리를 추구했다
ExampleThreadPool.cpp 라는 예제 프로그램을 이용하여 라이브러리의 작동모습을 볼 수 있다.
콘솔에 표시되는 로깅 정보를 추적하면 라이브러리의 작동 방식을 이해하기 쉬울 것이다.
예제프로그램 "./example [{ 1 | 0} [{ 1 | 0 }]]" 을 실행할 때 인자를 바꾸어주며 작동 되는 모습을 확인해 볼 수 있다.
./example: 쓰레드 즉시 종료(./example 0 0 과 같다)
./example 1: 의뢰된 모든 작업 완료 후 종료(./example 1 0 과 같다)
./example 1 1: 의뢰된 모든 작업 완료 후 종료, 종료할 때 종료 사용자함수 처리하고 종료
./example 0 1: 쓰레즈 즉시 종료, 종료할 때 종료 사용자함수 처리하고 종료

처리순서:
-> atp_create()로 쓰레드 풀 생성 
	{	--반복--
		-> atp_alloc()로 사용자함수에 넘겨줄 데이타 생성
		-> atp_addQueue() 로 작업 의뢰
	}
-> atp_destroy() 로 쓰레드 종료

함수 설명:

1. atp_create(): 쓰레드 풀을 생성한다
2. atp_destroy(): 쓰레드 풀을 종료한다, endwaittime의 default 값은 5e+6 (5초) 이다
3. atp_addQueue(): 쓰레드 풀에 작업을 의뢰한다
4. atp_setwaittime(): 쓰레드가 스스로 깨어나는 시간을 설정한다 (default 3 sec)
5. atp_setfunc(): 쓰레드 종료시 처리하고 호출하고 싶은 사용자함수, 쓰레드가 Idle time 일 때 호출하고 싶은 사용자함수를 설정한다
6. atp_worklock(): 사용자 함수간에 락이 필요한 경우 호출
7. atp_workunlock(): atp_worklock()를 호출 한 경우 반드시 락을 해제 한다
8. atp_getWorkLockCount():	작업쓰레드간에 동기화를 위한 락 대기열 숫자 조회
9. atp_getThreadCount(): atp_create() 호출시 지정한 워크 쓰레드 숫자를 가져온다
10. atp_getThreadInfo(): 워크쓰레드목록을 가져온다
11. atp_getRealtimeQueueCount(): atp_addQueue()로 요청한 작업 목록 중 아직 워크쓰레드에 작업의뢰를 하지 못한 realtime 우선순위의 작업목록 수를 조회한다.
12. atp_getNormalQueueCount(): atp_addQueue()로 요청한 작업 목록 중 아직 워크쓰레드에 작업의뢰를 하지 못한 normal 우선순위의 작업목록 수를 조회한다.
13. atp_alloc(): atp_addQueue() 함수로 작업을 의뢰할때 사용되는 인자는 반드시 이 함수를 사용하여 메모리를 생성한 후 사용하도록 한다. 메모리 해제는 라이브러리가 자동으로 수행한다.

enum 변수 설명:

1. ATP_END: atp_destroy() 호출시 종료 모드
2. ATP_STAT: 워크 쓰레드의 현재 작동 상태를 표시한다

구조체 설명:

1. ATP_DATA: 워크쓰레드가 사용자 함수를 호출할 때 사용되는 구조체로 사용자함수에서 사용되 데이타는 이 구조체에 넣어준다
2. THREADINFO: 워크쓰레드의 정보를 관리하는 구조체로 쓰레드 수 만큼 메모리를 생성하여 배열로 관리한다

컴파일 방법:
gnu make 를 통하여 컴파일 한다
make clean: 생성된 오브젝트를 삭제한다
make clean all: make debug 와 같이 동작함
make clean release: 디버그정보 없이 코드를 최적화한다
make clean ddugtrace: debug 정보 생성 및 쓰레드의 동작상황을 콘솔에 표시한다
make clean debug: debug 정보 생성

기타:
release 모드 컴파일시는 libAsyncThreadPool.a 가 생성되고
이외의 모드 컴파일시는 libAsyncThreadPool_d.a 가 생성되며 디버그 정보를 포함한다
편집 및 디버깅은 visual studio 2019 을 사용했다.
VS환경을 원격지 리눅스개발 모드로 설정하여 컴파일 및 시험은 원격지리눅스를 이용하였다.
pthread 라이브러리를 이용한 간단한 소스지만 사용하기 쉽고 편리한 멀티타스킹 환경을 제공한다.
관리 쓰레드의 CPU사용량을 최적화 한다
관리쓰레드는 워크쓰레드에 일을 맡길 때 워크쓰레드가 작업을 시작하는데 걸리는 평균시간 만큼 기다렸다가 다음 작업을 준다.

워크쓰레드락(atp_worklock()) 사용 시 락에 걸린 시간도 타스크의 elasped time으로 산정되므로 확인 필요함.

종료모드가  gracefully 라면 종료요구 시점과 타스크가 일을 시작한 시점은 다르다
즉, 종료 요구를 하고 밀린 작업들을 모두 수행해야 하므로 한참 뒤에 잡을 수행하는 타스크가 시작될 수 있다.
타스크의 종료대기 시각은 타스크의 시작 시각을 기준으로 한다.
그러므로 종료모드가  gracefully 라면 종료 대기시간은 종료요청 시각으로부터 많이 길어질 수 있다.
exit 함수 실행요구를 했는데 타스크가 대시시간을 넘어서도 종료되지 않았다면 exit함수는 호출되지 않고 강제종료 시킨다
종료모드가  gracefully 이고 exit 함수 실행요구를 하지 않으면 요청된 모든 잡의 마지막 셋트가 종료대기의 검증대상이 된다.
종료모드가  force 이고 exit 함수 실행요구를 하지 않으면 현재 처리중인 타스크의 처리시간이 검증대상이 된다.