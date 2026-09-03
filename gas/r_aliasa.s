//
// r_aliasa.s
// x86 assembly-language alias drawing code
//

#include "qasm.h"
#include "d_ifacea.h"

#if	id386

	.data
FALIAS_Z_CLIP_PLANE:	.long	0x40800000
PS_SCALE: 				.long	0x40800000

#define newv					20
#define oldv					16
#define fv						12
#define numpoints				8
#define lerped_vert				-12
#define lightcos				-16
#define one						-20
#define byte_to_dword_ptr_var	-24
#define zi_loc					-28
#define tmpint 					-32

	.text
	.align 4
.globl C(R_AliasTransformFinalVerts)
C(R_AliasTransformFinalVerts):
	pushl	%ebp
	movl	%esp, %ebp
	subl	$32, %esp
	pushl	%ebx
	pushl	%esi
	pushl	%edi

	fld1
	fstps	one(%ebp)
	movl	numpoints(%ebp), %ecx

top_of_loop:
//	/*
//	lerped_vert[0] = r_lerp_move[0] + oldv->v[0]*r_lerp_backv[0] + newv->v[0]*r_lerp_frontv[0];
//	lerped_vert[1] = r_lerp_move[1] + oldv->v[1]*r_lerp_backv[1] + newv->v[1]*r_lerp_frontv[1];
//	lerped_vert[2] = r_lerp_move[2] + oldv->v[2]*r_lerp_backv[2] + newv->v[2]*r_lerp_frontv[2];
//	*/

	movl	oldv(%ebp), %esi
	movl	newv(%ebp), %edi

	xorl	%ebx, %ebx

	movb	DTRIVERTX_V0(%esi), %bl
	movl	%ebx, byte_to_dword_ptr_var(%ebp)
	fildl	byte_to_dword_ptr_var(%ebp)
	fmuls	C(r_lerp_backv)                                // oldv[0]*rlb[0]

	movb	DTRIVERTX_V1(%esi), %bl
	movl	%ebx, byte_to_dword_ptr_var(%ebp)
	fildl	byte_to_dword_ptr_var(%ebp)
	fmuls	C(r_lerp_backv)+4                              // oldv[1]*rlb[1] | oldv[0]*rlb[0]

	movb	DTRIVERTX_V2(%esi), %bl
	movl	%ebx, byte_to_dword_ptr_var(%ebp)
	fildl	byte_to_dword_ptr_var(%ebp)
	fmuls	C(r_lerp_backv)+8                              // oldv[2]*rlb[2] | oldv[1]*rlb[1] | oldv[0]*rlb[0]

	movb	DTRIVERTX_V0(%edi), %bl
	movl	%ebx, byte_to_dword_ptr_var(%ebp)
	fildl	byte_to_dword_ptr_var(%ebp)
	fmuls	C(r_lerp_frontv)                               // newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | oldv[0]*rlb[0]

	movb	DTRIVERTX_V1(%edi), %bl
	movl	%ebx, byte_to_dword_ptr_var(%ebp)
	fildl	byte_to_dword_ptr_var(%ebp)
	fmuls	C(r_lerp_frontv)+4                             // newv[1]*rlf[1] | newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | oldv[0]*rlb[0]

	movb	DTRIVERTX_V2(%edi), %bl
	movl	%ebx, byte_to_dword_ptr_var(%ebp)
	fildl	byte_to_dword_ptr_var(%ebp)
	fmuls	C(r_lerp_frontv)+8                             // newv[2]*rlf[2] | newv[1]*rlf[1] | newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | oldv[0]*rlb[0]

	fxch	%st(5)                                         // oldv[0]*rlb[0] | newv[1]*rlf[1] | newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | newv[2]*rlf[2]
	faddp	%st(0), %st(2)                                 // newv[1]*rlf[1] | oldv[0]*rlb[0] + newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] | newv[2]*rlf[2]
	faddp	%st(0), %st(3)                                 // oldv[0]*rlb[0] + newv[0]*rlf[0] | oldv[2]*rlb[2] | oldv[1]*rlb[1] + newv[1]*rlf[1] | newv[2]*rlf[2]
	fxch	%st(1)                                         // oldv[2]*rlb[2] | oldv[0]*rlb[0] + newv[0]*rlf[0] | oldv[1]*rlb[1] + newv[1]*rlf[1] | newv[2]*rlf[2]
	faddp	%st(0), %st(3)                                 // oldv[0]*rlb[0] + newv[0]*rlf[0] | oldv[1]*rlb[1] + newv[1]*rlf[1] | oldv[2]*rlb[2] + newv[2]*rlf[2]
	fadds	C(r_lerp_move)                                 // lv0 | oldv[1]*rlb[1] + newv[1]*rlf[1] | oldv[2]*rlb[2] + newv[2]*rlf[2]
	fxch	%st(1)                                         // oldv[1]*rlb[1] + newv[1]*rlf[1] | lv0 | oldv[2]*rlb[2] + newv[2]*rlf[2]
	fadds	C(r_lerp_move)+4                               // lv1 | lv0 | oldv[2]*rlb[2] + newv[2]*rlf[2]
	fxch	%st(2)                                         // oldv[2]*rlb[2] + newv[2]*rlf[2] | lv0 | lv1
	fadds	C(r_lerp_move)+8                               // lv2 | lv0 | lv1
	fxch	%st(1)                                         // lv0 | lv2 | lv1
	fstps	lerped_vert(%ebp)                              // lv2 | lv1
	fstps	lerped_vert+8(%ebp)                            // lv2
	fstps	lerped_vert+4(%ebp)                            // (empty)

	movl	C(currententity), %eax
	movl	ENTITY_FLAGS(%eax), %eax
	movl	$0x31c00, %ebx                                 // RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM
	andl	%ebx, %eax
	je	not_powersuit

