//
// Public declarations of flashlight functions
//
#ifndef __FLASHLIGHT_H
#define __FLASHLIGHT_H

void Cmd_Flashlight(edict_t* ent);
void FlashlightReset(edict_t* self);
void Use_Flashlight (edict_t *player);
void flashlight_think (edict_t *self);

#endif // __FLASHLIGHT_H
