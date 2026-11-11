# Robotic Arm Platform Learning and Development Roadmap

This roadmap is the source of truth for the planned development order of `Robotic_arm_01`. It preserves the low-level ESP32 and ST3215 learning work while extending the project toward kinematics, simulation, planning, Jetson/ROS 2 integration, perception, neural networks, and reinforcement learning.

The sequence is intentionally progressive. Each phase should produce a small, testable capability that the next phase can rely on.

## Evidence rules

A phase is complete only when its completion criteria have been demonstrated and recorded.

- Source present means only that code exists.
- A successful build means only that the selected source and configuration compiled and linked.
- A successful flash means only that the binary was written and verified.
- Boot logs mean only that the flashed application reached the logged path.
- UART success means only that a packet was transmitted or a valid response was received.
- Physical motion is verified only through observed hardware behavior.
- Simulation results do not prove safe or correct physical behavior.
- An empty header, placeholder file, data type, or planned interface is not an implemented feature.

## Long-term architecture

### Responsibility split

**ESP32 — low-level control**

- Validate bounded joint commands.
- Enforce joint, speed, and timing limits.
- Run deterministic periodic motion updates.
- Communicate with the ST3215 servos.
- Report measured state and low-level faults.
- Enter a safe state when commands are invalid, stale, or communication fails.

**Jetson — high-level intelligence**

- Run ROS 2 nodes and system coordination.
- Process camera data and estimate objects or targets.
- Plan trajectories and manipulation behavior.
- Run neural-network and learned-policy inference.
- Send validated high-level commands and consume robot state.

**Shared core mathematics**

- Define geometry, coordinate frames, units, joint state, and robot state.
- Implement FK, IK, and trajectory math without ESP-IDF, ROS 2, MuJoCo, or transport dependencies.
- Use the same mathematical conventions in simulation and on physical hardware.

### Target data flow

```text
camera / operator / task / learned policy
                  -> perception and planning on Jetson
                  -> validated time-stamped robot command
                  -> ESP32 limits and low-level motion control
                  -> ST3215 servo bus
                  -> measured robot state and faults
                  -> Jetson and/or MuJoCo consumers
```

MuJoCo and the physical arm should use consistent command and state concepts. They do not need identical transport implementations, but they must agree on joint order, units, coordinate frames, limits, timestamps, and error semantics.

## Current repository checkpoint — November 10, 2026

A fresh `idf.py build` passed with ESP-IDF v5.5.6 for the inspected working tree. No flash or hardware operation was performed during this documentation update.

### Implemented in source and included in the build

- [x] ESP-IDF C++ application with an `extern "C"` `app_main()` entry point.
- [x] Four logical joints and five physical-servo target slots.
- [x] Per-joint limits, defaults, and maximum rates.
- [x] Joint clamping and rate-limited updates.
- [x] Mapping from four logical joints to five calibrated ST3215 positions.
- [x] UART1 setup at 1 Mbps using GPIO18 for RX and GPIO19 for TX.
- [x] ST3215 ping packet construction and response validation path.
- [x] Individual position-write packet construction and transmission path.
- [x] Present-position read request and response-validation path.
- [x] Five-servo synchronized-write packet construction and transmission path.
- [x] A 50 Hz FreeRTOS task that maps and transmits a fixed default joint pose.
- [x] Separation of joint math, servo configuration, driver code, motion-task code, and startup code.

### Partial scaffolding, not completed features

- `ArmGeometry` and `CartesianPosition` types exist in the working tree.
- Drawing-derived geometry constants exist for the base height, shoulder-to-elbow offsets, and elbow-to-tool distance.
- `kinematics.h` is empty and not registered in `main/CMakeLists.txt`; there is no FK or IK function.
- `command_task.h` and `command_task.cpp` are empty and not registered in the build.

### Important current limitations

- The motion task initializes both current and target joints to the same fixed default pose. There is no live command source.
- The periodic task does not react to a failed motion cycle with a defined fault or safe state.
- Physical zero positions, directions, joint limits, and speed limits are represented in configuration but are not proven final by repository evidence.
- There are no repository tests for the pure joint math or packet construction.
- There is no current repository evidence of successful flashing, boot, servo responses, synchronized physical motion, or feedback accuracy for this firmware state.
- There is no simulator, trajectory planner, ROS 2 package, Jetson integration, perception pipeline, neural network, or reinforcement-learning environment.

### Low-level control concepts to preserve

