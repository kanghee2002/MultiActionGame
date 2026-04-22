# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 프로젝트 개요

Unreal Engine 5.6 C++ 프로젝트 — 히어로(Knight / Archer / Wizard) 3종과 Boss 간 전투를 다루는 리슨 서버 기반 멀티플레이 액션 게임. PvE 모드에서는 AI가 Boss를 조종한다. 프로젝트명과 기본 게임 모듈은 모두 `MultiActionGame`. 보조 작업 디렉터리 `c:\Program Files\Epic Games\UE_5.6`은 엔진 설치 위치이며 읽기 전용으로만 사용(수정하지 말고 참조 용도로만).

## 빌드 / 실행

UE 5.6이 `c:\Program Files\Epic Games\UE_5.6`에 설치되어 있다고 가정한다. 이 저장소에는 자체 빌드 스크립트가 없으므로 엔진의 BatchFiles를 직접 호출한다.

- **Visual Studio 프로젝트 파일 재생성** (C++ 클래스 추가/이름 변경, `.Build.cs`/`.Target.cs` 수정 후 필요):
  `"c:/Program Files/Epic Games/UE_5.6/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe" -projectfiles -project="c:/Projects/MultiActionGame_portable/MultiActionGame.uproject" -game -rocket -progress`
- **에디터 타깃 빌드** (일반적인 개발 빌드):
  `"c:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Build.bat" MultiActionGameEditor Win64 Development -Project="c:/Projects/MultiActionGame_portable/MultiActionGame.uproject" -WaitMutex`
- **스탠드얼론 게임 타깃 빌드**: 위 커맨드에서 `MultiActionGameEditor` → `MultiActionGame`으로 바꾼다.
- **Clean**: 같은 BatchFiles 경로의 `Clean.bat`에 동일한 인자.
- **에디터 실행**:
  `"c:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor.exe" "c:/Projects/MultiActionGame_portable/MultiActionGame.uproject"`

자동화된 테스트는 없다 — `*.Tests` 모듈도 없고 Gauntlet 테스트도 없다. 동작 확인은 에디터의 PIE로 하거나, `MainMenu`의 Host/Join 흐름으로 리슨 서버 여러 개를 띄워서 수동 검증한다.

## 동기화 / 작업 흐름

이 프로젝트는 **원격 저장소로 push/pull을 하지 않는다.** 커밋은 전부 로컬에 남기고, 다른 컴퓨터로 작업을 넘길 때는 `.patch` 파일 **하나**를 구글 드라이브 등으로 공유해 반영한다. 한 번에 한 대에서만 작업하므로 최신 상태와 컨플릭트 없음이 보장된다는 전제로 운영한다.

**표준 절차 — `git format-patch --binary --stdout`으로 단일 파일 동기화**

바이너리(`.uasset` / `.umap` 등)를 base85 델타로 `.patch` 안에 직접 임베드하므로 에셋을 별도로 공유할 필요가 없고, 커밋 메타데이터까지 그대로 복원되어 양쪽 컴퓨터의 히스토리가 일치한다.

- **A 컴퓨터 (보내는 쪽)**: 공유할 마지막 N개 커밋을 하나의 파일로 묶는다.
  ```bash
  git format-patch --binary -<N> HEAD --stdout > sync_<YYYYMMDD>.patch
  ```
  범위로 지정하려면 `-<N>` 대신 `<from>..<to>` (예: `abc123..HEAD`) 사용.
- **B 컴퓨터 (받는 쪽)**: 드라이브에서 받은 `.patch`를 한 줄로 적용한다.
  ```bash
  git am sync_<YYYYMMDD>.patch
  ```
  적용 중 충돌이 나면 `git am --abort`로 되돌린 뒤 원인 확인 (컨플릭트가 나면 전제가 깨진 것이므로 무리해서 해결하지 말고 상황부터 점검).

**지켜야 할 규칙**

- `git push` / `git pull` / `git fetch` 등 원격 상호작용 명령은 사용자가 명시적으로 요청하지 않는 한 **절대 실행하지 않는다.**
- 커밋 단위는 자기 완결적이어야 한다 — 관련 `.uasset` / `.umap` 변경과 C++ 변경을 같은 커밋에 묶어야 반대편에서 재현이 깔끔하다.
- 대량 바이너리 변경이 포함된 커밋은 `.patch` 용량이 커질 수 있다는 점을 염두에 둔다 (새 에셋은 델타 효율이 떨어지며 풀 파일 크기 + base85 오버헤드 ~25%가 붙음).

