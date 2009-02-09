/* nbtk-entry.h: Plain entry actor
 *
 * Copyright (C) 2008 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Thomas Wood <thomas@linux.intel.com>
 */

#ifndef __NBTK_ENTRY_H__
#define __NBTK_ENTRY_H__

G_BEGIN_DECLS

#include <nbtk/nbtk-bin.h>

#define NBTK_TYPE_ENTRY                (nbtk_entry_get_type ())
#define NBTK_ENTRY(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_ENTRY, NbtkEntry))
#define NBTK_IS_ENTRY(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_ENTRY))
#define NBTK_ENTRY_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_ENTRY, NbtkEntryClass))
#define NBTK_IS_ENTRY_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_ENTRY))
#define NBTK_ENTRY_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_ENTRY, NbtkEntryClass))

typedef struct _NbtkEntry              NbtkEntry;
typedef struct _NbtkEntryPrivate       NbtkEntryPrivate;
typedef struct _NbtkEntryClass         NbtkEntryClass;

struct _NbtkEntry
{
  /*< private >*/
  NbtkBin parent_instance;

  NbtkEntryPrivate *priv;
};

struct _NbtkEntryClass
{
  NbtkBinClass parent_class;
};

GType nbtk_entry_get_type (void) G_GNUC_CONST;

NbtkWidget *          nbtk_entry_new      (const gchar *text);
G_CONST_RETURN gchar *nbtk_entry_get_text (NbtkEntry *entry);
void                  nbtk_entry_set_text (NbtkEntry *entry, const gchar *text);
ClutterActor*         nbtk_entry_get_clutter_text (NbtkEntry *entry);

G_END_DECLS

#endif /* __NBTK_ENTRY_H__ */
