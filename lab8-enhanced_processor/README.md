# Lab 8 — Enhanced Processor

A 16-bit enhanced processor implemented in Verilog on the DE1-SoC FPGA board, 
extending the Lab 7 simple processor with memory access, a program counter, 
conditional branching, and ALU flag registers.

## Key Features
- 6-stage FSM controller (T0–T5)
- Memory interface: load (`ld`), store (`st`) instructions
- Program counter with auto-increment and branch support
- ALU flag registers: Zero, Carry, Negative
- Conditional branching: `eq`, `neq`, `cc`, `cs`, `pl`, `mi`
- AND instruction added to ALU

## Instruction Set
| Instruction | Description |
|-------------|-------------|
| `mv rX, rY` | Move register to register |
| `mv rX, #D` | Move immediate (sign extended) |
| `mvt rX, #D` | Move immediate shifted left 8 |
| `add rX, rY/\#D` | Addition |
| `sub rX, rY/\#D` | Subtraction |
| `and rX, rY/\#D` | Bitwise AND |
| `ld rX, [rY]` | Load from memory |
| `st rX, [rY]` | Store to memory |
| `b{cond}` | Conditional branch |

## Tools
- Verilog HDL
- Intel Quartus Prime
- DE1-SoC FPGA (Cyclone V)
