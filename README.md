# ChadOS Peeter

**ChadOS** is a passion project operating system built by **Krabumb** and **Gouuu**, with the goal of understanding what a computer truly is, and unleashing the raw power it holds.  

Unlike many hobby kernels that stay in text mode, ChadOS aims to provide a **graphical environment**. At its core, ChadOS strives to be:  

- Usable with at least a **terminal interface**
- Powered by a **built-in scripting language (cosliv)**  
- Equipped with a **minimal library of executables (cosbin)**  
- A foundation for exploring both **low-level systems programming** and **higher-level OS design**  

---

## 🛠️ Build & Installation

Before compiling ChadOS, you’ll need to set up the **cross-compilation toolchain** for `x86_64-elf`.

### 1. Install the Toolchain
From the project root:  
```bash
cd compiler
./install.sh
```

This script will automatically install and configure the `x86_64-elf` toolchain.

### 2. Compile ChadOS
Back in the project root, simply run:  
```bash
make all
```

### 3. Clean Build Files
To remove compiled objects and build artifacts:  
```bash
make clean
```

### 4. Run with QEMU
To boot ChadOS inside QEMU:  
```bash
make run
```

---

## 🚀 Vision
ChadOS is **NOT** meant to replace your daily Linux distro — it’s a playground to **learn and experiment**. Expect frequent breaking changes and strange bugs.  

This is **not just another OS**.  
This is **my ChadOS**.
