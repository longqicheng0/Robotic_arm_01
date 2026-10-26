# ESP-IDF Robotic Arm Learning Roadmap

This file is the persistent source of truth for learning progress and verified implementation status. A checked box means the item was completed **and verified**. Generated code, a successful build, a successful flash, serial output, and physical motion are separate forms of evidence.

## Progress summary

| Phase | Goal | Status | Verified evidence | Next milestone |
|---|---|---|---|---|
| 1. ESP-IDF Fundamentals | Understand project structure, build, execution, and logs | **In progress** | Workspace inspected; minimal source/build scaffold reviewed locally | Activate an ESP-IDF environment and report `idf.py --version` |
| 2. PWM and Servo Hardware Control | Drive five servo outputs with LEDC | Not started | None | Complete Phase 1 |
| 3. Modular C Software Design | Separate types, configuration, and interfaces | Not started | None | Complete Phase 2 |
| 4. Motion Control Fundamentals | Enforce limits and generate smooth targets | Not started | None | Complete Phase 3 |
| 5. FreeRTOS Fundamentals and Integration | Separate input and motion using a queue | Not started | None | Complete Phase 4 |
| 6. PS5 DualSense Integration | Translate wireless input into existing commands | Not started; compatibility gate provisionally passed | User-provided schematic identifies ESP32-WROOM-32UE; official datasheet confirms BR/EDR | Physically corroborate module and external antenna before Bluetooth integration |
| 7. System Integration and Verification | Validate and document the complete system | Not started | None | Complete Phases 1–6 |

## Project boundaries and evidence rules

- Four logical joints: base, shoulder, elbow, and gripper.
- Five physical actuator outputs: base, shoulder A, shoulder B, elbow, and gripper.
- Initial input: USB serial. Final input: PS5 DualSense through Bluepad32, only after confirming a Bluetooth Classic-capable target.
- Keep the architecture minimal: input module -> validated command -> queue -> motion task -> servo driver -> five outputs.
- Do not add ROS 2, Jetson integration, inverse kinematics, or advanced control algorithms during this roadmap.
- Never energize servos from an unverified supply or assume the ESP32 board can power them.
- Never check a hardware item without observed evidence recorded in the verification log.
- A compilation result proves only that the selected source/configuration compiled. It does not prove that the firmware was flashed or that hardware behaved correctly.

## Checkpoint ledger

- **Current phase:** Phase 1 — ESP-IDF Fundamentals
- **Current micro-goal:** Confirm VS Code reloads the generated ESP-IDF IntelliSense configuration and resolves `esp_log.h`.
- **Current file:** Project-wide environment; no learner code edit yet.
- **Known state:** The repository contained blank placeholder build/source files. The minimal Phase 1 scaffold now exists. `idf.py` and `xtensa-esp32-elf-gcc` were not found in the current shell or common local paths.
- **Pending learner evidence:** After reloading VS Code, the editor diagnostic for `esp_log.h` disappears; temporary `ESP_LOGE` exercise resumes afterward.
- **Hardware state:** User-provided schematic identifies ESP32-WROOM-32UE; physical module marking, external antenna, USB serial port, flash result, logs, and all physical motion remain unverified.

---

## Phase 1 — ESP-IDF Fundamentals

### 1. Learning objectives

- Explain what ESP-IDF provides and how it differs from the Arduino abstraction layer.
- Trace application startup from bootloader and ESP-IDF initialization to `app_main()`.
- Identify the role of root `CMakeLists.txt`, component `CMakeLists.txt`, C sources, and headers.
- Explain generated `sdkconfig` versus version-controlled `sdkconfig.defaults`.
- Configure a target, build firmware, flash it, monitor logs, and diagnose basic failures.
- Use `ESP_LOGI`, `ESP_LOGW`, and `ESP_LOGE` according to severity.

### 2. Prerequisite knowledge

- Navigate directories in a terminal and run a command.
- Recognize a C function, include directive, string, and statement terminator.
- Know that compiling source code and running it on hardware are separate steps.

### 3. Key concepts and explanations

- **ESP-IDF:** Espressif's native C/C++ SDK. It supplies the startup code, hardware drivers, FreeRTOS integration, configuration system, build tools, bootloader, partition table, and flashing/monitoring tools.
- **ESP-IDF versus Arduino:** Arduino emphasizes a simplified `setup()`/`loop()` programming model and portable high-level libraries. ESP-IDF exposes Espressif's native component, configuration, driver, and RTOS APIs. Arduino can itself run as an ESP-IDF component, but this project uses native ESP-IDF.
- **Lifecycle:** ROM code starts first, then the second-stage bootloader selects and loads an application, ESP-IDF initializes system services and the scheduler, and the ESP-IDF main task calls `app_main()`. If `app_main()` returns, that main task can delete itself; the system does not become an Arduino-style implicit loop.
- **Translation unit:** A `.c` file plus its included headers is compiled into an object file. Headers normally declare interfaces and shared types; `.c` files define behavior and private state.
- **Component:** An ESP-IDF build unit containing source files, include paths, and declared dependencies. The `main` directory is a special component automatically included by the project build.
- **CMake layers:** The root file names and initializes the project. `main/CMakeLists.txt` registers the source files and public include paths for the `main` component.
- **Configuration:** `sdkconfig` is the generated, active project configuration and may vary by local target. `sdkconfig.defaults` stores intentional baseline defaults that should be reproducible. Defaults do not override values already present in an existing `sdkconfig`.
- **Logging:** `ESP_LOGI` records normal milestones, `ESP_LOGW` records recoverable or suspicious conditions, and `ESP_LOGE` records failures. The tag identifies the module producing the message.
- **Debug loop:** Observe the exact error -> classify environment/build/flash/runtime/hardware -> change one thing -> repeat the smallest relevant check -> record evidence.

Minimum execution flow:

```text
reset/power-on
    -> ROM boot code
    -> ESP-IDF bootloader
    -> ESP-IDF system + FreeRTOS initialization
    -> main task calls app_main()
    -> application code logs and later starts application work
```

### 4. Relevant project files

- `CMakeLists.txt` — root ESP-IDF project declaration.
- `main/CMakeLists.txt` — registers Phase 1 source with the `main` component.
- `main/app_main.c` — application entry point and first logs.
- `sdkconfig.defaults` — future version-controlled configuration defaults; intentionally empty initially.
- Generated later: `sdkconfig` and `build/`.

### 5. Detailed implementation checklist

#### Environment and identity

