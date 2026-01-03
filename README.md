# Distributed Operating System Simulator (C)
A high-performance simulation of a **Distributed Operating System** developed for the **Software Systems (Sistemas Operativos)** course at **UTN FRBA**. This project implements a modular architecture using **POSIX Sockets** and **Multithreading** to simulate hardware-level resource management.

## 🏗️ System Architecture
The system is designed as a distributed network of specialized modules:

### 🧠 Kernel (The Orchestrator)
The central manager of the system:
  - Multi-level Scheduling: Supports FIFO, Priority, and CMN (Custom Multilevel) algorithms through specialized configuration flags (-1.1 to -1.3).
  - Syscall Handling: A robust switch-case architecture that processes requests like PROCESS_CREATE, THREAD_JOIN, MUTEX_LOCK, and IO.
  - Asynchronous Operations: Utilizes detached threads to handle complex operations like DUMP_MEMORY without blocking the entire scheduler.

### ⚡ CPU (Instruction Cycle)
Simulates the hardware execution core:
  - Fetch-Decode-Execute: Implementation of the standard instruction cycle.
  - Interrupt Logic: A dedicated thread constantly listens on a separate Interrupt Port to handle quantum expiration or preemptive signals.
  - Context Switching: Dynamic loading and saving of Execution Contexts (Registers: AX, BX, CX, DX, PC, etc.) from/to the Memory module.

### 📟 Memory (MMU)
A simulated RAM manager:
  - Partition Strategies: Implements Fixed Partitions and Dynamic Partitioning with multiple allocation algorithms: First-Fit, Best-Fit, and Worst-Fit.
  - Concurrency: Handles simultaneous requests from Kernel and CPU using a multi-threaded server architecture.
  - Memory Protection: Validates logical addresses against base and limit registers to prevent unauthorized access.

### 📂 FileSystem (Storage)
A custom file manager:
  - Bitarray Management: Tracks block occupancy using a bitmap.dat mapped into memory via mmap.
  - Indexed Allocation: Implements an indexed block structure to store files, managing metadata and data blocks independently.
  - Persistent Storage: Simulated disk storage via binary files (bloques.dat) with configurable access delays.



## 🛠️ Technical Highlights
- Inter-Process Communication (IPC): Developed a custom communication protocol over TCP Sockets, featuring Handshakes and Packet Serialization.
- Advanced Synchronization: Extensive use of Semaphores and Mutexes to ensure thread safety during resource contention.
- Memory Safety: Strict management of pointers and memory allocation, verified with Valgrind to ensure a leak-free execution.
- Modular Build System: Automated compilation via Makefiles for independent module deployment.

## 👥 Contributors
- Juan Fernandez
- Juan Pablo Montemarani
- Manuel Rafael
- Fausto Rodríguez
- Juan Tarducci
  
## 🚀 Getting Started

### Dependencias
Install the so-commons-library (required for networking and logging):

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

### Compilation & Execution
Compile each module independently:
```bash
# Example for Kernel
cd kernel && make
./bin/kernel {pseudocode_path} {size} {test_flag}
```
Flags example: -1.1 (FIFO), -4 (Dynamic Partitions), -6 (Stress Test).
