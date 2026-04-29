# Dedicated Server Migration — 리슨 → 데디케이티드 전환 가이드

## 개요

현재 프로젝트는 리슨 서버 방식으로 동작하는 멀티 액션 게임이다. 호스트 플레이어의 프로세스가 서버 겸 플레이어 1 역할을 동시에 수행하며, `UMultiGameInstance::Host`에서 `World->ServerTravel("/Game/TestMap?listen?game=...")`로 게임 맵에 진입한다. 이 문서는 해당 구조를 데디케이티드 서버 방식으로 확장하기 위한 단계별 작업 가이드이다.

구현 목표: **별도 프로세스로 돌아가는 헤드리스 서버가 항시 대기하고, 클라이언트는 IP로 Join해 게임에 참여**. 세션 디스커버리(서버 브라우저)는 범위 밖이며 필요 시 별도 단계로 확장한다.

## 1. 핵심 개념 — 리슨 vs 데디케이티드

| 구분 | 리슨 서버 (현재) | 데디케이티드 서버 |
|---|---|---|
| 서버 == 플레이어? | O. 호스트가 서버이자 로컬 플레이어 1 | **X.** 서버는 폰/컨트롤러/HUD/입력이 없는 헤드리스 프로세스 |
| 실행 주체 | 메뉴에서 `ServerTravel`로 같은 프로세스가 서버로 전환 | 독립 `MultiActionGameServer.exe`가 커맨드라인으로 맵 로드 후 클라 대기 |
| 설정 주체 | 호스트 클라의 `GameInstance` 상태 = 서버 상태 | 서버는 클라 `GameInstance`를 모름. **실행 인자(URL/CLI)**로 설정 전달 |
| 빌드 타깃 | Editor / Game 타깃만 있어도 충분 | `Server` 타깃 필수 (`Type = TargetType.Server`) |

데디케이티드 환경은 **렌더링/오디오/UMG 뷰포트/입력이 존재하지 않는다는 전제**가 모든 설계에 번진다. 리슨 서버에서는 `HasAuthority()` 분기 안에서 위젯을 만들어도 호스트의 로컬 플레이어가 있어 자연스럽게 표시되지만, 데디에서는 소유자 자체가 없어 `CreateWidget`/`AddToViewport` 호출이 무의미하거나 오류를 만든다.

## 2. 설계 원칙 — 설정 경로의 분리

이번 전환에서 **가장 중요한 개념적 축**. 설정 값은 성격에 따라 서로 다른 경로로 전달돼야 하며, 절대 섞이면 안 된다.

| 범주 | 값의 예 | 결정 시점 | 전달 경로 | 수신 지점 |
|---|---|---|---|---|
| **서버 전역 (매치 설정)** | `BossAI`, `BossHealth`, `BossAttackDamage`, `BossAttackCost`, `BossSkillCooldown` | 서버 기동 시점에 고정 | `MultiActionGameServer.exe "/Game/TestMap?BossAI=1?BossHealth=1000"` 또는 `-BossHealth=1000` CLI | `AGameModeBase::InitGame(Options, ...)` / `FCommandLine::Get()` |
| **플레이어별** | `CharacterType` | 각 클라의 선택 시점 | `ClientTravel("ip?CharacterType=1")` URL | `AGameModeBase::InitNewPlayer` |

**섞으면 생기는 문제**: 전역 값을 클라 URL로 받게 설계하면 "먼저 접속한 플레이어가 보스 난이도 결정", "두 번째 플레이어의 다른 값이 무시되거나 덮어씀" 같은 동시성 버그가 구조적으로 열린다. 전역 설정은 반드시 서버 기동 시점에 확정된다.

현재 코드는 `UMultiGameInstance`의 필드(`BossHealth`, `BossAttackDamage` 등)가 **호스트 클라의 GameInstance = 서버 GameInstance**라는 리슨 서버 특성에 기대 서버 측에서 읽혀 왔다. 데디 전환 후에는 이 경로가 끊기므로, 전역 설정은 전부 `AMainGameMode` 멤버로 옮기고 `InitGame` 경유로 주입한다.

## 3. 단계별 작업

### Phase 1 — 환경 구성 및 헤드리스 동작 확인

목적: 실제 코드 수정 없이 데디 환경에서 현재 구현이 어떻게 깨지고 어디가 멀쩡한지 파악한다.

#### 1-A. PIE 데디케이티드 모드로 기존 코드 점검

에디터만으로 헤드리스 시뮬레이션이 가능하므로, Server Target 빌드 전에 이 단계로 먼저 버그를 노출시킨다.

UE 5.6 기준 위치는 두 군데 중 하나에서 설정한다(버전/레이아웃에 따라 라벨이 조금씩 다를 수 있으므로 정확한 문구보다는 **찾아야 할 두 옵션의 이름으로 식별**):
- Play 버튼 옆 드롭다운 → `Advanced Settings` (또는 `Play Settings`)
- 또는 Editor Preferences → Level Editor → Play → Multiplayer

두 옵션을 찾아 설정:
- `Play Net Mode` = **Play As Client**
- `Number of Players` = 2 이상 (관전 창 + 실제 클라)
- `Run Dedicated Server` = **체크**

Play 버튼 클릭 → 서버 창 1개(헤드리스, 로그 위주) + 클라 창 N개 생성

