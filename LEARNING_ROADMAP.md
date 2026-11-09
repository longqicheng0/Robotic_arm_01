# ESP-IDF C++ ST3215 Robotic Arm Learning Roadmap

This file is the current source of truth for learning progress and implementation status.

A successful source review, build, flash, boot, UART exchange, and observed arm movement are separate forms of evidence. A checked item means the stated evidence was actually obtained.

## Project goal

Build a clean C++ control stack for the Waveshare RoArm-M2-S using ESP-IDF and its five ST3215 serial-bus servos.

The arm has:

- Four logical joints: base, shoulder, elbow, and gripper.
- Five physical servos: base, shoulder A, shoulder B, elbow, and gripper.
- One shared ST3215 communication bus.
- Servo IDs 11, 12, 13, 14, and 15.
- A 12 V, 5 A, 60 W supply.

## Current progress

| Phase | Goal | Status | Current evidence |
|---|---|---|---|
| 1 | ESP-IDF and C++ foundation | Complete | ESP-IDF v5.5.6 project builds for ESP32 |
| 2 | ST3215 UART communication | Complete foundation | UART1 setup and read-only ping implementation build |
| 3 | Joint-space motion math | Complete | Limits, rate limiting, and all-joint update logic build |
| 4 | Joint-to-servo conversion | Complete | Four joint angles map into five calibrated servo targets |
| 5 | ST3215 position command | In progress | Function boundary and first six packet bytes exist; no position packet is transmitted yet |
| 6 | Feedback and calibration | Not started | Calibration types exist; physical values are not finalized |
| 7 | Synchronized five-servo output | Not started | Target array exists; synchronized transmission is not implemented |
| 8 | Periodic motion application | Not started | `app_main()` initializes UART only |
| 9 | Command source | Decision pending | No source currently provides live target joint angles |
| 10 | Higher-level arm control | Later | Controller input and kinematics are deferred |

## Current checkpoint

- Current file: `main/joint_driver.cpp`
- Current function: `st3215::writePosition`
- Current micro-goal: finish constructing the 13-byte ST3215 position-write packet.
- Implemented packet fields: headers, servo ID, length, write instruction, and target-position register address.
- Next fields: position, time, speed, and checksum.
- Current runtime behavior: no position command is sent.
- Current hardware behavior: no motion from this new driver has been verified.

## System architecture

```text
command source
    -> requested JointPositions
    -> clamp each joint to JointLimits
    -> rate-limit each joint
    -> map four joints to five ServoTargets
    -> apply per-servo zero and direction calibration
    -> build ST3215 serial packets
    -> transmit synchronized servo targets
```

The shoulder is one logical joint with two physical servos. Shoulder A and Shoulder B receive the same logical shoulder angle but use separate calibration entries.

## PWM servos versus ST3215 serial-bus servos

### Conventional PWM servo

- Each servo normally needs its own control signal from the controller.
- The controller generates a repeated pulse waveform for every servo.
- Power and ground may be distributed in parallel, but each servo's signal line is independently controlled.
- The signal does not normally include an address or servo ID.
- Position feedback is usually handled internally by the servo and is not returned digitally to the controller.
- Five independently controlled PWM servos normally require five controller signal channels.

### ST3215 serial-bus servo

- All servos share the communication bus and can be daisy-chained.
- Each servo has a unique bus ID.
- The controller sends structured serial packets over the shared bus.
- A packet contains a destination ID, instruction, command data, and checksum.
- Every servo can see the packet, but the addressed servo acts on an individual command.
- A synchronized-write packet can contain data for several servo IDs so multiple servos receive their targets from one bus transaction.
- The controller can request digital feedback such as present position.

The bus is the shared communication path. A packet is one message carried over that path. `ServoIdx` is the program's array position; `busId` is the servo address placed in a serial packet. They are not interchangeable.

## Phase 1 — ESP-IDF and C++ foundation

### Completed

- [x] Configure the ESP-IDF project for target `esp32`.
- [x] Activate and build with ESP-IDF v5.5.6.
- [x] Identify the programming port as `/dev/cu.usbserial-110` during verified board work.
- [x] Use C++ source files and an `extern "C"` `app_main()` entry point.
- [x] Separate headers, implementation files, configuration, and application startup.
- [x] Keep build, flash, boot logs, serial communication, and physical motion as separate evidence.

## Phase 2 — ST3215 UART communication

### Completed foundation

- [x] Use `UART_NUM_1` at 1 Mbps.
- [x] Use GPIO18 for RX and GPIO19 for TX.
- [x] Configure and install the ESP-IDF UART driver.
- [x] Build a six-byte ST3215 ping packet.
- [x] Calculate and validate the ST3215 checksum.
- [x] Send ping bytes and validate a status response.
- [x] Record servo IDs 11 through 15 for the five physical actuators.

### Important model

- The daisy-chain order does not determine the servo ID.
- A ping identifies a responding ID, not the servo's physical location.
- The two shoulder servos are both active actuators; neither is treated as a passive follower.

## Phase 3 — Joint-space motion math

### Completed

- [x] Define `JointId` for Base, Shoulder, Elbow, and Gripper.
- [x] Store requested and current positions in degrees.
- [x] Define minimum, maximum, default, and maximum rate for every logical joint.
- [x] Clamp requested angles to joint limits.
- [x] Rate-limit motion using maximum degrees per second and elapsed time.
- [x] Update all logical joints with one function.

### Core equations

```text
safeTarget = clamp(requestedTarget, minimum, maximum)
maximumStep = maximumDegreesPerSecond * deltaTimeSeconds
nextPosition = move currentPosition toward safeTarget by at most maximumStep
```

