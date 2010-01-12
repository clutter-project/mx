/*
 * mx-application: application class
 *
 * Copyright 2010 Intel Corporation.
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
 *
 * Written by: Thomas Wood <thomas.wood@intel.com>
 */

#ifndef _MX_APPLICATION_H
#define _MX_APPLICATION_H

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MX_TYPE_APPLICATION mx_application_get_type()

#define MX_APPLICATION(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_APPLICATION, MxApplication))

#define MX_APPLICATION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_APPLICATION, MxApplicationClass))

#define MX_IS_APPLICATION(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_APPLICATION))

#define MX_IS_APPLICATION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_APPLICATION))

#define MX_APPLICATION_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_APPLICATION, MxApplicationClass))

typedef struct _MxApplication MxApplication;
typedef struct _MxApplicationClass MxApplicationClass;
typedef struct _MxApplicationPrivate MxApplicationPrivate;

struct _MxApplication
{
  GObject parent;

  MxApplicationPrivate *priv;
};

struct _MxApplicationClass
{
  GObjectClass parent_class;

  /* vfuncs */
  ClutterStage* (*create_window) (MxApplication *app);
  void          (*raise)         (MxApplication *app);
};

GType mx_application_get_type (void) G_GNUC_CONST;

typedef enum
{
  MX_APPLICATION_SINGLE_INSTANCE = 1,
  MX_APPLICATION_KEEP_ALIVE      = 1 << 2,
  MX_APPLICATION_CLUTTER_GTK     = 1 << 3
} MxApplicationFlags;


MxApplication* mx_application_new (gint                 *argc,
                                   gchar              ***argv,
                                   const gchar          *name,
                                   MxApplicationFlags    flags);
void           mx_application_run  (MxApplication      *application);
void           mx_application_quit (MxApplication      *application);

ClutterStage* mx_application_create_window (MxApplication *app);

void                  mx_application_add_window    (MxApplication *application,
                                                    ClutterStage  *window);
void                  mx_application_remove_window (MxApplication *application,
                                                    ClutterStage  *window);

G_CONST_RETURN GList* mx_application_get_windows   (MxApplication *application);


G_END_DECLS

#endif /* _MX_APPLICATION_H */
