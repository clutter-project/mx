/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-expander.h: Expander Widget
 *
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
 *
 * Written by: Thomas Wood <thomas.wood@intel.com>
 *
 */


#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_EXPANDER
#define _MX_EXPANDER

#include <glib-object.h>
#include <mx/mx-bin.h>

G_BEGIN_DECLS

#define MX_TYPE_EXPANDER mx_expander_get_type()

#define MX_EXPANDER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_EXPANDER, MxExpander))

#define MX_EXPANDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_EXPANDER, MxExpanderClass))

#define MX_IS_EXPANDER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_EXPANDER))

#define MX_IS_EXPANDER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_EXPANDER))

#define MX_EXPANDER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_EXPANDER, MxExpanderClass))

typedef struct _MxExpanderPrivate MxExpanderPrivate;

/**
 * MxExpander:
 *
 * The contents of the this structure are private and should only be accessed
 * through the public API.
 */
typedef struct {
  /*< private >*/
  MxBin parent;

  MxExpanderPrivate *priv;
} MxExpander;

typedef struct {
  MxBinClass parent_class;

  /* signals */
  void (* expand_complete)   (MxExpander *expander);

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);

} MxExpanderClass;

GType mx_expander_get_type (void);

ClutterActor  *mx_expander_new (void);

void           mx_expander_set_label    (MxExpander  *expander,
                                         const gchar *label);

gboolean       mx_expander_get_expanded (MxExpander  *expander);
void           mx_expander_set_expanded (MxExpander  *expander,
                                         gboolean     expanded);

G_END_DECLS

#endif /* _MX_EXPANDER */
