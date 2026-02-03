<img width="1280" height="720" alt="4Souls_썸네일" src="https://github.com/user-attachments/assets/78d1d418-3ea7-45f1-a9b7-ee2042d32531" />

## 멀티 대전 액션 게임
- PC, 개인 프로젝트
- Unreal Engine 5, C++, Blueprint
- Listen Server, Behavior Tree, Network Emulation, Blackboard, AIController

## 주요 기능
### 캐릭터 회전 반응성 개선

- 이유
    - 고지연 네트워크 환경에서 캐릭터 회전 지연 발생  →  조작감 저하
- 방법
    - 클라이언트 입력 시 **서버의 응답을 기다리지 않고 즉시 회전 시작 + 서버로 방향 전송**
    - **서버 응답 수신 시 자연스럽게 보정**
- 효과
    - **회전 반응 시간 34% 단축 (1.7s → 1.1s)**
    - 입력 시 즉각 반응을 통해 조작 반응성 개선
- 단점
    - 서버에서 입력과 다른 방향 지정 시 부자연스러운 회전 발생
    - 본 게임 특성 상 회전 중 강제 회전 변경 요소 없음
    - 실제 플레이 및 시연 시 약간의 지터링만 발생

<table>
  <tr>
    <td align="center"><b>Bad</b></td>
    <td align="center"><b>Good</b></td>
  </tr>
  <tr>
    <td><img src="https://github.com/user-attachments/assets/d2ba3b7f-9ebd-45fc-9798-edda245ecee6" width="100%"></td>
    <td><img src="https://github.com/user-attachments/assets/79c77fd9-3f7c-440a-b2a4-3dead44cf00d" width="100%"></td>
  </tr>
</table>

### Server / Client 권한에 종속된 캐릭터 구조 개선
- 이유
    - 서버는 보스, 클라이언트는 영웅을 플레이하도록 초기 설계
    - **캐릭터 타입 (보스/영웅)과 실행 주체 (Server/Client)가 결합**
- 방법
    - 캐릭터 타입과 실행 권한을 분리
    - Client: 서버에게 요청
    - Server: 검증 후 실행 및 동기화
    - → 캐릭터 로직을 권한 기반 실행 구조로 리팩토링
- 효과
    - Server / Client에 관계없이 동일한 플레이 결과
    - 테스트, 시연, 확장에 유연한 구조로 개선
    - 실제 PvE 도입 시 캐릭터 로직 수정 없이 확장
    
### Behavior Tree 기반 AI Boss
- 이유
    - 시연 중 다수가 PvP 적응에 어려움을 겪는 문제 인식
    - PvP 적응 위한 PvE 개발 필요성
- 방법
    - **AI Controller**
        - **다음 타겟 / 패턴 선택 로직** (가중치/누적합 기반)
        - 선택 결과를 Blackboard 값으로 반영
    - **Blackboard**
        - **다음 타겟 / 패턴 저장**
        - Behavior Tree의 조건 분기 기준 역할
    - **Behavior Tree**
        - Blackboard 값 변경 시 트리 재판단
        - **현재 패턴에 맞는 Task 실행**
    - **Boss Character**
        - 각 패턴 종료 시점 감지
        - AI Controller에 **다음 타겟 / 패턴 선택 요청**
- 효과
    - Behavior Tree, Blackboard 기반 상태 **→** **AI 상태 추적을 통한 디버깅 및 테스트**
    - 신규 패턴 추가 시 BT Task 추가 + 패턴 선정 로직 확장

### 로직 / 데이터 분리 기반 캐릭터 구조

- 이유
    - 캐릭터 공통 로직의 안정성 필요
    - 캐릭터 추가에 대비한 확장성
- 방법
    - **BaseCharacter (C++)**
        - 캐릭터 상위 클래스
        - 전투, 이동, 입력 처리 등 **핵심 로직 구현**
    - **DerivedCharacter (Blueprint)**
        - 개별 캐릭터 클래스
        - 스킬, 애니메이션, 사운드 등 **콘텐츠 구현**
    - St**atData (Data Asset)**
        - 체력, 공격력, 쿨타임 등 **수치 데이터**
- 효과
    - 신규 캐릭터 추가 시 Data Asset, Blueprint만으로 확장
    - **Data Asset 기반 수치 관리**
        - **코드 수정 없이 데이터 수정만으로 밸런스 조정**

### 기타
- **Behavior Tree, BlackBoard, AIController**를 이용한 AI 보스
- **캐릭터 기본 클래스는 C++로**, **개별 캐릭터는 Blueprint로** 상속받아 구현
- 개별 캐릭터의 RPC 내부 함수를 **Blueprint Implementable Event**로 구현

## 의미
### 멀티플레이 네트워크 구조 이해 및 적용
Unreal Engine 5 기반 Listen Server 환경에서 RPC와 Replication을 직접 적용하며, 클라이언트–서버 구조와 클라이언트 간 상태 동기화 흐름을 구현했습니다. 이를 통해 서버 권한 기반 구조에서 데이터가 어떤 경로로 전파되고, 각 클라이언트에 반영되는지에 대해 이해할 수 있었습니다.

### 서버/클라이언트 권한 분리 리팩토링 및 엔진 기능 활용
서버/클라이언트 권한에 종속된 캐릭터 구조를 리팩토링하여, 실행 주체에 따라 책임이 명확히 분리되도록 개선했습니다. 또한 Network Emulation과 Behavior Tree를 활용해 네트워크 환경별 동작을 검증하고, 언리얼 엔진의 네트워크 디버깅 및 AI 시스템을 실제 프로젝트에 적용하는 경험을 쌓았습니다.
