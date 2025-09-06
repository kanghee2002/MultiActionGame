![Animation_MAG_Sync1](https://github.com/user-attachments/assets/d0516588-c9f1-4e6f-bf67-4a965927b89d)
![Animation_MAG_Sync1](https://github.com/user-attachments/assets/e5d418d1-b6fd-4c48-b906-846ba40ba004)


## 멀티 대전 액션 게임
- Unreal 5, C++, Blueprint, Window
- 팀 프로젝트 (4인)
- 11월 공모전을 목표로 현재 개발 진행 중

## 기술
- **Listen server** 방식과 **RPC, Replication**을 활용한 **클라이언트 동기화**
- 공격, 점프 등 기본 동작과 피격에 따른 체력 UI 업데이트 동기화, 캐릭터 선택에 따른 **개별 캐릭터 생성**
- 핵심 게임 로직을 서버에서 검증 및 처리, 치팅 방지
- **캐릭터 기본 클래스는 C++로**, **개별 캐릭터는 Blueprint로** 상속받아 작성
 - 개별 캐릭터의 RPC 내부 함수를 **Blueprint Implementable Event**로 구현
