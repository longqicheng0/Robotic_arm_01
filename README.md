# Robotic_arm_01

`Robotic_arm_01` is a learning-first robotic arm platform built around the Waveshare RoArm-M2-S. The project is growing from ESP32 servo control into a complete manipulation stack with kinematics, MuJoCo simulation, trajectory planning, Jetson-based perception, ROS 2, neural networks, and reinforcement learning.

The project deliberately develops one dependable layer at a time. Low-level motion and safety come before autonomous behavior on physical hardware.

## Current repository status

The current firmware is an ESP-IDF C++ project for ESP32. A fresh ESP-IDF v5.5.6 build of the inspected working tree passed on November 10, 2026.

Implemented in the current source:

- Four logical joints: base, shoulder, elbow, and gripper.
- Five ST3215 servo targets: base, two coupled shoulder servos, elbow, and gripper.
- UART1 configuration at 1 Mbps on GPIO18 (RX) and GPIO19 (TX).
- ST3215 ping, individual position write, present-position read, and five-servo synchronized-write packet paths.
- Joint clamping, rate-limited joint updates, joint-to-servo conversion, and per-servo calibration data.
- A 50 Hz FreeRTOS motion task that repeatedly commands one fixed default pose.
- Early robot-geometry and Cartesian-position data structures.

Not yet established by the repository:

- A live command source or changing target pose.
- Final physical calibration, safety interlocks, stale-command handling, or a fault-state strategy.
- Current flash, boot, UART-response, or physical-motion evidence for the inspected firmware.
- Forward or inverse kinematics. The current `kinematics.h` file is empty and is not part of the build.
- MuJoCo, trajectory planning, Jetson, ROS 2, vision, neural-network, or reinforcement-learning code.

A passing build proves only that the selected source compiled and linked. It does not prove that the firmware was flashed or that the arm moved correctly.

## Target architecture

```text
MuJoCo / operator / planner / learned policy
                    |
          validated robot command
                    v
Jetson: perception, planning, ROS 2, AI inference
                    |
       bounded command + robot state interface
                    v
ESP32: validation, joint limits, motion timing,
       ST3215 communication, low-level safety
                    |
                    v
             physical robotic arm
```

Simulation and physical hardware should expose consistent robot command and state concepts so higher-level code can change backends without changing the robot mathematics. Kinematics, geometry, and trajectory math should remain independent of ESP-IDF, ROS 2, MuJoCo, and any particular transport.

## Development sequence

1. Refactor and harden the current C++ firmware.
2. Implement forward kinematics (FK).
3. Implement inverse kinematics (IK).
4. Build and validate a MuJoCo physics model.
5. Add smooth joint-space and Cartesian trajectory planning.
6. Integrate a Jetson computer and ROS 2 while retaining the ESP32 for low-level control.
7. Add computer vision and neural-network perception.
8. Train manipulation policies with reinforcement learning in simulation, then consider guarded physical deployment.

See [LEARNING_ROADMAP.md](LEARNING_ROADMAP.md) for the progressive goals, prerequisites, safety gates, and completion criteria for every phase.

## Design principles

- Keep the code minimal, readable, modular, and testable.
- Prefer small, explicit data flows over hidden state.
- Use fixed-size data and deterministic control paths in firmware.
- Check inputs, communication results, and state transitions.
- Keep core robot mathematics independent of hardware and frameworks.
- Avoid premature abstraction and unnecessary dependencies.
- Make simulation and hardware follow the same command/state contract.
- Validate commands and establish safe failure behavior before autonomous hardware control.

## Hardware control model

The arm has four logical joints but five physical actuators because the shoulder uses two servos. All ST3215 servos share one serial bus and are addressed by ID rather than by their daisy-chain position.

| Servo | Bus ID |
|---|---:|
| Base | 11 |
| Shoulder A | 12 |
| Shoulder B | 13 |
| Elbow | 14 |
| Gripper | 15 |

The documented power source is 12 V, 5 A (60 W). Treat physical power-up and motion as separate safety-sensitive steps; a successful build is not permission to energize the arm.

## Build

Use an ESP-IDF v5.5.6 environment, then run:

```sh
idf.py build
```

Flashing, monitoring, bus communication, and physical motion must each be verified separately. Do not deploy autonomous commands to the arm until the safety criteria in the roadmap are met.

## References

- [Waveshare ST3215 documentation](https://docs.waveshare.com/ST3215_Servo)
- [Waveshare ST3215 serial command examples](https://docs.waveshare.com/ST3215_Servo/Serial-Debugging-Tool-Use)
- [ESP-IDF UART API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html)
- [ESP-IDF FreeRTOS overview](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos_idf.html)