- The ST3215 daisy chain is one shared communication bus. Each packet carries a destination ID; physical cable order does not select a servo.
- `ServoIdx` is an array position inside the program, while `busId` is the address carried by a serial packet. They are not interchangeable.
- The shoulder is one logical joint driven by two physical servos. Both receive the shoulder command but have separate calibration entries.
- Joint targets are clamped to physical limits before rate limiting:

  ```text
  safeTarget = clamp(requestedTarget, minimum, maximum)
  maximumStep = maximumDegreesPerSecond * deltaTimeSeconds
  nextPosition = move currentPosition toward safeTarget by at most maximumStep
  ```

- Joint angles are converted to servo steps using the selected zero and direction, followed by a valid ST3215 step clamp:

  ```text
  direction = reversed ? -1 : +1
  rawSteps = zeroStep + direction * jointDegrees * (4096 / 360)
  positionSteps = round and clamp rawSteps to [0, 4095]
  ```

Angle wrapping and joint limiting solve different problems. Wrapping finds an equivalent cyclic angle; clamping enforces the mechanism's physical range.

## Roadmap summary

| Phase | Goal | Status |
|---|---|---|
| 1 | Refactor and harden the C++ control foundation | In progress |
| 2 | Forward kinematics | Not started; data scaffolding only |
| 3 | Inverse kinematics | Not started |
| 4 | MuJoCo physics simulation | Not started |
| 5 | Trajectory planning | Not started |
| 6 | Jetson and ROS 2 integration | Not started |
| 7 | Computer vision and neural networks | Not started |
| 8 | Reinforcement learning | Not started |

## Phase 1 — Code refactoring and control-foundation hardening

**Goal:** Turn the current firmware into a small, clear, deterministic foundation that is safe to extend.

Apply Uncle Bob's Clean Code ideas where they improve readability—clear names, small cohesive functions, and single responsibilities—together with NASA/JPL-inspired embedded practices such as bounded work, explicit state, fixed-size data, defensive checks, and deterministic behavior. These are engineering guides, not reasons to add abstraction.

The existing separation into arm math, servo configuration, driver, motion task, and startup is useful progress, but this phase is not complete. It still needs test coverage, a real command/state boundary, explicit error behavior, and hardware safety validation.

### Learning and implementation objectives

- Keep functions small and cohesive, names explicit, and control flow easy to inspect.
- Use fixed-size containers, explicit-width types, bounded work per cycle, and no unnecessary dynamic allocation in the control path.
- Make units visible in names and interfaces: degrees, radians, meters, seconds, encoder steps.
- Keep pure joint and robot math independent of ESP-IDF and UART.
- Keep hardware access behind narrow driver functions with truthful return values.
- Check every important result and define what happens after communication or task failures.
- Remove unused placeholders until their phase begins, or clearly keep them outside the build without presenting them as features.
- Add the smallest useful tests first: clamping, rate limiting, degree-to-step conversion, joint-to-servo mapping, and packet bytes/checksums.
- Define minimal command and state data that can later be shared conceptually with MuJoCo and ROS 2.
- Avoid generalized frameworks, dependency injection systems, or plugin architectures unless a concrete need appears.

### Safety work required before autonomous physical control

- Validate physical joint limits, servo zeros, direction signs, and coupled-shoulder behavior.
- Reject invalid, non-finite, out-of-range, or incorrectly ordered commands.
- Define startup, shutdown, timeout, stale-command, lost-communication, and repeated-write-failure behavior.
- Provide a deliberate enable/disable or emergency-stop path appropriate to the hardware.
- Make autonomous commands opt-in and distinguish bench/simulation operation from powered-arm operation.
- Verify at low speed with a clear workspace, reachable power cutoff, and no person in the arm's motion envelope.

### Completion criteria

- [ ] The build passes with warnings reviewed and no unused placeholder presented as implemented functionality.
- [ ] Pure control math has repeatable tests that run without ESP32 hardware.
- [ ] A minimal validated command enters the control loop through one explicit boundary.
- [ ] Robot state and driver errors leave the low-level layer through one explicit boundary.
- [ ] All motion-cycle results are handled and a safe response is defined for every expected failure class.
- [ ] Calibration and limits are recorded from controlled hardware checks.
- [ ] Flash, boot, valid UART exchange, feedback reads, and guarded synchronized motion are separately demonstrated.
- [ ] The safety behaviors above are tested before high-level or autonomous software can command physical motion.

## Phase 2 — Forward kinematics (FK)

**Goal:** Convert a validated joint configuration into the end-effector pose in a documented robot coordinate frame.

Start with the arm's position geometry before adding orientation. The current geometry constants are drawing-derived assumptions and must be checked against the real arm and the chosen joint-zero definitions.

### Learning and implementation objectives

- Define the base frame, positive axis directions, positive joint rotations, zero pose, joint order, and units.
- Draw the kinematic chain and verify every link measurement and offset.
- Learn homogeneous transforms or an equivalent explicit transform representation.
- Implement FK as pure C++ math with no UART, FreeRTOS, ROS 2, or MuJoCo dependencies.
- Test simple poses by hand before testing arbitrary angles.
- Compare computed positions with measurements or a simple visualization.

