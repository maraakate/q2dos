//
// r_parta.s
// x86 assembly-language particle code.
//

#include "qasm.h"

#if	id386

	.data
eight_thousand_hex: .single	32768.0

// old as from Apple cctools has no knowledge of '.bss'
#ifdef __APPLE__
.zerofill	__DATA,__bss,short_izi, 1, 4
.zerofill	__DATA,__bss,transformed_vec, 12, 4
.zerofill	__DATA,__bss,local_vec, 12, 4
.zerofill	__DATA,__bss,ebpsave, 1, 4
.zerofill	__DATA,__bss,v, 1, 4
.zerofill	__DATA,__bss,u, 1, 4
.zerofill	__DATA,__bss,tmp, 1, 4
.zerofill	__DATA,__bss,zi, 1, 4

#else
	.bss
.lcomm	short_izi, 1

	.align 4

#ifdef ELF
.local	transformed_vec, local_vec, ebpsave, v, u, tmp, zi
.comm	transformed_vec, 12, 4
.comm	local_vec, 12, 4
.comm	ebpsave, 1, 4
.comm	v, 1, 4
.comm	u, 1, 4
.comm	tmp, 1, 4
.comm	zi, 1, 4
#else
.lcomm	transformed_vec, 12, 4
.lcomm	local_vec, 12, 4
.lcomm	ebpsave, 1, 4
.lcomm	v, 1, 4
.lcomm	u, 1, 4
.lcomm	tmp, 1, 4
.lcomm	zi, 1, 4
#endif
#endif

	.text
#define PARTICLE_33	0
#define PARTICLE_66	1
#define PARTICLE_OPAQUE	2

	.align 4
.globl C(R_DrawParticle)
C(R_DrawParticle):
//
// save trashed variables
//
	movl %ebp, ebpsave
	pushl %esi
	pushl %edi
	pushl %ebx

//
// transform the particle
//
// VectorSubtract (pparticle->origin, r_origin, local);
	movl  C(partparms)+partparms_particle, %esi
	flds  0(%esi)          // p_o.x
	fsubs C(r_origin)     // p_o.x-r_o.x
	flds  4(%esi)          // p_o.y | p_o.x-r_o.x
	fsubs C(r_origin)+4     // p_o.y-r_o.y | p_o.x-r_o.x
	flds  8(%esi)          // p_o.z | p_o.y-r_o.y | p_o.x-r_o.x
	fsubs C(r_origin)+8     // p_o.z-r_o.z | p_o.y-r_o.y | p_o.x-r_o.x
	fxch  %st(2)                      // p_o.x-r_o.x | p_o.y-r_o.y | p_o.z-r_o.z
	fstps local_vec+0        // p_o.y-r_o.y | p_o.z-r_o.z
	fstps local_vec+4        // p_o.z-r_o.z
	fstps local_vec+8        // (empty)

// transformed[0] = DotProduct(local, r_pright);
// transformed[1] = DotProduct(local, r_pup);
// transformed[2] = DotProduct(local, r_ppn);
	flds local_vec+0        // l.x
	fmuls C(r_pright)+0     // l.x*pr.x
	flds local_vec+4        // l.y | l.x*pr.x
	fmuls C(r_pright)+4     // l.y*pr.y | l.x*pr.x
	flds local_vec+8        // l.z | l.y*pr.y | l.x*pr.x
	fmuls C(r_pright)+8     // l.z*pr.z | l.y*pr.y | l.x*pr.x
	fxch %st(2)                      // l.x*pr.x | l.y*pr.y | l.z*pr.z
	faddp %st(0), %st(1)                 // l.x*pr.x + l.y*pr.y | l.z*pr.z
	faddp %st(0), %st(1)                 // l.x*pr.x + l.y*pr.y + l.z*pr.z
	fstps transformed_vec+0 // (empty)

	flds local_vec+0        // l.x
	fmuls C(r_pup)+0        // l.x*pr.x
	flds local_vec+4        // l.y | l.x*pr.x
	fmuls C(r_pup)+4        // l.y*pr.y | l.x*pr.x
	flds local_vec+8        // l.z | l.y*pr.y | l.x*pr.x
	fmuls C(r_pup)+8        // l.z*pr.z | l.y*pr.y | l.x*pr.x
	fxch %st(2)                      // l.x*pr.x | l.y*pr.y | l.z*pr.z
	faddp %st(0), %st(1)                 // l.x*pr.x + l.y*pr.y | l.z*pr.z
	faddp %st(0), %st(1)                 // l.x*pr.x + l.y*pr.y + l.z*pr.z
	fstps transformed_vec+4 // (empty)

	flds local_vec+0        // l.x
	fmuls C(r_ppn)+0        // l.x*pr.x
	flds local_vec+4        // l.y | l.x*pr.x
	fmuls C(r_ppn)+4        // l.y*pr.y | l.x*pr.x
	flds local_vec+8        // l.z | l.y*pr.y | l.x*pr.x
	fmuls C(r_ppn)+8        // l.z*pr.z | l.y*pr.y | l.x*pr.x
	fxch %st(2)                      // l.x*pr.x | l.y*pr.y | l.z*pr.z
	faddp %st(0), %st(1)                 // l.x*pr.x + l.y*pr.y | l.z*pr.z
	faddp %st(0), %st(1)                 // l.x*pr.x + l.y*pr.y + l.z*pr.z
	fstps transformed_vec+8 // (empty)

