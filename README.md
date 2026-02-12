# ZeroMQ Python - C++ Porting Project
ZeroMQ의 주요 통신 패턴을 이해하고, Python 코드로 작성된 예제를 C++ 환경으로 1:1 대응 포팅하여 소켓 동작 방식과 메시지 흐름을 분석하는 프로젝트.  
각 패턴의 연결 설정, 데이터 교환, 연결 해제 흐름을 중심으로 구현되었습니다.

## Implemented Patterns
1. REQ-REP (Request-Reply)

+ Client -> Request 전송
+ Server -> Reply 응답
+ 한 요청에는 반드시 한 응답이 대응된다.
+ 반드시 send -> recv -> send -> recv 순서를 유지해야 한다.
  
2. PUB-SUB (Publish-Subscribe)

+ Publisher -> 다수 Subscriber
+ 응답을 기다리지 않아 순서의 개념이 없는 비동기적 통신 구조
+ 받고자 하는 메시지를 필터링할 수 있다.

3. PUSH-PULL (Pipeline)

+ 작업 분산용 패턴
+ Round-robin 방식으로 메시지를 분배하여 병렬 작업 처리 구조에 적합하다.

4. DEALER-ROUTER

+ 비동기 Request-Reply 확장 패턴
+ 싱글 스레드/멀티 스레드 구조 구현
   

