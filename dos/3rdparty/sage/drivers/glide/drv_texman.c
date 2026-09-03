/**
 * \file drv_texman.c
 * Texture memory management.
 */


#include <stdlib.h>

#include <glide.h>
#include <g3ext.h>
#include "GL/gl.h"

#include "glinternal.h"
#include "main/context.h"
#include "util/list.h"
#include "tnl/tnl.h"
#include "driver.h"
#include "drv.h"


static MEM_RANGE *tmfree[2];	/**< Free memory on each TMU */
static MEM_RANGE *rpool;	/**< Memory range pool */


/**
 * Allocate new memory range.
 *
 * \param start start address
 * \param end end address
 *
 * \return memory range
 */
static MEM_RANGE *
tm_new_range (GLuint start, GLuint end)
{
    MEM_RANGE *range;
    if (rpool != NULL) {
	range = rpool;
	rpool = rpool->next;
    } else {
	range = malloc(sizeof(MEM_RANGE));
    }
    range->start = start;
    range->end = end;
    return range;
}


/**
 * Delete range by adding it to the unused pool.
 *
 * \param range memory range
 */
static void
tm_delete_range (MEM_RANGE *range)
{
    range->next = rpool;
    rpool = range;
}


/**
 * Delete range and coallesce adjacent free blocks.
 *
 * \param tmu corresponding TMU
 * \param range memory range
 */
static void
tmu_remove_range (GLuint tmu, MEM_RANGE *range)
{
    MEM_RANGE *prev, *tmp;

    if (allow_texuma) {
	tmu = GR_TMU0;
    }

    if (range->start == range->end) {
	tm_delete_range(range);
	return;
    }

    prev = NULL;
    tmp = tmfree[tmu];
    while (tmp != NULL) {
	if (range->start > tmp->start) {
	    prev = tmp;
	    tmp = tmp->next;
	} else {
	    break;
	}
    }

    range->next = tmp;
    if (tmp != NULL) {
	if (range->end == tmp->start &&
	    tmp->start & tex_bound_mask) {
	    tmp->start = range->start;
	    tm_delete_range(range);
	    range = tmp;
	}
    }
    if (prev != NULL) {
	if (prev->end == range->start &&
	    range->start & tex_bound_mask) {
	    prev->end = range->end;
	    prev->next = range->next;
	    tm_delete_range(range);
	} else {
	    prev->next = range;
	}
    } else {
	tmfree[tmu] = range;
    }
}


/**
 * Find oldest block, using a pseudo-FIFO cache.
 *
 * \param tmu location to search
 *
 * \return oldest block
 */
static TEX_OBJ *
tmu_find_oldest (GLuint tmu)
{
    TEX_OBJ *obj;
    list_forback (obj, ctx_tex_obj_list) {
	TFX_OBJ *ptr = obj->driverData;
	if (ptr && ptr->in &&
	    (ptr->tmu == tmu ||
	     ptr->tmu == GR_TMU_BOTH ||
	     ptr->tmu == GR_TMU_SPLIT ||
	     allow_texuma)) {
	    break;
	}
    }
    return obj;
}


/**
 * Move object out of texture memory.
 *
 * \param obj texture object
 */
void
tmu_moveout (TEX_OBJ *obj)
{
    TFX_OBJ *ptr = obj->driverData;

    if (!ptr->in) {
	return;
    }

    switch (ptr->tmu) {
	case GR_TMU0:
	case GR_TMU1:
	    tmu_remove_range(ptr->tmu, ptr->range[ptr->tmu]);
	    break;
	case GR_TMU_SPLIT:
	case GR_TMU_BOTH:
	    tmu_remove_range(GR_TMU0, ptr->range[GR_TMU0]);
	    tmu_remove_range(GR_TMU1, ptr->range[GR_TMU1]);
	    break;
    }

    ptr->in = GL_FALSE;
    ptr->tmu = GR_TMU_NONE;
}


/**
 * Find free block.
 *
 * \param tmu location to search
 * \param size needed size
 *
 * \return block address
 */
static GLuint
tmu_find_start_address (GLuint tmu, GLuint size)
{
    GLuint result;
    MEM_RANGE *prev, *tmp;
    TEX_OBJ *obj;

    if (allow_texuma) {
	tmu = GR_TMU0;
    }

    while (GL_TRUE) {
	prev = NULL;
	tmp = tmfree[tmu];
	while (tmp != NULL) {
	    if (tmp->end - tmp->start >= size) {	/* fits */
		result = tmp->start;
		tmp->start += size;
		if (tmp->start == tmp->end) {		/* perfect fit */
		    if (prev != NULL) {
			prev->next = tmp->next;
		    } else {
			tmfree[tmu] = tmp->next;
		    }
		    tm_delete_range(tmp);
		}
		return result;
	    }
	    prev = tmp;
	    tmp = tmp->next;
	}
	/* no free space, discard oldest */
	obj = tmu_find_oldest(tmu);
	tmu_moveout(obj);
    }
}