관찰 포인트:
- 서버 창 로그에 `AMainGameMode::BeginPlay`가 찍히는지
- `GetIsBossAI()`가 `false`로 나오며 PvE용 보스가 스폰 안 되는지 (GameInstance 경로의 한계가 여기서 드러남 — 서버 프로세스의 GameInstance는 메뉴를 거치지 않아 기본값이다)
- `ConstructorHelpers::FClassFinder`가 서버 측에서도 문제없이 로드되는지 (UMG는 엔진 플러그인이라 서버 빌드에도 링크되므로 통과가 정상)
- 클라 창에서 `UInGameHUD`가 정상 생성/바인딩되는지

**사전 조사 체크리스트** (Phase 3 진입 전에 반드시 완료):

- [x] **BP에서 `UMultiGameInstance`의 Boss* 필드를 실제로 읽는 지점 목록화**
- [x] **`-1.0f` 센티넬의 소비 규약 확인**

**조사 결과 (확정)**:

- **소비 지점**: Boss 캐릭터 BP의 **Event BeginPlay** 그래프. 처음 의심했던 `BP_SetAIStat`은 스태미나 회복률/이동 속도/회전 모드만 세팅하며 Boss* 필드를 읽지 않는다. 실제 소비는 BeginPlay에서 이루어진다.
- **소비 구조**:
  ```
  Event BeginPlay
    → GetPlayerController(0) → Cast MainPlayerController → InitializeBossHealth (HUD 바인딩, 모든 process)
    → HasAuthority 가드 (서버에서만 이후 실행)
    → Cast to MultiGameInstance
        ├─ BossHealth > 0       → SET DefaultMaxHealth → SET CurrentMaxHealth → ResetHealth
        ├─ BossAttackDamage > 0 → SET BasicAttackDamage
        ├─ BossAttackCost > 0   → SET LightAttackStaminaCost
        └─ BossSkillCooldown > 0→ SET SkillCooldown
  ```
- **4개 필드 전부 실사용**: dead code 없음. 전원 이관 대상.
- **`-1.0f` 센티넬 규약은 이미 BP에 구현되어 있음**: 각 SET 경로 앞의 `> 0` Branch 가드가 폴백 역할이다. 메뉴 입력이 없으면 `-1` 기본값이 남고 가드에 걸려 SET이 스킵되어 캐릭터 BP CDO 기본값이 그대로 적용된다. **새 규약을 만들 필요 없이 기존 규약을 그대로 계승**한다.
- **PvP 모드에도 적용됨**: BeginPlay는 AI/플레이어 구분 없이 모든 Boss 액터 스폰 시 실행되므로, 사람이 조종하는 보스에게도 난이도 설정이 적용되고 있었다.

이 결과로 Phase 3의 작업 범위가 확정됐다: **C++ 4개 필드 + getter를 `AMainGameMode`에 추가, `InitGame`에서 URL 옵션 파싱, Boss BP Event BeginPlay의 `Cast to MultiGameInstance` 노드만 `Cast to MainGameMode`로 교체** (그래프 구조 자체는 보존).

**Phase 1-B 진입 전 추가 확인 항목** (Phase 1-A 결과를 토대로 한 2차 조사):

- [x] **(우선순위 1) Hero 캐릭터 BP(Knight/Archer/Wizard)도 같은 GameInstance 직독 패턴을 가지는가**
  - **결과: 없음.** 세 BP 모두 `Find in Blueprints`로 `GameInstance` 검색 시 0 hit. **재배선 대상은 Boss BP 단일로 확정.**
- [x] **(우선순위 2) 재배선 대상 Boss BP 에셋 특정**
  - **결과: `GruxPlayerCharacter`** (Parent Class: C++ `ABaseCharacter`). Event BeginPlay는 이 BP의 Event Graph에 직접 놓여 있음(`(Override)` 마킹 없음은 정상 — BP Event는 리스너 앵커이기 때문). 상속 체인이 단순(BP → C++)이라 중간 레이어 없음.