//	/*
//	**    lerped_vert[0] += lightnormal[0] * POWERSUIT_SCALE
//	**    lerped_vert[1] += lightnormal[1] * POWERSUIT_SCALE
//	**    lerped_vert[2] += lightnormal[2] * POWERSUIT_SCALE
//	*/

	xorl	%ebx, %ebx
	movb	DTRIVERTX_LNI(%edi), %bl
	movl	$12, %eax
	mull	%ebx
	leal	C(r_avertexnormals)(,%eax,1), %eax

	flds	(%eax)                                         // n[0]
	fmuls	PS_SCALE                                       // n[0] * PS
	flds	4(%eax)                                        // n[1] | n[0] * PS
	fmuls	PS_SCALE                                       // n[1] * PS | n[0] * PS
	flds	8(%eax)                                        // n[2] | n[1] * PS | n[0] * PS
	fmuls	PS_SCALE                                       // n[2] * PS | n[1] * PS | n[0] * PS
	flds	lerped_vert(%ebp)                              // lv0 | n[2] * PS | n[1] * PS | n[0] * PS
	faddp	%st(0), %st(3)                                 // n[2] * PS | n[1] * PS | n[0] * PS + lv0
	flds	lerped_vert+4(%ebp)                            // lv1 | n[2] * PS | n[1] * PS | n[0] * PS + lv0
	faddp	%st(0), %st(2)                                 // n[2] * PS | n[1] * PS + lv1 | n[0] * PS + lv0
	fadds	lerped_vert+8(%ebp)                            // n[2] * PS + lv2 | n[1] * PS + lv1 | n[0] * PS + lv0
	fxch	%st(2)                                         // LV0 | LV1 | LV2
	fstps	lerped_vert(%ebp)                              // LV1 | LV2
	fstps	lerped_vert+4(%ebp)                            // LV2
	fstps	lerped_vert+8(%ebp)                            // (empty)

