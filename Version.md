# Version 3.3.3 B262

**내부 빌드:** 3.3.3 B262 (2026-08-24) — 현재 배포 버전

> 버전 변경 시 `Src/Peripheral/define.h` 의 아래 항목을 함께 수정할 것
> ```c
> /* Version.md 의 버전과 동일하게 유지할 것 */
> #define FIRMWARE_VERSION    "3.3.3 B262"
> ```
>
> B263(플래시 사용자 데이터 보존 NOLOAD 수정)은 테스트 중이며 아직 미배포 상태 — 아래 변경 이력 참고

## 주요 기능

- 예약기능
- 데이터 백업기능
- 플래시 메모리 디버그 (초기화 문제)
- 문자 15회로 추가
- 센서 보정 (-5ppm)
- 내부플래시 저장 부분
- `t_data_index` 64로 제한
- 힙스택 사이즈 조절

---

## 3.3.3 B261 변경 사항 (2026-06-23)

### USB 로그 파일명

- 세션당 고정 파일명 사용 (매번 새 파일 생성 → 단일 파일 재사용)
- 파일명에 시리얼 번호 + 타임스탬프 포함
- 파일명 길이 단축
- USB 익스포트 인덱스 범위 초과 버그 수정
- 플래시 인덱스 쓰기 버그 수정

### SMS 밀도 보고

- -5 밀도 오프셋 제거
- 안티-지터 필터링 추가 (1ppm 단위 변화 허용, 히스테리시스 제거)
- 밀도 노멀라이즈 헬퍼 제거
- 블로어 팬 PWM 최대/최소 기본값 조정

### H2O2 센서

- EWMA 필터링 비활성화, SafetyPPM 제거
- 센서 보정 기본값: 2ppm → 5ppm → 0ppm (`ConstantH2O2SensorCalibration`)
  - 0ppm = 보정 없음. 현장에서 필요 시 플래시 설정으로 조정

### 보안 로그

- HH:MM:SS 포맷으로 인코딩 길이 수정
- 뷰어 호환을 위한 HH:MM:00 고정 포맷 유지
- 시간 문자열 인코딩 길이 20바이트로 고정

### 예약/타이머

- 예약 정지 시 타이머 값 손상 방지
- LCD 시간 설정 상태 처리 강화 및 설정 fallthrough 수정

### 플래시 저장 주기 및 타이머 보정

- 로그 저장 주기: 1분마다 → 5분마다 (`iFiveMinuteCounter` 기준)
- 플래시 erase (약 2초) 동안 정지된 시간만큼 타이머 자동 보정
  - 보정 대상: `uiEndTimeCounter`, `uiFinishTime`, `uiWaitTime[2]`
  - 보정값: `FLASH_SAVE_DELAY_COMPENSATION` = 200 centisec
- SterileProcess 종료 후 `Write_LogData_Flash` / `Write_Flash` 즉시 호출 추가

---

## 3.3.3 B262 변경 사항 (2026-08-24)

### ISR-메인루프 레이스 컨디션 하드닝

- TIM7 인터럽트(`Interrupt.c`)와 메인 루프(`process.c`, `lcd.c`)가 공유하는데 `volatile` 선언이 누락되어 있던 변수 5개에 `volatile` 추가
  - `ProcessMode`, `uiWaitTime[5]`, `uireservetime`, `PeristalticPumpOnOff_Flag`, `FinishTimeControl_Spary`
  - 컴파일러 최적화로 인해 ISR이 갱신한 값을 메인 루프가 캐시된 값으로 계속 읽는 문제 방지

### 플래시 RFID 데이터 손상 버그 수정

- `Read_Flash()`의 RFID 읽기 루프가 `j<5`로 되어 있어 `RFIDValue[5][4]` 배열 범위를 벗어나 `H2O2Volume[0]`의 바이트를 매번 덮어쓰던 문제 수정 (`j<4`로 정정)
  - flash에 값을 쓴 뒤 곧바로 `Read_Flash()`가 재호출되면서 RFID 1번 슬롯의 H2O2Volume 값이 매번 오염되고, 이후 재저장 시 오염된 값이 flash에도 전파될 수 있었음
  - 2025-10-23 RFID 기능 추가 커밋부터 존재하던 버그

---

## 3.3.3 B263 변경 사항 (2026-08-24) — 테스트 중, 미배포

### 플래시 사용자 데이터 보존 (링커 스크립트)

- `STM32F405RGTX_FLASH.ld`의 `.user_data`(Sector 6), `.user_log_data1~4`(Sector 7~10) 출력 섹션을 `(NOLOAD)`로 지정
  - 기존에는 이 섹션들이 0으로 채워진 PROGBITS로 `.elf`/`.hex`에 포함되어, 펌웨어를 다시 굽거나 디버그 다운로드할 때마다 Sector 6~10(설정값/로그 데이터)이 지워지는 문제가 있었음
  - `NOLOAD` 지정 후 해당 섹션이 `.hex`/`.bin`에서 제외되어, 코드 영역만 갱신되고 저장된 설정/로그 데이터가 보존됨
