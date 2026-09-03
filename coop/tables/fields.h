/*
 * =======================================================================
 *
 * Game fields to be saved.
 *
 * =======================================================================
 */ 

{"origin", FOFS(s.origin), F_VECTOR},
{"angles", FOFS(s.angles), F_VECTOR},
{"owner", FOFS(owner), F_EDICT, FFL_NOSPAWN},
{"model", FOFS(model), F_LSTRING},
{"model2", FOFS(model2), F_LSTRING}, // DG: new
{"model3", FOFS(model3), F_LSTRING}, // DG: new
{"model4", FOFS(model4), F_LSTRING}, // DG: new
{"classname", FOFS(classname), F_LSTRING},
{"spawnflags", FOFS(spawnflags), F_INT},
{"angle", FOFS(s.angles), F_ANGLEHACK},
{"target", FOFS(target), F_LSTRING},
{"targetname", FOFS(targetname), F_LSTRING},
{"killtarget", FOFS(killtarget), F_LSTRING},
{"team", FOFS(team), F_LSTRING},
{"pathtarget", FOFS(pathtarget), F_LSTRING},
{"deathtarget", FOFS(deathtarget), F_LSTRING},
{"combattarget", FOFS(combattarget), F_LSTRING},
{"target_ent", FOFS(target_ent), F_EDICT, FFL_NOSPAWN},
{"speed", FOFS(speed), F_FLOAT},
{"accel", FOFS(accel), F_FLOAT},
{"decel", FOFS(decel), F_FLOAT},
{"aspeed", FOFS(aspeed), F_FLOAT}, // DG: new
{"mass", FOFS(mass), F_INT},
{"message", FOFS(message), F_LSTRING},
{"goalentity", FOFS(goalentity), F_EDICT, FFL_NOSPAWN},
{"movetarget", FOFS(movetarget), F_EDICT, FFL_NOSPAWN},
{"prethink", FOFS(prethink), F_FUNCTION, FFL_NOSPAWN},
{"think", FOFS(think), F_FUNCTION, FFL_NOSPAWN},
{"blocked", FOFS(blocked), F_FUNCTION, FFL_NOSPAWN},
{"touch", FOFS(touch), F_FUNCTION, FFL_NOSPAWN},
{"use", FOFS(use), F_FUNCTION, FFL_NOSPAWN},
{"pain", FOFS(pain), F_FUNCTION, FFL_NOSPAWN},
{"die", FOFS(die), F_FUNCTION, FFL_NOSPAWN},
{"wait", FOFS(wait), F_FLOAT},
{"delay", FOFS(delay), F_FLOAT},
{"random", FOFS(random), F_FLOAT},
{"move_origin", FOFS(move_origin), F_VECTOR},
{"move_angles", FOFS(move_angles), F_VECTOR},
{"style", FOFS(style), F_INT},
{"health", FOFS(health), F_INT},
{"map", FOFS(map), F_LSTRING},
{"dmg", FOFS(dmg), F_INT},
{"sounds", FOFS(sounds), F_INT},
{"count", FOFS(count), F_INT},
{"chain", FOFS(chain), F_EDICT, FFL_NOSPAWN},
{"volume", FOFS(volume), F_FLOAT},
{"attenuation", FOFS(attenuation), F_FLOAT},
{"enemy", FOFS(enemy), F_EDICT, FFL_NOSPAWN},
{"oldenemy", FOFS(oldenemy), F_EDICT, FFL_NOSPAWN},
{"activator", FOFS(activator), F_EDICT, FFL_NOSPAWN},
{"groundentity", FOFS(groundentity), F_EDICT, FFL_NOSPAWN},
{"teamchain", FOFS(teamchain), F_EDICT, FFL_NOSPAWN},
{"teammaster", FOFS(teammaster), F_EDICT, FFL_NOSPAWN},
{"mynoise", FOFS(mynoise), F_EDICT, FFL_NOSPAWN},
{"mynoise2", FOFS(mynoise2), F_EDICT, FFL_NOSPAWN},
// noise_index, noise_index2?
{"volume", FOFS(volume), F_FLOAT},
{"attenuation", FOFS(attenuation), F_FLOAT},
{"wait", FOFS(wait), F_FLOAT},
{"delay", FOFS(delay), F_FLOAT},
{"random", FOFS(random), F_FLOAT},
// last_sound_time?
// watertype, waterlevel?
{"move_origin", FOFS(move_origin), F_VECTOR},
{"move_angles", FOFS(move_angles), F_VECTOR},
// light_level?
{"style", FOFS(style), F_INT},
// Note: "item" needs to be way down *after* spawn_temp_t::item!

