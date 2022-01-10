이 예제는 Asterisk AMI 처리를 하는 기본 적인 구성이다<br />
이 예제에서 사용되는 AsyncThreadPool 은 <a href=https://github.com/woenho/pthread_pool.git>https://github.com/woenho/pthread_pool.git</a> 에서 받을 수 있다.<br />
tstpool 클래스를 이용하여 tcp 처리를 하고 http 로 들어욘 연결이 ami 에 필요한 요정을 하도록 구성할 수 있는 기본이다<br />
또한, 스스로 AMI event 에 따른 작업을 수행할 수도 있다.<br />
실제 작업은 AsyncThreadPool 을 이용하여 멀티타스킹 작업을 수행한다.<br />
여기서 고려사항은 AMI 이벤트가 순차적으로 시작하지만 종료는 순차적으로 되지 않는 다는 것이다.<br />
지금 예제는 하는일 없이 로그만 찍지만 실제 어떤 작업을 걸어 본다면 각기 다른 종요시간울 가진다<br />
즉, 상황에 따라서는 뒤에 처리해야할 hangup이 DialEnd 이벤트 처리가 종료되지 않는 시점에 발생할 수 있다는 것이다.<br />
dialEnd 이벤트가 외부 서버에 접근하여 무언가 하는데 Hangup 이벤트가 해당 채널정보를 지워버린다면 문제가 될 수 있다는 것이다.<br />
신중히 고려해야 한다. 이 경우라면 채널정보를 지우지 말고 채널이 hangup 상태라는 플래그를 활용하는 방법도 있다.<br />
<br />

