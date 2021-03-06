/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright (C) 2011 The Linux Box Corporation
 * Author: Adam C. Emerson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * ---------------------------------------
 */
/**
 * @addtogroup FSAL
 * @{
 */

#include "nlm_list.h"
#include "fsal.h"
#include "fsal_api.h"
#include "nfs_exports.h"
#include "nfs_core.h"

/**
 * @file fsal_destroyer.c
 * @author Adam C. Emerson <aemerson@linuxbox.com>
 * @brief Kill the FSAL with prejudice
 */

/**
 * @brief Dispose of lingering file handles
 *
 * @param[in] export Export to clean up
 */

extern struct glist_head fsal_list;

static void shutdown_handles(struct fsal_export *export)
{
	/* Handle iterator */
	struct glist_head *hi = NULL;
	/* Next pointer in handle iteration */
	struct glist_head *hn = NULL;
	/* FSAL return code */
	fsal_status_t fsal_status;

	if (glist_empty(&export->handles)) {
		return;
	}

	LogDebug(COMPONENT_FSAL,
		 "Extra file handles hanging around.");
	glist_for_each_safe(hi, hn, &export->handles) {
		struct fsal_obj_handle *h
			= glist_entry(hi,
				      struct fsal_obj_handle,
				      handles);
		if (h->refs != 0) {
			LogDebug(COMPONENT_FSAL,
				 "Extra references hanging around.");
			h->refs = 0;
		}
		fsal_status = h->ops->release(h);
		if (FSAL_IS_ERROR(fsal_status)) {
			LogMajor(COMPONENT_FSAL,
				 "Error releasing handle: %d",
				fsal_status.major);
		}
	}
}

/**
 * @brief Dispose of lingering DS handles
 *
 * @paramp[in] export Export to clean up
 */
static void shutdown_ds_handles(struct fsal_export *export)
{
	/* Handle iterator */
	struct glist_head *hi = NULL;
	/* Next pointer in handle iteration */
	struct glist_head *hn = NULL;
	/* FSAL return code */
	nfsstat4 status = 0;
	if (glist_empty(&export->ds_handles)) {
		return;
	}

	LogDebug(COMPONENT_FSAL,
		 "Extra DS file handles hanging around.");
	glist_for_each_safe(hi, hn, &export->ds_handles) {
		struct fsal_ds_handle *h
			= glist_entry(hi,
				      struct fsal_ds_handle,
				      ds_handles);
		h->ops->release(h);
		if (h->refs != 0) {
			LogDebug(COMPONENT_FSAL,
				 "Extra references hanging around.");
			h->refs = 0;
		}
		status = h->ops->release(h);
		if (status != 0) {
			LogMajor(COMPONENT_FSAL,
				 "Error releasing handle: %d",
				 status);
		}
	}
}

/**
 * @brief Shut down an individual export
 *
 * @param[in] export The export to shut down
 */

static void shutdown_export(struct fsal_export *export)
{
	fsal_status_t fsal_status;

	shutdown_handles(export);
	shutdown_ds_handles(export);

	if (export->refs != 0) {
		LogDebug(COMPONENT_FSAL,
			 "Extra references hanging around to export.");
		export->refs = 0;
	}

	fsal_status = export->ops->release(export);
	if (FSAL_IS_ERROR(fsal_status)) {
		LogMajor(COMPONENT_FSAL,
			 "Cannot release FSAL export object!");
	}
}

/**
 * @brief Destroy FSALs
 */

void destroy_fsals(void)
{
	/* Module iterator */
	struct glist_head *mi = NULL;
	/* Next module */
	struct glist_head *mn = NULL;

	glist_for_each_safe(mi, mn, &fsal_list) {
		/* The module to destroy */
		struct fsal_module *m
			= glist_entry(mi,
				      struct fsal_module,
				      fsals);
		/* Iterator over exports */
		struct glist_head *ei = NULL;
		/* Next export */
		struct glist_head *en = NULL;

		LogEvent(COMPONENT_FSAL,
			 "Shutting down exports for FSAL %s",
			 m->name);
		glist_for_each_safe(ei, en, &m->exports) {
			/* The module to destroy */
			struct fsal_export *e
				= glist_entry(ei,
					      struct fsal_export,
					      exports);
			shutdown_export(e);
		}
		LogEvent(COMPONENT_FSAL,
			 "Exports for FSAL %s shut down",
			 m->name);
		if (m->refs != 0) {
			LogDebug(COMPONENT_FSAL,
				 "Extra references hanging around to "
				 "FSAL %s", m->name);
			/**
			 * @todo Forcibly blowing away all references
			 * should work fine on files and objects if
			 * we're shutting down, however it will cause
			 * trouble once we have stackable FSALs.  As a
			 * practical matter, though, if the system is
			 * working properly we shouldn't even reach
			 * this point.
			 */
			m->refs = 0;
		}
		if (m->dl_handle) {
			int rc = 0;
			LogEvent(COMPONENT_FSAL,
				 "Unloading FSAL %s",
				 m->name);
			rc = m->ops->unload(m);
			if (rc != 0) {
				LogMajor(COMPONENT_FSAL,
					 "Unload of %s failed with error %d",
					 m->name, rc);
			}
			LogEvent(COMPONENT_FSAL,
				 "FSAL %s unloaded",
				 m->name);
		}
	}
}

/** @} */
