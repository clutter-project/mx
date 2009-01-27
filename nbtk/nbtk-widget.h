/* nbtk-widget.h: Base class for Nbtk actors
 *
 * Copyright (C) 2007 OpenedHand
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
 */

#ifndef __NBTK_WIDGET_H__
#define __NBTK_WIDGET_H__

#include <clutter/clutter.h>
#include <nbtk/nbtk-types.h>

G_BEGIN_DECLS

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

GType      nbtk_widget_get_type       (void) G_GNUC_CONST;

void       nbtk_widget_set_padding    (NbtkWidget         *actor,
                                      const NbtkPadding *padding);
void       nbtk_widget_get_padding    (NbtkWidget         *actor,
                                      NbtkPadding       *padding);

void       nbtk_widget_set_alignment  (NbtkWidget         *actor,
                                      gdouble            x_align,
                                      gdouble            y_align);
void       nbtk_widget_get_alignment  (NbtkWidget         *actor,
                                      gdouble           *x_align,
                                      gdouble           *y_align);
void       nbtk_widget_set_alignmentx (NbtkWidget         *actor,
                                      ClutterFixed       x_align,
                                      ClutterFixed       y_align);
void       nbtk_widget_get_alignmentx (NbtkWidget         *actor,
                                      ClutterFixed      *x_align,
                                      ClutterFixed      *y_align);
void         nbtk_widget_set_style_pseudo_class (NbtkWidget *actor,
                                              const gchar *pseudo_class);
guint      nbtk_widget_get_dnd_threshold (NbtkWidget *actor);
void       nbtk_widget_set_dnd_threshold (NbtkWidget *actor, guint threshold);
void       nbtk_widget_setup_child_dnd (NbtkWidget *actor, ClutterActor *child);
void       nbtk_widget_undo_child_dnd (NbtkWidget *actor, ClutterActor *child);
const gchar* nbtk_widget_get_style_pseudo_class (NbtkWidget *actor);
void         nbtk_widget_set_style_class_name (NbtkWidget  *actor,
                                               const gchar *style_class);
const gchar* nbtk_widget_get_style_class_name (NbtkWidget  *actor);
G_END_DECLS

#endif /* __NBTK_WIDGET_H__ */
