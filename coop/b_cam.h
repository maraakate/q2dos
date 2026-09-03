// b_coopcam.h
// blinky
// Created: 2000/06
// Edited:  2000/07/17
//

#ifndef B_CAM_H
#define B_CAM_H

/* FS: Flags for sv_vote_disallow_flags */
#define	BCAM_NOSUMMON			0x00000002 /* 2 */
#define	BCAM_NOTELEPORT			0x00000004 /* 4 */
#define	BCAM_NOSTATS			0x00000008 /* 8 */
#define	BCAM_NORUNRUN			0x00000010 /* 16 */
#define	BCAM_NOCHASE			0x00000020 /* 32 */

struct BlinkyClient_s
{
	edict_t	*cam_target;
	edict_t *cam_decoy; // if camming, this is our sub
	// save some stuff when camming (to restore correctly)
	int runrun;
	float save_fov;
	int save_hand;
	int save_weapon;

	int nopickup;
};

typedef struct BlinkyClient_s BlinkyClient_t;

void Blinky_UpdateCameraThink(edict_t *ent);
void Blinky_BeginRunFrame();
void Blinky_RunRun(edict_t *ent, usercmd_t *ucmd);

void Cmd_Cam_f(edict_t *ent);
void Cmd_Stats_f(edict_t *ent);
void Cmd_Summon_f(edict_t *ent);
void Cmd_Teleport_f(edict_t *ent);
void Cmd_NoSummon_f(edict_t *ent);
void Cmd_Runrun_f(edict_t *ent);
void Blinky_OnClientTerminate(edict_t *self);
void Blinky_CalcViewOffsets(edict_t * ent, vec3_t v);

void stopBlinkyCam (edict_t *ent);

extern void P_ProjectSource ( gclient_t * client , vec3_t point , vec3_t distance , vec3_t forward , vec3_t right , vec3_t result ) ;

#endif // B_CAM_H
