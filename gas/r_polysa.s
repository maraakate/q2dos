//
// r_polysa.s
// x86 assembly-language polygon model drawing code
//

#include "qasm.h"
#include "d_ifacea.h"

#if	id386

// !!! if this is changed, it must be changed in d_polyse.c too !!!
//#define DPS_MAXSPANS			MAXHEIGHT+1	
// 1 extra for spanpackage that marks end

//#define	SPAN_SIZE	(((DPS_MAXSPANS + 1 + ((CACHE_SIZE - 1) / spanpackage_t_size)) + 1) * spanpackage_t_size)
//#define SPAN_SIZE (1024+1+1+1)*32



	.data

	.align	4
p00_minus_p20:	.single		0
p01_minus_p21:	.single		0
p10_minus_p20:	.single		0
p11_minus_p21:	.single		0
t0:		.single		0
t0_int:		.long		0
t1:		.single		0
t1_int:		.long		0
xstepdenominv:	.single		0
ystepdenominv:	.single		0
//temp0:			.single		0
//temp1:			.single		0
//Ltemp:			.single		0

aff8entryvec_table:	.long	LDraw8, LDraw7, LDraw6, LDraw5
 .long	LDraw4, LDraw3, LDraw2, LDraw1,	LDraw8IR, LDraw7IR, LDraw6IR, LDraw5IR, LDraw4IR, LDraw3IR, LDraw2IR, LDraw1IR

lzistepx:		.long	0

// old as from Apple cctools does not recognize .extern
#ifndef __APPLE__
.extern C(alias_colormap)
#endif

	.text

//----------------------------------------------------------------------
// affine triangle gradient calculation code
//----------------------------------------------------------------------

#define skinwidth	8+0
.globl C(R_PolysetCalcGradients)
C(R_PolysetCalcGradients):
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx

//
// p00_minus_p20 = r_p0[0] - r_p2[0];
// p01_minus_p21 = r_p0[1] - r_p2[1];
// p10_minus_p20 = r_p1[0] - r_p2[0];
// p11_minus_p21 = r_p1[1] - r_p2[1];
//

	movl	C(r_p0)+0, %eax
	movl	C(r_p0)+4, %ebx
	subl	C(r_p2)+0, %eax
	subl	C(r_p2)+4, %ebx
	movl	%eax, p00_minus_p20
	movl	%ebx, p01_minus_p21
	fildl	p00_minus_p20
	fildl	p01_minus_p21
	movl	C(r_p1)+0, %eax
	movl	C(r_p1)+4, %ebx
	subl	C(r_p2)+0, %eax
	subl	C(r_p2)+4, %ebx
	fstps	p01_minus_p21
	fstps	p00_minus_p20
	movl	%eax, p10_minus_p20
	movl	%ebx, p11_minus_p21
	fildl	p10_minus_p20
	fildl	p11_minus_p21
	fstps	p11_minus_p21
	fstps	p10_minus_p20
//
// xstepdenominv = 1.0 / (float)d_xdenom;
//
// ystepdenominv = -xstepdenominv;
//

//
// put FPU in single precision ceil mode
//

	fldcw	fpu_sp24_ceil_cw

	fildl	C(d_xdenom)         // d_xdenom
	fdivrs	float_1             // 1 / d_xdenom
	fsts	xstepdenominv       // 
	fmuls	float_minus_1       // -( 1 / d_xdenom )

