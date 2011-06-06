/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspsdk.h>
#include <pspthreadman_kernel.h>
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"
#include "printk.h"
#include "utils.h"
#include "strsafe.h"
#include "dirent_track.h"
#include "main.h"

static inline void lock() {}
static inline void unlock() {}

static struct IoDirentEntry *g_head = NULL, *g_tail = NULL;

static struct IoDirentEntry *new_dirent(void)
{
	struct IoDirentEntry *entry;

	entry = oe_malloc(sizeof(*entry));

	if(entry == NULL) {
		return entry;
	}

	memset(entry->path, 0, sizeof(entry->path));
	entry->dfd = entry->iso_dfd = -1;
	entry->next = NULL;

	return entry;
}

static int add_dfd(struct IoDirentEntry *slot)
{
	lock();

	if(g_head == NULL) {
		g_head = g_tail = slot;
	} else {
		g_tail->next = slot;
		g_tail = slot;
	}

	unlock();

	return 0;
}

static int remove_dfd(struct IoDirentEntry *slot)
{
	int ret;
	struct IoDirentEntry *fds, *prev;

	lock();

	for(prev = NULL, fds = g_head; fds != NULL; prev = fds, fds = fds->next) {
		if(slot == fds) {
			break;
		}
	}

	if(fds != NULL) {
		if(prev == NULL) {
			g_head = fds->next;

			if(g_head == NULL) {
				g_tail = NULL;
			}
		} else {
			prev->next = fds->next;

			if(g_tail == fds) {
				g_tail = prev;
			}
		}

		oe_free(fds);
		ret = 0;
	} else {
		ret = -1;
	}

	unlock();

	return ret;
}

int dirent_add(SceUID dfd, SceUID iso_dfd, const char *path)
{
	struct IoDirentEntry *p;
   
	p = new_dirent();

	if(p == NULL) {
		return -1;
	}

	p->dfd = dfd;
	p->iso_dfd = iso_dfd;
	STRCPY_S(p->path, path);
	add_dfd(p);

	return 0;
}

void dirent_remove(struct IoDirentEntry *p)
{
	remove_dfd(p);
}

struct IoDirentEntry *dirent_search(SceUID dfd)
{
	struct IoDirentEntry *fds;

	if (dfd < 0)
		return NULL;

	for(fds = g_head; fds != NULL; fds = fds->next) {
		if(fds->dfd == dfd)
			break;
	}

	return fds;
}