/**
 * Allocate block.
 *
 * \param tmu location to search
 * \param size needed size
 *
 * \return memory range
 */
static MEM_RANGE *
tmu_add_object (GLuint tmu, GLuint size)
{
    MEM_RANGE *range;
    GLuint start;

    start = tmu_find_start_address(tmu, size);
    range = tm_new_range(start, start + size);

    return range;
}


/**
 * Move object into texture memory.
 *
 * \param obj texture object
 * \param where location
 */
void
tmu_movein (TEX_OBJ *obj, GLuint where)
{
    GLint i, l;
    GLuint loc;
    GLuint texmemsize;
    TFX_OBJ *ptr = obj->driverData;
    GrTexInfo *info = (GrTexInfo *)ptr;

    loc = where;
    if (ptr->in) {
	if (ptr->tmu == where) {
	    return;
	}
	if (ptr->tmu == GR_TMU_SPLIT || where == GR_TMU_SPLIT) {
	    tmu_moveout(obj);
	} else {
	    if (ptr->tmu == GR_TMU_BOTH) {
		return;
	    }
	    loc = GR_TMU_BOTH;
	    where = GR_TMU1 - ptr->tmu;
	}
    }

    switch (where) {
	case GR_TMU0:
	case GR_TMU1:
	    texmemsize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, info);
	    ptr->range[where] = tmu_add_object(where, texmemsize);

	    for (i = info->largeLodLog2, l = ptr->minLevel; /* XXX XX X */
		 i >= info->smallLodLog2; i--, l++) {
		grTexDownloadMipMapLevel(where,
					 ptr->range[where]->start,
					 i,
					 info->largeLodLog2,
					 info->aspectRatioLog2,
					 info->format,
					 GR_MIPMAPLEVELMASK_BOTH,
					 obj->image[0][l].data);
	    }
	    break;
	case GR_TMU_SPLIT:
	case GR_TMU_BOTH:
	    break;
    }

    ptr->in = GL_TRUE;
    ptr->tmu = loc;
}


/**
 * Initialize one TMU.
 *
 * \param tmu texture unit
 */
static int
tmu_init (GLuint tmu)
{
    MEM_RANGE *range, *last;
    GLuint start, end, blockstart, blockend, chunk;

    start = grTexMinAddress(tmu);
    end = grTexMaxAddress(tmu);
    if (end == 0) {
	return -1; /* FS: This is bad, so let's early out. */
    }

    chunk = tex_bound_mask + 1;
    if (chunk == 0) {
	chunk = end - start;
    }

    last = NULL;
    blockstart = start;
    while (blockstart < end) {
	if (blockstart + chunk > end) {
	    blockend = end;
	} else {
	    blockend = blockstart + chunk;
	}

	range = tm_new_range(blockstart, blockend);
	range->next = NULL;

	if (last != NULL) {
	    last->next = range;
	} else {
	    tmfree[tmu] = range;
	}
	last = range;

	blockstart += chunk;
    }

    return 0;
}


/**
 * Find best TMU to download object.
 *
 * \param tmu default location
 * \param texObj texture object
 *
 * \return best location
 */
GLuint
tm_best_tmu (GLuint tmu, TEX_OBJ *texObj)
{
    GLuint size;
    MEM_RANGE *tmp;
    TFX_OBJ *fxObj;
    GrTexInfo *info;

    if (allow_texuma || !allow_multitex) {
	return tmu;
    }

    fxObj = texObj->driverData;
    info = (GrTexInfo *)fxObj;
    size = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, info);

    tmp = tmfree[tmu];
    while (tmp != NULL) {
	if (tmp->end - tmp->start >= size) {
	    return tmu;
	}
	tmp = tmp->next;
    }
    tmu ^= GR_TMU1;

    tmp = tmfree[tmu];
    while (tmp != NULL) {
	if (tmp->end - tmp->start >= size) {
	    return tmu;
	}
	tmp = tmp->next;
    }
    tmu ^= GR_TMU1;

    return tmu;
}


/**
 * Initialize texture manager.
 *
 * \return 0 if success
 *
 * \todo the return value is not accurate
 */
int
tm_init (void)
{
    rpool = NULL;
    tmfree[0] = NULL;
    tmfree[1] = NULL;

    if (allow_texuma) {
	grEnable(GR_TEXTURE_UMA_EXT);
    }

    if (tmu_init(GR_TMU0) < 0) {
	return -1;
    }
    if (!allow_texuma && allow_multitex) {
	if (tmu_init(GR_TMU1) < 0) {
	    return -1;
	}
    }

    return 0;
}


/**
 * Cleanup texture manager.
 */
void
tm_fini (void)
{
    MEM_RANGE *range, *tmp;

    for (range = rpool; range != NULL; ) {
	tmp = range;
	range = range->next;
	free(tmp);
    }

    for (range = tmfree[0]; range != NULL; ) {
	tmp = range;
	range = range->next;
	free(tmp);
    }

    for (range = tmfree[1]; range != NULL; ) {
	tmp = range;
	range = range->next;
	free(tmp);
    }
}