// ceil () for light so positive steps are exaggerated, negative steps
// diminished,  pushing us away from underflow toward overflow. Underflow is
// very visible, overflow is very unlikely, because of ambient lighting
//
// t0 = r_p0[4] - r_p2[4];
// t1 = r_p1[4] - r_p2[4];
// r_lstepx = (int)
// 		ceil((t1 * p01_minus_p21 - t0 * p11_minus_p21) * xstepdenominv);
// r_lstepy = (int)
// 		ceil((t1 * p00_minus_p20 - t0 * p10_minus_p20) * ystepdenominv);
//
	movl	C(r_p0)+16, %eax
	movl	C(r_p1)+16, %ebx
	subl	C(r_p2)+16, %eax
	subl	C(r_p2)+16, %ebx

	fstps	ystepdenominv       // (empty)

	movl	%eax, t0_int
	movl	%ebx, t1_int
	fildl	t0_int              // t0
	fildl	t1_int              // t1 | t0
	fxch	%st(1)              // t0 | t1
	fstps	t0                  // t1
	fsts	t1                  // t1
	fmuls	p01_minus_p21       // t1 * p01_minus_p21
	flds	t0                  // t0 | t1 * p01_minus_p21
	fmuls	p11_minus_p21       // t0 * p11_minus_p21 | t1 * p01_minus_p21
	flds	t1                  // t1 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	fmuls	p00_minus_p20       // t1 * p00_minus_p20 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	flds	t0                  // t0 | t1 * p00_minus_p20 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	fmuls	p10_minus_p20       // t0 * p10_minus_p20 | t1 * p00_minus_p20 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	fxch	%st(2)              // t0 * p11_minus_p21 | t0 * p10_minus_p20 | t1 * p00_minus_p20 | t1 * p01_minus_p21
	// FS: The two lines below are swapped.  GAS generates the wrong opcode for this.
	fsubrp	%st(0), %st(3)      // t0 * p10_minus_p20 | t1 * p00_minus_p20 | t1 * p01_minus_p21 - t0 * p11_minus_p21
	fsubp	%st(0), %st(1)      // t1 * p00_minus_p20 - t0 * p10_minus_p20 | t1 * p01_minus_p21 - t0 * p11_minus_p21
	fxch	%st(1)              // t1 * p01_minus_p21 - t0 * p11_minus_p21 | t1 * p00_minus_p20 - t0 * p10_minus_p20
	fmuls	xstepdenominv       // r_lstepx | t1 * p00_minus_p20 - t0 * p10_minus_p20
	fxch	%st(1)
	fmuls	ystepdenominv       // r_lstepy | r_lstepx
	fxch	%st(1)              // r_lstepx | r_lstepy
	fistpl	C(r_lstepx)
	fistpl	C(r_lstepy)

//
// put FPU back into extended precision chop mode
//
	fldcw	fpu_chop_cw

//
// t0 = r_p0[2] - r_p2[2];
// t1 = r_p1[2] - r_p2[2];
// r_sstepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) *
// 		xstepdenominv);
// r_sstepy = (int)((t1 * p00_minus_p20 - t0* p10_minus_p20) *
// 		ystepdenominv);
//
	movl	C(r_p0)+8, %eax
	movl	C(r_p1)+8, %ebx
	subl	C(r_p2)+8, %eax
	subl	C(r_p2)+8, %ebx
	movl	%eax, t0_int
	movl	%ebx, t1_int
	fildl	t0_int              // t0
	fildl	t1_int              // t1 | t0
	fxch	%st(1)              // t0 | t1
	fstps	t0                  // t1
	fsts	t1                  // (empty)

	fmuls	p01_minus_p21       // t1 * p01_minus_p21
	flds	t0                  // t0 | t1 * p01_minus_p21
	fmuls	p11_minus_p21       // t0 * p11_minus_p21 | t1 * p01_minus_p21
	flds	t1                  // t1 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	fmuls	p00_minus_p20       // t1 * p00_minus_p20 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	flds	t0                  // t0 | t1 * p00_minus_p20 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	fmuls	p10_minus_p20       // t0 * p10_minus_p20 | t1 * p00_minus_p20 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	fxch	%st(2)              // t0 * p11_minus_p21 | t0 * p10_minus_p20 | t1 * p00_minus_p20 | t1 * p01_minus_p21
	// FS: The two lines below are swapped.  GAS generates the wrong opcode for this.
	fsubrp	%st(0), %st(3)      // t0 * p10_minus_p20 | t1 * p00_minus_p20 | t1 * p01_minus_p21 - t0 * p11_minus_p21
	fsubp	%st(0), %st(1)      // t1 * p00_minus_p20 - t0 * p10_minus_p20 | t1 * p01_minus_p21 - t0 * p11_minus_p21
	fxch	%st(1)              // t1 * p01_minus_p21 - t0 * p11_minus_p21 | t1 * p00_minus_p20 - t0 * p10_minus_p20
	fmuls	xstepdenominv       // r_lstepx | t1 * p00_minus_p20 - t0 * p10_minus_p20
	fxch	%st(1)
	fmuls	ystepdenominv       // r_lstepy | r_lstepx
	fxch	%st(1)              // r_lstepx | r_lstepy
	fistpl	C(r_sstepx)
	fistpl	C(r_sstepy)

