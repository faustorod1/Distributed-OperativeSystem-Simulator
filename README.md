# Distributed Operating System Simulator (C)
A high-performance simulation of a **Distributed Operating System** developed for the **Software Systems (Sistemas Operativos)** course at **UTN FRBA**. This project implements a modular architecture using **POSIX Sockets** and **Multithreading** to simulate hardware-level resource management.

## 🏗️ System Architecture
The platform is composed of four independent modules that interact via a custom networking protocol:

* **Kernel:** The central orchestrator. It manages process and thread scheduling (FIFO, Prioridades, CMN), handles **Syscalls**, and manages inter-module synchronization using `pthreads`.
* **CPU:** Simulates the Fetch-Decode-Execute cycle. It interfaces with the Kernel via `Dispatch` and `Interrupt` sockets and translates logical addresses to physical ones through the MMU.
* **Memory:** Manages simulated RAM using **Paging/Segmentation**. Supports memory dump operations (`DUMP_MEMORY`) and protection. Also implements a flexible allocation system, supporting multiple algorithms such as First-Fit, Best-Fit, and Worst-Fit to optimize RAM utilization and manage fragmentation.
* **FileSystem:** Handles persistent storage, implementing disk structures and metadata protocols.

## 🛠️ Technical Highlights
Based on the provided source code, this implementation features:
* **Thread-Safe Kernel:** Advanced management of `pthread_create` and `pthread_detach` to handle asynchronous operations like memory dumps.
* **Robust Syscall Interface:** Implementation of a wide range of operations: `PROCESS_CREATE`, `THREAD_JOIN`, `MUTEX_LOCK`, `IO` requests, and `DUMP_MEMORY`.
* **Custom IPC Protocol:** Reliable communication using the `so-commons-library`, including Handshake protocols and operation code (`op_code`) handling.
* **Memory Management Unit (MMU):** Logic for simulating realistic memory access and handling `SEGMENTATION_FAULT` conditions.

## 👥 Contributors
- Juan Fernandez
- Juan Pablo Montemarani
- Manuel Rafael
- Fausto Rodríguez
- Juan Tarducci


  
# tp-scaffold

Esta es una plantilla de proyecto diseñada para generar un TP de Sistemas
Operativos de la UTN FRBA.

## Dependencias

Para poder compilar y ejecutar el proyecto, es necesario tener instalada la
biblioteca [so-commons-library] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

## Compilación y ejecución

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante de la compilación se guardará en la carpeta `bin` del
módulo. Ejemplo:

```sh
cd kernel
make
./bin/kernel
```

## Importar desde Visual Studio Code

Para importar el workspace, debemos abrir el archivo `tp.code-workspace` desde
la interfaz o ejecutando el siguiente comando desde la carpeta raíz del
repositorio:

```bash
code tp.code-workspace
```

## Checkpoint

Para cada checkpoint de control obligatorio, se debe crear un tag en el
repositorio con el siguiente formato:

```
checkpoint-{número}
```

Donde `{número}` es el número del checkpoint, ejemplo: `checkpoint-1`.

Para crear un tag y subirlo al repositorio, podemos utilizar los siguientes
comandos:

```bash
git tag -a checkpoint-{número} -m "Checkpoint {número}"
git push origin checkpoint-{número}
```

> [!WARNING]
> Asegúrense de que el código compila y cumple con los requisitos del checkpoint
> antes de subir el tag.

## Entrega

Para desplegar el proyecto en una máquina Ubuntu Server, podemos utilizar el
script [so-deploy] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-deploy.git
cd so-deploy
./deploy.sh -r=release -p=utils -p=kernel -p=cpu -p=memoria -p=filesystem "tp-{año}-{cuatri}-{grupo}"
```

El mismo se encargará de instalar las Commons, clonar el repositorio del grupo
y compilar el proyecto en la máquina remota.

> [!NOTE]
> Ante cualquier duda, pueden consultar la documentación en el repositorio de
> [so-deploy], o utilizar el comando `./deploy.sh --help`.

## Guías útiles

- [Cómo interpretar errores de compilación](https://docs.utnso.com.ar/primeros-pasos/primer-proyecto-c#errores-de-compilacion)
- [Cómo utilizar el debugger](https://docs.utnso.com.ar/guias/herramientas/debugger)
- [Cómo configuramos Visual Studio Code](https://docs.utnso.com.ar/guias/herramientas/code)
- **[Guía de despliegue de TP](https://docs.utnso.com.ar/guías/herramientas/deploy)**

[so-commons-library]: https://github.com/sisoputnfrba/so-commons-library
[so-deploy]: https://github.com/sisoputnfrba/so-deploy