{"light", 0, F_IGNORE}, // FIXME: what's this good for?!
// fields of moveinfo
{"endfunc", FOFS(moveinfo.endfunc), F_FUNCTION, FFL_NOSPAWN},
// fields of monsterinfo
{"stand", FOFS(monsterinfo.stand), F_FUNCTION, FFL_NOSPAWN},
{"idle", FOFS(monsterinfo.idle), F_FUNCTION, FFL_NOSPAWN},
{"search", FOFS(monsterinfo.search), F_FUNCTION, FFL_NOSPAWN},
{"walk", FOFS(monsterinfo.walk), F_FUNCTION, FFL_NOSPAWN},
{"run", FOFS(monsterinfo.run), F_FUNCTION, FFL_NOSPAWN},
{"dodge", FOFS(monsterinfo.dodge), F_FUNCTION, FFL_NOSPAWN},
{"attack", FOFS(monsterinfo.attack), F_FUNCTION, FFL_NOSPAWN},
{"melee", FOFS(monsterinfo.melee), F_FUNCTION, FFL_NOSPAWN},
{"sight", FOFS(monsterinfo.sight), F_FUNCTION, FFL_NOSPAWN},
{"checkattack", FOFS(monsterinfo.checkattack), F_FUNCTION, FFL_NOSPAWN},
{"currentmove", FOFS(monsterinfo.currentmove), F_MMOVE, FFL_NOSPAWN},

// zaero-specific stuff, several fields added, especially edicts
// timeout?
{"active", FOFS(active), F_INT},
// seq?
{"spawnflags2", FOFS(spawnflags2), F_INT},
// oldentnum?
{"laser", FOFS(laser), F_EDICT, FFL_NOSPAWN},
// weaponsound_time?
{"zRaduisList", FOFS(zRaduisList), F_EDICT, FFL_NOSPAWN},
{"zSchoolChain", FOFS(zSchoolChain), F_EDICT, FFL_NOSPAWN},
// zDistance?
{"rideWith0", FOFS(rideWith[0]), F_EDICT, FFL_NOSPAWN},
{"rideWith1", FOFS(rideWith[1]), F_EDICT, FFL_NOSPAWN},
{"rideWithOffset0", FOFS(rideWithOffset[0]), F_VECTOR},
{"rideWithOffset1", FOFS(rideWithOffset[1]), F_VECTOR},
{"mangle", FOFS(mangle), F_VECTOR},
// visorFrames?
{"mteam", FOFS(mteam), F_LSTRING},
{"mirrortarget", 0, F_IGNORE}, // FIXME: ??
{"mirrorlevelsave", 0, F_IGNORE}, // FIXME: ??

{"sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP},
{"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
{"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
{"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP},
{"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
{"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
{"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
{"noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP},
{"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
{"item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP},
{"item", FOFS(item), F_ITEM},
{"gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP},
{"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
{"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
{"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
{"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},

{"bad_area", FOFS(bad_area), F_EDICT}, /* FS: Coop: Rogue specific */
{"hint_chain", FOFS(hint_chain), F_EDICT}, /* FS: Coop: Rogue specific */
{"monster_hint_chain", FOFS(monster_hint_chain), F_EDICT}, /* FS: Coop: Rogue specific */
{"target_hint_chain", FOFS(target_hint_chain), F_EDICT}, /* FS: Coop: Rogue specific */
{"goal_hint", FOFS(monsterinfo.goal_hint), F_EDICT}, /* FS: Coop: Rogue specific */
{"badMedic1", FOFS(monsterinfo.badMedic1), F_EDICT}, /* FS: Coop: Rogue specific */
{"badMedic2", FOFS(monsterinfo.badMedic2), F_EDICT}, /* FS: Coop: Rogue specific */
{"last_player_enemy", FOFS(monsterinfo.last_player_enemy), F_EDICT}, /* FS: Coop: Rogue specific */
{"commander", FOFS(monsterinfo.commander), F_EDICT}, /* FS: Coop: Rogue specific */
{"blocked", FOFS(monsterinfo.blocked), F_FUNCTION, FFL_NOSPAWN}, /* FS: Coop: Rogue specific */
{"duck", FOFS(monsterinfo.duck), F_FUNCTION, FFL_NOSPAWN}, /* FS: Coop: Rogue specific */
{"unduck", FOFS(monsterinfo.unduck), F_FUNCTION, FFL_NOSPAWN}, /* FS: Coop: Rogue specific */
{"sidestep", FOFS(monsterinfo.sidestep), F_FUNCTION, FFL_NOSPAWN}, /* FS: Coop: Rogue specific */

{NULL, 0, 0, 0} 