//
// t0 = r_p0[3] - r_p2[3];
// t1 = r_p1[3] - r_p2[3];
// r_tstepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) *
// 		xstepdenominv);
// r_tstepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) *
// 		ystepdenominv);
//
	movl	C(r_p0)+12, %eax
	movl	C(r_p1)+12, %ebx
	subl	C(r_p2)+12, %eax
	subl	C(r_p2)+12, %ebx

	movl	%eax, t0_int
	movl	%ebx, t1_int
	fildl	t0_int             // t0
	fildl	t1_int             // t1 | t0
	fxch	%st(1)             // t0 | t1
	fstps	t0                 // t1
	fsts	t1                 // (empty)

	fmuls	p01_minus_p21      // t1 * p01_minus_p21
	flds	t0                 // t0 | t1 * p01_minus_p21
	fmuls	p11_minus_p21      // t0 * p11_minus_p21 | t1 * p01_minus_p21
	flds	t1                 // t1 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	fmuls	p00_minus_p20      // t1 * p00_minus_p20 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	flds	t0                 // t0 | t1 * p00_minus_p20 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	fmuls	p10_minus_p20      // t0 * p10_minus_p20 | t1 * p00_minus_p20 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	fxch	%st(2)             // t0 * p11_minus_p21 | t0 * p10_minus_p20 | t1 * p00_minus_p20 | t1 * p01_minus_p21
	// FS: The two lines below are swapped.  GAS generates the wrong opcode for this.
	fsubrp	%st(0), %st(3)     // t0 * p10_minus_p20 | t1 * p00_minus_p20 | t1 * p01_minus_p21 - t0 * p11_minus_p21
	fsubp	%st(0), %st(1)     // t1 * p00_minus_p20 - t0 * p10_minus_p20 | t1 * p01_minus_p21 - t0 * p11_minus_p21
	fxch	%st(1)             // t1 * p01_minus_p21 - t0 * p11_minus_p21 | t1 * p00_minus_p20 - t0 * p10_minus_p20
	fmuls	xstepdenominv      // r_lstepx | t1 * p00_minus_p20 - t0 * p10_minus_p20
	fxch	%st(1)
	fmuls	ystepdenominv      // r_lstepy | r_lstepx
	fxch	%st(1)             // r_lstepx | r_lstepy
	fistpl	C(r_tstepx)
	fistpl	C(r_tstepy)

//
// t0 = r_p0[5] - r_p2[5];
// t1 = r_p1[5] - r_p2[5];
// r_zistepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) *
// 		xstepdenominv);
// r_zistepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) *
// 		ystepdenominv);
//
	movl	C(r_p0)+20, %eax
	movl	C(r_p1)+20, %ebx
	subl	C(r_p2)+20, %eax
	subl	C(r_p2)+20, %ebx

	movl	%eax, t0_int
	movl	%ebx, t1_int
	fildl	t0_int             // t0
	fildl	t1_int             // t1 | t0
	fxch	%st(1)             // t0 | t1
	fstps	t0                 // t1
	fsts	t1                 // (empty)

	fmuls	p01_minus_p21      // t1 * p01_minus_p21
	flds	t0                 // t0 | t1 * p01_minus_p21
	fmuls	p11_minus_p21      // t0 * p11_minus_p21 | t1 * p01_minus_p21
	flds	t1                 // t1 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	fmuls	p00_minus_p20      // t1 * p00_minus_p20 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	flds	t0                 // t0 | t1 * p00_minus_p20 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	fmuls	p10_minus_p20      // t0 * p10_minus_p20 | t1 * p00_minus_p20 | t0 * p11_minus_p21 | t1 * p01_minus_p21
	fxch	%st(2)             // t0 * p11_minus_p21 | t0 * p10_minus_p20 | t1 * p00_minus_p20 | t1 * p01_minus_p21
	// FS: The two lines below are swapped.  GAS generates the wrong opcode for this.
	fsubrp	%st(0), %st(3)     // t0 * p10_minus_p20 | t1 * p00_minus_p20 | t1 * p01_minus_p21 - t0 * p11_minus_p21
	fsubp	%st(0), %st(1)     // t1 * p00_minus_p20 - t0 * p10_minus_p20 | t1 * p01_minus_p21 - t0 * p11_minus_p21
	fxch	%st(1)             // t1 * p01_minus_p21 - t0 * p11_minus_p21 | t1 * p00_minus_p20 - t0 * p10_minus_p20
	fmuls	xstepdenominv      // r_lstepx | t1 * p00_minus_p20 - t0 * p10_minus_p20
	fxch	%st(1)
	fmuls	ystepdenominv      // r_lstepy | r_lstepx
	fxch	%st(1)             // r_lstepx | r_lstepy
	fistpl	C(r_zistepx)
	fistpl	C(r_zistepy)

