<img width="1280" height="720" alt="4Souls_썸네일" src="https://github.com/user-attachments/assets/78d1d418-3ea7-45f1-a9b7-ee2042d32531" />

## 멀티 대전 액션 게임
- PC, 개인 프로젝트
- Unreal Engine 5, C++, Blueprint
- Behavior Tree, Network Emulation, Blackboard, AIController

## 주요 기능
### RPC, Replication을 활용한 네트워크 동기화 (Listen server)
- **캐릭터 회전 반응성 개선**
- 클라이언트 입력 시 바로 회전 시작 + 서버로 방향 전송
- 서버의 회전 값 도착 시 보정
- 반응 시간 약 **34% 단축** (1732ms → 1139ms)

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


### 캐릭터 선택 및 개별 캐릭터 생성
<img width="390" height="436" alt="image" src="https://github.com/user-attachments/assets/ea65c2a4-f94a-4739-b101-5ae085f8660e" />

### 기타
- **Behavior Tree, BlackBoard, AIController**를 이용한 AI 보스
- **캐릭터 기본 클래스는 C++로**, **개별 캐릭터는 Blueprint로** 상속받아 구현
- 개별 캐릭터의 RPC 내부 함수를 **Blueprint Implementable Event**로 구현

## 의미
Unreal Engine 5를 활용하여 **Listen Server** 방식으로 **RPC**와 **Replication**을 적용해 보면서 **클라이언트–서버 구조**의 기본 원리를 이해할 수 있었습니다. 특히, 클라이언트 간의 동기화가 어떤 방식으로 이루어지는지를 직접 구현하며 멀티플레이 게임의 근간이 되는 네트워크 구조에 대해 배웠습니다. 서버/클라이언트 권한에 종속된 캐릭터 구조를 리팩토링해보기도 하고, **Network Emulation**과 **Behavior Tree**를 사용해보기도 하며 언리얼 엔진이 제공하는 기능들을 적절히 활용하는 법을 익히기도 했습니다.

멀티플레이 구현에서 서버의 역할과 데이터 전달 과정의 중요성을 깨달았고, 이 프로젝트 이후에는 한 단계 나아가 데디케이티드 서버 방식을 학습하며 보다 확장성 있는 멀티플레이 구조를 심도 있게 공부해보고자 합니다.