- [x] **(우선순위 3) `UMultiGameInstance` 전체 필드 훑어 누락 확인**
  - **결과: 5개(`IsBossAI` + `BossHealth`/`BossAttackDamage`/`BossAttackCost`/`BossSkillCooldown`)로 확정, 누락 없음.** `SelectedCharacterType`은 이미 URL 옵션(`CharacterType=N`)으로 `InitNewPlayer` 경로 타고 있음 — 이관 대상 아님. `EGraphicSetting`, `MenuClass`/`InGameMenuClass`/`Menu`/`InGameMenu`는 클라 개인 설정/UI 상태 — 이관 대상 아님.
  - **부가 수정**: 생성자([MultiGameInstance.cpp:27-31](../Source/MultiActionGame/Private/MultiGameInstance.cpp#L27-L31))에서 `BossAttackDamage`/`BossAttackCost`/`BossSkillCooldown` 3개가 초기화 누락 상태였음(UObject 메모리 0-fill 덕분에 `0.0f`로 시작 → Boss BP `> 0` Branch가 False → 우연히 동작 중). 전부 `-1.0f`로 명시 초기화하도록 수정 완료. 이제 4개 필드가 일관되게 `-1.0f` 센티넬을 사용.
- [ ] **(우선순위 4) PIE에서 URL 옵션 전달 가능 여부** — Phase 3-B 구현 후 PIE 데디 모드 Advanced Settings의 `Server Map URL` 필드(또는 유사 필드)로 검증. 없으면 1-C(헤드리스 exe) 단계에서 검증.
- [ ] **(우선순위 5) ServerTravel URL 체인 인코딩 동작 확인** — Phase 3-B/3-F 구현 후 `UE_LOG`로 `InitGame` Options 문자열과 `HasOption`/`ParseOption` 결과를 찍어 검증.

**메뉴 → Host 흐름 확인 결과** (우선순위 3의 연장선):
- `WBP_MainMenu` Event Graph는 비어 있고, 로직은 전부 C++ `UMainMenu`에 있음.
- `UMainMenu::ConfirmSetting()` ([MainMenu.cpp:316-353](../Source/MultiActionGame/Private/MenuSystem/MainMenu.cpp#L316-L353))이 TextBox 4개(`BossHealthInput` / `BossAttackDamageInput` / `BossAttackCostInput` / `BossSkillCooldownInput`)를 `FDefaultValueHelper::ParseFloat`로 파싱해 `UMultiGameInstance` 필드에 직접 저장. **파싱 실패(빈 문자열 포함) 시 필드를 건드리지 않음** → 생성자 초기값 `-1.0f` 유지. 이 동작이 Phase 3-F의 `Host` URL 조립 시 `> 0` 가드와 맞물려 "센티넬 규약"이 리슨 경로에서도 일관되게 유지된다.
- `UMainMenu::HostServer()` ([MainMenu.cpp:177-197](../Source/MultiActionGame/Private/MenuSystem/MainMenu.cpp#L177-L197))은 PvE 여부에 따라 `SetIsBossAI(true/false)`만 세팅한 뒤 `MenuInterface->Host(CharacterType)` 호출.

우선순위 1~3 완료로 **사전 조사 종결**. 4, 5는 구현 중 로그로 자연스럽게 검증되므로 별도 선행 작업 필요 없음. Phase 1-A의 PIE 데디 실행은 "깨지는 지점 확인"이 목적인데 조사 결과로 이미 밝혀진 상태이므로 **생략하고 Phase 1-B로 직행 가능**(시간 여유가 있으면 기준선 확보용으로 돌려봐도 무방).

#### 1-B. Server Target 추가 + Source Build 엔진 전환

`Source/MultiActionGameServer.Target.cs` 생성 (`MultiActionGame.Target.cs`와 동일 스타일에 `Type = TargetType.Server`만 다름).

**Launcher 엔진은 Server 타깃을 빌드할 수 없다.** UE 5.3+ Epic Games Launcher 배포판은 Server 타깃용 precompiled 바이너리(`BuildSettings.precompiled` 등)를 포함하지 않는다. `BaseEngine.ini`의 `[InstalledPlatforms]` 추가나 엔진의 `UnrealServer.Target.cs` 신규 작성으로 일부 단계는 통과할 수 있어도 결국 monolithic 링크 시점에 막힌다 (이 프로젝트도 그 경로로 시도했다 실패). **Source Build 엔진이 사실상 유일한 정공법** — Epic 공식 포럼·문서·커뮤니티 튜토리얼이 일관되게 같은 결론.

**Source Build 엔진 준비** (한 컴퓨터에서 1회, 시간 3~6시간 + 디스크 200GB+):

1. **GitHub ↔ Epic 계정 연결**: https://www.unrealengine.com/account/connections → GitHub 연결 → `@EpicGames` 조직 초대 수락 → `https://github.com/EpicGames/UnrealEngine` 접근 확인
2. **소스 clone** (공백 없는 짧은 경로 권장):
   ```bash
   git clone --depth=1 --branch 5.6 https://github.com/EpicGames/UnrealEngine.git C:/UE5Source
   ```
3. **의존성 다운로드** (~100GB, 1~3시간):
   ```bash
   (cd /c/UE5Source && ./Setup.bat)
   ```
4. **VS 프로젝트 파일 생성** (수십 초):
   ```bash
   (cd /c/UE5Source && ./GenerateProjectFiles.bat)
   ```
5. **Visual Studio에서 엔진 빌드** (3~6시간):
   - `C:/UE5Source/UE5.sln` 열기 (VS 2022)
   - Configuration `Development Editor` / Platform `Win64`
   - Solution Explorer → Engine → `UE5` 프로젝트 우클릭 → Build
   - 산출물: `C:/UE5Source/Engine/Binaries/Win64/UnrealEditor.exe`

**프로젝트를 Source Build 엔진에 연결**:

```powershell
# 1. 레지스트리에 Source Build 엔진을 'UE5Source' 이름으로 등록 (HKCU 영역)
New-Item -Path 'HKCU:\SOFTWARE\Epic Games\Unreal Engine\Builds' -Force | Out-Null
New-ItemProperty -Path 'HKCU:\SOFTWARE\Epic Games\Unreal Engine\Builds' -Name 'UE5Source' -Value 'C:/UE5Source' -PropertyType String -Force
```

2. `.uproject`의 `"EngineAssociation": "5.6"` → `"UE5Source"`로 수정
3. **다른 컴퓨터 동기화 보호** (중요 — 동기화 흐름 §"동기화 / 작업 흐름" 참조):
   ```bash
   git update-index --skip-worktree MultiActionGame.uproject
   ```
   이 컴퓨터에서만 빌드한다는 전제로, EngineAssociation 변경이 `.patch`에 섞여 다른 컴퓨터로 전파되지 않게 한다. `git status`에 `S` 플래그가 붙어 표시되며 커밋 대상에서도 제외된다.

**프로젝트 빌드** — Git Bash에서 `Build.bat`은 공백 경로(`c:/Program Files/...`) 처리 이슈로 깨지므로 **`UnrealBuildTool.exe`를 직접 호출**한다:

```bash
PROJECT="c:/Users/admin/Documents/Unreal Projects/MultiActionGame/MultiActionGame.uproject"
UBT="C:/UE5Source/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe"

# 0. (Launcher 엔진으로 빌드한 잔재 있으면) 정리
rm -rf "$(dirname "$PROJECT")/Binaries" "$(dirname "$PROJECT")/Intermediate"

# 1. VS 프로젝트 파일 재생성 (~30초)
"$UBT" -projectfiles -project="$PROJECT" -game -rocket -progress

# 2. Server 타깃 빌드 (첫 빌드 ~25분)
"$UBT" MultiActionGameServer Win64 Development -Project="$PROJECT" -WaitMutex

# 3. Editor 타깃 빌드 (첫 빌드 ~1분 — 엔진 모듈은 이미 빌드됨)
"$UBT" MultiActionGameEditor Win64 Development -Project="$PROJECT" -WaitMutex
```

산출물:
- `Binaries/Win64/MultiActionGameServer.exe` (~300MB monolithic 데디 서버)
- `Binaries/Win64/UnrealEditor-MultiActionGame.dll` (Editor용 모듈 — Cook commandlet과 외부 `-game` 클라 모두 이 모듈을 로드)

빌드 실패 시 대부분 `PublicDependencyModuleNames` 또는 `AdditionalDependencies`에 서버에 없는 모듈 참조가 있는 경우. 현재 [MultiActionGame.Build.cs](../Source/MultiActionGame/MultiActionGame.Build.cs)의 의존성(`Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `AIModule`)은 전부 서버에서도 링크되므로 그대로 통과해야 정상.

#### 1-C. Cook + 헤드리스 실행

**Cook이 먼저 필요하다.** monolithic Server 타깃은 cooked content를 요구한다. cook 없이 실행하면 `Failed to load premade asset registry` + `BufferReader assertion`으로 즉시 크래시한다.

**UAT 우회**: UE 5.6의 `BuildCookRun`은 `Magick.NET-Q16-HDRI-AnyCPU` NuGet 패키지의 보안 경고가 dotnet build의 오류로 승격되어 빌드 자체가 실패한다. **`UnrealEditor-Cmd.exe`의 `cook` commandlet을 직접 호출**해서 UAT/Gauntlet 빌드 단계를 건너뛴다:

```bash
"C:/UE5Source/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$PROJECT" -run=Cook -targetplatform=WindowsServer -unattended -log
```

소요 시간: 첫 cook ~10초~수 분 (셰이더 캐시 활용). 결과물은 `Saved/Cooked/WindowsServer/`. 사전에 UnrealEditor가 한 번 떠서 셰이더 캐시(DDC)를 채워 둔 상태가 가장 빠르다.

**서버 실행** — **반드시 cmd 창에서 직접 실행한다.** Git Bash에서 호출하면 MSYS의 자동 경로 변환으로 `/Game/TestMap`이 `C:/Program Files/Git/Game/TestMap`으로 둔갑하여 `FilenameToLongPackageName failed to convert ... contains illegal characters ':'` 크래시. PowerShell의 background도 콘솔 분리(`-NoNewWindow` 또는 stdin EOF) 시 process가 즉시 종료되는 케이스가 잦으니 보수적으로 cmd 창에서 직접 띄운다:

```cmd
"<프로젝트>\Binaries\Win64\MultiActionGameServer.exe" /Game/TestMap?listen?Port=7777 -log
```

URL 옵션 주의:
- **`?listen` 필수**: monolithic Server 타깃이라도 NetDriver listening 트리거에 `?listen`이 필요하다. 빠지면 서버 process가 살아있어도 NetDriver가 socket bind/listen을 시작하지 않는다. `?listen`이 NetMode를 리슨 서버로 바꾸는 게 아니다 — `IsDedicatedServer = 1` 그대로이고 단지 NetDriver 활성화 스위치 역할.
- **`?Port=N` 권장**: 명령줄 `-port=N` 형식은 인식이 일관되지 않을 수 있어 default 7777로 fallback되는 경우가 있음. URL 옵션 형태가 안정적.

로그 확인 포인트:
- `LogNet: Created socket for bind address: 0.0.0.0:7777`
- `LogNet: ... IpNetDriver listening on port 7777`
- `LogWorldPartition: ... NetMode = Dedicated Server, IsDedicatedServer = 1`
- `LogTemp: Warning: [GameMode] End Begin Play` (Boss 스폰까지 정상)

#### 1-D. 클라에서 Join

**PIE 클라는 데디 서버 join에 부적합하다.** PIE는 어떤 모드(Selected Viewport / New Editor Window / Standalone Game)로 띄우든 내부적으로 PIE world conversion을 거쳐 `/Game/UEDPIE_0_TestMap` 같은 메모리 패키지를 만든다. 클라가 그 메모리 패키지의 visibility를 서버에 동기화하려 하면 진짜 데디 서버는 이 패키지를 모르므로 `MissingLevelPackage`로 연결을 끊는다 (이 프로젝트 첫 시도에서 직접 확인된 함정).

**해결**: 에디터 외부에서 `UnrealEditor.exe`를 `-game` 모드로 실행해 진짜 게임 클라이언트로 띄운다. 새 cmd 창에서:

```cmd
"C:\UE5Source\Engine\Binaries\Win64\UnrealEditor.exe" "<프로젝트>\MultiActionGame.uproject" 127.0.0.1:7777?CharacterType=1 -game -log -ResX=1280 -ResY=720
```

- URL 인자(`127.0.0.1:7777?CharacterType=1`)가 실행 직후 자동 join 트리거
- 메뉴 흐름을 검증하려면 URL 빼고 실행 후 메뉴에서 IP 입력
- `UnrealEditor-MultiActionGame.dll`이 빌드되어 있어야 한다 (Phase 1-B의 Editor 타깃 빌드)

성공 시 **서버 콘솔**에 다음 흐름이 보이고 끊기지 않는다:

```
LogNet: NotifyAcceptingConnection accepted from: 127.0.0.1:XXXXX
LogNet: Login request: ?Name=...?CharacterType=N
LogTemp: [GameMode] Choose Player Start -> Knight
LogTemp: [GameMode] Spawn Knight
LogTemp: [GameMode] Add Hero to Array
LogTemp: OnPossess: ...
LogNet: Join succeeded
```

클라 화면에 캐릭터가 스폰되어 입력으로 움직이면 Phase 1 완료.

**WP + 데디 함정 (간단 메모)**: TestMap이 World Partition을 사용하면 데디 서버에서 셀 visibility 동기화 이슈로 `MissingLevelPackage`가 다시 발생할 수 있다. 본 프로젝트는 단일 arena 게임이라 WP가 의미 없으므로 `World Settings → World Partition → Enable Streaming`을 끄는 것으로 회피했다 (자세한 배경은 §4 "흔한 함정" 참조).

### Phase 2 — 클라이언트 전용 경로 가드

목적: 데디 서버 프로세스가 실수로 클라 전용 API(주로 `CreateWidget` 계열)에 도달하는 경로를 차단한다.

현재 구조 분석:
- `UMultiGameInstance::LoadMenu` / `LoadInGameMenu` / `LoadMainMenu` → 메뉴 UX에서만 호출. 데디 서버가 메뉴를 띄울 이유가 없어 경로 자체가 트리거되지 않을 가능성이 높음
- `AMainPlayerController::CreateInGameHUD` → 이미 `IsLocalPlayerController()` 가드가 있어 데디 서버에서 자동 skip. 유지
- `UInGameHUD::InitializeHUD` 계열 → 클라에서 생성된 위젯 내부 로직이라 가드 불필요

실제 필요한 가드:

```cpp
void UMultiGameInstance::LoadMenu()
{
    if (GetNetMode() == NM_DedicatedServer) return;
    // 기존 로직
}
```

`LoadInGameMenu`, `LoadMainMenu`도 동일 패턴. 생성자의 `FClassFinder`는 로드만 하고 인스턴스화하지 않으므로 가드 우선순위 낮음(안전장치로 넣어도 무방하되 필수 아님).

판별 API 정리:
- `GetNetMode() == NM_DedicatedServer`: 서버 프로세스가 데디 모드로 돌고 있는 경우에만 true
- `IsRunningDedicatedServer()` (`Engine/Public/Misc/CommandLine.h`): 프로세스 레벨 판별, 더 저렴하고 GameInstance에서도 쓸 수 있음
- `HasAuthority()`: 리슨 서버에서도 true이므로 "데디에서만 skip"의 의도에는 부적절

### Phase 3 — 설정 경로 분리 (핵심 리팩토링)

목적: **§2의 설계 원칙을 코드에 반영**. 서버 전역 설정과 플레이어별 설정이 물리적으로 다른 경로로 흐르게 만든다.

#### 3-A. 이관 대상 확정

Phase 1-A 사전 조사에서 아래가 확정되었다:

- **`UMultiGameInstance::IsBossAI`**: C++([MainGameMode.cpp:31](../Source/MultiActionGame/Private/MainGameMode.cpp#L31))에서 직접 읽음. 이관 대상
- **`UMultiGameInstance::BossHealth` / `BossAttackDamage` / `BossAttackCost` / `BossSkillCooldown`** (4개): Boss BP Event BeginPlay에서 서버 권위(HasAuthority) 가드 후 `> 0` Branch와 함께 소비. 4개 모두 실사용 중이므로 전부 이관 대상

총 5개 필드를 `AMainGameMode`로 옮긴다. dead code는 없으므로 제거 작업은 없다.

#### 3-B. 서버 전역 설정을 AMainGameMode로 이관

3-A에서 확정한 필드만 옮긴다. [MainGameMode.h](../Source/MultiActionGame/Public/MainGameMode.h)에 멤버 추가:

```cpp
protected:
    UPROPERTY(BlueprintReadOnly, Category = "Match")
    bool bBossAI = false;

    UPROPERTY(BlueprintReadOnly, Category = "Match")
    float BossHealth = -1.0f;  // -1 = 기본값 사용

    UPROPERTY(BlueprintReadOnly, Category = "Match")
    float BossAttackDamage = -1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Match")
    float BossAttackCost = -1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Match")
    float BossSkillCooldown = -1.0f;

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
```

[MainGameMode.cpp](../Source/MultiActionGame/Private/MainGameMode.cpp) 구현:

```cpp
void AMainGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    bBossAI = UGameplayStatics::HasOption(Options, TEXT("BossAI"));

    auto ReadFloat = [&](const TCHAR* Key, float& Out) {
        if (UGameplayStatics::HasOption(Options, Key))
        {
            Out = FCString::Atof(*UGameplayStatics::ParseOption(Options, Key));
        }
    };
    ReadFloat(TEXT("BossHealth"),       BossHealth);
    ReadFloat(TEXT("BossAttackDamage"), BossAttackDamage);
    ReadFloat(TEXT("BossAttackCost"),   BossAttackCost);
    ReadFloat(TEXT("BossSkillCooldown"),BossSkillCooldown);

    UE_LOG(LogTemp, Warning, TEXT("[GameMode] Match init: BossAI=%d Health=%.1f"), bBossAI, BossHealth);
}
```

**`-1.0f` 센티넬 규약** (기존 BP 규약 계승):
- Boss BP Event BeginPlay의 `> 0` Branch 가드가 이미 폴백 로직을 구현하고 있다. 값이 `0` 이하(-1 기본값 포함)이면 SET이 스킵되어 캐릭터 BP CDO의 기본값이 그대로 살아남는다
- `InitGame`의 Options 파싱이 키가 없을 때 `-1`을 유지하므로 이 규약이 `AMainGameMode` → BP 방향으로도 자연스럽게 이어진다
- Phase 3-D의 BP 재배선에서 `> 0` Branch를 건드리지 않고 Cast 타깃만 바꾸는 이유가 이것이다 — 폴백 규약 자체를 옮길 필요가 없다
- 전제 조건: `UMultiGameInstance` 생성자에서 4개 Boss* 필드가 전부 `-1.0f`로 초기화돼 있어야 리슨 경로에서도 규약이 성립한다. 사전 조사 단계에서 누락됐던 3개 필드는 이미 초기화 완료. `AMainGameMode`의 `-1.0f` 기본값도 이 규약의 연속이다.

서버 실행 예 (cmd 창에서):
```cmd
MultiActionGameServer.exe /Game/TestMap?listen?Port=7777?BossAI?BossHealth=1000?BossAttackDamage=15 -log
```

CLI 플래그(`FCommandLine::Get()` + `FParse`) 병행은 **이 단계에선 추가하지 않는다**. URL 옵션 하나로 충분하며, 둘 다 지원하면 우선순위 규칙 문서화/테스트 부담이 생긴다. 운영 단계에서 서버 관리 스크립트가 필요해지면 그때 CLI 지원을 덧붙이되, 그 시점에 URL 우선인지 CLI 우선인지 명시적으로 결정해 이 문서에 추가한다.

#### 3-C. BeginPlay의 Boss AI 트리거 교체

[MainGameMode.cpp:29-53](../Source/MultiActionGame/Private/MainGameMode.cpp#L29-L53)의:
```cpp
if (multiGameInstance->GetIsBossAI()) { ... }
```
를 다음으로 교체:
```cpp
if (bBossAI) { ... }
```

이제 BossAI 여부는 GameInstance에 의존하지 않고, 리슨/데디 양쪽에서 URL 옵션으로만 결정된다.

#### 3-D. Boss BP Event BeginPlay 재배선 (필수)

Phase 1-A 조사에서 확인된 실 소비처. **`Cast to MultiGameInstance` 노드의 타깃만 `Cast to MainGameMode`로 교체**하고, 나머지 그래프(`> 0` Branch 4개, SET 체인, `ResetHealth` 호출, `HasAuthority` 가드)는 전혀 건드리지 않는다.

C++ 측에 BP에서 깔끔하게 읽을 수 있도록 getter를 추가한다:

```cpp
// AMainGameMode.h
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Match")
float GetBossHealthSetting() const { return BossHealth; }

UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Match")
float GetBossAttackDamageSetting() const { return BossAttackDamage; }

UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Match")
float GetBossAttackCostSetting() const { return BossAttackCost; }

UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Match")
float GetBossSkillCooldownSetting() const { return BossSkillCooldown; }
```

`BlueprintPure`로 선언하면 BP 그래프에서 실행 핀 없이 값 핀만 연결할 수 있어 노드 수가 줄어든다. 기존 BP의 `As Multi Game Instance → BossHealth` 데이터 흐름이 `As Main Game Mode → GetBossHealthSetting` 패턴으로 거의 일대일 대응된다.

**재배선 절차 (BP 에디터에서)**:
1. Boss BP 에셋 열기 → Event Graph 탭
2. BeginPlay 흐름의 `Get Game Instance` + `Cast to MultiGameInstance` 두 노드를 `Get Game Mode` + `Cast to MainGameMode`로 교체
3. 기존 `As Multi Game Instance` 핀에 연결돼 있던 4개 데이터 와이어를 새 `As Main Game Mode`의 `GetBoss*Setting` getter 반환 값에 재연결
4. Compile → Save
5. PIE로 리슨/데디 양쪽 동작 확인 (옵션을 넣은 경우 값 적용, 옵션 없는 경우 BP CDO 기본값 유지)

#### 3-E. 플레이어별 설정은 기존 경로 유지

[MainGameMode.cpp:192-241](../Source/MultiActionGame/Private/MainGameMode.cpp#L192-L241)의 `InitNewPlayer`에서 `CharacterType`을 URL 옵션으로 파싱하는 로직은 **데디에서도 그대로 동작**한다. 각 클라가 `ClientTravel` 시 자기 `CharacterType=N`을 URL에 붙이고, 서버가 그 클라 전용 PlayerController에 세팅하는 구조는 이미 올바름. 수정 불필요.

[MultiGameInstance.cpp:56-80 `Host`](../Source/MultiActionGame/Private/MultiGameInstance.cpp#L56-L80), [83-103 `Join`](../Source/MultiActionGame/Private/MultiGameInstance.cpp#L83-L103)에서 URL에 `CharacterType`을 붙이는 부분도 유지.

> **UI 데이터 경로는 이번 이관과 무관**: [UInGameHUD::InitializeBossHealthComponent](../Source/MultiActionGame/Private/InGameHUD.cpp#L188)는 `UHealthComponent`(`ReplicatedUsing=OnRep_CurrentHealth`)를 직접 바인딩해 서버의 체력 상태를 받는다. 클라는 GameInstance의 Boss* 필드를 참조하지 않으므로 HUD 관련 코드는 이번 리팩토링에서 건드릴 필요가 없다.

#### 3-F. UMultiGameInstance의 서버 전역 필드 의미 재정의

[MultiGameInstance.h](../Source/MultiActionGame/Public/MultiGameInstance.h)의 `BossHealth`, `BossAttackDamage`, `BossAttackCost`, `BossSkillCooldown`, `IsBossAI`는 **더 이상 서버 상태가 아님**. 이제는 `UMainMenu`의 입력 버퍼(사용자가 호스트 모드에서 설정한 값을 잠시 보관)로만 쓰인다.

**기존 메뉴 흐름은 보존**:
- `UMainMenu::ConfirmSetting()`의 TextBox 파싱 → GameInstance 필드 세팅 로직 **수정 불필요**. 파싱 실패 시 필드를 건드리지 않아 `-1.0f`가 유지되는 동작이 `Host()`의 `> 0` 가드와 맞물려 "키 생략 = 센티넬 유지" 규약을 자연스럽게 만든다.
- `UMainMenu::HostServer()`의 `SetIsBossAI` + `MenuInterface->Host()` 호출도 그대로. `Host()` 내부의 URL 조립 부분만 확장하면 된다.

용도별 정리:
- **리슨 호스트 모드 (기존 유지)**: 메뉴 입력 값 → `GameInstance::Host()`가 `ServerTravel` URL에 붙여서 `AMainGameMode::InitGame`에 전달
- **데디 모드**: 클라의 메뉴에서 보스 설정 입력 자체가 불가. 서버 관리자가 CLI로 고정. 메뉴 UI에서는 데디 Join 경로 선택 시 해당 입력 필드를 숨기거나 `IsEnabled=false`로 처리

`GameInstance::Host`의 `ServerTravel` URL 구성도 같이 확장:
```cpp
FString Address = "/Game/TestMap?listen?game=/Game/General/MainGameModeBP.MainGameModeBP_C";
FString TravelURL = FString::Printf(TEXT("%s?CharacterType=%d"), *Address, static_cast<int32>(CharacterType));
if (IsBossAI)          TravelURL += TEXT("?BossAI");
if (BossHealth > 0)    TravelURL += FString::Printf(TEXT("?BossHealth=%.1f"), BossHealth);
// ... 나머지 필드
World->ServerTravel(TravelURL);
```

이렇게 하면 리슨/데디 양쪽에서 `AMainGameMode::InitGame`이 같은 옵션 포맷을 소비하게 되어 경로가 통일된다.

### Phase 4 — 메뉴 UX 정리

목적: 리슨 서버와 데디케이티드 서버를 모두 지원하거나, 데디 전용으로 정리한다.

#### 옵션 A: 리슨/데디 병행 (개발 단계 추천)

[UMainMenu](../Source/MultiActionGame/Public/MenuSystem/MainMenu.h)에 변경:
- Host 버튼: 기존 리슨 서버 플로우 유지 (개발/로컬 테스트용)
- Join 메뉴: IP 입력 필드가 데디 서버 IP:port를 받을 수 있도록 안내 문구 추가 (`예: 192.168.0.10:7777`)

장점: 디버깅 편의. 빠르게 로컬에서 테스트할 때는 Host, 실제 데디 동작 확인할 때는 미리 띄운 서버에 Join.

#### 옵션 B: 데디 전용

- Host 버튼 제거 또는 "개발 빌드에서만 표시"로 조건부 (`#if WITH_EDITOR` / `!UE_BUILD_SHIPPING`)
- 초기 화면은 Join 중심으로 정리
- 서버 기동은 별도 관리 스크립트/배치 파일

장점: UX 단순. 실사용 배포 시점에 권장.

결정 기준: 아직 게임 로직 개발이 한창이라면 **A 선택**. 데디 전환이 완료되고 운영 단계에 들어갈 때 B로 정리.

### Phase 5 — 세션 디스커버리 (선택적)

지금까지는 클라가 IP:port를 수동 입력해 Join한다. 여러 서버를 운영하거나 퍼블릭 매칭이 필요하면 세션 시스템이 필요하다.

선택지:
- **Steam OnlineSubsystem**: 개인/인디 게임의 실질적 표준. 무료지만 Steam 배포 전제
- **Epic Online Services (EOS)**: 크로스플랫폼, 무료. 계정 연동 복잡도 있음
- **커스텀 HTTP 백엔드**: 최대한 단순하게 서버 목록만 필요하면 Node.js/Go 한 프로세스로도 충분. 보안/확장성은 직접 챙겨야 함

도입 시 필요한 작업:
- `Config/DefaultEngine.ini`에 `[OnlineSubsystem]` 섹션 설정
- `UMultiGameInstance`에 `IOnlineSessionPtr` 획득 및 `CreateSession` / `FindSessions` / `JoinSession` 델리게이트 연결
- `UMainMenu`에 서버 브라우저 UI 추가

학습 비용이 상당하므로 **게임 기능이 안정되기 전까지는 직접 IP 입력으로 버티고, 필요해지는 시점에만 진행하는 것을 권장**한다.

## 4. 공통 참조

### 빌드/실행 커맨드 한 줄 정리

전제: Source Build 엔진(`C:/UE5Source`) 사용. 경로는 컴퓨터별로 조정. Git Bash 기준.

```bash
# 환경 변수
PROJECT="c:/Users/admin/Documents/Unreal Projects/MultiActionGame/MultiActionGame.uproject"
PROJDIR="$(dirname "$PROJECT")"
UBT="C:/UE5Source/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe"
EDITOR_CMD="C:/UE5Source/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"

# 1. 프로젝트 파일 재생성 (Target.cs 추가 / 엔진 전환 후)
"$UBT" -projectfiles -project="$PROJECT" -game -rocket -progress

# 2. Server 타깃 빌드
"$UBT" MultiActionGameServer Win64 Development -Project="$PROJECT" -WaitMutex

# 3. Editor 타깃 빌드 (Cook commandlet과 -game 모드 클라가 의존)
"$UBT" MultiActionGameEditor Win64 Development -Project="$PROJECT" -WaitMutex

# 4. Cook (Server target용 cooked content 생성, UAT 우회)
"$EDITOR_CMD" "$PROJECT" -run=Cook -targetplatform=WindowsServer -unattended -log
```

서버/클라 실행은 **cmd 창에서 직접** (Git Bash의 MSYS 경로 변환 함정 회피):

```cmd
:: 서버 실행 (예: BossAI 모드 + 커스텀 보스 체력)
"<프로젝트>\Binaries\Win64\MultiActionGameServer.exe" /Game/TestMap?listen?Port=7777?BossAI?BossHealth=1200 -log

:: 클라 자동 join (외부 -game 모드, PIE 회피)
"C:\UE5Source\Engine\Binaries\Win64\UnrealEditor.exe" "<프로젝트>\MultiActionGame.uproject" 127.0.0.1:7777?CharacterType=1 -game -log -ResX=1280 -ResY=720
```

### 런타임 판별 패턴

| 상황 | 쓸 API |
|---|---|
| 데디 서버 프로세스인지 | `IsRunningDedicatedServer()` |
| 현재 월드/액터가 데디 서버 네트모드인지 | `GetNetMode() == NM_DedicatedServer` |
| 네트워크 권한(authority) 있는지 (리슨 호스트 + 데디 둘 다) | `HasAuthority()` |
| 로컬 플레이어 소유의 PlayerController인지 | `IsLocalPlayerController()` |

"데디에서만 skip" 의도면 `IsRunningDedicatedServer()` 또는 `GetNetMode() == NM_DedicatedServer`를 쓴다. `HasAuthority()`는 리슨 서버에서도 true라 목적에 맞지 않는다.

### 디버깅 팁

- 서버 로그에 `[GameMode]` 같은 고정 태그를 붙여 클라 로그와 구분되게 출력
- `stat net` 콘솔 명령으로 대역폭/RPC 빈도 확인
- `p.NetShowCorrections 1` 으로 이동 보정 시각화 (캐릭터 움직임 테스트 시)
- 리슨과 데디의 동작이 갈라지는 의심이 들면 같은 맵/캐릭터를 양쪽으로 실행해 `UE_LOG`로 분기 추적

### 흔한 함정

- `UMultiGameInstance`의 매치 설정 필드를 성급히 삭제하지 말 것. 클라 입력 버퍼 역할은 여전히 필요하다 (§3-E)
- 서버 실행 시 URL 인코딩 주의: Windows cmd는 `?`를 그대로 받지만 PowerShell은 따옴표로 감싸야 함 (`"?BossAI?BossHealth=1000"`). Git Bash는 `/Game/...`을 자동 경로 변환해 깨지므로 데디 서버/클라 실행은 cmd 창에서 직접
- `ServerTravel` 호출 시점에 서버가 아직 `InitGame`에 도달하지 않아 클라가 붙으면 연결 실패. 기동 직후 몇 초는 대기하는 게 안전
- **World Partition + 데디**: UE5 신규 map은 World Partition이 기본 적용된다. 데디 서버에서 클라가 셀 visibility를 보고하는 시점에 서버가 그 셀을 스트림 인하지 못한 상태면 `MissingLevelPackage`로 끊긴다 (서버 streaming 옵션 / 타이밍 / streaming source 위치 문제). 단일 arena 게임이라 WP가 의미 없으면 `World Settings → Enable Streaming` 해제 후 재 cook으로 회피한다 (이 프로젝트가 채택한 길). WP를 의도적으로 쓰는 경우엔 `bIsServerStreamingEnabled` 등 데디 운영 설정을 별도로 정복해야 한다
- **PIE 클라로 데디 join 금지**: PIE는 메모리 패키지(`/Memory/UEDPIE_0_*`)를 만들어 진짜 데디와 호환되지 않는다. 클라 검증은 외부 `-game` 모드(§1-D)로
- `InitGame`과 `BeginPlay`의 실행 순서: `InitGame` → `PreInitializeComponents` → `BeginPlay`. 옵션 파싱은 `InitGame`에서 하고 `BeginPlay`는 이미 세팅된 멤버를 읽기만 하게 구성