//
// #if	id386ALIAS
// 	a_sstepxfrac = r_sstepx << 16;
// 	a_tstepxfrac = r_tstepx << 16;
// #else
// 	a_sstepxfrac = r_sstepx & 0xFFFF;
// 	a_tstepxfrac = r_tstepx & 0xFFFF;
// #endif
//
	movl	C(d_pdrawspans), %eax
	cmpl	$(C(R_PolysetDrawSpans8_Opaque)), %eax
	movl	C(r_sstepx), %eax
	movl	C(r_tstepx), %ebx
	jne	translucent
// #if id386ALIAS
	shll	$16, %eax
	shll	$16, %ebx
	jmp	done_with_steps
// #else
translucent:
	andl	$65536, %eax
	andl	$65536, %ebx
// #endif
done_with_steps:
	movl	%eax, C(a_sstepxfrac)
	movl	%ebx, C(a_tstepxfrac)

//
// a_ststepxwhole = skinwidth * (r_tstepx >> 16) + (r_sstepx >> 16);
//
	movl	C(r_tstepx), %ebx
	movl	C(r_sstepx), %ecx
	sarl	$16, %ebx
	movl	skinwidth(%ebp), %eax
	mull	%ebx
	sarl	$16, %ecx
	addl	%ecx, %eax
	movl	%eax, C(a_ststepxwhole)

	popl	%ebx
	popl	%ebp
	ret

//----------------------------------------------------------------------
// 8-bpp horizontal span drawing code for affine polygons, with smooth
// shading and no transparency
//----------------------------------------------------------------------

//===================================
//===================================

#define pspans	4+8

.globl C(D_PolysetAff8Start)
C(D_PolysetAff8Start):

.globl C(R_PolysetDrawSpans8_Opaque)
C(R_PolysetDrawSpans8_Opaque):

	pushl	%esi				// preserve register variables
	pushl	%ebx

	movl	pspans(%esp),%esi	// point to the first span descriptor
	movl	C(r_zistepx),%ecx

	pushl	%ebp				// preserve caller's stack frame
	pushl	%edi

	rorl	$16,%ecx			// put high 16 bits of 1/z step in low word
	movl	spanpackage_t_count(%esi),%edx

	movl	%ecx,lzistepx

LSpanLoop:

//		lcount = d_aspancount - pspanpackage->count;
//
//		errorterm += erroradjustup;
//		if (errorterm >= 0)
//		{
//			d_aspancount += d_countextrastep;
//			errorterm -= erroradjustdown;
//		}
//		else
//		{
//			d_aspancount += ubasestep;
//		}

	movl	C(d_aspancount),%eax
	subl	%edx,%eax

	movl	C(erroradjustup),%edx
	movl	C(errorterm),%ebx
	addl	%edx,%ebx
	js		LNoTurnover

	movl	C(erroradjustdown),%edx
	movl	C(d_countextrastep),%edi
	subl	%edx,%ebx
	movl	C(d_aspancount),%ebp
	movl	%ebx,C(errorterm)
	addl	%edi,%ebp
	movl	%ebp,C(d_aspancount)
	jmp		LRightEdgeStepped

LNoTurnover:
	movl	C(d_aspancount),%edi
	movl	C(ubasestep),%edx
	movl	%ebx,C(errorterm)
	addl	%edx,%edi
	movl	%edi,C(d_aspancount)

LRightEdgeStepped:
	cmpl	$1,%eax

	jl		LNextSpan
	jz		LExactlyOneLong