- [x] Inspect existing repository files before editing. Evidence: all project source/build placeholders were one blank line on 2026-10-25.
- [x] Check whether `idf.py`, the toolchain, `IDF_PATH`, or a common local ESP-IDF installation is visible. Evidence: none was found in the inspected shell on 2026-10-25.
- [x] Install and verify Installation Manager prerequisites. Evidence: CMake 4.4.4 and `dfu-util` 0.11 were found under `/opt/homebrew/bin` on 2026-10-25.
- [x] Pass the EIM CLI Python sanity checks. Evidence: Python version, pip, venv, standard library, ctypes, and SSL/HTTPS all reported `[PASS]` on 2026-10-26.
- [x] Install and independently activate a supported ESP-IDF environment. Evidence: EIM activation script loaded ESP-IDF v5.5.6 and its tools on 2026-10-26.
- [x] Record `idf.py --version` output. Evidence: `ESP-IDF v5.5.6` in an activated verification shell; learner-terminal activation remains the current checkpoint.
- [x] Identify the module variant from design evidence. Evidence: user-provided schematic labels M2 as `ESP32-WROOM-32UE` on 2026-10-26.
- [ ] Corroborate that the physically installed module marking is ESP32-WROOM-32UE; flash-tool evidence has confirmed its internal ESP32-D0WD-V3 chip family but not the module/antenna suffix.
- [x] Record the ESP-IDF target as `esp32`, verified against the module datasheet and flash-tool detection of ESP32-D0WD-V3 revision 3.1.
- [x] Identify the serial port without guessing. Evidence: `/dev/cu.usbserial-110` appeared after the board was connected and its character-device node was verified on 2026-10-26.

#### Minimum project

- [x] Create/verify the root ESP-IDF `CMakeLists.txt`.
- [x] Create/verify `main/CMakeLists.txt` registering only `app_main.c`.
- [x] Create/verify a minimal `main/app_main.c` with informational and warning logs.
- [x] Keep future-stage placeholder modules out of the current component build.
- [ ] Explain each line of both CMake files without referring to notes.
- [x] Explain who calls `app_main()` and what happens if it returns. Evidence: learner correctly identified that only the main task ends when `app_main()` returns; the ESP32 system and FreeRTOS scheduler continue.

#### Configure, build, flash, and observe

- [x] Run `idf.py set-target esp32` after identifying the module from the schematic. Evidence: generated `sdkconfig` records `CONFIG_IDF_TARGET="esp32"` and Xtensa architecture on 2026-10-26.
- [x] Save the verified non-default 4 MB flash choice into tracked `sdkconfig.defaults`. Evidence: minimal defaults file contains only `CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y` plus generated comments.
- [x] Run `idf.py build` successfully. Evidence: 1027 build steps completed and `Project build complete` was reported on 2026-10-26.
- [x] Record the selected target and ESP-IDF version used for the successful build. Evidence: target `esp32`, ESP-IDF v5.5.6.
- [x] Identify the generated bootloader, partition table, application binary, and ELF output in `build/`. Evidence: application binary 162,256 bytes, Xtensa ELF 2,869,352 bytes, bootloader 26,000 bytes, partition table 3,072 bytes.
- [x] Connect the board and identify its serial port. Evidence: learner reported `/dev/cu.usbserial-110`; it was absent from the pre-connection inventory and present afterward.
- [x] Flash with `idf.py -p /dev/cu.usbserial-110 flash`. Evidence: bootloader, application, and partition table writes all passed hash verification on 2026-10-26.
- [x] Open the monitor with `idf.py -p /dev/cu.usbserial-110 monitor`.
- [x] Observe the exact `arm_app` information and warning messages after reset. Evidence: both expected messages appeared immediately after ESP-IDF called `app_main()` on 2026-10-26.
- [x] Exit the monitor using its documented shortcut/menu sequence. Evidence: learner confirmed exit on 2026-10-26.
- [ ] Add one truthful `ESP_LOGE` test message, observe it, then remove it so the normal application does not falsely report a failure.
- [ ] Make one intentional source edit and confirm that the incremental build recompiles the relevant source.

#### Debug workflow

- [ ] Distinguish “command not found” from compilation, linking, flashing, and runtime errors.
- [ ] Cause and fix one safe build error, such as a missing semicolon.
- [ ] Record the first useful compiler diagnostic rather than only the final build-failed line.
- [ ] Explain why a clean build is usually unnecessary for a normal source edit.

### 6. Small hands-on exercises

1. **Environment checkpoint:** In an ESP-IDF-enabled terminal, run `idf.py --version` and explain whether that proves the board is connected. It does not; it proves only that the tool frontend is available.
2. **Build-graph prediction:** Before building, predict which `.c` file is registered. Confirm it from `main/CMakeLists.txt`.
3. **Log severity:** Write one accurate example situation for info, warning, and error severity.
4. **Execution reasoning:** Predict whether the two Phase 1 logs appear once or repeatedly and explain why.
5. **Controlled compile failure:** Temporarily remove one semicolon, build, locate the first diagnostic, restore it, and rebuild.

### 7. Testing and verification procedures

```sh
# Run from the project root in an ESP-IDF-enabled shell.
idf.py --version
idf.py set-target <VERIFIED_TARGET>
idf.py build
idf.py -p <VERIFIED_PORT> flash
idf.py -p <VERIFIED_PORT> monitor
```

Expected evidence:

- Environment: `idf.py --version` prints a specific ESP-IDF version.
- Build: command exits successfully and creates application, bootloader, and partition-table artifacts.
- Flash: tool identifies the connected chip and completes writing/verifying.
- Runtime: monitor shows `arm_app` and both Phase 1 messages after reset.
- Understanding: learner can explain the three project files and lifecycle in their own words.

### 8. Completion criteria (definition of done)

- [ ] ESP-IDF version, exact target, and serial port are recorded.
- [ ] Project builds from an ESP-IDF-enabled shell.
- [ ] The identified physical board is flashed successfully.
- [ ] Expected logs are observed in the serial monitor.
- [ ] One controlled build error is diagnosed and repaired.
- [ ] Learner independently explains build files, `app_main()` execution, configuration, and the build/flash/monitor distinction.

### 9. Common mistakes and debugging tips

- `idf.py: command not found`: the ESP-IDF environment was not installed or activated in this terminal.
- `$IDF_PATH` empty: use the official export/activation method for the installed version; do not guess a path.
- Wrong target: delete no files blindly. Verify the chip, then use `idf.py set-target`; note that changing target regenerates configuration/build state.
- Build includes unwanted files: inspect the `SRCS` list in the component CMake file.
- Flash cannot open port: check port name, cable data capability, permissions, and whether another monitor owns the port.
- Boot loops or garbage serial: capture the complete startup log and verify board/clock/baud/configuration before changing application logic.
- No application log: distinguish failed flash, wrong port, reset/boot mode, filtered log level, and an application that did not reach the log statement.
- Avoid using `ESP_LOGE` for ordinary status; false errors make later diagnosis unreliable.

### 10. Official learning resources

- [ESP-IDF Get Started](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)
- [Start a project on Linux and macOS](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-start-project.html)
- [Build system guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html)
- [`idf.py` frontend](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-py.html)
- [IDF Monitor](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-monitor.html)
- [Logging library](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html)
- [Project configuration](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/kconfig.html)

### 11. Knowledge-check questions

1. What does ESP-IDF initialize before it calls `app_main()`?
2. Why is `app_main()` not the processor's literal reset entry point?
3. What different decisions belong in the root and component CMake files?
4. What is the difference between `sdkconfig` and `sdkconfig.defaults`?
5. What does a successful build prove, and what does it not prove?
6. Which log level belongs to a recoverable calibration fallback, and why?
7. If a log is absent, what evidence separates a flash problem from a runtime path problem?

