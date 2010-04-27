/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2000,2007 Ronald G. Minnich <rminnich@gmail.com>
 * Copyright (C) 2005 Eswar Nallusamy, LANL
 * Copyright (C) 2005 Tyan
 * (Written by Yinghai Lu <yhlu@tyan.com> for Tyan)
 * Copyright (C) 2007 coresystems GmbH
 * (Written by Stefan Reinauer <stepan@coresystems.de> for coresystems GmbH)
 * Copyright (C) 2007 Carl-Daniel Hailfinger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

/* We will use 4K bytes only */
/* disable HyperThreading is done by eswar*/
/* other's is the same as AMD except remove amd specific msr */

#define CacheSize CONFIG_DCACHE_RAM_SIZE
#define CacheBase (0xd0000 - CacheSize)

#include <cpu/x86/stack.h>
#include <cpu/x86/mtrr.h>

	/* Save the BIST result */
	movl	%eax, %ebp

CacheAsRam:
	// Check whether the processor has HT capability
	movl	$01, %eax
	cpuid
	btl	$28, %edx
	jnc	NotHtProcessor
	bswapl	%ebx
	cmpb	$01, %bh
	jbe	NotHtProcessor

	// It is a HT processor; Send SIPI to the other logical processor
	// within this processor so that the CAR related common system registers
	// are programmed accordingly

	// Use some register that is common to both logical processors
	// as semaphore. Refer Appendix B, Vol.3
	xorl	%eax, %eax
	xorl	%edx, %edx
	movl	$0x250, %ecx
	wrmsr

	// Figure out the logical AP's APIC ID; the following logic will work
	// only for processors with 2 threads
	// Refer to Vol 3. Table 7-1 for details about this logic
	movl	$0xFEE00020, %esi
	movl	(%esi), %ebx
	andl	$0xFF000000, %ebx
	bswapl	%ebx
	btl	$0, %ebx
	jnc	LogicalAP0
	andb	$0xFE, %bl
	jmp	Send_SIPI
LogicalAP0:
	orb	$0x01, %bl
Send_SIPI:
	bswapl	%ebx  // ebx - logical AP's APIC ID

	// Fill up the IPI command registers in the Local APIC mapped to default address
	// and issue SIPI to the other logical processor within this processor die.
Retry_SIPI:
	movl	%ebx, %eax
	movl	$0xFEE00310, %esi
	movl	%eax, (%esi)

	// SIPI vector - F900:0000
	movl	$0x000006F9, %eax
	movl	$0xFEE00300, %esi
	movl	%eax, (%esi)

	movl	$0x30, %ecx
SIPI_Delay:
	pause
	decl	%ecx
	jnz	SIPI_Delay

	movl	(%esi), %eax
	andl	$0x00001000, %eax
	jnz	Retry_SIPI

	// Wait for the Logical AP to complete initialization
LogicalAP_SIPINotdone:
	movl	$0x250, %ecx
	rdmsr
	orl	%eax, %eax
	jz	LogicalAP_SIPINotdone

NotHtProcessor:

#if 1
	/* Set the default memory type and enable fixed and variable MTRRs */
	movl	$MTRRdefType_MSR, %ecx
	xorl	%edx, %edx
	/* Enable Variable and Fixed MTRRs */
	movl	$0x00000c00, %eax
	wrmsr
#endif

	/* Clear all MTRRs */
	xorl	%edx, %edx
	movl	$fixed_mtrr_msr, %esi

clear_fixed_var_mtrr:
	lodsl	(%esi), %eax
	testl	%eax, %eax
	jz	clear_fixed_var_mtrr_out

	movl	%eax, %ecx
	xorl	%eax, %eax
	wrmsr

	jmp	clear_fixed_var_mtrr

fixed_mtrr_msr:
	.long	0x250, 0x258, 0x259
	.long	0x268, 0x269, 0x26A
	.long	0x26B, 0x26C, 0x26D
	.long	0x26E, 0x26F
var_mtrr_msr:
	.long	0x200, 0x201, 0x202, 0x203
	.long	0x204, 0x205, 0x206, 0x207
	.long	0x208, 0x209, 0x20A, 0x20B
	.long	0x20C, 0x20D, 0x20E, 0x20F
	.long	0x000 /* NULL, end of table */

