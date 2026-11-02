# Signal Control Node

`signal-control-node` is a work-in-progress C and FreeRTOS firmware project for
a small real-time signal monitor and control node.

The planned target is an STM32 Nucleo-F446RE. The current implementation runs
on a host with the FreeRTOS POSIX port.

## Current implementation

- `sigctl` provides bounded moving-average filtering, threshold classification,
  and two-threshold hysteresis.
- `sigctl_node` parses bounded `STATUS` and `SET THRESHOLDS <low> <high>`
  commands and owns fault-aware output state.
- A three-task FreeRTOS harness uses statically allocated queues and stacks,
  injects a processing stall, and verifies the latched safe-output behavior.

The command parser operates on in-memory lines; it is not a UART transport.

## Build and test

Requirements: CMake 3.23 or newer, a C compiler, Git, and network access for
the initial FreeRTOS download.

```sh
cmake --preset host
cmake --build --preset host
ctest --preset host --output-on-failure
```

The build fetches FreeRTOS-Kernel from its official repository at the pinned
MIT-licensed revision `ce221a8bb468e462ca6b435cef66a9636e00baf4`.

## Current limits

The host tests do not validate STM32 execution, ADC, I2C, UART, GPIO, PWM,
physical timing, or hardware debugging. Those require the planned firmware
target and physical board work. No hardware results are claimed yet.