//
// set up advancetable
//
	movl	C(a_ststepxwhole),%ecx
	movl	C(r_affinetridesc)+atd_skinwidth,%edx

	movl	%ecx,advancetable+4	// advance base in t
	addl	%edx,%ecx

	movl	%ecx,advancetable	// advance extra in t
	movl	C(a_tstepxfrac),%ecx

	movw	C(r_lstepx),%cx
	movl	%eax,%edx			// count

	movl	%ecx,tstep
	addl	$7,%edx

	shrl	$3,%edx				// count of full and partial loops
	movl	spanpackage_t_sfrac(%esi),%ebx

	movw	%dx,%bx
	movl	spanpackage_t_pz(%esi),%ecx

	negl	%eax

	movl	spanpackage_t_pdest(%esi),%edi
	andl	$7,%eax		// 0->0, 1->7, 2->6, ... , 7->1

	subl	%eax,%edi	// compensate for hardwired offsets
	subl	%eax,%ecx

	subl	%eax,%ecx
	movl	spanpackage_t_tfrac(%esi),%edx

	movw	spanpackage_t_light(%esi),%dx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	pushl	%esi

	pushl	%eax
	movb	C(iractive), %al
	cmpb	$0, %al
	popl	%eax
	jne	IRInsert

	movl	spanpackage_t_ptex(%esi),%esi
	jmp		*aff8entryvec_table(,%eax,4) // fixed  Warning: indirect jmp without `*'

IRInsert:
	movl spanpackage_t_ptex(%esi), %esi
	addl	$8, %eax
	jmp		*aff8entryvec_table(,%eax,4)

// %bx = count of full and partial loops
// %ebx high word = sfrac
// %ecx = pz
// %dx = light
// %edx high word = tfrac
// %esi = ptex
// %edi = pdest
// %ebp = 1/z
// tstep low word = C(r_lstepx)
// tstep high word = C(a_tstepxfrac)
// C(a_sstepxfrac) low word = 0
// C(a_sstepxfrac) high word = C(a_sstepxfrac)

//===
//Standard Draw Loop
//===
LDrawLoop:

	movb C(iractive), %al
	cmpb $0, %al
	jne LDrawLoopIR

// FIXME: do we need to clamp light? We may need at least a buffer bit to
// keep it from poking into tfrac and causing problems

LDraw8:
	cmpw	(%ecx),%bp
	jl		Lp1
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,(%ecx)
	movb	0x12345678(%eax),%al
LPatch8:
	movb	%al,(%edi)
Lp1:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw7:
	cmpw	2(%ecx),%bp
	jl		Lp2
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,2(%ecx)
	movb	0x12345678(%eax),%al
LPatch7:
	movb	%al,1(%edi)
Lp2:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw6:
	cmpw	4(%ecx),%bp
	jl		Lp3
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,4(%ecx)
	movb	0x12345678(%eax),%al
LPatch6:
	movb	%al,2(%edi)
Lp3:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw5:
	cmpw	6(%ecx),%bp
	jl		Lp4
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,6(%ecx)
	movb	0x12345678(%eax),%al
LPatch5:
	movb	%al,3(%edi)
Lp4:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw4:
	cmpw	8(%ecx),%bp
	jl		Lp5
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,8(%ecx)
	movb	0x12345678(%eax),%al
LPatch4:
	movb	%al,4(%edi)
Lp5:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw3:
	cmpw	10(%ecx),%bp
	jl		Lp6
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,10(%ecx)
	movb	0x12345678(%eax),%al
LPatch3:
	movb	%al,5(%edi)
Lp6:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw2:
	cmpw	12(%ecx),%bp
	jl		Lp7
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,12(%ecx)
	movb	0x12345678(%eax),%al
LPatch2:
	movb	%al,6(%edi)
Lp7:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw1:
	cmpw	14(%ecx),%bp
	jl		Lp8
	xorl	%eax,%eax
	movb	%dh,%ah
	movb	(%esi),%al
	movw	%bp,14(%ecx)
	movb	0x12345678(%eax),%al
LPatch1:
	movb	%al,7(%edi)
Lp8:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$8,%edi
	addl	$16,%ecx

	decw	%bx
	jnz		LDrawLoop

	popl	%esi				// restore spans pointer
LNextSpan:
	addl	$(spanpackage_t_size),%esi	// point to next span
