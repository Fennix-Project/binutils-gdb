#as: -march=rv64ic -mabi=lp64
#objdump: -dr

.*:     file format elf64-(little|big)riscv


Disassembly of section .text:

0000000000000000 <target>:
[^:]+:[ 	]+6521[ 	]+lui[ 	]+a0,0x8
[^:]+:[ 	]+2505[ 	]+addw[ 	]+a0,a0,1 # .*
[^:]+:[ 	]+6509[ 	]+lui[ 	]+a0,0x2
[^:]+:[ 	]+f015051b[ 	]+addw[ 	]+a0,a0,-255 # .*
[^:]+:[ 	]+12345537[ 	]+lui[ 	]+a0,0x12345
[^:]+:[ 	]+2505[ 	]+addw[ 	]+a0,a0,1 # .*
[^:]+:[ 	]+000f2537[ 	]+lui[ 	]+a0,0xf2
[^:]+:[ 	]+3455051b[ 	]+addw[ 	]+a0,a0,837 # .*
[^:]+:[ 	]+0532[ 	]+sll[ 	]+a0,a0,0xc
[^:]+:[ 	]+0505[ 	]+add[ 	]+a0,a0,1
[^:]+:[ 	]+00f12537[ 	]+lui[ 	]+a0,0xf12
[^:]+:[ 	]+3455051b[ 	]+addw[ 	]+a0,a0,837 # .*
[^:]+:[ 	]+0532[ 	]+sll[ 	]+a0,a0,0xc
[^:]+:[ 	]+0505[ 	]+add[ 	]+a0,a0,1
[^:]+:[ 	]+ff010537[ 	]+lui[ 	]+a0,0xff010
[^:]+:[ 	]+f015051b[ 	]+addw[ 	]+a0,a0,-255 # .*
[^:]+:[ 	]+054e[ 	]+sll[ 	]+a0,a0,0x13
[^:]+:[ 	]+80150513[ 	]+add[ 	]+a0,a0,-2047
[^:]+:[ 	]+0536[ 	]+sll[ 	]+a0,a0,0xd
[^:]+:[ 	]+f0150513[ 	]+add[ 	]+a0,a0,-255
[^:]+:[ 	]+0010051b[ 	]+addw[ 	]+a0,zero,1
[^:]+:[ 	]+151a[ 	]+sll[ 	]+a0,a0,0x26
[^:]+:[ 	]+1565[ 	]+add[ 	]+a0,a0,-7
[^:]+:[ 	]+0536[ 	]+sll[ 	]+a0,a0,0xd
[^:]+:[ 	]+34550513[ 	]+add[ 	]+a0,a0,837
[^:]+:[ 	]+0532[ 	]+sll[ 	]+a0,a0,0xc
[^:]+:[ 	]+0505[ 	]+add[ 	]+a0,a0,1
[^:]+:[ 	]+01fc4537[ 	]+lui[ 	]+a0,0x1fc4
[^:]+:[ 	]+c915051b[ 	]+addw[ 	]+a0,a0,-879 # .*
[^:]+:[ 	]+0536[ 	]+sll[ 	]+a0,a0,0xd
[^:]+:[ 	]+1565[ 	]+add[ 	]+a0,a0,-7
[^:]+:[ 	]+0536[ 	]+sll[ 	]+a0,a0,0xd
[^:]+:[ 	]+34550513[ 	]+add[ 	]+a0,a0,837
[^:]+:[ 	]+0532[ 	]+sll[ 	]+a0,a0,0xc
[^:]+:[ 	]+0505[ 	]+add[ 	]+a0,a0,1