//
// make sure that the transformed particle is not in front of
// the particle Z clip plane.  We can do the comparison in 
// integer space since we know the sign of one of the inputs
// and can figure out the sign of the other easily enough.
//
//	if (transformed[2] < PARTICLE_Z_CLIP)
//		return;

	movl transformed_vec+8, %eax
	andl  %eax, %eax
	js   endpartfunc
	cmpl  float_particle_z_clip, %eax
	jl   endpartfunc

//
// project the point by initiating the 1/z calc
//
//	zi = 1.0 / transformed[2];
	flds   float_1
	fdivs  transformed_vec+8

// prefetch the next particle
	movl C(s_prefetch_address), %ebp
	movl (%ebp), %ebp

// finish the above divide
	fstps  zi

// u = (int)(xcenter + zi * transformed[0] + 0.5)
// v = (int)(ycenter - zi * transformed[1] + 0.5)
	flds zi                           // zi
	fmuls transformed_vec+0    // zi * transformed[0]
	flds zi                           // zi | zi * transformed[0]
	fmuls transformed_vec+4    // zi * transformed[1] | zi * transformed[0]
	fxch %st(1)                        // zi * transformed[0] | zi * transformed[1]
	fadds C(xcenter)                      // xcenter + zi * transformed[0] | zi * transformed[1]
	fxch %st(1)                        // zi * transformed[1] | xcenter + zi * transformed[0]
	flds C(ycenter)                      // ycenter | zi * transformed[1] | xcenter + zi * transformed[0]
	fsubp %st(0), %st(1)                // ycenter - zi * transformed[1] | xcenter + zi * transformed[0]
	fxch  %st(1)                        // xcenter + zi * transformed[0] | ycenter + zi * transformed[1]
	fadds  float_point5                   // xcenter + zi * transformed[0] + 0.5 | ycenter - zi * transformed[1]
	fxch  %st(1)                        // ycenter - zi * transformed[1] | xcenter + zi * transformed[0] + 0.5 
	fadds  float_point5                   // ycenter - zi * transformed[1] + 0.5 | xcenter + zi * transformed[0] + 0.5 
	fxch  %st(1)                        // u | v
	fistpl u                // v
	fistpl v                // (empty)

//
// clip out the particle
//

//	if ((v > d_vrectbottom_particle) || 
//		(u > d_vrectright_particle) ||
//		(v < d_vrecty) ||
//		(u < d_vrectx))
//	{
//		return;
//	}

	movl u, %ebx
	movl v, %ecx
	cmpl C(d_vrectbottom_particle), %ecx
	jg  endpartfunc
	cmpl C(d_vrecty), %ecx
	jl  endpartfunc
	cmpl C(d_vrectright_particle), %ebx
	jg  endpartfunc
	cmpl C(d_vrectx), %ebx
	jl  endpartfunc

