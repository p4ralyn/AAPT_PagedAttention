# 🚀 Agentic AI Paging Technology (AAPT)
**Native OS-Level Copy-on-Write Prefix Caching for the xv6 RISC-V Kernel**

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![RISC-V](https://img.shields.io/badge/RISC--V-2D303E?style=for-the-badge&logo=riscv&logoColor=white)
![Kernel](https://img.shields.io/badge/Kernel-xv6-blueviolet?style=for-the-badge)

AAPT is a hardware-aware memory manager natively built within the MIT xv6 RISC-V kernel to solve the memory redundancy problem in multi-agent AI systems. By implementing kernel-level Copy-on-Write (CoW), AAPT enables concurrent agents to share identical physical memory frames for foundational prompts, duplicating them only upon mutation.

---

## 🧠 The Problem: KV Cache Redundancy
Traditional operating systems deeply copy memory across process forks, leading to:
* **Massive Redundancy:** Multiple agents store identical copies of the "System Prompt".
* **Resource Exhaustion:** Rapid physical RAM depletion during Key-Value (KV) cache initialization.
* **Abstraction Gaps:** User-space solutions (like vLLM) cannot optimize native hardware mechanics like TLBs and hardware page table walks.

## ⚡ The AAPT Solution
AAPT synthesizes the theories of **PagedAttention** and **vAttention** directly into the kernel space.

### Technical Core
* **Physical Reference Tracking (`kalloc.c`):** A global `ref_count[]` array tracks concurrent mappings to single frames, protected by `kmem.lock` spinlocks.
* **CoW Forking (`vm.c`):** A custom `sys_aapt_cow_fork()` call revokes `PTE_W` (Write) permissions for both parent and child, enforcing a read-only shared state.
* **Trap-Driven Duplication (`trap.c`):** Intercepts RISC-V Exception Cause 15 (Store Page Fault) to dynamically allocate and duplicate pages only when an agent attempts unique token generation.

---

## 📊 Performance Results
Empirical benchmarking shows that AAPT successfully mitigates **100% of redundant KV cache allocation** during process initialization.

| Event Phase | Std xv6 Pages | AAPT Pages | Savings |
| :--- | :--- | :--- | :--- |
| **System Prompt** | 16 | 16 | Base |
| **3 Active Agents** | 82 | **34** | **~58.5%** |
| **Token Generation**| 82 | 64 | **~22%** |

*AAPT maintains a constrained physical footprint during the fork phase, scaling only when unique generation begins.*

---

## 🛠️ Usage
**1. Compile and Boot:**
```bash
make clean
make qemu
```
**2. Run Benchmark:**
```bash
$ aapt_benchmark
```

## 👥 Engineering Team
* **Pragun Kathuria:** Page Fault Handler & Exception Routing.
* **Ashank Kumar Singh:** Page Table Mapping & CoW Restructuring.
* **Avani Gupta:** Core Allocator & Reference Tracking.
* **Tanvi Pandey:** System Call Plumbing & User-space Validation.

---
*Developed at Shiv Nadar Institution of Eminence (SNIoE).*