/*
 * Copyright 2009 Intel Corporation.
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
 */
#ifndef _NBTK_GTK_LIGHT_SWITCH
#define _NBTK_GTK_LIGHT_SWITCH

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define NBTK_GTK_TYPE_LIGHT_SWITCH nbtk_gtk_light_switch_get_type ()

#define NBTK_GTK_LIGHT_SWITCH(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_GTK_TYPE_LIGHT_SWITCH, NbtkGtkLightSwitch))

#define NBTK_GTK_LIGHT_SWITCH_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_GTK_TYPE_LIGHT_SWITCH, NbtkGtkLightSwitchClass))

#define NBTK_GTK_IS_LIGHT_SWITCH(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_GTK_TYPE_LIGHT_SWITCH))

#define NBTK_GTK_IS_LIGHT_SWITCH_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_GTK_TYPE_LIGHT_SWITCH))

#define NBTK_GTK_LIGHT_SWITCH_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_GTK_TYPE_LIGHT_SWITCH, NbtkGtkLightSwitchClass))

typedef struct {
  GtkDrawingArea parent;
} NbtkGtkLightSwitch;

typedef struct {
  GtkDrawingAreaClass parent_class;

  void (*switch_flipped) (NbtkGtkLightSwitch *lightswitch, gboolean state);
} NbtkGtkLightSwitchClass;

GType nbtk_gtk_light_switch_get_type (void);

void nbtk_gtk_light_switch_set_active (NbtkGtkLightSwitch *lightswitch, gboolean active);
gboolean nbtk_gtk_light_switch_get_active (NbtkGtkLightSwitch *lightswitch);

GtkWidget* nbtk_gtk_light_switch_new (void);

G_END_DECLS

#endif /* _NBTK_GTK_LIGHT_SWITCH */