//
// compute addresses of zbuffer, framebuffer, and 
// compute the Z-buffer reference value.
//
// EBX      = U
// ECX      = V
//
// Outputs:
// ESI = Z-buffer address
// EDI = framebuffer address
//
// ESI = d_pzbuffer + (d_zwidth * v) + u;
	movl C(d_pzbuffer), %esi           // esi = d_pzbuffer
	movl C(d_zwidth), %eax             // eax = d_zwidth
	mull %ecx                          // eax = d_zwidth*v
	addl %ebx, %eax                    // eax = d_zwidth*v+u
	shll $1, %eax                      // eax = 2*(d_zwidth*v+u)
	addl %eax, %esi                    // esi = ( short * ) ( d_pzbuffer + ( d_zwidth * v ) + u )

// initiate
// izi = (int)(zi * 0x8000);
	flds zi
	fmuls eight_thousand_hex

// EDI = pdest = d_viewbuffer + d_scantable[v] + u;
	leal C(d_scantable)(,%ecx,4),%edi
	movl (%edi), %edi
	addl C(d_viewbuffer), %edi
	addl %ebx, %edi

// complete
// izi = (int)(zi * 0x8000);
	fistpl tmp
	movl tmp, %eax
	movw %ax, (short_izi)

//
// determine the screen area covered by the particle,
// which also means clamping to a min and max
//
//	pix = izi >> d_pix_shift;
	xorl %edx, %edx
	movw (short_izi), %dx
	movl C(d_pix_shift), %ecx
	shrw %cl, %dx

//	if (pix < d_pix_min)
//		pix = d_pix_min;
	cmpl C(d_pix_min), %edx
	jge check_pix_max
	movl C(d_pix_min), %edx
	jmp skip_pix_clamp

//	else if (pix > d_pix_max)
//		pix = d_pix_max;
check_pix_max:
	cmpl C(d_pix_max), %edx
	jle skip_pix_clamp
	movl C(d_pix_max), %edx

skip_pix_clamp:

//
// render the appropriate pixels
//
// ECX = count (used for inner loop)
// EDX = count (used for outer loop)
// ESI = zbuffer
// EDI = framebuffer
//
	movl %edx, %ecx

	cmpl $1, %ecx
	ja  over

over:

//
// at this point:
//
// ECX = count
//
	pushl %ecx
	pushl %edi
	pushl %esi

top_of_pix_vert_loop:

top_of_pix_horiz_loop:

//	for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
//	{
//		for (i=0 ; i<pix ; i++)
//		{
//			if (pz[i] <= izi)
//			{
//				pdest[i] = blendparticle( color, pdest[i] );
//			}
//		}
//	}
	xorl   %eax, %eax

	movw  (%esi), %ax

	cmpw  (short_izi), %ax
	jg    end_of_horiz_loop

	movl  C(partparms)+partparms_color, %eax

	cmpl $PARTICLE_66, C(partparms)+partparms_level
	je blendfunc_66
	jl blendfunc_33
// BlendParticle100
	movb	%al, (%edi)
	jmp	done_blending
blendfunc_33:
	movl C(vid)+vid_alphamap, %ebp
	xorl %ebx, %ebx

	movb (%edi), %bl
	shll $8, %ebx

	addl %ebx, %ebp
	addl %eax, %ebp

	movb (%ebp), %al

	movb %al, (%edi)
	jmp done_blending
blendfunc_66:
	movl C(vid)+vid_alphamap, %ebp
	xorl %ebx, %ebx

	shll $8, %eax
	movb (%edi), %bl

	addl %ebx, %ebp
	addl %eax, %ebp

	movb (%ebp), %al

	movb %al, (%edi)

done_blending:

	addl	$1, %edi
	addl	$2, %esi

end_of_horiz_loop:
	decl   %ecx
	jnz   top_of_pix_horiz_loop

	popl   %esi
	popl   %edi

	movl   C(d_zwidth), %ebp
	shll   $1, %ebp

	addl   %ebp, %esi
	addl   C(r_screenwidth), %edi

	popl   %ecx
	pushl  %ecx

	pushl  %edi
	pushl  %esi

	decl   %edx
	jnz   top_of_pix_vert_loop

	popl   %ecx
	popl   %ecx
	popl   %ecx

endpartfunc:
	popl %ebx
	popl %edi
	popl %esi
	movl ebpsave, %ebp
	ret

#endif	// id386
