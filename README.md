
PicoRV32 - A Size-Optimized RISC-V CPU
======================================

PicoRV32 is a CPU core that implements the [RISC-V RV32I Instruction Set](http://riscv.org/).

Tools (gcc, binutils, etc..) can be obtained via the [RISC-V Website](http://riscv.org/download.html#tab_tools).
The examples bundled with PicoRV32 (such as the firmware for `make test`) expect a `riscv32-unknown-elf-` toolchain
installed in `$PATH` (see [build instructions below](#building-a-pure-rv32i-toolchain)).

PicoRV32 is free and open hardware licensed under the [ISC license](http://en.wikipedia.org/wiki/ISC_license)
(a license that is similar in terms to the MIT license or the 2-clause BSD license).

#### Table of Contents

- [Features and Typical Applications](#features-and-typical-applications)
- [Files in this Repository](#files-in-this-repository)
- [Verilog Module Parameters](#verilog-module-parameters)
- [Cycles per Instruction Performance](#cycles-per-instruction-performance)
- [PicoRV32 Native Memory Interface](#picorv32-native-memory-interface)
- [Pico Co-Processor Interface (PCPI)](#pico-co-processor-interface-pcpi)
- [Custom Instructions for IRQ Handling](#custom-instructions-for-irq-handling)
- [Building a pure RV32I Toolchain](#building-a-pure-rv32i-toolchain)
- [Evaluation: Timing and Utilization on Xilinx 7-Series FPGAs](#evaluation-timing-and-utilization-on-xilinx-7-series-fpgas)


Features and Typical Applications
---------------------------------

- Small (~1000 LUTs in a 7-Series Xilinx FPGA)
- High fMAX (~250 MHz on 7-Series Xilinx FPGAs)
- Selectable native memory interface or AXI4-Lite master
- Optional IRQ support (using a simple custom ISA)
- Optional Co-Processor Interface

This CPU is meant to be used as auxiliary processor in FPGA designs and ASICs. Due
to its high fMAX it can be integrated in most existing designs without crossing
clock domains. When operated on a lower frequency, it will have a lot of timing
slack and thus can be added to a design without compromising timing closure.

For even smaller size it is possible disable support for registers `x16`..`x31` as
well as `RDCYCLE[H]`, `RDTIME[H]`, and `RDINSTRET[H]` instructions, turning the
processor into an RV32E core.

Furthermore it is possible to choose between a dual-port and a single-port
register file implementation. The former provides better performance while
the latter results in a smaller core.

*Note: In architectures that implement the register file in dedicated memory
resources, such as many FPGAs, disabling the 16 upper registers and/or
disabling the dual-port register file may not further reduce the core size.*

The core exists in two variations: `picorv32` and `picorv32_axi`. The former
provides a simple native memory interface, that is easy to use in simple
environments, and the latter provides an AXI-4 Lite Master interface that can
easily be integrated with existing systems that are already using the AXI
standard.

A separate core `picorv32_axi_adapter` is provided to bridge between the native
memory interface and AXI4. This core can be used to create custom cores that
include one or more PicoRV32 cores together with local RAM, ROM, and
memory-mapped peripherals, communicating with each other using the native
interface, and communicating with the outside world via AXI4.

The optional IRQ feature can be used to react to events from the outside, implement
fault handlers, or catch instructions from a larger ISA and emulate them in
software.

The optional Pico Co-Processor Interface (PCPI) can be used to implement
non-branching instructions in an external coprocessor. An implementation
of a core that implements the `MUL[H[SU|U]]` instructions is provided.


Files in this Repository
------------------------

#### README.md

You are reading it right now.

#### picorv32.v

This Verilog file contains the following Verilog modules:

| Module                  | Description                                                   |
| ----------------------- | ------------------------------------------------------------- |
| `picorv32`              | The PicoRV32 CPU                                              |
| `picorv32_axi`          | The version of the CPU with AXI4-Lite interface               |
| `picorv32_axi_adapter`  | Adapter from PicoRV32 Memory Interface to AXI4-Lite           |
| `picorv32_pcpi_mul`     | A PCPI core that implements the `MUL[H[SU|U]]` instructions   |

Simply copy this file into your project.

#### Makefile and testbench.v

A basic test environment. Run `make test`, `make test_sp` and/or `make test_axi` to run
the test firmware in different hardware configurations.

*Note: The test bench is using Icarus Verilog. However, Icarus Verilog 0.9.7
(the latest release at the time of writing) has a few bugs that prevent the
test bench from running. Upgrade to the latest github master of Icarus Verilog
to run the test bench.*

#### firmware/

A simple test firmware. This runs the basic tests from `tests/`, some C code, tests IRQ
handling and the multiply PCPI core.

All the code in `firmware/` is in the public domain. Simply copy whatever you can use.

#### tests/

Simple instruction-level tests from [riscv-tests](https://github.com/riscv/riscv-tests).

#### dhrystone/

Another simple test firmware that runs the Dhrystone benchmark.

#### scripts/

Various scripts and examples for different (synthesis) tools and hardware architectures.


Verilog Module Parameters
-------------------------

The following Verilog module parameters can be used to configure the PicoRV32
core.

#### ENABLE_COUNTERS (default = 1)

This parameter enables support for the `RDCYCLE[H]`, `RDTIME[H]`, and
`RDINSTRET[H]` instructions. This instructions will cause a hardware
trap (like any other unsupported instruction) if `ENABLE_COUNTERS` is set to zero.

*Note: Strictly speaking the `RDCYCLE[H]`, `RDTIME[H]`, and `RDINSTRET[H]`
instructions are not optional for an RV32I core. But chances are they are not
going to be missed after the application code has been debugged and profiled.
This instructions are optional for an RV32E core.*

#### ENABLE_REGS_16_31 (default = 1)

This parameter enables support for registers the `x16`..`x31`. The RV32E ISA
excludes this registers. However, the RV32E ISA spec requires a hardware trap
for when code tries to access this registers. This is not implemented in PicoRV32.

#### ENABLE_REGS_DUALPORT (default = 1)

The register file can be implemented with two or one read ports. A dual ported
register file improves performance a bit, but can also increase the size of
the core.

#### LATCHED_MEM_RDATA (default = 0)

Set this to 1 if the `mem_rdata` is kept stable by the external circuit after a
transaction. In the default configuration the PicoRV32 core only expects the
`mem_rdata` input to be valid in the cycle with `mem_valid && mem_ready` and
latches the value internally.

This parameter is only available for the `picorv32` core. In the
`picorv32_axi` core this is implicitly set to 0.

#### TWO_STAGE_SHIFT (default = 1)

By default shift operations are performed in two stages: first shift in units
of 4 bits and then shift in units of 1 bit. This speeds up shift operations,
but adds additional hardware. Set this parameter to 0 to disable the two-stage
shift to further reduce the size of the core.

#### TWO_CYCLE_COMPARE (default = 0)

This relaxes the longest data path a bit by adding an additional FF stage
at the cost of adding an additional clock cycle delay to the conditional
branch instructions.

*Note: Enabling this parameter will be most effective when retiming (aka
"register balancing") is enabled in the synthesis flow.*

#### TWO_CYCLE_ALU (default = 0)

This adds an additional FF stage in the ALU data path, improving timing
at the cost of an additional clock cycle for all instructions that use
the ALU.

*Note: Enabling this parameter will be most effective when retiming (aka
"register balancing") is enabled in the synthesis flow.*

#### CATCH_MISALIGN (default = 1)

Set this to 0 to disable the circuitry for catching misaligned memory
accesses.

#### CATCH_ILLINSN (default = 1)

Set this to 0 to disable the circuitry for catching illegal instructions.

#### ENABLE_PCPI (default = 0)

Set this to 1 to enable the Pico Co-Processor Interface (PCPI).

#### ENABLE_MUL (default = 0)

This parameter internally enables PCPI and instantiates the `picorv32_pcpi_mul`
core that implements the `MUL[H[SU|U]]` instructions. The external PCPI
interface only becomes functional when ENABLE_PCPI is set as well.

#### ENABLE_IRQ (default = 0)

Set this to 1 to enable IRQs. (see "Custom Instructions for IRQ Handling" below
for a discussion of IRQs)

#### ENABLE_IRQ_QREGS (default = 1)

Set this to 0 to disable support for the `getq` and `setq` instructions. Without
the q-registers, the irq return address will be stored in x3 (gp) and the IRQ
bitmask in x4 (tp), the global pointer and thread pointer registers according
to the RISC-V ABI.  Code generated from ordinary C code will not interact with
those registers.

Support for q-registers is always disabled when ENABLE_IRQ is set to 0.

#### ENABLE_IRQ_TIMER (default = 1)

Set this to 0 to disable support for the `timer` instruction.

Support for the timer is always disabled when ENABLE_IRQ is set to 0.

#### MASKED_IRQ (default = 32'h 0000_0000)

A 1 bit in this bitmask corresponds to a permanently disabled IRQ.

#### LATCHED_IRQ (default = 32'h ffff_ffff)

A 1 bit in this bitmask indicates that the corresponding IRQ is "latched", i.e.
when the IRQ line is high for only one cycle, the interrupt will be marked as
pending and stay pending until the interrupt handler is called (aka "pulse
interrupts" or "edge-triggered interrupts").

Set a bit in this bitmask to 0 to convert an interrupt line to operate
as "level sensitive" interrupt.

#### PROGADDR_RESET (default = 32'h 0000_0000)

The start address of the program.

#### PROGADDR_IRQ (default = 32'h 0000_0010)

The start address of the interrupt handler.


Cycles per Instruction Performance
----------------------------------

*A short reminder: This core is optimized for size, not performance.*

Unless stated otherwise, the following numbers apply to a PicoRV32 with
ENABLE_REGS_DUALPORT active and connected to a memory that can accommodate
requests within one clock cycle.

The average Cycles per Instruction (CPI) is 4 to 5, depending on the mix of
instructions in the code. The CPI numbers for the individual instructions
can be found in the table below. The column "CPI (SP)" contains the
CPI numbers for a core built without ENABLE_REGS_DUALPORT.

| Instruction          |  CPI | CPI (SP) |
| ---------------------| ----:| --------:|
| direct jump (jal)    |    3 |        3 |
| ALU reg + immediate  |    3 |        3 |
| ALU reg + reg        |    3 |        4 |
| branch (not taken)   |    3 |        4 |
| memory load          |    5 |        5 |
| memory store         |    5 |        6 |
| branch (taken)       |    5 |        6 |
| indirect jump (jalr) |    6 |        6 |
| shift operations     | 4-14 |     4-15 |

When `ENABLE_MUL` is activated, then a `MUL` instruction will execute
in 40 cycles and a `MULH[SU|U]` instruction will execute in 72 cycles.

Dhrystone benchmark results: 0.311 DMIPS/MHz (547 Dhrystones/Second/MHz)

For the Dhrystone benchmark the average CPI is 4.144.


PicoRV32 Native Memory Interface
--------------------------------

The native memory interface of PicoRV32 is a simple valid-ready interface
that can run one memory transfer at a time:

    output        mem_valid
    output        mem_instr
    input         mem_ready

    output [31:0] mem_addr
    output [31:0] mem_wdata
    output [ 3:0] mem_wstrb
    input  [31:0] mem_rdata

The core initiates a memory transfer by asserting `mem_valid`. The valid
signal stays high until the peer asserts `mem_ready`. All core outputs
are stable over the `mem_valid` period.

#### Read Transfer

In a read transfer `mem_wstrb` has the value 0 and `mem_wdata` is unused.

The memory reads the address `mem_addr` and makes the read value available on
`mem_rdata` in the cycle `mem_ready` is high.

There is no need for an external wait cycle. The memory read can be implemented
asynchronously with `mem_ready` going high in the same cycle as `mem_valid`, or
`mem_ready` being tied to constant 1.

#### Write Transfer

In a write transfer `mem_wstrb` is not 0 and `mem_rdata` is unused. The memory
write the data at `mem_wdata` to the address `mem_addr` and acknowledges the
transfer by asserting `mem_ready`.

There is no need for an external wait cycle. The memory can acknowledge the
write immediately  with `mem_ready` going high in the same cycle as
`mem_valid`, or `mem_ready` being tied to constant 1.

#### Look-Ahead Interface

The PicoRV32 core also provides a "Look-Ahead Memory Interface" that provides
all information about the next memory transfer one clock cycle earlier than the
normal interface.

    output        mem_la_read
    output        mem_la_write
    output [31:0] mem_la_addr
    output [31:0] mem_la_wdata
    output [ 3:0] mem_la_wstrb

In the clock cycle before `mem_valid` goes high, this interface will output a
pulse on `mem_la_read` or `mem_la_write` to indicate the start of a read or
write transaction in the next clock cycles.

*Note: The signals `mem_la_read`, `mem_la_write`, and `mem_la_addr` are driven
by combinatorial circuits within the PicoRV32 core. It might be harder to
achieve timing closure with the look-ahead interface than with the normal
memory interface described above.*


Pico Co-Processor Interface (PCPI)
----------------------------------

The Pico Co-Processor Interface (PCPI) can be used to implement non-branching
instructions in external cores:

    output        pcpi_valid
    output [31:0] pcpi_insn
    output [31:0] pcpi_rs1
    output [31:0] pcpi_rs2
    input         pcpi_wr
    input  [31:0] pcpi_rd
    input         pcpi_wait
    input         pcpi_ready

When an unsupported instruction is encountered and the PCPI feature is
activated (see ENABLE_PCPI above), then `pcpi_valid` is asserted, the
instruction word itself is output on `pcpi_insn`, the `rs1` and `rs2`
fields are decoded and the values in those registers are output
on `pcpi_rs1` and `pcpi_rs2`.

An external PCPI core can then decode the instruction, execute it, and assert
`pcpi_ready` when execution of the instruction is finished. Optionally a
result value can be written to `pcpi_rd` and `pcpi_wr` asserted. The
PicoRV32 core will then decode the `rd` field of the instruction and
write the value from `pcpi_rd` to the respective register.

When no external PCPI core acknowledges the instruction within 16 clock
cycles, then an illegal instruction exception is raised and the respective
interrupt handler is called. A PCPI core that needs more than a couple of
cycles to execute an instruction, should assert `pcpi_wait` as soon as
the instruction has been decoded successfully and keep it asserted until
it asserts `pcpi_ready`. This will prevent the PicoRV32 core from raising
an illegal instruction exception.


Custom Instructions for IRQ Handling
------------------------------------

*Note: The IRQ handling features in PicoRV32 do not follow the RISC-V
Privileged ISA specification. Instead a small set of very simple custom
instructions is used to implement IRQ handling with minimal hardware
overhead.*

The following custom instructions are only supported when IRQs are enabled
via the `ENABLE_IRQ` parameter (see above).

The PicoRV32 core has a built-in interrupt controller with 32 interrupt inputs. An
interrupt can be triggered by asserting the corresponding bit in the `irq`
input of the core.

When the interrupt handler is started, the `eoi` End Of Interrupt (EOI) signals
for the handled interrupts go high. The `eoi` signals go low again when the
interrupt handler returns.

The IRQs 0-2 can be triggered internally by the following built-in interrupt sources:

| IRQ | Interrupt Source                   |
| ---:| -----------------------------------|
|   0 | Timer Interrupt                    |
|   1 | SBREAK or Illegal Instruction      |
|   2 | BUS Error (Unalign Memory Access)  |

This interrupts can also be triggered by external sources, such as co-processors
connected via PCPI.

The core has 4 additional 32-bit registers `q0 .. q3` that are used for IRQ
handling. When the IRQ handler is called, the register `q0` contains the return
address and `q1` contains a bitmask of all IRQs to be handled. This means one
call to the interrupt handler needs to service more than one IRQ when more than
one bit is set in `q1`.

Registers `q2` and `q3` are uninitialized and can be used as temporary storage
when saving/restoring register values in the IRQ handler.

All of the following instructions are encoded under the `custom0` opcode. The f3
and rs2 fields are ignored in all this instructions.

See [firmware/custom_ops.S](firmware/custom_ops.S) for GNU assembler macros that
implement mnemonics for this instructions.

See [firmware/start.S](firmware/start.S) for an example implementation of an
interrupt handler assembler wrapper, and [firmware/irq.c](firmware/irq.c) for
the actual interrupt handler.

#### getq rd, qs

This instruction copies the value from a q-register to a general-purpose
register.

    0000000 ----- 000XX --- XXXXX 0001011
    f7      rs2   qs    f3  rd    opcode

Example:

    getq x5, q2

#### setq qd, rs

This instruction copies the value from a general-purpose register to a
q-register.

    0000001 ----- XXXXX --- 000XX 0001011
    f7      rs2   rs    f3  qd    opcode

Example:

    setq q2, x5

#### retirq

Return from interrupt. This instruction copies the value from `q0`
to the program counter and re-enables interrupts.

    0000010 ----- 00000 --- 00000 0001011
    f7      rs2   rs    f3  rd    opcode

Example:

    retirq

#### maskirq

The "IRQ Mask" register contains a bitmask of masked (disabled) interrupts.
This instruction writes a new value to the irq mask register and reads the old
value.

    0000011 ----- XXXXX --- XXXXX 0001011
    f7      rs2   rs    f3  rd    opcode

Example:

    maskirq x1, x2

The processor starts with all interrupts disabled.

An illegal instruction or bus error while the illegal instruction or bus error
interrupt is disabled will cause the processor to halt.

#### waitirq

Pause execution until an interrupt becomes pending. The bitmask of pending IRQs
is written to `rd`.

    0000100 ----- 00000 --- XXXXX 0001011
    f7      rs2   rs    f3  rd    opcode

Example:

    waitirq x1

#### timer

Reset the timer counter to a new value. The counter counts down clock cycles and
triggers the timer interrupt when transitioning from 1 to 0. Setting the
counter to zero disables the timer. The old value of the counter is written to
`rd`.

    0000101 ----- XXXXX --- XXXXX 0001011
    f7      rs2   rs    f3  rd    opcode

Example:

    timer x1, x2


Building a pure RV32I Toolchain
-------------------------------

The default settings in the [riscv-tools](https://github.com/riscv/riscv-tools) build
scripts will build a compiler, assembler and linker that can target any RISC-V ISA,
but the libraries are built for RV32G and RV64G targets. Follow the instructions
below to build a complete toolchain (including libraries) that target a pure RV32I
CPU.

The following commands will build the RISC-V gnu toolchain and libraries for a
pure RV32I target, and install it in `/opt/riscv32i`:

    # Ubuntu packages needed:
    sudo apt-get install autoconf automake autotools-dev curl libmpc-dev libmpfr-dev \
            libgmp-dev gawk build-essential bison flex texinfo gperf

    sudo mkdir /opt/riscv32i
    sudo chown $USER /opt/riscv32i

    git clone https://github.com/riscv/riscv-gnu-toolchain riscv-gnu-toolchain-rv32i
    cd riscv-gnu-toolchain-rv32i
    git checkout 4bcd4f5

    mkdir build; cd build
    ../configure --with-xlen=32 --with-arch=I --prefix=/opt/riscv32i
    make -j$(nproc)

The commands will all be named using the prefix `riscv32-unknown-elf-`, which
makes it easy to install them side-by-side with the regular riscv-tools, which
are using the name prefix `riscv64-unknown-elf-` by default.

*Note: This instructions are for git rev 4bcd4f5 (2015-12-14) of riscv-gnu-toolchain.*


Evaluation: Timing and Utilization on Xilinx 7-Series FPGAs
-----------------------------------------------------------

The following evaluations have been performed with Vivado 2015.1.

#### Timing on Xilinx 7-Series FPGAs

The `picorv32_axi` module with enabled `TWO_CYCLE_ALU` has been placed and
routed for Xilinx Artix-7T (xc7a15t-fgg484), Xilinx Kintex-7T (xc7k70t-fbg676),
and Xilinx Virtex-7T (xc7v585t-ffg1761) devices in all speed grades. A binary
search is used to find the lowest clock period for which the design meets
timing.

See `make table.txt` in [scripts/vivado/](scripts/vivado/).

| Device               | Speedgrade | Clock Period (Freq.) |
|:-------------------- |:----------:| --------------------:|
| Xilinx Artix-7T      | -1         |     4.2 ns (238 MHz) |
| Xilinx Artix-7T      | -2         |     3.5 ns (285 MHz) |
| Xilinx Artix-7T      | -3         |     3.2 ns (312 MHz) |
| Xilinx Kintex-7T     | -1         |     2.8 ns (357 MHz) |
| Xilinx Kintex-7T     | -2         |     2.3 ns (434 MHz) |
| Xilinx Kintex-7T     | -3         |     2.1 ns (476 MHz) |
| Xilinx Virtex-7T     | -1         |     2.6 ns (384 MHz) |
| Xilinx Virtex-7T     | -2         |     2.3 ns (434 MHz) |
| Xilinx Virtex-7T     | -3         |     2.1 ns (476 MHz) |

#### Utilization on Xilinx 7-Series FPGAs

The following table lists the resource utilization in area-optimized synthesis
for the following three cores:

- **PicoRV32 (small):** The `picorv32` module without counter instructions,
  without two-stage shifts, with externally latched `mem_rdata`, and without
  catching of misaligned memory accesses and illegal instructions.

- **PicoRV32 (regular):** The `picorv32` module in its default configuration.

- **PicoRV32 (large):** The `picorv32` module with enabled PCPI, IRQ and MUL
  features.

See `make area` in [scripts/vivado/](scripts/vivado/).

| Core Variant       | Slice LUTs | LUTs as Memory | Slice Registers |
|:------------------ | ----------:| --------------:| ---------------:|
| PicoRV32 (small)   |        751 |             48 |             422 |
| PicoRV32 (regular) |        901 |             48 |             564 |
| PicoRV32 (large)   |       1718 |             88 |            1002 |

