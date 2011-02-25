/*
 * mx-spinner.h: a loading indicator widget
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
 * Written by: Chris Lord <chris@linux.intel.com>
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_SPINNER_H
#define _MX_SPINNER_H

#include <glib-object.h>
#include <mx/mx-widget.h>

G_BEGIN_DECLS

#define MX_TYPE_SPINNER mx_spinner_get_type()

#define MX_SPINNER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_SPINNER, MxSpinner))

#define MX_SPINNER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_SPINNER, MxSpinnerClass))

#define MX_IS_SPINNER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_SPINNER))

#define MX_IS_SPINNER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_SPINNER))

#define MX_SPINNER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_SPINNER, MxSpinnerClass))

typedef struct _MxSpinner MxSpinner;
typedef struct _MxSpinnerClass MxSpinnerClass;
typedef struct _MxSpinnerPrivate MxSpinnerPrivate;

/**
 * MxSpinner:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxSpinner
{
  /*< private >*/
  MxWidget parent;

  MxSpinnerPrivate *priv;
};

struct _MxSpinnerClass
{
  MxWidgetClass parent_class;

  /* signals */
  void (* looped) (MxSpinner *spinner);

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
};

GType mx_spinner_get_type (void) G_GNUC_CONST;

ClutterActor *mx_spinner_new (void);

void mx_spinner_set_animating (MxSpinner *spinner, gboolean animating);
gboolean mx_spinner_get_animating (MxSpinner *spinner);

G_END_DECLS

#endif /* _MX_SPINNER_H */
