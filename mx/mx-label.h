/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-label.h: Plain label actor
 *
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
 * Written by: Thomas Wood <thomas@linux.intel.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef __MX_LABEL_H__
#define __MX_LABEL_H__

G_BEGIN_DECLS

#include <mx/mx-widget.h>

#define MX_TYPE_LABEL                (mx_label_get_type ())
#define MX_LABEL(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_LABEL, MxLabel))
#define MX_IS_LABEL(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_LABEL))
#define MX_LABEL_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_LABEL, MxLabelClass))
#define MX_IS_LABEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_LABEL))
#define MX_LABEL_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_LABEL, MxLabelClass))

typedef struct _MxLabel              MxLabel;
typedef struct _MxLabelPrivate       MxLabelPrivate;
typedef struct _MxLabelClass         MxLabelClass;

/**
 * MxLabel:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
struct _MxLabel
{
  /*< private >*/
  MxWidget parent_instance;

  MxLabelPrivate *priv;
};

struct _MxLabelClass
{
  MxWidgetClass parent_class;

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_label_get_type (void) G_GNUC_CONST;

ClutterActor         *mx_label_new              (void);
ClutterActor         *mx_label_new_with_text    (const gchar *text);
const gchar          *mx_label_get_text         (MxLabel     *label);
void                  mx_label_set_text         (MxLabel     *label,
                                                 const gchar *text);
gboolean              mx_label_get_use_markup   (MxLabel *label);
void                  mx_label_set_use_markup   (MxLabel     *label,
                                                 gboolean use_markup);
ClutterActor *        mx_label_get_clutter_text (MxLabel     *label);

MxAlign               mx_label_get_x_align      (MxLabel *label);
void                  mx_label_set_x_align      (MxLabel *label,
                                                 MxAlign  align);
MxAlign               mx_label_get_y_align      (MxLabel *label);
void                  mx_label_set_y_align      (MxLabel *label,
                                                 MxAlign  align);

gboolean              mx_label_get_line_wrap    (MxLabel *label);
void                  mx_label_set_line_wrap    (MxLabel  *label,
                                                 gboolean  line_wrap);

void     mx_label_set_fade_out (MxLabel *label, gboolean fade);
gboolean mx_label_get_fade_out (MxLabel *label);

void     mx_label_set_show_tooltip (MxLabel *label, gboolean show_tooltip);
gboolean mx_label_get_show_tooltip (MxLabel *label);

G_END_DECLS

#endif /* __MX_LABEL_H__ */