## 아키텍처

### 네트코드 모델

전투 관련 모든 액션은 **클라 → `Server_*` RPC → 애니메이션용 `Multicast_*` RPC** 트리플릿을 따르고, 원격 클라에 보여야 하는 상태값은 `Replicated` 또는 `ReplicatedUsing=OnRep_*`로 선언한다. 새 공격 능력을 추가할 때는 [BaseCharacter.h](Source/MultiActionGame/Public/Character/BaseCharacter.h)에 자리 잡힌 패턴(`LightAttack` / `Server_LightAttack` / `Multicast_PlayLightAttackAnimation`)을 그대로 따르고, 디자이너가 블루프린트에서 게임플레이/애니를 연결할 수 있도록 `BP_*` `BlueprintImplementableEvent` 짝을 같이 만든다.

회전 처리는 두 가지 모드가 공존한다. 일부 캐릭터는 `bUseReplicatedRotation`을 켜고 `Server_SetRotation` + `OnRep_ReplicatedRotation` + `SynchronizeRotation()` 경로로 서버 권위적 회전을 사용하고(예: Boss), 다른 캐릭터는 언리얼 CharacterMovement 기본 회전 복제를 쓴다. 어떤 쪽이 맞는지는 능력 자체가 서버에서 정확한 방향을 요구하는지로 판단한다.

### 호스팅 / 세션 흐름

