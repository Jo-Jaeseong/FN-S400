# RTOS 적용 검토

## 현재 구조 요약
- 메인 루프에서 UART 수신 플래그와 주기 플래그를 폴링해 LCD 처리와 주기별 프로세스를 호출하고 있습니다.【F:Src/main.c†L171-L205】
- TIM7 주기 인터럽트에서 10ms/100ms/500ms/1s/1min 플래그를 생성하고, 타이머 카운터와 종료 타이머를 갱신합니다.【F:Src/Peripheral/Interrupt.c†L34-L88】
- UART 수신 완료 인터럽트에서 수신 플래그를 세트하고 다음 수신을 다시 걸어둡니다.【F:Src/Peripheral/Interrupt.c†L92-L96】
- I/O 동작 간 간격을 유지하기 위해 HAL_Delay를 사용하고 있어, 태스크 전환이 들어가면 블로킹 대기 방식으로 동작합니다.【F:Src/Peripheral/process.c†L86-L96】

## RTOS 전환 시 영향이 큰 구체 코드 지점
- **주기 함수들**: `CentiSecondProcess`, `DeliSecondProcess`, `HalfSecondProcess`, `OneSecondProcess`, `OneMinuteProcess`가 메인 루프에서 플래그 기반으로 호출됩니다. 이 구간은 RTOS에서 주기 태스크/소프트웨어 타이머로 직접 대응되는 핵심 후보입니다.【F:Src/main.c†L171-L196】
- **타이밍 생성기**: TIM7 ISR에서 플래그 세팅과 카운터 감소/증가가 모두 수행됩니다. RTOS 도입 시에도 동일한 정밀도를 유지해야 하므로, 소프트웨어 타이머 전환 시 분해 범위를 명확히 잡아야 합니다.【F:Src/Peripheral/Interrupt.c†L34-L88】
- **UART 수신 경로**: ISR이 플래그를 세트하고 다음 수신을 재설정합니다. RTOS 도입 시 ISR→큐/통지로 전환하지 않으면 우선순위 역전과 폴링 지연이 남습니다.【F:Src/Peripheral/Interrupt.c†L92-L96】
- **블로킹 지연**: `EnforceIoActionGap`에서 `HAL_Delay`를 사용해 최소 간격을 확보합니다. RTOS에서는 태스크 지연으로 교체하지 않으면 전체 시스템 응답성이 저하될 수 있습니다.【F:Src/Peripheral/process.c†L86-L96】

## RTOS 적용 시 핵심 고려사항
1. **주기 처리의 분해**
   - 현재는 TIM7 ISR에서 플래그를 세팅하고 메인 루프가 처리합니다. RTOS에서는 각 주기 작업을 **소프트웨어 타이머** 또는 **주기 태스크**로 분리하는 것이 자연스럽습니다.
2. **IRQ → 태스크 통지 전환**
   - UART 수신 플래그는 큐/세마포어/태스크 통지로 전환해 ISR에서 바로 태스크를 깨우는 구조가 적합합니다.
3. **블로킹 대기 제거**
   - `HAL_Delay`로 지연을 주는 구간은 태스크 지연(`vTaskDelay`/`vTaskDelayUntil`)로 대체해야 시스템 전체 지터와 우선순위 역전 위험을 줄일 수 있습니다.【F:Src/Peripheral/process.c†L86-L96】
4. **우선순위 및 스택 설계**
   - 제어 루프(펌프/히터/팬) → 통신(UART/LTE/USB) → UI/LCD 순으로 우선순위를 설계하고, 각 태스크 스택과 힙 크기를 사전에 산정해야 합니다.

## 태스크/타이머 분해 제안 (초기 구조)
| 기능 | 현 구조 | RTOS 전환 제안 |
| --- | --- | --- |
| UART 수신 처리 | 플래그 폴링 | UART ISR → 큐/태스크 통지 → `CommTask` |
| 10ms/100ms/500ms/1s/1min 처리 | TIM7 플래그 | 소프트웨어 타이머 또는 주기 태스크 |
| LCD 처리 | 메인 루프 호출 | `UiTask` (이벤트 기반) |
| 공정 제어(센서/펌프 등) | 주기 함수 | `ControlTask` (가장 높은 우선순위) |
| USB Host 프로세스 | 메인 루프 | `UsbTask` 또는 저우선 주기 태스크 |

## 이행 단계(추천)
1. **CubeMX에서 FreeRTOS 활성화** 후 HAL 드라이버/미들웨어가 충돌하지 않도록 SysTick/TIM7 사용 정책을 확정합니다.
2. **메인 루프의 주기 함수**(CentiSecond/DeliSecond/HalfSecond/OneSecond/OneMinute)를 각각 태스크 또는 소프트웨어 타이머 콜백으로 분리합니다.【F:Src/main.c†L171-L196】
3. **UART 수신 플래그 제거**: ISR에서 큐로 데이터 전달 후 통신 태스크에서 파싱/처리하도록 변경합니다.【F:Src/Peripheral/Interrupt.c†L92-L96】
4. **HAL_Delay 제거**: 태스크 지연으로 전환하고, 필요한 경우 뮤텍스로 공유자원 보호를 추가합니다.【F:Src/Peripheral/process.c†L86-L96】
5. **TIM7 기반 로직 재검토**: 카운터/플래그 생성 로직을 그대로 유지할지, RTOS 타이머로 이관할지 결정하고, 이관 시 카운터 감소/증가 로직을 각각의 주기 태스크로 분해합니다.【F:Src/Peripheral/Interrupt.c†L34-L88】

## 리스크 및 점검 항목
- **타이밍 정밀도**: TIM7 기반 시간 누적과 RTOS tick 간의 정합성 확인 필요.【F:Src/Peripheral/Interrupt.c†L34-L88】
- **우선순위 역전**: 공정 제어 태스크가 통신/UI에 의해 지연되지 않도록 우선순위 설계 필요.
- **스택/힙 부족**: LCD/USB/파일시스템 태스크는 스택 사용량이 많을 수 있어 별도 산정 필요.
- **공유 자원 보호**: ISR에서 갱신되는 카운터/플래그와 태스크 간 공유 데이터의 경합 여부를 점검해야 합니다.【F:Src/Peripheral/Interrupt.c†L34-L88】

## 추가 제안
- RTOS 적용 전, 각 주기 함수의 실행 시간 측정(프로파일링)을 통해 주기별 최대 실행 시간을 파악하는 것이 안전합니다.
- 향후 이벤트 기반 구조로 전환하면 불필요한 폴링을 줄이고 전력/응답성을 개선할 수 있습니다.