not_powersuit:
//	/*
//	fv->flags = 0;
//
//	fv->xyz[0] = DotProduct(lerped_vert, aliastransform[0]) + aliastransform[0][3];
//	fv->xyz[1] = DotProduct(lerped_vert, aliastransform[1]) + aliastransform[1][3];
//	fv->xyz[2] = DotProduct(lerped_vert, aliastransform[2]) + aliastransform[2][3];
//	*/

	movl	fv(%ebp), %eax
	movl	$0, FINALVERT_FLAGS(%eax)

	flds	lerped_vert(%ebp)                              // lv0
	fmuls	C(aliastransform)                              // lv0*at[0][0]
	flds	lerped_vert+4(%ebp)                            // lv1 | lv0*at[0][0]
	fmuls	C(aliastransform)+4                            // lv1*at[0][1] | lv0*at[0][0]
	flds	lerped_vert+8(%ebp)                            // lv2 | lv1*at[0][1] | lv0*at[0][0]
	fmuls	C(aliastransform)+8                            // lv2*at[0][2] | lv1*at[0][1] | lv0*at[0][0]
	fxch	%st(2)                                         // lv0*at[0][0] | lv1*at[0][1] | lv2*at[0][2]
	faddp	%st(0), %st(1)                                 // lv0*at[0][0] + lv1*at[0][1] | lv2*at[0][2]
	faddp	%st(0), %st(1)                                 // lv0*at[0][0] + lv1*at[0][1] + lv2*at[0][2]
	fadds	C(aliastransform)+12                           // FV.X

	flds	lerped_vert(%ebp)                              // lv0
	fmuls	C(aliastransform)+16                           // lv0*at[1][0]
	flds	lerped_vert+4(%ebp)                            // lv1 | lv0*at[1][0]
	fmuls	C(aliastransform)+20                           // lv1*at[1][1] | lv0*at[1][0]
	flds	lerped_vert+8(%ebp)                            // lv2 | lv1*at[1][1] | lv0*at[1][0]
	fmuls	C(aliastransform)+24                           // lv2*at[1][2] | lv1*at[1][1] | lv0*at[1][0]
	fxch	%st(2)                                         // lv0*at[1][0] | lv1*at[1][1] | lv2*at[1][2]
	faddp	%st(0), %st(1)                                 // lv0*at[1][0] + lv1*at[1][1] | lv2*at[1][2]
	faddp	%st(0), %st(1)                                 // lv0*at[1][0] + lv1*at[1][1] + lv2*at[1][2]
	fadds	C(aliastransform)+28                           // FV.Y | FV.X
	fxch	%st(1)                                         // FV.X | FV.Y
	fstps	FINALVERT_X(%eax)                              // FV.Y

	flds	lerped_vert(%ebp)                              // lv0
	fmuls	C(aliastransform)+32                           // lv0*at[2][0]
	flds	lerped_vert+4(%ebp)                            // lv1 | lv0*at[2][0]
	fmuls	C(aliastransform)+36                           // lv1*at[2][1] | lv0*at[2][0]
	flds	lerped_vert+8(%ebp)                            // lv2 | lv1*at[2][1] | lv0*at[2][0]
	fmuls	C(aliastransform)+40                           // lv2*at[2][2] | lv1*at[2][1] | lv0*at[2][0]
	fxch	%st(2)                                         // lv0*at[2][0] | lv1*at[2][1] | lv2*at[2][2]
	faddp	%st(0), %st(1)                                 // lv0*at[2][0] + lv1*at[2][1] | lv2*at[2][2]
	faddp	%st(0), %st(1)                                 // lv0*at[2][0] + lv1*at[2][1] + lv2*at[2][2]
	fadds	C(aliastransform)+44                           // FV.Z | FV.Y
	fxch	%st(1)                                         // FV.Y | FV.Z
	fstps	FINALVERT_Y(%eax)                              // FV.Z
	fstps	FINALVERT_Z(%eax)                              // (empty)

//	/*
//	**  lighting
//	**
//	**  plightnormal = r_avertexnormals[newv->lightnormalindex];
//	**	lightcos = DotProduct (plightnormal, r_plightvec);
//	**	temp = r_ambientlight;
//	*/

	xorl	%ebx, %ebx
	movb	DTRIVERTX_LNI(%edi), %bl
	movl	$12, %eax
	mull	%ebx
	leal	C(r_avertexnormals)(,%eax,1), %eax
	leal	C(r_plightvec), %ebx

	flds	(%eax)
	fmuls	(%ebx)
	flds	4(%eax)
	fmuls	4(%ebx)
	flds	8(%eax)
	fmuls	8(%ebx)
	fxch	%st(2)
	faddp	%st(0), %st(1)
	faddp	%st(0), %st(1)
	fstps	lightcos(%ebp)
	movl	lightcos(%ebp), %eax
	movl	C(r_ambientlight), %ebx

//	/*
//	if (lightcos < 0)
//	{
//		temp += (int)(r_shadelight * lightcos);
//
//		// clamp; because we limited the minimum ambient and shading light, we
//		// don't have to clamp low light, just bright
//		if (temp < 0)
//			temp = 0;
//	}
//
//	fv->v[4] = temp;
//	*/

	orl	%eax, %eax
	jns	store_fv4

	flds	C(r_shadelight)
	fmuls	lightcos(%ebp)
	fistpl	tmpint(%ebp)
	addl	tmpint(%ebp), %ebx

	orl	%ebx, %ebx
	jns	store_fv4

	movl	$0, %ebx
store_fv4:
	movl	fv(%ebp), %edi
	movl	%ebx, FINALVERT_V4(%edi)

	movl	FINALVERT_FLAGS(%edi), %edx