`UMultiGameInstance::Host`는 언리얼 세션 인터페이스를 쓰지 않고 `World->ServerTravel`로 `"/Game/TestMap?listen?game=/Game/General/MainGameModeBP.MainGameModeBP_C?CharacterType=<n>"`에 직접 이동한다. `Join`은 IP로 `ClientTravel`을 호출한다. **선택한 캐릭터 타입은 URL 옵션으로 전달**되어 `AMainGameMode::InitNewPlayer`([MainGameMode.cpp:192](Source/MultiActionGame/Private/MainGameMode.cpp#L192))에서 파싱, `AMainPlayerController`에 기록된다. 이후 컨트롤러는 `BeginPlay`에서 `ServerSetCharacterType`을 호출해 `SelectedCharacterType`을 소유 클라로 재복제 — 이 덕분에 메뉴를 거치지 않고 붙는 클라(PIE 추가 창 등)도 타입을 올바로 갖게 된다.

`AMainGameMode::SpawnDefaultPawnFor_Implementation`은 `SelectedCharacterType`을 보고 `KnightCharacter` / `ArcherCharacter` / `WizardCharacter` / `BossCharacter` 중 하나를 스폰한다. `ChoosePlayerStart_Implementation`은 `"Boss"` 또는 `"Hero"` 태그가 붙은 `PlayerStart`를 골라 맞춘다. **`TestMap`의 `PlayerStart` 액터에는 반드시 해당 태그를 달아줘야** 하며, 빠지면 엔진 기본 경로로 폴백된다.

### 게임 흐름

`AMainGameMode`는 `BossCharacters`, `HeroCharacters` 배열과 `HeroDeathCount`를 관리한다. 히어로가 참가할 때마다 `IncreaseBossMaxHealth`가 보스 최대 체력을 스케일 업한다. 승패는 `ProcessPlayerDeath`에서 결정 — 보스가 죽으면 히어로 승, `HeroDeathCount == HeroCharacters.Num()`가 되면 보스 승. 결과는 컨트롤러 순회로 `Client_CreateGameOverWidget`을 호출해 브로드캐스트하며, GameState 기반 종료 처리는 없다. 새로운 "매치 종료" 조건은 `ProcessPlayerDeath` / `ProcessGameOver`에 추가해야 한다.

### PvE / Boss AI

PvE 모드에서는 메인 메뉴에서 `UMultiGameInstance::IsBossAI`가 세팅된다. `AMainGameMode::BeginPlay`가 이 플래그를 감지해 Boss 태그 `PlayerStart`에 보스 폰을 스폰하고 `GruxAIController`(`ABossAIController`)로 Possess한다. 컨트롤러는 BP 서브클래스에 설정된 `BehaviorTree`/`BlackboardData`를 구동한다. 타깃팅 점수는 세 가지 가중치(캐릭터 타입, 저체력 보너스, 거리)로 `SetTarget()`에서 계산된다. 액션 선택(`SetNextPattern()`)은 `TArray<ECharacterAction>`에 가중 개수만큼 항목을 채워넣고 랜덤으로 하나를 뽑는 방식 — 난이도 튜닝은 [BossAIController.h](Source/MultiActionGame/Public/AI/BossAIController.h) 상단 상수들을 조정하거나 [BossAIController.cpp:18](Source/MultiActionGame/Private/AI/BossAIController.cpp#L18)의 `AddPattern(..., count)` 가중치를 바꾸면 된다.

Behavior Tree는 전투 함수를 직접 호출하지 않는다 — 블랙보드에 `NextAction`(`ECharacterAction` 바이트)을 기록하고, BP BT 태스크가 `ABaseCharacter::PerformAction(action)`을 호출해 로컬 입력과 동일한 `LightAttack` / `HeavyAttack` / `UseSkill` / `JumpAction` / `SelfHeal` 디스패치 경로를 탄다. 이 구조 덕분에 AI와 플레이어가 같은 서버 RPC 경로를 공유한다.

### C++ / 블루프린트 분담

C++이 담당: 네트워킹, 스탯 배관, 타깃 선택, HUD 바인딩, 입력. 블루프린트가 담당: 구체적인 공격 히트박스, 애니메이션 몽타주, VFX, 캐릭터별 스탯 튜닝. 분할은 `ABaseCharacter`의 `BP_*` 훅으로 명시되어 있다 — `BP_CanAttack`, `BP_ExecuteLightAttack`, `BP_PlayLightAttackAnimation`, `BP_CanRoll`, `BP_CanSelfHeal`, `BP_SetAIStat` 등. 대부분의 구체 캐릭터는 `AKnightCharacter` / `ABaseCharacter`의 BP 서브클래스이고, **`KnightCharacter` / `TestCharacter1` 같은 C++ 클래스는 사실상 껍데기**다. 실제 캐릭터별 동작은 `Content/Paragon*`의 블루프린트 에셋에 들어 있다.

입력은 Enhanced Input으로 연결된다. `/Game/Input/InputActionGroup.InputActionGroup` 경로의 `UInputActionGroup` DataAsset 하나가 `ABaseCharacter` 생성자에서 `ConstructorHelpers::FObjectFinder`로 로드된다. 새 액션 추가는 `UInputActionGroup`에 `UInputAction*` 프로퍼티 추가 → `SetupPlayerInputComponent`에서 바인딩.

### UI

`AMainPlayerController`는 `OnPossess`(서버 경로)와 `OnRep_Pawn`(클라 경로) 양쪽에서 `UInGameHUD`를 생성한다 — 서버와 클라가 로컬 폰에 도달하는 시점이 달라서 두 경로가 모두 필요하므로 하나로 합치지 말 것. `UInGameHUD`는 `UHealthComponent` / `UStaminaComponent` 델리게이트에 바인딩되며, 모든 히어로가 공유로 보는 보스 체력 바도 같이 띄운다(`AMainPlayerController::InitializeBossHealth`로 연결). 상대 히어로/보스 머리 위 이름표는 `UWidgetComponent`에 붙는 `UHealthBarWidget`이고, `HideHealthBarWidget`이 로컬 플레이어 본인의 이름표는 숨긴다. 메뉴류(`UMainMenu`, `UInGameMenu`, `UGameOverWidget`)는 `UMenuWidget`을 상속하며 `IMenuInterface`(구현체: `UMultiGameInstance`)를 통해서만 Host/Join/LoadMainMenu 전이가 일어난다.

### 네이밍 / 레이아웃 규칙

- 공개 헤더: `Source/MultiActionGame/Public/<서브시스템>/*.h` (서브시스템: `AI`, `Character`, `MenuSystem`, `UI`, 그리고 모듈 루트의 일부 파일).
- 비공개 소스: `Source/MultiActionGame/Private/<서브시스템>/*.cpp` — Public 레이아웃과 동일 구조로 미러링.
- 모든 C++ 타입은 `MULTIACTIONGAME_API` 익스포트 매크로 사용(에디터와 다른 모듈에서의 접근에 필요).
- 블루프린트 에셋은 `Content/` 아래 폴더 의미대로 미러링한다(`Content/General/`, `Content/MenuSystem/`, `Content/Input/`, `Content/SkillData/`, `Content/Paragon<캐릭터>/`).
- `.editorconfig`는 **탭(폭 4)**, Allman 중괄호, `cpp_use_unreal_engine_macro_formatting = true`를 강제한다. 규칙을 맞춘다.

## 이 리포 고유의 주의점

- 모듈은 `EnhancedInput`과 `AIModule`에 의존한다. 새 코드를 넣고 링크 에러가 뜨면 [MultiActionGame.Build.cs:11](Source/MultiActionGame/MultiActionGame.Build.cs#L11)과 [MultiActionGame.uproject](MultiActionGame.uproject)의 `AdditionalDependencies`에 들어 있는지 먼저 확인.
- `DefaultEngine.ini`에 `GameInstanceClass=/Script/MultiActionGame.MultiGameInstance`, `GameDefaultMap=/Game/MenuSystem/MainMenu`, `EditorStartupMap=/Game/TestMap`, `GlobalDefaultGameMode=/Game/General/MainGameModeBP.MainGameModeBP_C`가 하드코딩되어 있다. C++ 클래스만 이 경로를 좌우하는 게 아니므로 클래스 이름을 바꾸면 ini도 같이 고쳐야 한다.
- `ParagonFey`, `ParagonSparrow`, `LavaMat`, `MWLandscapeAutoMaterial`, `FreeSampleAnimationSet` 콘텐츠 폴더는 Git LFS로 추적된다([.gitattributes](.gitattributes)). 이 영역에 대한 git 작업은 LFS가 설치되어 있어야 한다.
- `Content/`, `Saved/`, `Intermediate/`, `DerivedDataCache/`는 `.ignore`(ripgrep/fd 스코프 제어용)에 등록되어 있다. `.gitignore`와 혼동하지 말 것 — UE의 `Content/`는 반드시 버전 관리 대상이고, 단지 검색 도구가 이 폴더를 건너뛰게 만든 것뿐이다.
- 템플릿 잔재 파일이 남아 있다 — `TestActor`, `TestCharacter1`, `MyGameMode.cpp`, `MyPlayerController.*` — 대부분 비어 있는 껍데기이므로 참고용으로 쓰지 말고, 실제 예시는 `ABaseCharacter` / `AMainGameMode` / `AMainPlayerController`를 볼 것.

## Workflow Principles

### 1. Think Before Coding — 먼저 생각하고 코딩하라

- 작업 전에 **관련 파일을 읽고 TodoWrite로 계획**을 세울 것
- **사용자와 계획을 확인**한 뒤 구현 시작
- 가정이 있으면 **명시적으로 밝히고**, 불확실하면 코딩 전에 물어볼 것
- 여러 접근법이 가능하면 **트레이드오프와 함께 제시**하고 사용자가 선택하게 할 것
- 더 단순한 방법이 있으면 복잡한 방법 대신 **먼저 제안**할 것

### 2. Simplicity First — 단순함을 우선하라

- 요청받은 것 **이상의 기능을 추가하지 말 것** (불필요한 추상화, 설정 옵션, 에러 핸들링 금지)
- 한 번만 쓰이는 코드에 헬퍼/유틸리티/추상화를 만들지 말 것
- 200줄로 작성한 코드가 50줄로 가능하면 **다시 작성**할 것
- 자기 점검: "시니어 엔지니어가 이걸 보고 과하다고 할까?"

### 3. Surgical Changes — 외과적으로 변경하라

- **최소 변경 원칙** — 대규모 리팩토링 금지, 영향 범위 최소화
- 요청과 **무관한 코드·주석·포맷팅을 건드리지 말 것**
- 기존 코드 스타일을 따를 것 (본인 선호와 달라도)
- 자기 점검: "변경된 모든 줄이 사용자 요청으로 추적 가능한가?"

### 4. Goal-Driven Execution — 목표 중심으로 실행하라

- 각 변경에 대해 **간단한 고수준 설명** 제공
- 모호한 요청은 **검증 가능한 목표로 변환**하여 확인받을 것
- 다단계 작업에는 **단계별 검증 계획**을 제시할 것
- 작업 종료 시 변경 사항 요약

## 응답 언어·톤

- 항상 **한국어 존댓말**로 답변합니다.
- 코드 주석은 한국어, 식별자는 영어입니다. 기존 톤을 따릅니다.
- **이모지 금지** — 응답 본문, 코드 주석, 커밋 메시지, 문서 등 어디에도 이모지를 넣지 않습니다. 사용자가 명시적으로 요청한 경우에만 예외.
