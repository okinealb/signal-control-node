# Real-time signal monitor and control node

`realtime-node` is a planned C and FreeRTOS firmware project for a small
real-time signal monitor and control node.

The target system will sample an input, move data between tasks, process the
signal, control an output, expose UART telemetry, and handle injected faults.
The project will begin with host-side tests and simulation, then move to an
STM32 Nucleo-F446RE for hardware validation.

Current status: repository setup only. The detailed design record is kept
locally while the project is being planned. No firmware or hardware results are
claimed yet.