### Completion criteria

- [ ] Frame conventions, joint conventions, geometry sources, and assumptions are documented.
- [ ] Geometry values have known units and traceable measurements or drawings.
- [ ] FK returns a Cartesian pose for all valid joint configurations.
- [ ] Hand-calculated canonical poses match automated tests within a stated tolerance.
- [ ] Invalid/non-finite input is rejected or handled explicitly.
- [ ] At least several measured or visualized arm poses agree with the model within a recorded tolerance.

## Phase 3 — Inverse kinematics (IK)

**Goal:** Convert a reachable Cartesian target into one or more valid joint configurations.

Begin with the simplest arm model supported by Phase 2. Prefer a clear analytic solution when practical; add numerical methods only if the actual geometry or orientation requirements justify them.

### Learning and implementation objectives

- Understand reachable workspace, singularities, multiple solutions, and joint-limit filtering.
- Solve position-only IK before full pose IK.
- Make elbow-up/elbow-down or other solution branches explicit.
- Select solutions using limits, continuity, and distance from the current joint state.
- Keep IK pure and independently testable.
- Never send an IK result directly to hardware without command validation and trajectory generation.

### Completion criteria

- [ ] Reachable targets produce valid joint solutions within limits.
- [ ] Unreachable, singular, ambiguous, and non-finite targets return explicit status information.
- [ ] `FK(IK(target))` reproduces representative targets within a stated tolerance.
- [ ] Solution selection is deterministic and avoids unnecessary branch jumps.
- [ ] Workspace-boundary and singularity tests exist.
- [ ] IK is verified off-hardware before any guarded physical check.

## Phase 4 — MuJoCo physics simulation

**Goal:** Build a digital robot model that reproduces the arm's kinematic structure and enough dynamics/contact behavior to test later phases safely.

### Learning and implementation objectives

- Create a MuJoCo model with the correct link hierarchy, joint axes, limits, actuator mapping, and gripper representation.
- Add visually useful meshes only after the joint structure and collision geometry are correct.
- Define plausible masses, centers of mass, inertias, damping, friction, and actuator limits from measured or documented data where possible.
- Build a thin simulation adapter around the shared command/state concepts.
- Compare MuJoCo link poses against the independent FK implementation.
- Keep simulation-specific types outside the core mathematics.

### Completion criteria

- [ ] The model loads without warnings and has the expected degrees of freedom and joint limits.
- [ ] Named joints and state ordering match the documented robot convention.
- [ ] Canonical simulated poses agree with FK within a stated tolerance.
- [ ] Gravity, contacts, limits, and actuator behavior are stable for repeatable tests.
- [ ] The simulator accepts the planned robot command concept and reports the planned robot state concept.
- [ ] Known differences between the simulated and physical arm are documented.

## Phase 5 — Trajectory planning

**Goal:** Move between valid states smoothly while respecting joint, velocity, acceleration, workspace, and timing constraints.

### Learning and implementation objectives

- Start with time-parameterized joint-space interpolation.
- Add velocity and acceleration limits before optimizing speed.
- Sample and inspect trajectories in MuJoCo before physical execution.
- Add Cartesian path generation only after joint-space trajectories and IK are reliable.
- Detect IK failure, discontinuities, limit violations, and timing overruns before execution.
- Separate the slower planner from the deterministic low-level ESP32 update loop.

### Completion criteria

- [ ] Joint-space trajectories begin and end at the requested states and respect all configured limits.
- [ ] Position, velocity, and acceleration remain continuous to the selected planning standard.
- [ ] Invalid or infeasible requests fail before producing executable commands.
- [ ] Cartesian paths use FK/IK consistently and expose unreachable segments.
- [ ] Trajectories run repeatably in MuJoCo without limit or collision violations for the tested scenes.
- [ ] Only after the safety gate: representative trajectories run at reduced speed on hardware with monitored feedback and an immediate stop path.

## Phase 6 — Jetson and ROS 2 integration

**Goal:** Separate high-level perception and planning from deterministic ESP32 control while preserving a small, testable interface between them.

### Learning and implementation objectives

- Run ROS 2 on the Jetson; do not turn the ESP32 into the high-level planner.
- Define minimal time-stamped command, state, and fault data with explicit units and joint ordering.
- Choose a transport only after measuring bandwidth, latency, reliability, and recovery needs.
- Provide one ROS 2-facing adapter for the physical arm and one for MuJoCo while reusing core conventions.
- Keep communication loss, stale data, restarts, and version mismatches visible and safe.
- Add observability: command age, state age, error status, loop timing, and dropped-message information.