LNextSpanESISet:
	movl	spanpackage_t_count(%esi),%edx
	cmpl	$-999999,%edx		// any more spans?
	jnz		LSpanLoop			// yes

	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	popl	%ebx				// restore register variables
	popl	%esi
	ret

//=======
// IR active draw loop
//=======
LDrawLoopIR:

// FIXME: do we need to clamp light? We may need at least a buffer bit to
// keep it from poking into tfrac and causing problems

LDraw8IR:
	cmpw	(%ecx), %bp
	jl	Lp1IR
	xorl	%eax,%eax
	movb	(%esi), %al
	movb	C(irtable)(%eax), %al
	movw	%bp, (%ecx)
	movb	0x12345678(%eax),%al
LPatch8IR:
	movb	%al, (%edi)
Lp1IR:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw7IR:
	cmpw	2(%ecx), %bp
	jl	Lp2IR
	xorl	%eax,%eax
	movb	(%esi), %al
	movb	C(irtable)(%eax), %al
	movw	%bp, 2(%ecx)
	movb	0x12345678(%eax),%al
LPatch7IR:
	movb	%al, 1(%edi)
Lp2IR:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw6IR:
	cmpw	4(%ecx), %bp
	jl	Lp3IR
	xorl	%eax,%eax
	movb	(%esi), %al
	movb	C(irtable)(%eax), %al
	movw	%bp, 4(%ecx)
	movb	0x12345678(%eax),%al
LPatch6IR:
	movb	%al, 2(%edi)
Lp3IR:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw5IR:
	cmpw	6(%ecx), %bp
	jl	Lp4IR
	xorl	%eax,%eax
	movb	(%esi), %al
	movb	C(irtable)(%eax), %al
	movw	%bp, 6(%ecx)
	movb	0x12345678(%eax),%al
LPatch5IR:
	movb	%al, 3(%edi)
Lp4IR:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw4IR:
	cmpw	8(%ecx), %bp
	jl	Lp5IR
	xorl	%eax,%eax
	movb	(%esi), %al
	movb	C(irtable)(%eax), %al
	movw	%bp, 8(%ecx)
	movb	0x12345678(%eax),%al
LPatch4IR:
	movb	%al, 4(%edi)
Lp5IR:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw3IR:
	cmpw	10(%ecx), %bp
	jl	Lp6IR	
	xorl	%eax,%eax
	movb	(%esi), %al
	movb	C(irtable)(%eax), %al
	movw	%bp, 10(%ecx)
	movb	0x12345678(%eax),%al
LPatch3IR:
	movb	%al, 5(%edi)
Lp6IR:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw2IR:
	cmpw	12(%ecx), %bp
	jl	Lp7IR
	xorl	%eax,%eax
	movb	(%esi), %al
	movb	C(irtable)(%eax), %al
	movw	%bp, 12(%ecx)
	movb	0x12345678(%eax),%al
LPatch2IR:
	movb	%al, 6(%edi)
Lp7IR:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

LDraw1IR:
	cmpw	14(%ecx), %bp
	jl	Lp8IR
	xorl	%eax,%eax
	movb	(%esi), %al
	movb	C(irtable)(%eax), %al
	movw	%bp, 14(%ecx)
	movb	0x12345678(%eax),%al
LPatch1IR:
	movb	%al, 7(%edi)
Lp8IR:
	addl	tstep,%edx
	sbbl	%eax,%eax
	addl	lzistepx,%ebp
	adcl	$0,%ebp
	addl	C(a_sstepxfrac),%ebx
	adcl	advancetable+4(,%eax,4),%esi

	addl	$8,%edi
	addl	$16,%ecx

	decw	%bx
	jnz		LDrawLoopIR

	popl	%esi				// restore spans pointer
LNextSpanIR:
	addl	$(spanpackage_t_size),%esi	// point to next span
LNextSpanESISetIR:	
	movl	spanpackage_t_count(%esi),%edx
	cmpl	$-999999,%edx		// any more spans?
	jnz		LSpanLoop			// yes

	popl	%edi
	popl	%ebp				// restore the caller's stack frame
	popl	%ebx				// restore register variables
	popl	%esi
	ret

//=======
// Standard One-Long Draw
//=======
// draw a one-long span

