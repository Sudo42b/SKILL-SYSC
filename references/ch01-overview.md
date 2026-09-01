# Clause 1 — Overview

LRM pp. 23–25. 표준 자체의 적용 범위·용어 강도·C++ 기반선.

## 1.1–1.2 Scope / Purpose

SystemC = **ISO 표준 C++ 클래스 라이브러리** (system/hardware design + TLM). 새 언어 아님.
LRM은 구현자용 정의서지 튜토리얼 아님 (§1.2).

## 1.3 Word usage — 규칙 인용 시 강도 구분

| 단어 | 뜻 | 코드 리뷰 시 처리 |
|---|---|---|
| **shall** | 필수, 이탈 불가 | 위반 = 비적합 코드. 반드시 고침 |
| **should** | 권장 | 이유 있으면 이탈 가능. 근거 남길 것 |
| **may** | 허용 | 선택지. 강제 아님 |
| **can** | 가능/능력 서술 | 규칙 아님 |

`must`/`will`은 폐기됨 — 요구사항 서술에 쓰지 말 것.

> 인용 시 항상 원문 강도를 유지하라. "shall" 규칙을 "권장"으로 약화시키지 말고, "should"를 "필수"로 강화하지 말 것.

## 1.4 Subsets

벤더가 부분 구현해도 "SystemC" 이름 사용 가능 (부분 준수 주장). TLM-2.0 준수는 §9.2 참조.
→ 특정 벤더 시뮬레이터 대상 코드라면 지원 서브셋 확인 필요.

## 1.5 C++ 표준과의 관계

- **baseline = C++17** (ISO/IEC 14882:2017). 구현은 최소 C++17 지원 shall.
- 애플리케이션은 C++17 기능 자유롭게 사용 가능. 단 SystemC 기능을 쓸 때는 이 표준의 규칙·제약을 따라야 shall.
- 클래스 라이브러리 확장은 C++ 메커니즘으로 자유롭게 가능 (표준 위반 없는 한).
- C++ 문법상 적법하지만 LRM 위반인 프로그램이 존재 가능. 구현이 모든 위반을 검출할 의무 없음.

→ **함의**: 컴파일 통과 ≠ LRM 적합. 정적 검토가 필요한 이유.

## 1.6 문서 구조 (읽는 순서)

| 범위 | 내용 |
|---|---|
| Clause 3 | 용어·규약. 정확한 해석에 필수 |
| Clause 4 | 커널 동작 (elaboration/simulation). **모든 클래스 의미론의 기반** |
| Clause 5–8 | SystemC 클래스 라이브러리 public interface |
| Clause 9–16 | TLM-2.0 (interoperability layer + utilities) |
| Clause 17 | TLM-1 message passing, tlm_fifo, analysis ports |
| Annex A/B/C/D | 용어집 / 입문 / deprecated / 2011→2023 변경 |

Clause 5–8의 각 클래스는 다음 형식으로 기술됨 (§1.6):

1. 클래스 정의의 C++ 소스 리스팅
2. 클래스와 멤버 사용에 대한 제약 (constraints on usage)
3. 클래스와 멤버의 의미론 (semantics)
4. 관련 함수·typedef·매크로 설명
5. 정보성 예제 (typical + atypical)

→ 이 스킬의 `references/ch05`~`ch08`도 같은 순서를 따른다.
