//
// sys_dosa.s
// x86 assembly-language DOS-dependent routines.

#include "qasm.h"

	.data

	.align	4
fpenv:
	.long	0, 0, 0, 0, 0, 0, 0, 0

	.text

.globl C(MaskExceptions)
C(MaskExceptions):
	fnstenv	fpenv
	orl		$0x3F,fpenv
	fldenv	fpenv

	ret

	.data

	.align	4
.globl	fpu_ceil_cw, fpu_single_cw, fpu_chop_cw, fpu_full_cw, fpu_cw, fpu_pushed_cw, fpu_sp24_cw, fpu_sp24_ceil_cw
fpu_ceil_cw:		.long	0
fpu_single_cw:		.long	0
fpu_chop_cw:		.long	0
fpu_full_cw:		.long	0
fpu_cw:				.long	0
fpu_pushed_cw:		.long	0
fpu_sp24_cw:		.long	0
fpu_sp24_ceil_cw:	.long	0
	.text

.globl C(Sys_LowFPPrecision)
C(Sys_LowFPPrecision):
	fldcw	fpu_single_cw

	ret

.globl C(Sys_HighFPPrecision)
C(Sys_HighFPPrecision):
	fldcw	fpu_full_cw

	ret

.globl C(Sys_PushFPCW_SetHigh)
C(Sys_PushFPCW_SetHigh):
	fnstcw	fpu_pushed_cw
	fldcw	fpu_full_cw

	ret

.globl C(Sys_PopFPCW)
C(Sys_PopFPCW):
	fldcw	fpu_pushed_cw

	ret

.globl C(Sys_SetSP24_FPPrecision)
C(Sys_SetSP24_FPPrecision):
	fldcw	fpu_sp24_cw
	
	ret

.globl C(Sys_SetChopCW_FPPrecision)
C(Sys_SetChopCW_FPPrecision):
	fldcw	fpu_chop_cw

	ret

.globl C(Sys_SetFPCW)
C(Sys_SetFPCW):
	xorl %eax, %eax

	fnstcw	fpu_cw
	movl	fpu_cw, %eax

#if	id386
	andb	$0xF0, %ah
	orb		$0x03, %ah	  // round to nearest mode, extended precision
#endif
	movl	%eax, fpu_full_cw

#if	id386
	andb $0xF0, %ah
	orb  $0x0F, %ah       // RTZ/truncate/chop mode, extended precision
#endif
	movl %eax, fpu_chop_cw

#if	id386
	andb $0xF0, %ah
	orb  $0x0B, %ah       // ceil mode, extended precision
#endif
	movl %eax, fpu_ceil_cw

#if	id386
	andb $0xF0, %ah       // round to nearest, 24-bit single precision
#endif
	movl %eax, fpu_sp24_cw

#if	id386
	andb	$0xF0, %ah
	orb		$0x0C, %ah	  // chop mode, single precision
#endif
	movl	%eax, fpu_single_cw

#if	id386
	andb	$0xF0 ,%ah
	orb		$0x08, %ah	  // ceil mode, 24-bit single precision
#endif
	movl	%eax, fpu_sp24_ceil_cw

	ret