LExactlyOneLong:
	movb	C(iractive), %al
	cmpb	$0, %al
	jne	LExactlyOneLongIR

	movl	spanpackage_t_pz(%esi),%ecx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	movl	spanpackage_t_ptex(%esi),%ebx

	cmpw	(%ecx),%bp
	jl		LNextSpan
	xorl	%eax,%eax
	movl	spanpackage_t_pdest(%esi),%edi
	movb	spanpackage_t_light+1(%esi),%ah
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	(%ebx),%al
	movw	%bp,(%ecx)
	movb	0x12345678(%eax),%al
LPatch9:
	movb	%al,(%edi)

	jmp		LNextSpanESISet


//========
//========
// draw a one-long span

LExactlyOneLongIR:

	movl	spanpackage_t_pz(%esi),%ecx
	movl	spanpackage_t_zi(%esi),%ebp

	rorl	$16,%ebp	// put high 16 bits of 1/z in low word
	movl	spanpackage_t_ptex(%esi),%ebx

	cmpw	(%ecx),%bp
	jl		LNextSpanIR
	xorl	%eax,%eax
	movl	spanpackage_t_pdest(%esi),%edi
	addl	$(spanpackage_t_size),%esi	// point to next span
	movb	(%ebx),%al
	movb	C(irtable)(%eax), %al
	movw	%bp,(%ecx)
	movb	0x12345678(%eax),%al
LPatch9IR:
	movb	%al,(%edi)

	jmp	LNextSpanESISetIR

//===================================
//===================================
.globl C(D_Aff8Patch)
C(D_Aff8Patch):
	movl	C(alias_colormap),%eax
	movl	%eax,LPatch1-4
	movl	%eax,LPatch2-4
	movl	%eax,LPatch3-4
	movl	%eax,LPatch4-4
	movl	%eax,LPatch5-4
	movl	%eax,LPatch6-4
	movl	%eax,LPatch7-4
	movl	%eax,LPatch8-4
	movl	%eax,LPatch9-4
	movl	%eax,LPatch1IR-4
	movl	%eax,LPatch2IR-4
	movl	%eax,LPatch3IR-4
	movl	%eax,LPatch4IR-4
	movl	%eax,LPatch5IR-4
	movl	%eax,LPatch6IR-4
	movl	%eax,LPatch7IR-4
	movl	%eax,LPatch8IR-4
	movl	%eax,LPatch9IR-4

	ret



//===================================
//===================================

#define height	4+16

.globl C(R_PolysetScanLeftEdge)
C(R_PolysetScanLeftEdge):
	pushl	%ebp				// preserve caller stack frame pointer
	pushl	%esi				// preserve register variables
	pushl	%edi
	pushl	%ebx

	movl	height(%esp),%eax
	movl	C(d_sfrac),%ecx

	andl	$0xFFFF,%eax
	movl	C(d_ptex),%ebx
	orl		%eax,%ecx
	movl	C(d_pedgespanpackage),%esi
	movl	C(d_tfrac),%edx
	movl	C(d_light),%edi
	movl	C(d_zi),%ebp

// %eax: scratch
// %ebx: d_ptex
// %ecx: d_sfrac in high word, count in low word
// %edx: d_tfrac
// %esi: d_pedgespanpackage, errorterm, scratch alternately
// %edi: d_light
// %ebp: d_zi

//	do
//	{

LScanLoop:

//		d_pedgespanpackage->ptex = ptex;
//		d_pedgespanpackage->pdest = d_pdest;
//		d_pedgespanpackage->pz = d_pz;
//		d_pedgespanpackage->count = d_aspancount;
//		d_pedgespanpackage->light = d_light;
//		d_pedgespanpackage->zi = d_zi;
//		d_pedgespanpackage->sfrac = d_sfrac << 16;
//		d_pedgespanpackage->tfrac = d_tfrac << 16;
	movl	%ebx,spanpackage_t_ptex(%esi)
	movl	C(d_pdest),%eax
	movl	%eax,spanpackage_t_pdest(%esi)
	movl	C(d_pz),%eax
	movl	%eax,spanpackage_t_pz(%esi)
	movl	C(d_aspancount),%eax
	movl	%eax,spanpackage_t_count(%esi)
	movl	%edi,spanpackage_t_light(%esi)
	movl	%ebp,spanpackage_t_zi(%esi)
	movl	%ecx,spanpackage_t_sfrac(%esi)
	movl	%edx,spanpackage_t_tfrac(%esi)

