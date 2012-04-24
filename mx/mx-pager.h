/*
 * mx-pager.h: A container that allows you to display several pages of widgets
 *
 * Copyright 2012 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Danielle Madeley <danielle.madeley@collabora.co.uk>
 */

#ifndef __MX_PAGER_H__
#define __MX_PAGER_H__

#include <mx/mx.h>

G_BEGIN_DECLS

#define MX_TYPE_PAGER	(mx_pager_get_type ())
#define MX_PAGER(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_PAGER, MxPager))
#define MX_PAGER_CLASS(obj)	(G_TYPE_CHECK_CLASS_CAST ((obj), MX_TYPE_PAGER, MxPagerClass))
#define MX_IS_PAGER(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_PAGER))
#define MX_IS_PAGER_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), MX_TYPE_PAGER))
#define MX_PAGER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_PAGER, MxPagerClass))

typedef struct _MxPager MxPager;
typedef struct _MxPagerClass MxPagerClass;
typedef struct _MxPagerPrivate MxPagerPrivate;

struct _MxPager
{
  /*< private >*/
  MxStack parent;

  MxPagerPrivate *priv;
};

struct _MxPagerClass
{
  /*< private >*/
  MxStackClass parent_class;
};

GType mx_pager_get_type (void);
ClutterActor *mx_pager_new (void);

void mx_pager_insert_page (MxPager *self, ClutterActor *child, gint position);

void mx_pager_next (MxPager *self);
void mx_pager_previous (MxPager *self);

void mx_pager_set_current_page (MxPager *self, guint page, gboolean animate);
guint mx_pager_get_current_page (MxPager *self);
void mx_pager_set_current_page_by_actor (MxPager *self, ClutterActor *actor,
    gboolean animate);
ClutterActor *mx_pager_get_current_page_actor (MxPager *self);

ClutterActor *mx_pager_get_actor_for_page (MxPager *self, guint page);
guint mx_pager_get_n_pages (MxPager *self);

G_END_DECLS

#endif
