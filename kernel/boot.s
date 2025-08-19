; boot.s — Multiboot2 + 32→64 long mode trampoline (NASM)

; -------------------------
; Multiboot2 header (must be within first 32KiB of the file)
; -------------------------
section .multiboot2
align 8
header_start:
    dd 0xE85250D6                ; magic
    dd 0                         ; arch (i386, GRUB enters 32-bit protected mode)
    dd header_end - header_start ; length
    dd 0x100000000 - (0xE85250D6 + 0 + (header_end - header_start)) ; checksum

align 8                           ; framebuffer request tag
    dd 5         ; type = 5 (framebuffer request)
    dd 20        ; size
    dd 0         ; no preference
    dd 1024      ; width
    dd 768       ; height
    dd 32        ; bpp

align 8
    dd 0         ; end tag type
    dd 8         ; end tag size
header_end:

; -------------------------
; Code
; -------------------------
section .text
bits 32

extern kernel_main

global _start
_start:
    cli

    ; Save Multiboot info pointer (GRUB: EBX = mb_info)
    mov [mb_info32], ebx

    ; Basic 32-bit stack until we switch
    mov esp, stack_top

    ; -------------------------
    ; Minimal 64-bit GDT
    ; -------------------------
    lgdt [gdt64_ptr]

    ; -------------------------
    ; Build identity-mapped paging for 0..4GiB using 2MiB pages
    ; -------------------------
    ; PML4[0] -> PDPT
    mov eax, pdpt
    mov dword [pml4 + 0], eax
    mov dword [pml4 + 4], 0
    or  dword [pml4 + 0], 0x003      ; P|RW

    ; PDPT[0..3] -> PD0..PD3
    mov eax, pd0
    mov dword [pdpt + 0*8 + 0], eax
    mov dword [pdpt + 0*8 + 4], 0
    or  dword [pdpt + 0*8 + 0], 0x003

    mov eax, pd1
    mov dword [pdpt + 1*8 + 0], eax
    mov dword [pdpt + 1*8 + 4], 0
    or  dword [pdpt + 1*8 + 0], 0x003

    mov eax, pd2
    mov dword [pdpt + 2*8 + 0], eax
    mov dword [pdpt + 2*8 + 4], 0
    or  dword [pdpt + 2*8 + 0], 0x003

    mov eax, pd3
    mov dword [pdpt + 3*8 + 0], eax
    mov dword [pdpt + 3*8 + 4], 0
    or  dword [pdpt + 3*8 + 0], 0x003

    ; Fill PD0: 0..1GiB (2MiB pages)
    mov edi, pd0
    xor eax, eax                ; base = 0x00000000
    mov ecx, 512
.fill_pd0:
    mov edx, eax
    or  edx, 0x00000083         ; P|RW|PS(2MiB)
    mov dword [edi+0], edx
    mov dword [edi+4], 0
    add eax, 0x200000
    add edi, 8
    loop .fill_pd0

    ; Fill PD1: 1..2GiB
    mov edi, pd1
    mov eax, 0x40000000
    mov ecx, 512
.fill_pd1:
    mov edx, eax
    or  edx, 0x00000083
    mov dword [edi+0], edx
    mov dword [edi+4], 0
    add eax, 0x200000
    add edi, 8
    loop .fill_pd1

    ; Fill PD2: 2..3GiB
    mov edi, pd2
    mov eax, 0x80000000
    mov ecx, 512
.fill_pd2:
    mov edx, eax
    or  edx, 0x00000083
    mov dword [edi+0], edx
    mov dword [edi+4], 0
    add eax, 0x200000
    add edi, 8
    loop .fill_pd2

    ; Fill PD3: 3..4GiB (covers typical LFB @ ~0xE0000000)
    mov edi, pd3
    mov eax, 0xC0000000
    mov ecx, 512
.fill_pd3:
    mov edx, eax
    or  edx, 0x00000083
    mov dword [edi+0], edx
    mov dword [edi+4], 0
    add eax, 0x200000
    add edi, 8
    loop .fill_pd3

    ; Enable PAE (+ optionally PGE)
    mov eax, cr4
    or  eax, (1<<5)             ; PAE
    or  eax, (1<<7)             ; PGE (optional)
    mov cr4, eax

    ; Load PML4
    mov eax, pml4
    mov cr3, eax

    ; Enable Long Mode via EFER.LME
    mov ecx, 0xC0000080         ; IA32_EFER
    rdmsr
    or  eax, (1<<8)             ; LME
    wrmsr

    ; Enable paging (PG); PE is already set by GRUB
    mov eax, cr0
    or  eax, (1<<31)            ; PG
    mov cr0, eax

    ; Far jump to 64-bit code segment (selector 0x08)
    jmp 0x08:long_mode_entry

; -------------------------
; 64-bit code starts here
; -------------------------
bits 64
long_mode_entry:
    mov ax, 0x10                ; 64-bit data selector
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; 64-bit stack (aligned)
    mov rsp, stack_top
    and rsp, -16

    ; Pass Multiboot info pointer in RDI
    mov edi, dword [mb_info32]  ; zero-extends to RDI

    ; Call kernel main (SysV ABI: RDI=arg1)
    call kernel_main

.hang:
    hlt
    jmp .hang

; -------------------------
; Data / Tables
; -------------------------

align 8
section .data
; Long-mode GDT: null, 64-bit code, 64-bit data
;   code: 0x00209A0000000000  (L=1, D=0, P=1, type=exec/read)
;   data: 0x0000920000000000  (P=1, type=read/write)

gdt64:
    dq 0x0000000000000000
    dq 0x00209A0000000000     ; selector 0x08
    dq 0x0000920000000000     ; selector 0x10

gdt64_ptr:
    dw gdt64_end - gdt64 - 1
    dd gdt64                   ; low 32 bits of base (used in 32-bit lgdt)
    dd 0                       ; high 32 (ignored in 32-bit lgdt)

gdt64_end:

section .bss
align 4096
pml4:   resq 512
align 4096
pdpt:   resq 512
align 4096
pd0:    resq 512
align 4096
pd1:    resq 512
align 4096
pd2:    resq 512
align 4096
pd3:    resq 512

align 16
stack_space: resb 16384
stack_top:

align 8
mb_info32:  resd 1