// pretouch the next cache line
	movb	spanpackage_t_size(%esi),%al

//		d_pedgespanpackage++;
	addl	$(spanpackage_t_size),%esi
	movl	C(erroradjustup),%eax
	movl	%esi,C(d_pedgespanpackage)

//		errorterm += erroradjustup;
	movl	C(errorterm),%esi
	addl	%eax,%esi
	movl	C(d_pdest),%eax

//		if (errorterm >= 0)
//		{
	js		LNoLeftEdgeTurnover

//			errorterm -= erroradjustdown;
//			d_pdest += d_pdestextrastep;
	subl	C(erroradjustdown),%esi
	addl	C(d_pdestextrastep),%eax
	movl	%esi,C(errorterm)
	movl	%eax,C(d_pdest)

//			d_pz += d_pzextrastep;
//			d_aspancount += d_countextrastep;
//			d_ptex += d_ptexextrastep;
//			d_sfrac += d_sfracextrastep;
//			d_ptex += d_sfrac >> 16;
//			d_sfrac &= 0xFFFF;
//			d_tfrac += d_tfracextrastep;
	movl	C(d_pz),%eax
	movl	C(d_aspancount),%esi
	addl	C(d_pzextrastep),%eax
	addl	C(d_sfracextrastep),%ecx
	adcl	C(d_ptexextrastep),%ebx
	addl	C(d_countextrastep),%esi
	movl	%eax,C(d_pz)
	movl	C(d_tfracextrastep),%eax
	movl	%esi,C(d_aspancount)
	addl	%eax,%edx

//			if (d_tfrac & 0x10000)
//			{
	jnc		LSkip1

//				d_ptex += r_affinetridesc.skinwidth;
//				d_tfrac &= 0xFFFF;
	addl	C(r_affinetridesc)+atd_skinwidth,%ebx

//			}

LSkip1:

//			d_light += d_lightextrastep;
//			d_zi += d_ziextrastep;
	addl	C(d_lightextrastep),%edi
	addl	C(d_ziextrastep),%ebp

//		}
	movl	C(d_pedgespanpackage),%esi
	decl	%ecx
	testl	$0xFFFF,%ecx
	jnz		LScanLoop

	popl	%ebx
	popl	%edi
	popl	%esi
	popl	%ebp
	ret

//		else
//		{

LNoLeftEdgeTurnover:
	movl	%esi,C(errorterm)

//			d_pdest += d_pdestbasestep;
	addl	C(d_pdestbasestep),%eax
	movl	%eax,C(d_pdest)

//			d_pz += d_pzbasestep;
//			d_aspancount += ubasestep;
//			d_ptex += d_ptexbasestep;
//			d_sfrac += d_sfracbasestep;
//			d_ptex += d_sfrac >> 16;
//			d_sfrac &= 0xFFFF;
	movl	C(d_pz),%eax
	movl	C(d_aspancount),%esi
	addl	C(d_pzbasestep),%eax
	addl	C(d_sfracbasestep),%ecx
	adcl	C(d_ptexbasestep),%ebx
	addl	C(ubasestep),%esi
	movl	%eax,C(d_pz)
	movl	%esi,C(d_aspancount)

//			d_tfrac += d_tfracbasestep;
	movl	C(d_tfracbasestep),%esi
	addl	%esi,%edx

//			if (d_tfrac & 0x10000)
//			{
	jnc		LSkip2

//				d_ptex += r_affinetridesc.skinwidth;
//				d_tfrac &= 0xFFFF;
	addl	C(r_affinetridesc)+atd_skinwidth,%ebx

//			}

LSkip2:

//			d_light += d_lightbasestep;
//			d_zi += d_zibasestep;
	addl	C(d_lightbasestep),%edi
	addl	C(d_zibasestep),%ebp

//		}
//	} while (--height);
	movl	C(d_pedgespanpackage),%esi
	decl	%ecx
	testl	$0xFFFF,%ecx
	jnz		LScanLoop

	popl	%ebx
	popl	%edi
	popl	%esi
	popl	%ebp
	ret

#endif	// id386