### Completion criteria

- [ ] Jetson and ESP32 responsibilities are documented and enforced by the code boundaries.
- [ ] Message/interface definitions specify units, frames, timestamps, limits, versioning, and fault semantics.
- [ ] ROS 2 can command and observe MuJoCo through the shared concepts.
- [ ] The ESP32 rejects malformed, invalid, and stale high-level commands.
- [ ] Disconnect and restart tests lead to a defined safe state.
- [ ] After simulation and safety validation, the Jetson can command a guarded, low-speed hardware trajectory while receiving state and faults.

## Phase 7 — Computer vision and neural networks

**Goal:** Detect and localize task-relevant objects, then publish uncertainty-aware targets for planning rather than directly commanding servos.

### Learning and implementation objectives

- Calibrate the camera and define transforms between camera, robot base, and workspace frames.
- Establish a simple classical-vision or fiducial baseline before adding a neural network.
- Collect and label representative data with training, validation, and test splits.
- Train or fine-tune an object detector and measure accuracy, false detections, latency, and robustness.
- Convert detections into 3D locations with confidence and validity checks.
- Run inference on the Jetson and keep perception asynchronous from low-level control.
- Handle missing, stale, low-confidence, and out-of-workspace detections safely.

### Completion criteria

- [ ] Camera calibration and coordinate transforms are measured and documented.
- [ ] Perception output includes timestamp, frame, confidence, and validity information.
- [ ] Accuracy and latency meet recorded task-specific thresholds on held-out scenes.
- [ ] The planner rejects stale, uncertain, or unreachable perception targets.
- [ ] The full perception-to-plan path succeeds in simulation or an observe-only hardware mode.
- [ ] Physical closed-loop trials begin only after command validation and human-supervised safety checks pass.

## Phase 8 — Reinforcement learning

**Goal:** Train manipulation policies in simulation and deploy them only after quantitative evaluation and safety-constrained integration.

### Learning and implementation objectives

- Define observations and actions using the same robot state and command conventions as the rest of the platform.
- Begin with a narrow simulated task and a simple baseline controller.
- Design rewards that reflect the task without encouraging unsafe shortcuts.
- Randomize relevant dynamics, sensing, object placement, delay, and noise to study the simulation-to-reality gap.
- Keep policy inference on the Jetson; keep hard bounds and safety enforcement on the ESP32.
- Route policy outputs through the same validation and trajectory/safety layers as planned commands.
- Compare learned performance against non-learning baselines.

### Completion criteria

- [ ] A reproducible training environment, configuration, seed strategy, and evaluation protocol exist.
- [ ] The policy exceeds a documented baseline across held-out simulated conditions.
- [ ] Robustness is measured under dynamics, latency, noise, and object variations.
- [ ] Policy outputs cannot bypass joint, rate, workspace, timeout, or emergency-stop constraints.
- [ ] Jetson inference meets a measured latency budget in simulation or hardware-in-the-loop tests.
- [ ] Physical deployment begins with reduced speed, restricted workspace, human supervision, and an immediate stop path.
- [ ] Real-hardware results and failures are recorded separately from simulation results.

## Cross-phase working method

For every new capability:

1. State the smallest question the capability must answer.
2. Define units, frames, inputs, outputs, validity rules, and failure behavior.
3. Implement pure math or logic before framework integration where possible.
4. Add focused tests and hand-check at least one simple example.
5. Build and run off-hardware first.
6. Validate in MuJoCo when dynamics or motion are involved.
7. Review the physical safety gate before flashing or commanding actuators.
8. Record source, build, flash, runtime, communication, simulation, and physical evidence separately.
9. Update this roadmap only when the repository and recorded evidence support the new status.

## Current next checkpoint

Continue **Phase 1**. Before implementing FK, make the existing control foundation easier to test and safer to extend:

- Preserve the current clean separation between pure joint math and ST3215/FreeRTOS code.
- Add focused off-hardware tests for the existing math and packet logic.
- Define the smallest validated joint-command and robot-state boundary.
- Decide and document safe behavior for write failure, stale commands, startup, and shutdown.
- Validate the physical calibration and low-level motion evidence separately when hardware work is explicitly authorized.

Do not treat the current geometry structs or empty `kinematics.h` as completed FK.

## References

- [Waveshare ST3215 documentation](https://docs.waveshare.com/ST3215_Servo)
- [Waveshare ST3215 serial command examples](https://docs.waveshare.com/ST3215_Servo/Serial-Debugging-Tool-Use)
- [ESP-IDF UART API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html)
- [ESP-IDF FreeRTOS overview](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos_idf.html)
- [MuJoCo documentation](https://mujoco.readthedocs.io/)
- [ROS 2 documentation](https://docs.ros.org/)
