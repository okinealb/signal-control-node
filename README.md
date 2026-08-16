# Signal Control Node

`signal-control-node` is a planned C and FreeRTOS firmware project for a small
real-time signal monitor and control node.

The target system will sample an input, move data between tasks, process the
signal, control an output, expose UART telemetry, and handle injected faults.
The host-side `signal_control` library currently contains moving-average
filtering and threshold classification. The project will then move to
FreeRTOS simulation and an STM32 Nucleo-F446RE for hardware validation.

Current status: host-side signal logic and a FreeRTOS POSIX task/queue harness.
STM32 firmware, hardware behavior, and physical results are not claimed yet.