clear_fixed_var_mtrr_out:

/* 0x06 is the WB IO type for a given 4k segment.
 * segs is the number of 4k segments in the area of the particular
 *   register we want to use for CAR.
 * reg is the register where the IO type should be stored.
 */
.macro extractmask segs, reg
.if \segs <= 0
	/* The xorl here is superfluous because at the point of first execution
	 * of this macro, %eax and %edx are cleared. Later invocations of this
	 * macro will have a monotonically increasing segs parameter.
	 */
	xorl \reg, \reg
.elseif \segs == 1
	movl $0x06000000, \reg /* WB IO type */
.elseif \segs == 2
	movl $0x06060000, \reg /* WB IO type */
.elseif \segs == 3
	movl $0x06060600, \reg /* WB IO type */
.elseif \segs >= 4
	movl $0x06060606, \reg /* WB IO type */
.endif
.endm

/* size is the cache size in bytes we want to use for CAR.
 * windowoffset is the 32k-aligned window into CAR size
 */
.macro simplemask carsize, windowoffset
	.set gas_bug_workaround,(((\carsize - \windowoffset) / 0x1000) - 4)
	extractmask gas_bug_workaround, %eax
	.set gas_bug_workaround,(((\carsize - \windowoffset) / 0x1000))
	extractmask gas_bug_workaround, %edx
/* Without the gas bug workaround, the entire macro would consist only of the
 * two lines below.
	extractmask (((\carsize - \windowoffset) / 0x1000) - 4), %eax
	extractmask (((\carsize - \windowoffset) / 0x1000)), %edx
 */
.endm

#if CacheSize > 0x10000
#error Invalid CAR size, must be at most 64k.
#endif
#if CacheSize < 0x1000
#error Invalid CAR size, must be at least 4k. This is a processor limitation.
#endif
#if (CacheSize & (0x1000 - 1))
#error Invalid CAR size, is not a multiple of 4k. This is a processor limitation.
#endif

#if CacheSize > 0x8000
	/* enable caching for 32K-64K using fixed mtrr */
	movl	$0x268, %ecx  /* fix4k_c0000*/
	simplemask CacheSize, 0x8000
	wrmsr
#endif

	/* enable caching for 0-32K using fixed mtrr */
	movl	$0x269, %ecx  /* fix4k_c8000*/
	simplemask CacheSize, 0
	wrmsr

#if defined(CONFIG_XIP_ROM_SIZE) && defined(CONFIG_XIP_ROM_BASE)
#if defined(CONFIG_TINY_BOOTBLOCK) && CONFIG_TINY_BOOTBLOCK
#define REAL_XIP_ROM_BASE AUTO_XIP_ROM_BASE
#else
#define REAL_XIP_ROM_BASE CONFIG_XIP_ROM_BASE
#endif
	/* enable write base caching so we can do execute in place
	 * on the flash rom.
	 */
	movl	$0x202, %ecx
	xorl	%edx, %edx
	movl    $REAL_XIP_ROM_BASE, %eax
	orl     $MTRR_TYPE_WRBACK, %eax
	wrmsr

	movl	$0x203, %ecx
	movl	$0x0000000f, %edx
	movl	$(~(CONFIG_XIP_ROM_SIZE - 1) | 0x800), %eax
	wrmsr
#endif /* CONFIG_XIP_ROM_SIZE && CONFIG_XIP_ROM_BASE */

	/* enable cache */
	movl	%cr0, %eax
	andl	$0x9fffffff, %eax
	movl	%eax, %cr0

	/* Read the range with lodsl*/
	movl	$CacheBase, %esi
	cld
	movl	$(CacheSize >> 2), %ecx
	rep	lodsl

	/* Clear the range */
	movl	$CacheBase, %edi
	movl	$(CacheSize >> 2), %ecx
	xorl	%eax, %eax
	rep	stosl


#if 0
	/* check the cache as ram */
	movl	$CacheBase, %esi
	movl	$(CacheSize>>2), %ecx
.xin1:
	movl	%esi, %eax
	movl	%eax, (%esi)
	decl	%ecx
	je	.xout1
	add	$4, %esi
	jmp	.xin1
.xout1:

	movl	$CacheBase, %esi
//        movl    $(CacheSize>>2), %ecx
	movl $4, %ecx