---

## Phase 2 — PWM and Servo Hardware Control

### 1. Learning objectives

- Relate PWM period, frequency, duty count, and high-pulse width.
- Configure one ESP-IDF LEDC timer and independent channels.
- Calibrate safe pulse bounds per physical servo.
- Build a small servo driver that can address five outputs independently.
- Apply electrical power and grounding rules before physical testing.

### 2. Prerequisite knowledge

- Phase 1 build/flash/log workflow.
- Basic voltage, current, ground reference, and GPIO output concepts.
- Integer ranges, units, and function arguments in C.

### 3. Key concepts and explanations

- **GPIO:** A numbered digital pin with chip-specific capabilities and restrictions. A board label is not always identical to the underlying GPIO number.
- **PWM:** A repeating digital signal. Frequency is repetitions per second; period is `1 / frequency`; duty cycle is high-time divided by period.
- **Servo signal:** Common hobby servos interpret a repeated high pulse as a requested position. Nominal values such as 50 Hz and roughly 1–2 ms are starting conventions, not permission to use unsafe endpoints.
- **LEDC:** ESP32 hardware PWM. A timer establishes frequency/resolution; channels select duty and GPIO routing. Channels sharing a timer share its timing basis.
- **Quantization:** With resolution `N`, one period has approximately `2^N` duty steps. Pulse counts are derived from pulse width and period; unit clarity prevents overflow and mapping mistakes.
- **Power integrity:** Servo current must come from a suitable external supply. The external supply ground and ESP32 ground need a common reference. Brownouts and resets often indicate power/wiring problems, not faulty PWM code.
- **Calibration:** Individual mechanical assemblies may require different direction, offset, and min/max pulse widths. Start near neutral with the horn/linkage made safe, then expand conservatively.

### 4. Relevant project files

- Future `main/servo_driver.c` and `main/servo_driver.h`.
- `main/arm_config.h` for pins and calibration after Phase 3 organization.
- `main/app_main.c` for a temporary, guarded one-servo bench test.
- `sdkconfig.defaults` only for intentional non-default configuration.

### 5. Detailed implementation checklist

- [ ] Draw and review power, signal, and common-ground wiring before energizing a servo.
- [ ] Record verified board GPIO constraints and chosen candidate output pin.
- [ ] Calculate the period in microseconds for the chosen servo frequency.
- [ ] Calculate duty counts for neutral pulse width at the chosen resolution.
- [ ] Configure one LEDC timer and explain each field.
- [ ] Configure one LEDC channel with output initially disabled or at a reviewed neutral value.
- [ ] Verify PWM first without a linkage where practical.
- [ ] Command one servo to a conservative neutral pulse.
- [ ] Determine safe preliminary pulse bounds without forcing a mechanical stop.
- [ ] Define a minimal servo driver interface using explicit channel and pulse/angle units.
- [ ] Move LEDC details behind `servo_driver.c/.h`.
- [ ] Add five physical channel configurations only after pin review.
- [ ] Test each output individually with other channels inactive.
- [ ] Record per-servo direction, neutral, minimum, and maximum evidence.
- [ ] Verify that the two shoulder actuators are not mechanically driven against each other before coordinated tests.

### 6. Small hands-on exercises

1. Convert 50 Hz to period in milliseconds and microseconds.
2. Convert a neutral pulse width to duty percentage and timer counts.
3. Explain why duty percentage alone is ambiguous without frequency.
4. Write a pure calculation helper for pulse microseconds to duty counts.
5. Review one LEDC configuration structure and label timing versus channel fields.

### 7. Testing and verification procedures

- Build-only test before connecting servo power.
- With power off, continuity/wiring review and common-ground confirmation.
- If available, use a logic analyzer/oscilloscope to verify frequency and pulse width.
- First powered test: one unloaded servo, current-limited suitable supply, conservative neutral request, accessible power cutoff.
- Record commanded pulse, measured signal if available, observed motion, resets/brownouts, heating/noise, and supply behavior.
- Repeat independently for all five outputs; never infer one channel from another.

### 8. Completion criteria (definition of done)

- [ ] Driver API controls five identified physical outputs independently.
- [ ] PWM frequency and pulse-width calculations are explained and tested.
- [ ] Each output has a verified safe preliminary calibration range.
- [ ] No channel causes ESP32 resets or unsafe power behavior during the defined bench test.
- [ ] Physical results for all five servos are recorded from observation.

### 9. Common mistakes and debugging tips

- Powering several servos from the development board regulator.
- Missing common ground between signal source and servo supply.
- Confusing microseconds, milliseconds, duty percent, and raw counts.
- Selecting input-only, boot-strapping, flash-connected, or unavailable pins without checking the exact target/module.
- Reusing an LEDC channel or timer incompatibly.
- Starting at extreme pulses or testing with constrained linkages.
- Treating audible buzzing as normal instead of checking load, calibration, and power.

### 10. Official learning resources

- [ESP-IDF GPIO driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html)
- [ESP-IDF LEDC driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html)
- Exact servo datasheets and exact ESP32 module/board schematic must be added after hardware identification.

### 11. Knowledge-check questions

1. At 50 Hz, how long is one PWM period?
2. Why can two pulse widths with the same duty count mean different things under different timer settings?
3. What evidence establishes a safe endpoint?
4. Why should timer configuration be shared while channel calibration remains individual?
5. What symptoms suggest supply droop or missing common ground?

---

## Phase 3 — Modular C Software Design

### 1. Learning objectives

- Design small C modules with clear ownership and dependencies.
- Separate logical joints from physical actuators.
- Centralize hardware mappings and calibration without coupling motion policy to pins.
- Use enums, structs, header guards, and file-local functions correctly.

### 2. Prerequisite knowledge

- C functions, arrays, pointers, structs, enums, integer/floating types, and `const`.
- Phase 2's verified driver behavior and calibrations.
- The difference between compile-time configuration and runtime state.

### 3. Key concepts and explanations

- Headers publish the smallest stable interface; source files own implementation details.
- Header guards prevent repeated declarations during preprocessing.
- `static` at file scope keeps helpers/private data inside a module and reduces accidental coupling.
- Enums provide readable IDs, but array indexing requires range validation.
- Structs group values that belong together and make units/meaning explicit through names.
- Configuration describes the hardware and safe limits; state describes what the system is doing now.
- One logical shoulder command must map through separate calibration to two physical outputs because mounting may require different offsets or reversed directions.

### 4. Relevant project files

- `main/arm_types.h`
- `main/arm_config.h`
- `main/servo_driver.c/.h`
- `main/app_main.c`
- `main/CMakeLists.txt`

### 5. Detailed implementation checklist