//	/*
//	** do clip testing and projection here
//	*/
//	/*
//	if ( dest_vert->xyz[2] < ALIAS_Z_CLIP_PLANE )
//	{
//		dest_vert->flags |= ALIAS_Z_CLIP;
//	}
//	else
//	{
//		R_AliasProjectAndClipTestFinalVert( dest_vert );
//	}
//	*/

	movl	FINALVERT_Z(%edi), %eax
	andl	%eax, %eax
	js	alias_z_clip
	cmpl	FALIAS_Z_CLIP_PLANE, %eax 
	jl	alias_z_clip

//	/*
//	This is the code to R_AliasProjectAndClipTestFinalVert
//
//	float	zi;
//	float	x, y, z;
//
//	x = fv->xyz[0];
//	y = fv->xyz[1];
//	z = fv->xyz[2];
//	zi = 1.0 / z;
//
//	fv->v[5] = zi * s_ziscale;
//
//	fv->v[0] = (x * aliasxscale * zi) + aliasxcenter;
//	fv->v[1] = (y * aliasyscale * zi) + aliasycenter;
//	*/

	flds	one(%ebp)
	fdivs	FINALVERT_Z(%edi)                              // zi

	movl	32(%edi), %eax
	movl	64(%edi), %eax

	fsts	zi_loc(%ebp)                                   // zi
	fmuls	C(s_ziscale)                                   // fv5
	flds	FINALVERT_X(%edi)                              // x | fv5
	fmuls	C(aliasxscale)                                 // x * aliasxscale | fv5
	flds	FINALVERT_Y(%edi)                              // y | x * aliasxscale | fv5
	fmuls	C(aliasyscale)                                 // y * aliasyscale | x * aliasxscale | fv5
	fxch	%st(1)                                         // x * aliasxscale | y * aliasyscale | fv5
	fmuls	zi_loc(%ebp)                                   // x * asx * zi | y * asy | fv5
	fadds	C(aliasxcenter)                                // fv0 | y * asy | fv5
	fxch	%st(1)                                         // y * asy | fv0 | fv5
	fmuls	zi_loc(%ebp)                                   // y * asy * zi | fv0 | fv5
	fadds	C(aliasycenter)                                // fv1 | fv0 | fv5
	fxch	%st(2)                                         // fv5 | fv0 | fv1
	fistpl	FINALVERT_V5(%edi)                             // fv0 | fv1
	fistpl	FINALVERT_V0(%edi)                             // fv1
	fistpl	FINALVERT_V1(%edi)                             // (empty)

//	/*
//	if (fv->v[0] < r_refdef.aliasvrect.x)
//		fv->flags |= ALIAS_LEFT_CLIP;
//	if (fv->v[1] < r_refdef.aliasvrect.y)
//		fv->flags |= ALIAS_TOP_CLIP;
//	if (fv->v[0] > r_refdef.aliasvrectright)
//		fv->flags |= ALIAS_RIGHT_CLIP;
//	if (fv->v[1] > r_refdef.aliasvrectbottom)
//		fv->flags |= ALIAS_BOTTOM_CLIP;
//	*/

	movl	FINALVERT_V0(%edi), %eax
	movl	FINALVERT_V1(%edi), %ebx

	cmpl	C(r_refdef)+20, %eax
	jge	ct_alias_top
	orl	$ALIAS_LEFT_CLIP, %edx

ct_alias_top:
	cmpl	C(r_refdef)+24, %ebx                           // r_refdef.aliasvrect.y
	jge	ct_alias_right
	orl	$ALIAS_TOP_CLIP, %edx

ct_alias_right:
	cmp	C(r_refdef)+48, %eax                               // r_refdef.aliasvrectright
	jle	ct_alias_bottom
	orl	$ALIAS_RIGHT_CLIP, %edx

ct_alias_bottom:
	cmp	C(r_refdef)+52, %ebx                               // r_refdef.aliasvrectbottom
	jle	end_of_loop
	orl	$ALIAS_BOTTOM_CLIP, %edx

	jmp	end_of_loop

alias_z_clip:
	orl	$ALIAS_Z_CLIP, %edx

end_of_loop:
	movl	%edx, FINALVERT_FLAGS(%edi)
	addl	$DTRIVERTX_SIZE, oldv(%ebp)
	addl	$DTRIVERTX_SIZE, newv(%ebp)
	addl	$FINALVERT_SIZE, fv(%ebp)

	decl	%ecx
	jne	top_of_loop

	popl	%edi
	popl	%esi
	popl	%ebx
	movl	%ebp, %esp
	popl	%ebp
	ret

#endif	//id386
