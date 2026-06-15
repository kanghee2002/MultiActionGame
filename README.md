<img width="1280" height="720" alt="4Souls_썸네일" src="https://github.com/user-attachments/assets/78d1d418-3ea7-45f1-a9b7-ee2042d32531" />

## 4 Souls — 멀티 대전 액션 게임
- PC, 개인 프로젝트 (2025.07 ~ 2026.01)
- Unreal Engine 5, C++, Blueprint
- **루키상** (2025 **UNICON**)
- Listen · Dedicated Server, RPC / Replication, Network Emulation, Behavior Tree, Blackboard, AIController
- **플레이 영상**: https://youtu.be/B_vU_-hq0HA

> 보스 1명 vs 영웅 3명의 비대칭 PvP·PvE 액션 게임

## 주요 기능
### Listen Server → Dedicated Server 마이그레이션

- 이유
    - 초기 Listen Server 가정으로 작성된 코드를 Dedicated Server로 이전
    - Listen Server의 구조적 한계
        - **호스트 종료 시 매치 종료**
        - 서버–클라 간 **latency 비대칭으로 인한 형평성 문제**
    - **매치 설정값**이 호스트의 `GameInstance`를 통해 전달되는 구조 → Dedicated 서버는 메뉴를 거치지 않아 항상 기본값으로 동작
    - **Server-only 환경**에서 Listen Server 가정으로 인한 null 참조 버그 (`GetPlayerController(0)`)
- 방법
    - **매치 설정**: `AGameModeBase::InitGame`에서 URL 옵션을 파싱
        - `InitGame`은 GameMode lifecycle에서 **가장 일찍 수행**되므로 설정 주입 지점으로 선택
        - 매치 전역 설정(예: `?BossHealth=300`)과 플레이어별 설정 경로를 분리
    - **Server-only**: 호스트 컨트롤러에 의존하던 경로를 식별하고 가드 코드 적용
- 효과
    - **cmdline의 URL을 통한 매치 설정 주입** 가능
    - Listen 방식도 `ServerTravel` + URL을 사용해 **두 모드가 같은 진입점** 공유
    - Listen Server에 의존적인 코드를 추적/리팩토링
    - **Dedicated / Listen 두 모드 동시 지원**

#### Before (Listen)
호스트 메뉴 → TextBox 입력 → GameInstance → Boss BP(BeginPlay) → GameInstance getter → Set Health

#### After (Dedicated)
서버 cmdline ?BossHealth=300 → InitGame 파싱 → SpawnDefaultPawnFor → Set Health

> ※ `GetPlayerController(0)`: Listen Server는 호스트가 local PC를 보유해 valid, Dedicated Server는 첫 클라 join 전 PC가 0개라 null

### 캐릭터 회전 반응성 개선

- 이유
    - 고지연 네트워크 환경에서 캐릭터 회전 지연 발생 → 조작감 저하
    - **서버 권한 유지**와 **즉각적인 클라이언트 반응**을 동시에 만족시킬 필요
- 방법
    - 클라이언트 입력 시 **서버 응답을 기다리지 않고 즉시 회전 시작 + Server RPC로 방향 전송**
    - 서버는 입력을 처리해 방향 변수를 **Replication**
    - 클라이언트는 Replicated 변수 도착 시 **자연스럽게 보간**
    - 지연이 실제로 제거되었는지 **Network Emulation 측정으로 검증**
- 효과
    - **입력 → 시각 피드백 지연 607ms → 0ms 제거**
    - 180도 방향 전환 시간 **1732ms → 1125ms (607ms, 약 35% 단축)**
    - 서버 권한 유지 + 즉각적인 입력 반응성
- 단점
    - 서버에서 입력과 다른 방향 지정 시 부자연스러운 회전 발생 가능
    - 본 게임 특성상 회전 중 강제 회전 변경 요소가 없어 수용
    - 실제 플레이 및 시연 시 약간의 지터링만 발생
- 측정
    - 환경: Network Emulation (Min 100ms / Max 200ms, Packet Loss 5%, In/Out Traffic)
    - 지표: Start = 방향 입력 시점 / End = 목표 방향과 오차 1° 이내
    - 표본: 동일 조건 30회 반복 평균
    - Before 1732ms / After 1125ms — **이론 지연(150ms 평균 × 2(out+in) × 2(왕복) = 600ms)과 실측 개선폭(607ms)이 일치**

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

### Behavior Tree 기반 AI Boss
- 이유
    - **PvP 구현 시 만든 액션 로직을 PvE에서 그대로 재사용**하고자 함
    - UE5의 BT 인프라 (Blackboard, Decorator Abort) 활용 및 신규 패턴 확장 고려
- 방법 (**Decision / Execution 분리**)
    - **Decision**
        - **AI Controller**: 가중치/누적합 기반으로 다음 타겟·패턴 선택 (캐릭터 타입·체력·거리 기반 가중치)
        - **Blackboard**: 다음 타겟·패턴 저장 → BT 조건 분기 기준
    - **Execution**
        - **Behavior Tree**: Blackboard 변경 시 Re-evaluation(Abort), 현재 패턴에 맞는 Task 실행
        - **Boss Character**: 패턴 종료 감지 후 AI Controller에 다음 타겟·패턴 선정 요청 → 다시 Decision으로
    - **로직 재사용**: AI가 플레이어와 **동일한 액션 함수**를 재사용
        - 단일 Task → 액션 함수 `PerformAction(enum)` 호출 (플레이어의 키 바인딩과 동일 경로)
        - 신규 액션 추가 시 **BT 수정 없이 enum + switch 확장**만으로 대응
- 효과
    - PvP 동작을 수정하지 않고 **PvE 추가**
    - **BT 구조 단순화** (액션을 추가해도 Task는 1개로 유지)
    - Blackboard 변수 시각화로 **AI 의사결정 디버깅 및 테스트** 용이

### 기타
- **4종 캐릭터(보스/Knight/Archer/Wizard)** 구현 — 약·강공격, 스킬, 구르기, 힐 등 모든 액션을 **Client → Server RPC → Multicast** 한 경로로 통일
- **BaseCharacter**(C++)에 전투/이동/입력 등 핵심 로직, **개별 캐릭터**(Blueprint) 로 상속받아 스킬·애니메이션·사운드 구현
- 개별 캐릭터의 RPC 내부 함수를 **Blueprint Implementable Event**로 노출
- **StatData (Data Asset)** 기반 수치 관리 — 코드 수정 없이 밸런스 조정

### 서버/클라이언트 권한 분리 리팩토링 및 엔진 기능 활용
권한에 종속된 캐릭터 구조를 리팩토링하여 실행 주체에 따라 책임이 명확히 분리되도록 개선했습니다. 또한 Network Emulation으로 고지연·패킷 손실 환경을 재현해 회전 반응성 개선을 정량적으로 검증했고, Behavior Tree·Blackboard·AIController를 활용해 언리얼 엔진의 AI 시스템을 실제 프로젝트에 적용하는 경험을 쌓았습니다.