- [ ] Inspect current placeholders and preserve verified Phase 2 behavior.
- [ ] Define an enum for four logical joint IDs with a count sentinel.
- [ ] Define an enum or stable indexing method for five physical servo IDs.
- [ ] Define explicit angle/pulse/calibration structs with documented units.
- [ ] Add correct header guards or `#pragma once` according to project convention.
- [ ] Put verified GPIO and calibration constants in `arm_config.h`.
- [ ] Keep private LEDC helpers `static` in `servo_driver.c`.
- [ ] Make invalid IDs and out-of-range requests return/report an error.
- [ ] Implement separate shoulder A and B mapping from one logical shoulder target.
- [ ] Draw the include/dependency graph and remove circular dependencies.
- [ ] Update `main/CMakeLists.txt` only with modules actually compiled.
- [ ] Compile with warnings reviewed, not ignored.

### 6. Small hands-on exercises

1. Move one declaration from a `.c` file to a header and explain why callers need it.
2. Turn one implementation helper into a file-local `static` function.
3. Define logical and physical counts and test every valid/invalid boundary.
4. Map three sample shoulder angles through both physical calibration records by hand.
5. Change one pin constant and identify which modules should and should not change.

### 7. Testing and verification procedures

- Compile each module through the ESP-IDF build rather than separately guessing include paths.
- Use compile-time assertions where appropriate for array size/count agreement.
- Test mapping functions with minimum, neutral, maximum, and invalid inputs.
- Inspect dependency output/include graph if a circular include occurs.
- Re-run the Phase 2 physical-channel test after refactoring; a refactor must preserve behavior.

### 8. Completion criteria (definition of done)

- [ ] Four logical joints and five physical outputs are explicit and cannot be confused silently.
- [ ] High-level code contains no raw GPIO, LEDC channel, or pulse calibration values.
- [ ] Shoulder outputs use independent calibration from one logical request.
- [ ] Module headers expose only needed interfaces and build without warnings under the chosen settings.
- [ ] Phase 2 behavior is reverified after modularization.

### 9. Common mistakes and debugging tips

- Defining storage in a header and causing multiple-definition link errors.
- Omitting units from fields such as `min`, `position`, or `period`.
- Using enum values as indices without checking range/count.
- Putting every function in a global header instead of keeping helpers private.
- Creating circular includes when a forward declaration or smaller shared type header is enough.
- Assuming mirrored shoulder geometry means identical pulse direction.

### 10. Official learning resources

