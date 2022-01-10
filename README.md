```
/*
* programming. woenho@daum.net
* update. 2021-12-24 (X-Mas eve?)
* for. anybody
* using. free
* pay. comment to me
*/
```

tstpool class:

epoll을 사용하여 소켓을 관리한다.<br />
소켓은 non-blocking 모드로 동작한다.<br />
서버 포트 리슨에 성공하면 알아서 클라인언트 정보를 관리한다.<br />
클라이언트로부터 메시지가 올라오면 create 함수인자로 설정한 사용자함수(func)를 호출한다.<br />
listen 하지 않는 소켓을 메인소켓으로 지정할 수 있다.<br />
이때는 사용자가 직접 TST_SOCKET을 생성하고 m_connect에 추가해 주어야 한다<br />
<br />
현재 작업중인 소켓 말고 다른 클라이언트에 작업을 하고 싶으면 해당 소켓디스크립터를 찾아서 need_send() 함수를 호출할 것을 권장한다.<br />
need_send() 함수는 EPOLLOUT 를 epoll에 설정한다. 그러면 바로 사용자처리함수(THREADINFO::tst_func)를 워크 쓰레드가 호출해 준다.<br />
EPOLLOUT 은 tcp버퍼에 지금 데이타가 있더라도 추가로 더 넣을 수 있으면 바로 이벤트가 발생한다.<br />
즉 EPOLLOUT는 tcp버퍼가 꽉차 있지 않으면 무조건 발생한다.<br />
단순히 작은 메시지 하나만을 보내고 싶은 경우라면 소켓구조체를 획득한 후 send 구조체에 보낼 데이타를 설정하고 need_send()를 호출하면 워크쓰레드가 알아서 보내준다.<br />
send 구조체에 보낼 데이타를 설정하려면 반드시 pthread_mutex_lock(&m_mutexConnect);로 락을 걸고 소켓 구조체 검색을 시작해야 한다.<br />
send 구조체에 보낼 데이타를 설정 중에 해당 소켓이 닫히면 소켓구조체(send 구조체를 포함하고 있다)의 메모리를 삭제 처리된다.<br />
즉, 메모리 세그먼트 오류를 유발할 수 있다.<br />
그러므로 이 경우는 반드시 pthread_mutex_lock(&m_mutexConnect); 함수로 락을 걸고 사용하고 사용 후에는 락을 풀어주도록 한다.<br />
단순히 need_send()만을 호출하는 것은 소켓이 삭제 되었더라도 epoll_ctl(epfd, EPOLL_CTL_MOD...) 함수만 오류로 종료될 뿐이므로 시스템 안정성에 영향이 없다.<br />
<br />
현재 작업중인 소켓의 클라이언트에 메시지를 보내고 싶다면 두 가지 방법이 있다.<br />
직접 write() 를 호출하는 방법과 socket->send->req_len 에 적절한 값을 설정하는것이다. 굳이 need_send()를 호출할 필요가 없다.<br />
ex) socket->send->req_len=10, socket->send->req_pos=30, socket->send->com_len=0; 의 의미: socket->send->s 로 부터 30번째 자리부터 10바이트 송수신 하라는 뜻 완료되면 com_len에 10을 넣어준다<br />
EPOLLOUT 시에 위의 값을 설정하고 tst_send 를 리턴하면 보내주고, EPOLLIN 시에 tst_suspend 를 리턴하면 받아준다<br />
socket->send->req_len 에 적절한 값을 설정하고 tst_send를 리턴하면 워크쓰레드가 알아서 보내준다<br />
직접 write()를 한 경우 socket->send->req_len=0 하고 tst_suspend를 리턴한다.<br />
tst_send를 리턴하면 tcp send버퍼가 꽉차지 않는 한 바로 EPOLLOUT 이벤트가 발생하므로 무한루프 탈 수 있다.<br />
tcp는 tcp send버퍼의 메시지를 천천히 모두 전송해 준다.<br />
현재 작업 중인 소켓은 EPOLLONESHOT 플래그에 의해 작업중에는 이벤트가 발생하지 않으므로 pthread_mutex_lock(&m_mutexConnect) 락을 걸 필요 없다.<br />
<br />
데이타 수신 시에 하나의 메시지가 온전히 수신되지 않는 경우가 많다.<br />
하나의 메시지 크기가 크다면 여러개의 패킷으로 수신할 수 밖에 없다.<br />
이 때는 온전한 메시지가 도착할때 까지 수신을 반복해야 한다.<br />
그러므로 사용자처리함수(THREADINFO::tst_func)의 메시지 수신처리는 온전한 메시지가 수신될 때 까지 이어받기할 수 있도록 작성되어야 한다.<br />
아니면, req_len에 값을 설정해라.<br />
그 길이 까지는 쓰레드가 알아서 수신해 준다.<br />
req_len 까지 수신이 완료된 후 사용자 함수를 호출한다.<br />
업무상 결정된 패킷헤더를 사용자함수에서 처리한 후 헤더에서 수신할 메시지 크기를 req_len 에 설정하면 바디부분은 쓰레드가 알아서 수신하는 상황이 예상된다.<br />
수발신 메시지 버퍼의 크기에 변경이 필요할 수 있다.<br />
그러면 사용자처리함수(THREADINFO::tst_func)는 호출시 인자로 넘어가는 (tst::TST_SOCKET)의 버퍼크기를 재지정해라.(realloc)<br />
<br />
tstpool 클래스는 서버포트를 오픈하고 그 소켓유형(TST_DATA::type)을 서버(sock_listen)로 설정한다.<br />
이 소켓도 epoll 에 같이 넣어서 관리한다<br />
소켓유형(TST_DATA::type)이 서버(sock_listen)인 경우 tst::accept() 함수를 통해 자동처리된다.<br />
기본적으로 사용자처리함수(THREADINFO::tst_func)를 호출하지 않는다.<br />
즉, 사용자처리함수(THREADINFO::tst_func)는 소켓유형(TST_DATA::type)이 실제통신채널(client/sub)인 경우에 대하여만 처리한다.<br />
새로운 연결이 생길 때 마다 TST_SOCKET 구조체 크기 + tst::create()에서 지정한 max_data_size 만큼의 메모리를 확보한다.<br />
TST_SOCKET 구조체의 메모리 확보는 새로운 연결이 생길 때 마다 이루어지고 tst::m_connect 에 소켓번호를 키로 관리한다.<br />
만일 새로운 연결이 되었을 때 하고 싶은 간단한 설정이나 메시지 정도는 setEventNewConnected() 로 콜백함수를 지정할 수 있다.<br />
그러나 setEventNewConnected()설정된 콜백함수에서는 직접 소켓에 write 하는 것을 피하는 것이 좋다.<br />
이는 워크쓰레드가 아닌 메인 쓰레드에서 바로 호출 되는 것이기에 그렇다.<br />
메인쓰레드는 epoll을 관리하는 일로도 바쁘다...<br />
메시지를 보내고 싶다면 인자로 넘어온 socket 구조체의 send 에 적절한 값을 설정하고 tst_send 를 리턴하면 알아서 워크쓰레드가 메시지를 보내준다.<br />
메시지를 보내지 않더라도 뭔가 새로 연결된 소켓에 작업을 하고 싶으면 그냥 tst_send 를 리턴하면 <br />
바로 워크 쓰레드가 events에 EPOLLOUT을 설정하고 사용자처리함수(THREADINFO::tst_func)를 호출해 주도록 구성되이 있다.<br />
events 값{EPOLLIN,EPOLLOUT,EPOLLPRI,EPOLLRDHUD,EPOLLERR,EPOLLET,EPOLLONESHOT}은 TST_SOCKET::events 에 넣은 후 사용자처리함수를 호출한다.<br />
사용자처리함수(THREADINFO::tst_func)는 호출되면 인자로 넘어 온 socket->events 값에 따른 처리를 한다.<br />
그러나 이 중 EPOLLRDHUD,EPOLLERR 값에 대한 처리는 tstpool 클래스가 알아서 닫아주고 메모리 해제한다.<br />
현재 작업 중인 소켓이 아닌 특정 클라인언트와 연결을 끊고 싶으면 제공되는 tst:closesocket(sd)를 사용한다.<br />
메모리 해제 등을  자동제어한다.<br />
그러나 지금 호출된 사용자함수의 인자로 들어온 소켓을 종료하고 싶을 때는 직접 close 하지 말고, 함수의 리턴값으로 tst_disconnect를 설정하길 권장한다.<br />
<br />
EPOLLONESHOT: 레벨트리거를 이용하면서 이 플래그를 사용하지 않으면 이미 사용중인 소켓에 이벤트가 발생하여 소켓을 삭제하는 일이 발생할 수 있다.<br />
이는 심각한 오류가 발생될 수 있으므로 경험상 레벨트리거 이용 시에 반드시 사용하는 것을 권한다.(특정 조건이 유지되면 계속 감지)<br />
<br />
EPOLLIN, EPOLLPRI: 데이타 수신이 되었다... 수신처리를 한다.<br />
<br />
EPOLLOUT: 메시지를 보낼 수 거나 보냈다. 상황에 맞게 작업하면 된다.<br />
<br />
EPOLLET: 엣지트리거인 경우.. (비추천) 조건이 변할 때만 감지 한다. (프로그래밍상 고려사항이 많고 복잡해진다)<br />
<br />
각 소켓마다 확보된 객체(TST_DATA)의 req_len 가 설정되어 있으면 그 크기까지는 쓰레드가 알아서 송수신을 한다.<br />
req_len은 TST_DATA::s_len 보다 크지 않도록 주의하여야 한다. TST_data::s_len은 송수신 버퍼 크기이다.<br />
req_len==0 이거나 req_len==result_len 가 되면 사용자함수(THREADINFO::tst_func)를 호출한다.<br />
수신이나 발신에 있어 한 번에는 한 개의 메시지 크기를 넘어서는 송수신을 자동으로 하지 않기 위한 것이다<br />
<br />
주의:<br />
epoll의 적용은 시스템에 설치된 epoll 버전에 따라 조금씩 다르게 동작한다. 주의하라.<br />
<br />




