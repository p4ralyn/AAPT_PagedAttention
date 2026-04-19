# 🚀 Agentic AI Paging Technology (AAPT)
**Native OS-Level Copy-on-Write Prefix Caching for the xv6 RISC-V Kernel**

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![RISC-V](https://img.shields.io/badge/RISC--V-2D303E?style=for-the-badge&logo=riscv&logoColor=white)
![Kernel](https://img.shields.io/badge/Kernel-xv6-blueviolet?style=for-the-badge)

[cite_start]AAPT is a hardware-aware memory manager natively built within the MIT xv6 RISC-V kernel to solve the memory redundancy problem in multi-agent AI systems[cite: 93]. [cite_start]By implementing kernel-level Copy-on-Write (CoW), AAPT enables concurrent agents to share identical physical memory frames for foundational prompts, duplicating them only upon mutation[cite: 95, 116].

---

## 🧠 The Problem: KV Cache Redundancy
Traditional operating systems deeply copy memory across process forks, leading to:
* [cite_start]**Massive Redundancy:** Multiple agents store identical copies of the "System Prompt"[cite: 102, 103].
* [cite_start]**Resource Exhaustion:** Rapid physical RAM depletion during Key-Value (KV) cache initialization[cite: 92].
* [cite_start]**Abstraction Gaps:** User-space solutions (like vLLM) cannot optimize native hardware mechanics like TLBs and hardware page table walks[cite: 104, 110].

## ⚡ The AAPT Solution
[cite_start]AAPT synthesizes the theories of **PagedAttention** and **vAttention** directly into the kernel space[cite: 166].

### Technical Core
* [cite_start]**Physical Reference Tracking (`kalloc.c`):** A global `ref_count[]` array tracks concurrent mappings to single frames, protected by `kmem.lock` spinlocks[cite: 119, 120, 176].
* [cite_start]**CoW Forking (`vm.c`):** A custom `sys_aapt_cow_fork()` call revokes `PTE_W` (Write) permissions for both parent and child, enforcing a read-only shared state[cite: 123, 124, 174].
* [cite_start]**Trap-Driven Duplication (`trap.c`):** Intercepts RISC-V Exception Cause 15 (Store Page Fault) to dynamically allocate and duplicate pages only when an agent attempts unique token generation[cite: 129, 130, 171].

---

## 📊 Performance Results
[cite_start]Empirical benchmarking shows that AAPT successfully mitigates **100% of redundant KV cache allocation** during process initialization[cite: 96].

| Event Phase | Std xv6 Pages | AAPT Pages | Savings |
| :--- | :--- | :--- | :--- |
| **System Prompt** | 16 | 16 | Base |
| **3 Active Agents** | 82 | **34** | **~58.5%** |
| **Token Generation**| 82 | 64 | **~22%** |

[cite_start]*AAPT maintains a constrained physical footprint during the fork phase, scaling only when unique generation begins[cite: 139, 140].*

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
* [cite_start]**Pragun Kathuria:** Page Fault Handler & Exception Routing[cite: 171].
* [cite_start]**Ashank Kumar Singh:** Page Table Mapping & CoW Restructuring[cite: 173].
* [cite_start]**Avani Gupta:** Core Allocator & Reference Tracking[cite: 175].
* [cite_start]**Tanvi Pandey:** System Call Plumbing & User-space Validation[cite: 156, 157].

---
*Developed at Shiv Nadar Institution of Eminence (SNIoE).*
```