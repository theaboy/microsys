# microsys

microsys is a lightweight operating system simulation written in C.
The project explores core concepts such as command interpretation,
process execution, memory abstraction, and filesystem interaction.

The goal of this project is to incrementally build a minimal yet
structured system that resembles how real operating systems organize
and execute responsibilities.

## Features

- Custom command-line shell
- Interactive and batch execution modes
- Environment-style variable storage
- Basic filesystem operations
- Process execution using fork–exec–wait

## Architecture

The system is designed with modular components:

- `shell/` – command parsing and interpretation
- `memory/` – internal variable storage and lookup
- `fs/` – filesystem utilities and directory management
- `process/` – process creation and execution logic

This separation allows the system to grow naturally as new
capabilities are added.

## Build

```bash
make clean
make