- [ESP-IDF build-system component requirements](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html)
- [SEI CERT C Coding Standard](https://wiki.sei.cmu.edu/confluence/display/c/SEI+CERT+C+Coding+Standard)
- Exact compiler diagnostics/reference will be tied to the installed ESP-IDF toolchain version.

### 11. Knowledge-check questions

1. When should a declaration be in a header?
2. What does file-scope `static` protect?
3. Why are logical joints not equivalent to physical outputs in this arm?
4. Which values belong to configuration and which belong to live state?
5. How would a compile/link error reveal a header design mistake?

---

## Phase 4 — Motion Control Fundamentals

### 1. Learning objectives

- Represent arm poses and commands with explicit units.
- Enforce software limits before target generation.
- Update motion at a fixed discrete interval with a rate limit.
- Coordinate two physical shoulder actuators from one logical trajectory.

### 2. Prerequisite knowledge

- Phase 3 data types, configuration, and mapping.
- Difference between position, velocity, and elapsed time.
- Basic arithmetic, clipping, and sign handling.

### 3. Key concepts and explanations

- A position command asks for an angle; a velocity command asks how fast position should change.
- Software joint limits constrain logical motion before hardware-specific mapping.
- In discrete time, each update advances by at most `speed * dt`; use measured or controlled timing units consistently.
- A rate limiter selects the target directly when it is close, otherwise advances by a signed maximum step to avoid overshoot.
- Linear interpolation generates values between endpoints, but a periodic rate limiter is often simpler for interactive commands.
- Shoulder coordination means both outputs are derived in the same update from the same clamped logical shoulder state.

### 4. Relevant project files

- `main/arm_types.h`
- `main/arm_config.h`
- `main/motion_task.c/.h` (logic first; task creation waits for Phase 5)
- `main/servo_driver.c/.h`
- `main/app_main.c` for temporary predefined-pose tests.

### 5. Detailed implementation checklist

- [ ] Define an arm pose containing four logical joint positions with explicit units.
- [ ] Define per-joint minimum, maximum, default, and maximum rate.
- [ ] Implement/test a pure clamp helper.
- [ ] Implement/test one-step rate limiting using current, target, rate, and `dt`.
- [ ] Handle zero/negative `dt` and invalid rates deliberately.
- [ ] Prevent numerical overshoot near a target.
- [ ] Update all four logical joints from one coherent input snapshot.
- [ ] Convert the resulting four-joint pose to five physical servo targets.
- [ ] Apply separate shoulder calibration during physical mapping.
- [ ] Define two or more conservative predefined test poses.
- [ ] Move between poses at controlled rate with logs suitable for diagnosis.
- [ ] Decide and document what occurs when a target is invalid.

### 6. Small hands-on exercises

1. Calculate the maximum angular step for a stated rate and time step.
2. Implement a scalar clamp and test below/in/above-range cases.
3. Implement a scalar rate-limit step and test positive, negative, near-target, and already-at-target cases.
4. Tabulate logical shoulder target versus both physical servo targets.
5. Predict the number of updates needed for one predefined move.

### 7. Testing and verification procedures

- Test pure math on boundary values before connecting actuators.
- Log current/target/next values at a controlled rate; avoid flooding serial timing.
- Verify monotonic approach without overshoot for both directions.
- Confirm every emitted logical target is within software limits.
- With hardware made safe, command conservative poses and compare expected versus observed direction/smoothness.
- Stop immediately on binding, conflicting shoulder motion, resets, or unexpected direction and record evidence.

### 8. Completion criteria (definition of done)

- [ ] Four-joint poses are clamped to documented safe logical limits.
- [ ] Rate limiter behaves correctly at boundaries and in both directions.
- [ ] Four logical targets consistently produce five calibrated physical targets.
- [ ] The shoulder actuators move together without observed fighting in the defined test.
- [ ] The arm moves between predefined poses at the configured maximum rate.

### 9. Common mistakes and debugging tips

- Mixing degrees, radians, seconds, milliseconds, and RTOS ticks.
- Applying limits after calibration/mirroring instead of in the logical domain.
- Updating the two shoulder servos at different logical times.
- Using integer division and unexpectedly producing zero step size.
- Overshooting because the last step is not clamped to remaining distance.
- Equating smooth command values with physically smooth motion despite backlash, load, or supply issues.

### 10. Official learning resources

- [ESP-IDF high-resolution timer](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_timer.html)
- [ESP-IDF LEDC driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html)

### 11. Knowledge-check questions

1. How does a velocity limit constrain a position command over one update?
2. Why clamp logical angles before servo calibration mapping?
3. What prevents a final rate-limited step from overshooting?
4. Why must shoulder A and B derive from the same logical snapshot?
5. What evidence distinguishes smooth targets from smooth physical motion?

---

## Phase 5 — FreeRTOS Fundamentals and Integration

### 1. Learning objectives

- Explain task scheduling, priorities, blocking, and periodic execution.
- Use a queue to transfer commands with clear ownership.
- Keep input processing independent from deterministic motion updates.
- Implement and verify safe command timeout behavior.

### 2. Prerequisite knowledge

- Phase 4 motion update function and data ownership.
- Pointers, structs, return values, and basic concurrency vocabulary.
- Difference between elapsed time and iteration count.

### 3. Key concepts and explanations

- An RTOS schedules ready tasks; a task that blocks allows other work to run efficiently.
- Priority affects which ready task runs, not how “important” its code looks. Excessive priorities can starve work.
- `vTaskDelay()` delays relative to when it is called; `vTaskDelayUntil()` supports a steadier periodic schedule.
- A queue copies fixed-size messages and creates an ownership boundary between producers and consumers.
- `xQueueSend()` preserves a sequence if capacity exists; `xQueueOverwrite()` is intended for a length-one latest-value mailbox.
- Blocking time is part of behavior. `portMAX_DELAY`, zero wait, and bounded waits solve different problems.
- Shared mutable state without an ownership/synchronization rule creates race conditions.
- A command timeout defines what motion does when fresh input stops; the safe action must be explicit and tested.

### 4. Relevant project files

- `main/motion_task.c/.h`
- `main/command_task.c/.h`
- `main/arm_types.h`
- `main/app_main.c`
- `main/CMakeLists.txt`

### 5. Detailed implementation checklist

- [ ] Draw task states for the proposed command and motion tasks.
- [ ] Define one compact command message with units and timestamp/sequence semantics.
- [ ] Choose queue length and send behavior based on whether every command or only the latest matters.
- [ ] Create the queue before creating tasks that use it.
- [ ] Create a periodic motion task with reviewed stack size and priority.
- [ ] Use `vTaskDelayUntil()` for a defined update period.
- [ ] Measure/log timing without destabilizing the loop.
- [ ] Create a USB serial command task with bounded parsing and validation.
- [ ] Send validated commands through the queue; do not let parsing code call the servo driver.
- [ ] Receive commands without indefinitely preventing motion updates.
- [ ] Implement a monotonic command-age timeout.
- [ ] Define timeout behavior: controlled hold or neutral/stop policy appropriate to servo position control.
- [ ] Test malformed, partial, too-long, and out-of-range serial input.
- [ ] Document task priorities, blocking points, and ownership.

### 6. Small hands-on exercises

1. Create one task that logs at a slow interval and identify its blocked/ready states.
2. Compare relative-delay and delay-until timing on paper.
3. Pass a counter struct through a queue and explain copy semantics.
4. Decide whether this arm needs command history or the latest target, and justify queue length/API.
5. Simulate a sender stopping and trace the timeout state transition.

### 7. Testing and verification procedures

- Confirm both tasks start and then spend expected time blocked rather than busy-looping.
- Timestamp motion updates and calculate jitter/min/max period over a finite sample.
- Send known valid commands and confirm receipt/validation/motion logs in order.
- Send malformed and out-of-range inputs; verify no unsafe target reaches the driver.
- Stop input and measure time until timeout policy activates.
- Flood input within a controlled test and observe/document queue-full policy.

### 8. Completion criteria (definition of done)

- [ ] Input and motion execute as separate, appropriately blocked tasks.
- [ ] A queue is the defined command ownership boundary.
- [ ] Periodic motion timing is measured and meets the documented tolerance.
- [ ] Invalid input cannot reach the servo driver as an unchecked target.
- [ ] Timeout behavior is observed at the expected time and is safe for the actual arm.

### 9. Common mistakes and debugging tips

- Busy loops with no delay/blocking operation.
- Passing a pointer to temporary stack data instead of queueing a value safely.
- Creating consumers before their queue/resource is ready.
- Using a high priority to mask flawed blocking behavior.
- Assuming `vTaskDelay()` produces an exact fixed phase.
- Blocking forever on input so the motion task never handles timeout/state.
- Logging every fast-loop iteration and changing the timing being measured.

### 10. Official learning resources

- [ESP-IDF FreeRTOS overview](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html)
- [ESP-IDF FreeRTOS API reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos_idf.html)
- [FreeRTOS task documentation](https://www.freertos.org/Documentation/02-Kernel/04-API-references/01-Task-creation/00-TaskHandle)
- [FreeRTOS queue documentation](https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/00-QueueManagement)

### 11. Knowledge-check questions

1. What makes a task ready, running, or blocked?
2. Why does `vTaskDelayUntil()` better suit a periodic update?
3. When is a length-one overwrite queue appropriate?
4. What data does a FreeRTOS queue copy?
5. What must happen when command freshness expires?

---

## Phase 6 — PS5 DualSense Integration

### 1. Learning objectives

- Verify radio/protocol compatibility before adding dependencies.
- Integrate Bluepad32 into native ESP-IDF with controlled version/configuration choices.
- Normalize controller inputs into the existing command type.
- Handle connection, timeout, disconnection, and reconnection safely.

### 2. Prerequisite knowledge

- Completed USB-command-to-motion path from Phase 5.
- Exact ESP32 variant and ESP-IDF version.
- Basic Bluetooth Classic versus BLE distinction.
- Existing command validation and timeout behavior.

### 3. Key concepts and explanations

- The DualSense protocol used by Bluepad32 is Bluetooth Classic BR/EDR, not BLE.
- The original ESP32 family variants with BR/EDR support differ from ESP32-S3/C3/C6-class devices that do not provide Bluetooth Classic. The exact chip must be verified from markings/tool output and current vendor/library documentation.
- Controller callbacks report asynchronous connection/input events; they should update a bounded command path, not directly drive servos.
- Raw sticks require normalization, deadzones, sign conventions, and saturation.
- Velocity-style input lets a centered stick mean no requested change; position-style input maps absolute stick location to joint angle. This project initially favors velocity commands for intuitive manual control.
- Connection state is not command freshness. A connected controller can stop producing valid updates, so timeout handling still applies.

### 4. Relevant project files

- Future `main/gamepad_input.c/.h`
- `main/command_task.c/.h`
- `main/arm_types.h`
- `main/motion_task.c/.h`
- Root/component CMake and dependency manifest if required by the selected integration method.

### 5. Detailed implementation checklist

- [ ] Read exact chip marking and corroborate with flash-tool chip detection.
- [ ] Verify Bluetooth Classic BR/EDR support for that exact target.
- [ ] Stop and select compatible hardware if the target lacks required Classic support; do not hide the incompatibility with code.
- [ ] Record installed ESP-IDF version and choose a compatible Bluepad32 release/integration method.
- [ ] Review Bluepad32's current native ESP-IDF instructions and examples.
- [ ] Add the dependency with a pinned/reproducible version.
- [ ] Build the unmodified/minimal controller example for the verified target.
- [ ] Pair one DualSense and record connection evidence.
- [ ] Log raw stick, trigger, and button values at a controlled rate.
- [ ] Define axis signs, deadzones, scaling, and saturation in one mapping table.
- [ ] Implement `gamepad_input.c/.h` behind the existing command representation.
- [ ] Map all four logical joint controls without changing the servo driver.
- [ ] Ensure callbacks never bypass joint limits or motion rate limiting.
- [ ] Define connect/disconnect/reconnect state transitions.
- [ ] Exercise command timeout even while Bluetooth state appears connected.

### 6. Small hands-on exercises

1. Identify the board's chip and decide whether BR/EDR is supported using primary evidence.
2. Plot/tabulate raw axis values at left extreme, center, and right extreme.
3. Implement/test a scalar deadzone and normalization helper.
4. Create a controller-to-command mapping table with signs and units.
5. Trace what each module does during a disconnect while a joint was moving.

### 7. Testing and verification procedures

- Build compatibility test before altering motion code.
- Pair/reconnect test from power-on, controller-off, controller-on, and board-reset states.
- Observe raw inputs without servo power first.
- Verify centered controls produce zero velocity despite small stick noise.
- Verify extremes saturate at configured limits and never bypass joint limits/rate limits.
- Disconnect during a conservative controlled move and measure timeout response.
- Reconnect and require a deliberate fresh/neutral input policy before motion resumes.

### 8. Completion criteria (definition of done)

- [ ] Exact target is documented and verified as DualSense/BR/EDR compatible.
- [ ] Bluepad32 version and integration steps are reproducible with the chosen ESP-IDF version.
- [ ] Sticks/triggers/buttons are read and normalized with measured center/extreme values.
- [ ] All four logical joints are controlled through the existing command/motion/driver pipeline.
- [ ] Disconnect, stale-input timeout, and reconnect behavior are physically verified and recorded.

### 9. Common mistakes and debugging tips

- Assuming every ESP32-named chip supports Bluetooth Classic.
- Confusing BLE visibility with DualSense BR/EDR compatibility.
- Letting a callback call hardware drivers directly.
- Forgetting stick center offset, deadzone, inversion, or saturation.
- Treating “connected” as proof of fresh input.
- Allowing reconnection to reuse a stale nonzero command.
- Pulling an unpinned dependency that later changes compatibility.

### 10. Official learning resources

- [Bluepad32 repository](https://github.com/ricardoquesada/bluepad32)
- [Bluepad32 FAQ and compatibility notes](https://github.com/ricardoquesada/bluepad32/blob/main/docs/FAQ.md)
- [Bluepad32 supported gamepads](https://github.com/ricardoquesada/bluepad32/blob/main/docs/supported_gamepads.md)
- [ESP-IDF Bluetooth API overview](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/index.html)
- Exact ESP32 product datasheet will be linked after the chip is identified.

### 11. Knowledge-check questions

1. Why is the name “ESP32” insufficient to prove DualSense compatibility?
2. Where should deadzone and normalization occur in the architecture?
3. Why must the servo driver remain unchanged when adding a gamepad?
4. What is the difference between connection state and command freshness?
5. What prevents sudden motion immediately after reconnection?

---

## Phase 7 — System Integration and Verification

### 1. Learning objectives

- Integrate modules in a deterministic initialization order.
- Define observable fault behavior and evidence-based system tests.
- Measure timing and verify safety behaviors on actual hardware.
- Produce maintainable build, usage, and architecture documentation.

### 2. Prerequisite knowledge

- All prior phase definitions of done.
- Safe bench setup and emergency power removal.
- Git basics: status, diff, focused commits, and readable history.

### 3. Key concepts and explanations

- Initialization order establishes invariants: configuration/resources before tasks/callbacks that consume them.
- A fault is actionable only if detection, response, recovery, and logging are specified.
- Verification maps each requirement to a repeatable procedure and objective expected result.
- Diagnostics must reveal state without overwhelming timing or hiding root causes.
- Integration testing checks module interactions; it does not replace isolated tests.
- Documentation is part of the product because it makes builds, tests, and safety assumptions reproducible.

### 4. Relevant project files

- All `main/` modules and CMake files.
- `sdkconfig.defaults`
- `README.md`
- `LEARNING_ROADMAP.md`
- Optional focused test scripts/data added only when justified.

### 5. Detailed implementation checklist

- [ ] Define and document system initialization order.
- [ ] Verify failures leave outputs in the safest feasible state.
- [ ] Test each joint independently over its approved range.
- [ ] Test coordinated shoulder movement in both directions.
- [ ] Verify logical and physical limits at every boundary.
- [ ] Measure motion update period and timing tolerance under serial input.
- [ ] Repeat timing test under Bluetooth input.
- [ ] Verify malformed command rejection.
- [ ] Verify stale-command timeout.
- [ ] Verify controller disconnect and reconnect behavior.
- [ ] Power-cycle and reset during controlled test states.
- [ ] Check logs for clear module tags, severities, and actionable fault details.
- [ ] Document every public module interface and units.
- [ ] Add architecture/data-flow documentation to README.
- [ ] Add exact prerequisites, configure, build, flash, monitor, pairing, and operation instructions.
- [ ] Review `git diff` and create focused commits without generated build output or secrets.
- [ ] Run the final regression checklist from a clean documented setup.

### 6. Small hands-on exercises

1. Write one requirement as a test case with preconditions, action, expected result, and evidence.
2. Draw initialization dependencies and identify what must exist before tasks start.
3. Inject one safe invalid command and trace its rejection path.
4. Compare requested versus measured update timing and calculate worst-case deviation.
5. Have another person follow README setup steps without undocumented knowledge if possible.

### 7. Testing and verification procedures

- Use a test matrix containing test ID, firmware commit, hardware revision, setup, action, expected result, actual result, pass/fail, and evidence link/log.
- Begin each physical test with reviewed power/current limits, clear workspace, conservative speed, and reachable cutoff.
- Separate build, flash, boot, serial input, Bluetooth input, servo signal, and observed mechanical motion results.
- Repeat safety-critical disconnect/timeout/limit tests, not just nominal motion.
- Re-run the full matrix after changes affecting configuration, timing, mapping, dependencies, or hardware.

### 8. Completion criteria (definition of done)

- [ ] Firmware builds reproducibly from documented prerequisites and configuration.
- [ ] The verified board flashes/boots and reports expected diagnostic state.
- [ ] Each of five outputs and four logical joints passes its test cases.
- [ ] Joint limits, coordinated shoulders, smoothness, command validation, timeout, and reconnection pass.
- [ ] Architecture and all module interfaces/units are documented.
- [ ] No unresolved safety-critical defects remain; other known limitations are documented.
- [ ] Final verification evidence is tied to a firmware commit and hardware setup.

### 9. Common mistakes and debugging tips

- Enabling outputs before configuration/queue/state initialization completes.
- Testing only successful input paths.
- Changing hardware, firmware, and power setup simultaneously during diagnosis.
- Logging symptoms without timestamp, module, command, or state context.
- Calling a one-time successful motion a regression test.
- Committing generated `build/`, machine-specific ports, credentials, or transient `sdkconfig` changes unintentionally.

### 10. Official learning resources

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [ESP-IDF error handling](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/error-handling.html)
- [ESP-IDF core dump guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/core_dump.html)
- [Git documentation](https://git-scm.com/doc)
- Hardware-specific datasheets, schematics, and servo documentation will be linked after exact parts are recorded.

### 11. Knowledge-check questions

1. Which resources must exist before an input callback or task can use them?
2. What evidence proves a build, a flash, and physical motion respectively?
3. How should the system respond to invalid versus stale commands?
4. What should a fault log contain to be actionable?
5. Why must final evidence identify both firmware and hardware versions?

---

## Technical decisions

| Date | Decision | Reason/trade-off | Status |
|---|---|---|---|
| 2026-10-25 | Use native ESP-IDF with C | Exposes the intended component, driver, logging, configuration, and FreeRTOS model directly | Accepted |
| 2026-10-25 | Keep four logical joints separate from five physical outputs | Shoulder coordination belongs above independently calibrated actuator outputs | Accepted; implementation pending |
| 2026-10-25 | Use USB serial before Bluetooth | Establishes and verifies the command/motion boundary without radio/dependency complexity | Accepted; implementation pending |
| 2026-10-25 | Do not choose ESP-IDF target or Bluepad32 dependency until the exact chip is verified | DualSense uses Bluetooth Classic through Bluepad32; not every ESP32 variant supports BR/EDR | Required gate |
| 2026-10-25 | Compile only `app_main.c` during the first Phase 1 checkpoint | Existing future-stage files are blank placeholders and should not be implemented prematurely | Active |
| 2026-10-26 | EIM installer target `all` may be selected, but project `IDF_TARGET` must remain unset until the physical chip is identified | EIM's choice controls which support tools are installed; `idf.py set-target` controls this project's actual build target | Accepted clarification |
| 2026-10-26 | Use ESP-IDF target `esp32` for the ESP32-WROOM-32UE module identified in the user-provided schematic | Espressif documents this module as an original ESP32-family module with ESP32-D0WD-V3/D0WDR2-V3 SoC options and Bluetooth 4.2 BR/EDR + BLE | Provisionally accepted; physical marking and flash-tool detection still required |
| 2026-10-26 | Install exact ESP-IDF tag `v5.5.6` rather than v6.x or a moving release branch | Current Bluepad32 raw ESP-IDF instructions require v5.5 or newer and its CI exercises v5.5; an exact patch tag improves reproducibility | Selected baseline; installation pending |
| 2026-10-26 | Ignore the generated root-level `eim_config.toml` | It contains machine-specific absolute installation paths and EIM state; `sdkconfig.defaults` remains tracked as project configuration | Implemented and verified with `git check-ignore` |

## Problems encountered

| Date | Symptom/evidence | Classification | Resolution/status |
|---|---|---|---|
| 2026-10-25 | `idf.py` was not found in the current shell; no common local ESP-IDF path was found | Development environment | Open: activate an existing ESP-IDF environment or install it, then record `idf.py --version` |
| 2026-10-25 | Learner ran `idf.py --version`, `echo $IDF_PATH`, and `command -v idf.py`; both command lookup and `IDF_PATH` were empty | Development environment | Verified that the learner's normal terminal is not ESP-IDF-enabled; VS Code Espressif extension v2.3.0 is installed, but no configured setup was found |
| 2026-10-25 | ESP-IDF Installation Manager v0.20.1 opened and reported missing `dfu-util` and `cmake` | Development prerequisites | Open: install both with Homebrew, verify their versions, then reopen/refresh the manager |
| 2026-10-25 | CMake 4.4.4 and `dfu-util` 0.11 resolved from `/opt/homebrew/bin` after learner installation | Development prerequisites | Resolved and verified; refresh/reopen Installation Manager next |
| 2026-10-25 | Installation Manager GUI reported Python setup required, while the shell resolved Homebrew Python 3.10.20 with pip 26.0.1, `venv`, and OpenSSL 3.6.1 | macOS GUI environment | Open: relaunch VS Code from the configured shell so the GUI inherits Homebrew `PATH`, then recheck |
| 2026-10-26 | Initial relaunch command referred to nonexistent `Visual Studio Code.app/Contents/MacOS/Electron` | Setup instruction | Corrected executable path after inspection: `Visual Studio Code.app/Contents/MacOS/Code` |
| 2026-10-26 | EIM Python recheck still failed after VS Code was launched from the configured shell | macOS GUI environment | Use Espressif's documented EIM CLI/Homebrew route so EIM directly inherits the working terminal environment |
| 2026-10-26 | Homebrew refused to install the EIM formula until its third-party tap was explicitly trusted | Package-manager trust gate | Avoided changing trust settings: the installed `/Applications/eim.app` already contains a CLI-capable official EIM binary |
| 2026-10-26 | EIM GUI still reported Python setup required when its binary was launched directly with Homebrew Python first in `PATH` | Known EIM macOS GUI issue | Stop retrying GUI mode; use the same binary's `wizard` command, the workaround documented in Espressif issue 1039 |
| 2026-10-26 | EIM CLI wizard inherited Homebrew Python correctly and passed every sanity check | Development environment | Resolved by using terminal `wizard` mode; wizard stopped at target selection pending exact chip identification |
| 2026-10-25 | Root/component CMake, `app_main.c`, and future modules were one blank line | Project scaffold | Minimum Phase 1 files created; future modules left untouched and excluded from build |
| 2026-10-26 | Boot log reported physical flash size 4096 KB but image header size 2048 KB | Project configuration | Resolved: selected 4 MB in menuconfig, rebuilt/reflashed, and confirmed a clean 4 MB boot without the mismatch warning |
| 2026-10-26 | Learner terminal reported `zsh: command not found: rg` during configuration verification | Optional local utility | Use macOS `grep` for the same narrow check; this is not an ESP-IDF or configuration failure |
| 2026-10-26 | VS Code C/C++ diagnostic reported missing `esp_log.h` and include-path errors even though firmware compiled | Editor IntelliSense | Configuration generated: `.vscode/c_cpp_properties.json` points to `build/compile_commands.json`; `.vscode/` is ignored because it contains a machine-specific absolute path. Awaiting learner confirmation that a VS Code reload clears the diagnostic |

## Lessons learned

- File presence is not evidence that a project has an implemented or buildable scaffold; contents and build registration must be inspected.
- Tool availability in one shell is not evidence about board connection or the absence of all installations; capture the activated environment and version explicitly.
- Target selection is a hardware fact, not a naming guess.
- EIM installer tool-target selection and a project's ESP-IDF build target are different decisions; installing support for `all` does not make firmware universal or identify the connected chip.
- ESP32-WROOM-32UE is the external-antenna variant; Bluetooth/Wi-Fi testing requires the correct external antenna to be present, and schematic identity still needs physical corroboration.
- Detailed CMake study was paused at the learner's request after verifying the essential project-versus-component distinction; its checklist item remains incomplete.

## Verification log

| Date | Check | Evidence | Result | Scope limit |
|---|---|---|---|---|
| 2026-10-25 | Repository inspection | Listed project files, sizes, contents, and clean Git status | Passed | Does not prove build or runtime behavior |
| 2026-10-25 | ESP-IDF environment probe | `idf.py`, Xtensa compiler, `IDF_PATH`, and common install paths not found | Not available in inspected shell | Does not prove ESP-IDF is absent elsewhere on the computer |
| 2026-10-25 | Minimum scaffold review | Root CMake includes ESP-IDF project machinery; `main` registers only `app_main.c`; entry point uses ESP logging | Passed by source inspection | Not compiled because ESP-IDF tools are unavailable in this shell |
| 2026-10-25 | Learner terminal environment | `zsh: command not found: idf.py`; empty `IDF_PATH`; `command -v idf.py` returned nothing | Confirmed not enabled | Does not yet determine which ESP-IDF version should be installed |
| 2026-10-25 | Installation Manager prerequisite check | Screenshot showed `Prerequisites Missing`: `dfu-util`, `cmake`; local check found Homebrew 5.1.3 but neither command | Installer reached; prerequisite gate identified | ESP-IDF version has not been selected or installed |
| 2026-10-25 | Installation Manager prerequisites after install | `cmake --version` reported 4.4.4; `dfu-util --version` reported 0.11 | Passed | ESP-IDF framework/toolchain itself remains uninstalled |
| 2026-10-25 | Python sanity diagnosis | `/opt/homebrew/opt/python@3.10/libexec/bin/python3`; Python 3.10.20; pip 26.0.1; imports of `ssl` and `venv` succeeded | Shell Python meets displayed EIM requirements | EIM GUI did not inherit the shell's Homebrew path |
| 2026-10-26 | Embedded EIM CLI | `/Applications/eim.app/Contents/MacOS/eim --version` reported `eim 0.20.1`; help exposed `gui`, `wizard`, and installation commands | Passed | GUI Python recheck still pending using this direct launch route |
| 2026-10-26 | EIM CLI prerequisites and Python sanity | Git, CMake, and `dfu-util` satisfied; Python version, pip, venv, standard library, ctypes, and SSL/HTTPS all reported `[PASS]` | Passed | No ESP target or ESP-IDF version selected; framework/toolchain not installed yet |
| 2026-10-26 | Module identification from design evidence | User supplied schematic labels M2 `ESP32-WROOM-32UE`; Espressif datasheet identifies original ESP32 SoC family and Bluetooth 4.2 BR/EDR + BLE | Preliminary target `esp32` and DualSense radio compatibility confirmed | Physical module marking, antenna connection, and flash-tool chip detection remain unverified |
| 2026-10-26 | EIM target/version selection | Learner advanced with installer target `esp32` and exact ESP-IDF tag `v5.5.6` | Selection verified | Download and installation have not occurred or been verified yet |
| 2026-10-26 | ESP-IDF installation/tool activation | `activate_idf_v5.5.6.sh` loaded `IDF_PATH=/Users/elong/.espressif/v5.5.6/esp-idf`; `idf.py --version` returned `ESP-IDF v5.5.6`; Xtensa GCC 14.2.0 and Ninja 1.12.1 resolved | Passed in independent verification shell | Activation is session-scoped; learner's terminal and project build remain unverified |
| 2026-10-26 | Local EIM configuration ignore rule | `git check-ignore -v eim_config.toml` matched `.gitignore:58`; `git status --short` no longer listed the generated file | Passed | `.gitignore` edit remains an uncommitted workspace change |
| 2026-10-26 | Learner-terminal ESP-IDF activation | Learner ran `idf.py --version` in the activated terminal and reported `ESP-IDF v5.5.6` | Passed | Proves tool frontend availability, not project configuration, build, flash, or hardware connection |
| 2026-10-26 | Project target configuration | Learner's `idf.py set-target esp32` completed CMake configure/generate; `sdkconfig` records `CONFIG_IDF_TARGET="esp32"`; `build.ninja`, CMake cache, and project description exist | Passed | Build files were generated, but application compilation and hardware identity remain unverified |
| 2026-10-26 | Minimal Phase 1 firmware build | Learner reported all 1027 build steps completed; `robotic_arm_01.bin` generated at 162,256 bytes with 85% of the smallest application partition free; bootloader, partition table, and Xtensa debug ELF inspected | Passed | Proves compilation only; no board, flash, boot, logs, or physical behavior verified |
| 2026-10-26 | Serial-device inventory after build | Only `/dev/cu.Bluetooth-Incoming-Port`, `/dev/cu.debug-console`, and matching `tty` entries were visible | No ESP32 USB serial port detected | Board may be disconnected, cable may lack data, or USB interface/driver is not enumerated |
| 2026-10-26 | Serial-device inventory after connection | Learner's listing added `/dev/cu.usbserial-110`; device node exists and is readable/writable | Port identification passed | USB-UART enumeration does not itself prove the attached chip model or successful flashing |
| 2026-10-26 | First Phase 1 flash | esptool v4.12.0 detected ESP32-D0WD-V3 revision 3.1, dual-core 240 MHz with Wi-Fi/BT and 40 MHz crystal; bootloader, 162,256-byte application, and partition table each reported verified hashes; hard reset completed | Passed | Confirms chip family and written flash contents, but not application boot, serial logs, or any actuator behavior |
| 2026-10-26 | First Phase 1 runtime/serial log | Bootloader loaded factory app at `0x10000`; ESP-IDF v5.5.6 called `app_main()` on CPU0; expected `ESP_LOGI` and `ESP_LOGW` messages appeared; main task reported return from `app_main()` | Passed | No actuator behavior was enabled or tested; flash header remains configured for 2 MB despite detected 4 MB hardware |
| 2026-10-26 | Flash-size menuconfig edit | Saved `sdkconfig` now contains `CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y` and `CONFIG_ESPTOOLPY_FLASHSIZE="4MB"` | File inspection passed | Learner-terminal verification plus rebuild/reflash/runtime warning check remain pending |
| 2026-10-26 | Rebuilt application image after flash-size edit | Extended esptool image inspection reported ESP32, `Flash size: 4MB`, 40 MHz, DIO, valid checksum/hash, and ESP-IDF v5.5.6 | Passed | Corrected image has not yet been reflashed or boot-verified |
| 2026-10-26 | Corrected 4 MB image runtime | Boot log reported `SPI Flash Size : 4MB`, did not repeat the 4096 KB/2048 KB mismatch warning, loaded the application, and printed both expected `arm_app` messages | Passed | No servo or other actuator behavior was enabled or tested |
| 2026-10-26 | Minimal reproducible configuration | `idf.py save-defconfig` generated tracked `sdkconfig.defaults` containing `CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y`; `git diff --check` passed | Passed | Active generated `sdkconfig` remains intentionally ignored |

## Session resume procedure

1. Read the progress summary, checkpoint ledger, and latest verification log entries.
2. Confirm the repository's current saved state with `git status` and a focused diff.
3. Resume at the first unchecked item in the current micro-goal, not at a later phase.
4. After obtaining evidence, update the checkbox, verification log, problems/lessons, and next milestone.
5. Stop at one practical learner checkpoint unless the learner explicitly asks to continue.