.xin1x:
	movl	%esi, %eax

	movl	$0x4000, %edx
	movb	%ah, %al
.testx1:
	outb %al, $0x80
	decl	%edx
	jnz .testx1

	movl	(%esi), %eax
	cmpb 0xff, %al
	je .xin2  /* dont show */

	movl	$0x4000, %edx
.testx2:
	outb %al, $0x80
	decl	%edx
	jnz .testx2

.xin2:  decl     %ecx
	je	.xout1x
	add	$4, %esi
	jmp	.xin1x
.xout1x:

#endif

	movl	$(CacheBase + CacheSize - 4), %eax
	movl	%eax, %esp

	/* Load a different set of data segments */
#if CONFIG_USE_INIT
	movw	$CACHE_RAM_DATA_SEG, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss
#endif

lout:
	/* Restore the BIST result */
	movl	%ebp, %eax

	/* We need to set ebp ? No need */
	movl	%esp, %ebp
	pushl	%eax  /* bist */
	call	main

	/*
	FIXME : backup stack in CACHE_AS_RAM into mmx and sse and after we get STACK up, we restore that.
		It is only needed if we want to go back
	*/

        /* We don't need cache as ram for now on */
        /* disable cache */
	movl    %cr0, %eax
	orl    $(0x1<<30),%eax
	movl    %eax, %cr0

        /* clear sth */
	movl    $0x269, %ecx  /* fix4k_c8000*/
	xorl    %edx, %edx
	xorl    %eax, %eax
	wrmsr

#if CONFIG_DCACHE_RAM_SIZE > 0x8000
	movl    $0x268, %ecx  /* fix4k_c0000*/
	wrmsr
#endif

        /* Set the default memory type and disable fixed and enable variable MTRRs */
	movl    $0x2ff, %ecx
//	movl    $MTRRdefType_MSR, %ecx
	xorl    %edx, %edx
        /* Enable Variable and Disable Fixed MTRRs */
	movl    $0x00000800, %eax
	wrmsr

#if defined(CLEAR_FIRST_1M_RAM)
        /* enable caching for first 1M using variable mtrr */
	movl    $0x200, %ecx
	xorl    %edx, %edx
	movl     $(0 | 1), %eax
//	movl     $(0 | MTRR_TYPE_WRCOMB), %eax
	wrmsr

	movl    $0x201, %ecx
	movl    $0x0000000f, %edx /* AMD 40 bit 0xff*/
	movl    $((~(( 0 + 0x100000) - 1)) | 0x800), %eax
	wrmsr
#endif

        /* enable cache */
	movl    %cr0, %eax
	andl    $0x9fffffff,%eax
	movl    %eax, %cr0

#if defined(CLEAR_FIRST_1M_RAM)
        /* clear the first 1M */
	movl    $0x0, %edi
	cld
	movl    $(0x100000>>2), %ecx
	xorl    %eax, %eax
	rep     stosl

        /* disable cache */
	movl    %cr0, %eax
	orl    $(0x1<<30),%eax
	movl    %eax, %cr0

        /* enable caching for first 1M using variable mtrr */
	movl    $0x200, %ecx
	xorl    %edx, %edx
	movl     $(0 | 6), %eax
//	movl     $(0 | MTRR_TYPE_WRBACK), %eax
	wrmsr

	movl    $0x201, %ecx
	movl    $0x0000000f, %edx /* AMD 40 bit 0xff*/
	movl    $((~(( 0 + 0x100000) - 1)) | 0x800), %eax
	wrmsr

        /* enable cache */
	movl    %cr0, %eax
	andl    $0x9fffffff,%eax
	movl    %eax, %cr0
	invd

	/* FIXME: I hope we don't need to change esp and ebp value here, so we
	 * can restore value from mmx sse back But the problem is the range is
	 * some io related, So don't go back
	 */
#endif

	/* clear boot_complete flag */
	xorl	%ebp, %ebp
__main:
	post_code(0x11)
	cld			/* clear direction flag */

	movl	%ebp, %esi

	movl $ROMSTAGE_STACK, %esp
	movl	%esp, %ebp
	pushl %esi
	call copy_and_run

.Lhlt:
	post_code(0xee)
	hlt
	jmp	.Lhlt
