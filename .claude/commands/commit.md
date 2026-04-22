---
description: 현재 git 변경분과 Docs/CommitGuide.md를 참고해 커밋 메시지를 제안한다
allowed-tools: Bash(git status:*), Bash(git diff:*), Bash(git log:*), Read
---

현재 저장소의 변경분을 분석해 커밋 메시지를 추천해줘.

## 단계

1. 다음 명령을 병렬로 실행해 변경 상태를 파악한다.
   - `git status`
   - `git diff` (워킹 트리)
   - `git diff --staged` (스테이지된 변경)
   - `git log --oneline -5` (최근 커밋 톤 참고)
2. [Docs/CommitGuide.md](Docs/CommitGuide.md)를 읽어 규칙을 확인한다.
3. 규칙에 따라:
   - 변경 의미를 한두 줄로 요약한다.
   - 목적이 다른 변경이 섞여 있으면 커밋 분리를 제안한다.
   - 최종 제안은 코드 펜스 안에 "제목 → 빈 줄 → 본문 불릿" 줄글로만 제시한다.
4. 사용자가 승인하기 전에는 **실제 커밋을 만들지 않는다.**
