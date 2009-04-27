/*
 * nbtk-widget.h: Base class for Nbtk actors
 *
 * Copyright 2007 OpenedHand
 * Copyright 2008, 2009 Intel Corporation.
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

#if !defined(NBTK_H_INSIDE) && !defined(NBTK_COMPILATION)
#error "Only <nbtk/nbtk.h> can be included directly.h"
#endif

#ifndef __NBTK_WIDGET_H__
#define __NBTK_WIDGET_H__

#include <clutter/clutter.h>
#include <nbtk/nbtk-types.h>

G_BEGIN_DECLS

#define NBTK_TYPE_WIDGET_CHILD                  (nbtk_widget_child_get_type ())
#define NBTK_WIDGET_CHILD(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_WIDGET_CHILD, NbtkWidgetChild))
#define NBTK_IS_WIDGET_CHILD(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_WIDGET_CHILD))
#define NBTK_WIDGET_CHILD_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_WIDGET_CHILD, NbtkWidgetChildClass))
#define NBTK_IS_WIDGET_CHILD_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_WIDGET_CHILD))
#define NBTK_WIDGET_CHILD_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_WIDGET_CHILD, NbtkWidgetChildClass))

typedef struct _NbtkWidgetChild                 NbtkWidgetChild;
typedef struct _NbtkWidgetChildClass            NbtkWidgetChildClass;

struct _NbtkWidgetChild
{
  ClutterChildMeta parent_instance;

  guint dnd_disabled : 1;
};

struct _NbtkWidgetChildClass
{
  ClutterChildMetaClass parent_class;
};

GType nbtk_widget_child_get_type (void) G_GNUC_CONST;

#define NBTK_TYPE_WIDGET                 (nbtk_widget_get_type ())
#define NBTK_WIDGET(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_WIDGET, NbtkWidget))
#define NBTK_IS_WIDGET(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_WIDGET))
#define NBTK_WIDGET_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_WIDGET, NbtkWidgetClass))
#define NBTK_IS_WIDGET_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_WIDGET))
#define NBTK_WIDGET_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_WIDGET, NbtkWidgetClass))

typedef struct _NbtkWidget               NbtkWidget;
typedef struct _NbtkWidgetPrivate        NbtkWidgetPrivate;
typedef struct _NbtkWidgetClass          NbtkWidgetClass;

/**
 * NbtkWidget:
 *
 * Base class for stylable actors. The contents of the #NbtkWidget
 * structure are private and should only be accessed through the
 * public API.
 */
struct _NbtkWidget
{
  /*< private >*/
  ClutterActor parent_instance;

  NbtkWidgetPrivate *priv;
};

/**
 * NbtkWidgetClass:
 *
 * Base class for stylable actors.
 */
struct _NbtkWidgetClass
{
  /*< private >*/
  ClutterActorClass parent_class;

  /* vfuncs */
  void (* draw_background) (NbtkWidget         *self,
                            ClutterActor       *background,
                            const ClutterColor *color);

  /* signals */
  void (* style_changed) (NbtkWidget *self);
  void (* dnd_begin)     (NbtkWidget   *self,
			  ClutterActor *dragged,
			  ClutterActor *icon,
			  gint          x,
			  gint          y);
  void (* dnd_motion)    (NbtkWidget   *self,
			  ClutterActor *dragged,
			  ClutterActor *icon,
			  gint          x,
			  gint          y);
  void (* dnd_end)       (NbtkWidget   *self,
			  ClutterActor *dragged,
			  ClutterActor *icon,
			  gint          x,
			  gint          y);
  void (* dnd_dropped)   (NbtkWidget   *self,
			  ClutterActor *dragged,
			  ClutterActor *icon,
			  gint          x,
			  gint          y);
  void (* dnd_enter)     (NbtkWidget   *self,
			  ClutterActor *dragged,
			  ClutterActor *icon,
			  gint          x,
			  gint          y);
  void (* dnd_leave)     (NbtkWidget   *self,
			  ClutterActor *dragged,
			  ClutterActor *icon,
			  gint          x,
			  gint          y);
};

GType nbtk_widget_get_type (void) G_GNUC_CONST;

void                  nbtk_widget_set_style_pseudo_class (NbtkWidget   *actor,
                                                          const gchar  *pseudo_class);
G_CONST_RETURN gchar *nbtk_widget_get_style_pseudo_class (NbtkWidget   *actor);
void                  nbtk_widget_set_style_class_name   (NbtkWidget   *actor,
                                                          const gchar  *style_class);
G_CONST_RETURN gchar *nbtk_widget_get_style_class_name   (NbtkWidget   *actor);

guint                 nbtk_widget_get_dnd_threshold      (NbtkWidget   *actor);
void                  nbtk_widget_set_dnd_threshold      (NbtkWidget   *actor,
                                                          guint         threshold);
void                  nbtk_widget_setup_child_dnd        (NbtkWidget   *actor,
                                                          ClutterActor *child);
void                  nbtk_widget_undo_child_dnd         (NbtkWidget   *actor,
                                                          ClutterActor *child);

/* Only to be used by sub-classes of NbtkWidget */
ClutterActor *nbtk_widget_get_background_image (NbtkWidget *actor);
ClutterActor *nbtk_widget_get_border_image     (NbtkWidget *actor);
void          nbtk_widget_get_padding          (NbtkWidget  *widget,
                                                NbtkPadding *padding);

void          nbtk_widget_get_border           (NbtkWidget *actor,
                                                NbtkPadding *border);
G_END_DECLS

#endif /* __NBTK_WIDGET_H__ */
