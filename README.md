![Animation_MAG_Sync1](https://github.com/user-attachments/assets/d0516588-c9f1-4e6f-bf67-4a965927b89d)
![Animation_MAG_Sync1](https://github.com/user-attachments/assets/e5d418d1-b6fd-4c48-b906-846ba40ba004)


## 멀티 대전 액션 게임
- Unreal 5, C++, Blueprint, Window
- 팀 프로젝트 (4인)
- 11월 공모전을 목표로 현재 개발 진행 중

## 주요 기능
- **Listen server** 방식과 **RPC, Replication**을 활용한 **클라이언트 동기화**
- 공격, 점프 등 기본 동작과 피격에 따른 체력 UI 업데이트 동기화, 캐릭터 선택에 따른 **개별 캐릭터 생성**
- 핵심 게임 로직을 서버에서 검증 및 처리, 치팅 방지
- **캐릭터 기본 클래스는 C++로**, **개별 캐릭터는 Blueprint로** 상속받아 작성
 - 개별 캐릭터의 RPC 내부 함수를 **Blueprint Implementable Event**로 구현
- Paragon 에셋 커스터마이징

## 의미
Unreal Engine 5를 활용하여 **Listen Server** 방식으로 **RPC**와 **Replication**을 적용해 보면서 **클라이언트–서버 구조**의 기본 원리를 이해할 수 있었습니다. 특히, 클라이언트 간의 동기화가 어떤 방식으로 이루어지는지를 직접 구현하며 멀티플레이 게임의 근간이 되는 네트워크 구조에 대해 배웠습니다.

멀티플레이 구현에서 서버의 역할과 데이터 전달 과정의 중요성을 깨달았고, 이 프로젝트 이후에는 한 단계 나아가 데디케이티드 서버 방식을 학습하며 보다 확장성 있는 멀티플레이 구조를 심도 있게 공부해보고자 합니다.
