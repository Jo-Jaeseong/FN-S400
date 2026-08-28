# Version 3.3.3 B263

**내부 빌드:** 3.3.3 B263 (2026-08-28) — 현재 배포 버전

> 버전 변경 시 `Src/Peripheral/define.h` 의 아래 항목을 함께 수정할 것
> ```c
> /* Version.md 의 버전과 동일하게 유지할 것 */
> #define FIRMWARE_VERSION    "3.3.3 B263"
> ```

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

## 3.3.3 B263 변경 사항 (2026-08-28)

### 플래시 설정값 volatile 누락 (설정이 저장 안 되는 문제)

- `userConfig`, `userLogData1~4` (`flash.c`)가 `const`로만 선언되어 있어, GCC 12 툴체인에서 `-O0`에서도 읽기 코드가 실제 플래시 값 대신 컴파일 시점 초기값으로 상수 치환(constant folding)됨
  - `Write_Flash()`로 실제 플래시에 값을 써도 `Read_Flash()`가 항상 초기값을 반환 → USB 보안 on/off 등 설정이 LCD에서 바꿔도 반영 안 되는 것처럼 보이는 문제
  - 예전 GCC 7 툴체인에서는 문제 없었으나 GCC 12로 넘어오며 드러난 잠재 버그
  - `volatile const`로 수정하여 항상 실제 메모리를 읽도록 강제

### ISR-메인루프 공유 변수 volatile 누락

- `TestTime` (`lcd.c`/`Interrupt.c`/`process.c`)에 `volatile` 누락 — TIM7 인터럽트가 갱신하는 값을 메인 루프/화면 표시 코드가 캐시된 값으로 읽을 수 있던 문제 수정

### RFID 관련 버그

- `RFIDData.CurrentRFIDValue`(4바이트 배열)에 `sprintf(..., "%c%c%c%c", ...)`로 값을 쓰면서 널 종단문자까지 5바이트를 써서, 바로 뒤의 `RFIDValue[0][0]`을 매번 0으로 덮어쓰던 문제 수정 (`rfid.c`, `process.c`) — 직접 바이트 대입으로 변경
- `RFIDCompare()`에서 `(unsigned char *)`로 캐스팅한 뒤 포인터 비교하던 미정의 동작(UB) 코드를 일반 값 비교로 수정 (`process.c`) — 이 컴파일러/옵션에서는 우연히 정상 동작했지만 근본적으로 위험한 코드였음
- RFID 카드 헤더 검증 로직이 `&&`로 되어 있어 3바이트 중 하나라도 우연히 일치하면 유효 카드로 오인식하던 문제 수정 (`rfid.c`) — `||`로 정정(De Morgan)

### 배열 경계 초과로 인한 메모리 오염 (반복 발생)

- `GetDensity()`의 `arrDensity[5]` 배열에 인덱스 범위 체크가 없어, 5초 주기 리셋 전까지 계속 증가하며 배열 밖(인접한 `index`, `avgmax`, `H2O2Sensor_Flag` 등)을 반복적으로 덮어쓰던 문제 수정 (`adc.c`) — 순환 인덱스(`index % 5`)로 변경
- `OverHeatTempCheck()`의 `tempArr[10]` 배열도 동일한 패턴의 경계 초과 버그(`tempIndex>10`이 `tempIndex>=10`이어야 함) 수정 (`i2c.c`)

### LCD 입력값 검증 로직 오타

- device_version==8 기종의 InjectionPerMinute2 입력값 검증 조건이 `value==3||value==4||value||5`로 되어 있어 항상 참으로 평가되던 오타 수정 (`lcd.c`) — `value==5`로 정정

### 연동 세정펌프 카운터 정리

- `PeristalticPumpCnt`(펌프 사용 여부 판단 플래그, 1회 사용 후 재부팅을 요구하는 의도된 설계)가 켤 때마다 계속 증가만 하던 것을 `=1`로 고정 — 동작(재부팅 요구)은 그대로 유지하면서 값이 무한정 커지는 것만 방지 (`PeristalticPump.c`)

### SMS 전송 버퍼 크기 확장

- `LTE_Modem.c`의 여러 `sprintf` 호출이 40~70바이트 버퍼에 실제로는 더 긴 문자열을 조합해 넣어 스택 버퍼 오버플로우 위험이 있던 문제 수정 — 관련 버퍼를 100바이트로 통일

### LCD 버전 표시 방식 변경

- `DisplayVersion()`이 문자 3개(`ch1,ch2,ch3`)를 받아 "3.3.3" 형태로만 표시하던 방식에서, `FIRMWARE_VERSION` 문자열 전체(빌드 번호 포함, 예: "3.3.3 B263")를 받아 표시하도록 변경 (`lcd.c`, `lcd.h`)