Angle wrapping with `atan2(sin(angle), cos(angle))` is not a replacement for clamping. Wrapping finds an equivalent cyclic angle; clamping enforces the arm linkage's physical range.

## Phase 4 — Joint-to-servo conversion

### Completed

- [x] Define `ServoIdx` for Base, ShoulderA, ShoulderB, Elbow, and Gripper.
- [x] Define five-element servo-target and calibration arrays.
- [x] Store each servo's bus ID, zero step, and reversed direction flag.
- [x] Convert degrees to ST3215 encoder steps.
- [x] Round and clamp the final result to 0 through 4095.
- [x] Map the logical shoulder angle into both physical shoulder servo targets.

### Conversion

```text
direction = reversed ? -1 : +1
rawSteps = zeroStep + direction * jointDegrees * (4096 / 360)
positionSteps = round and clamp rawSteps to [0, 4095]
```

## Phase 5 — ST3215 position command

### In progress

- [x] Declare `st3215::writePosition(id, positionSteps, speedStepsPerSecond)`.
- [x] Reject positions above 4095.
- [x] Allocate a 13-byte position packet.
- [x] Fill the two headers, ID, packet length, write instruction, and register address.
- [ ] Split `positionSteps` into low and high bytes.
- [ ] Set the two movement-time bytes.
- [ ] Split `speedStepsPerSecond` into low and high bytes.
- [ ] Calculate the checksum over the required packet bytes.
- [ ] Send the complete packet with the UART driver.
- [ ] Check the number of accepted bytes.
- [ ] Wait for UART transmission to finish.
- [ ] Replace the temporary `ESP_ERR_NOT_SUPPORTED` result with truthful transmission status.

## Phase 6 — Present-position feedback and calibration

### Planned

- [ ] Implement a present-position read request and validate its response.
- [ ] Record the real zero step for every physical servo.
- [ ] Record whether each physical servo is reversed relative to its logical joint.
- [ ] Define the real minimum, maximum, default, and maximum speed for every logical joint.
- [ ] Confirm the manufacturer-listed maximum ST3215 command speed and its protocol units.
- [ ] Set the planned ST3215 command speed to 95% of that confirmed maximum.
- [ ] Store the final five-servo calibration array in project configuration.
- [ ] Confirm that Shoulder A and Shoulder B convert the same logical angle into mechanically compatible servo targets.

## Phase 7 — Synchronized five-servo output

### Planned

- [ ] Define a function that accepts all five `ServoTargets`.
- [ ] Build an ST3215 synchronized-write packet containing the five servo IDs and targets.
- [ ] Include the appropriate speed for each servo.
- [ ] Transmit the complete synchronized command in one bus transaction.
- [ ] Connect `mapJointPositionsToServoTargets()` to synchronized transmission.
- [ ] Verify and record the resulting multi-servo hardware behavior when this phase is run.

This roadmap intentionally proceeds directly from calibration and limits to synchronized five-servo output. The previously proposed offline packet test, guarded single-servo test, and five separate servo trials are not part of the current plan.

## Phase 8 — Periodic motion application

### Planned

- [ ] Create a periodic FreeRTOS motion task.
- [ ] Use elapsed time to update the rate-limited joint positions.
- [ ] Convert the updated joint positions into five servo targets.
- [ ] Send synchronized targets at the selected update rate.
- [ ] Start the task from `app_main()` after UART initialization.

## Phase 9 — Command source

A command source supplies the requested joint angles that the motion task follows. Without one, the control pipeline has no changing target.

Possible sources are:

- Fixed or scripted poses stored in the firmware.
- Commands typed through the USB serial connection from a computer.
- A PS5 controller added later.
- A future higher-level kinematics module.

USB serial was previously suggested only because it is a simple way to enter target angles from the Mac before adding a controller. It is not required. The final source will be selected when the motion task is ready.

## Phase 10 — Higher-level control

### Later

- [ ] Add the selected live command source.
- [ ] Add named poses or pose sequences if useful.
- [ ] Add PS5 DualSense input if still desired.
- [ ] Implement forward kinematics.
- [ ] Implement inverse kinematics for Cartesian end-effector targets.
- [ ] Use present-position feedback for arm-level closed-loop control if required.

## Intentionally excluded from the current plan

- PWM/LEDC servo control.
- C-only implementation; the project uses C++.
- Offline packet-test milestone.
- Guarded single-servo actuation milestone.
- Separate motion trials for all five servos.
- Explicit fault-state machinery.
- Stale-command stopping logic.
- ROS 2, networking, JSON protocols, or Jetson integration before basic arm motion works.

## Evidence rules

- Source present means only that code exists.
- A successful build means only that the selected source and configuration compiled and linked.
- A successful flash means only that the binary was written and verified.
- Boot logs mean only that the flashed application reached the logged path.
- UART success means only that a packet was transmitted or a valid response was received.
- Physical motion is verified only from observed hardware behavior.

## Resume procedure

1. Read this roadmap and the current Git diff.
2. Inspect the current checkpoint in `main/joint_driver.cpp`.
3. Run the smallest verification relevant to the latest edit.
4. Continue one conceptual dependency at a time.
5. When the learner sends `c` or `check`, inspect the saved code before advancing.
6. Do not flash motion firmware unless the learner explicitly asks to flash it.

## References

- [Waveshare ST3215 documentation](https://docs.waveshare.com/ST3215_Servo)
- [Waveshare ST3215 serial command examples](https://docs.waveshare.com/ST3215_Servo/Serial-Debugging-Tool-Use)
- [ESP-IDF UART API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html)
- [ESP-IDF FreeRTOS overview](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos_idf.html)
