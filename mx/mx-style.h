/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
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
 *
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef __MX_STYLE_H__
#define __MX_STYLE_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define MX_TYPE_STYLE                 (mx_style_get_type ())
#define MX_STYLE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_STYLE, MxStyle))
#define MX_IS_STYLE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_STYLE))
#define MX_STYLE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_STYLE, MxStyleClass))
#define MX_IS_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_STYLE))
#define MX_STYLE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_STYLE, MxStyleClass))

typedef struct _MxStyle               MxStyle;
typedef struct _MxStylePrivate        MxStylePrivate;
typedef struct _MxStyleClass          MxStyleClass;

/**
 * MxStylable:
 *
 * This is an opaque structure whose members cannot be directly accessed.
 */
/* forward declaration */
typedef struct _MxStylable            MxStylable; /* dummy typedef */
typedef struct _MxStylableIface       MxStylableIface;

typedef enum { /*< prefix=MX_STYLE_ERROR >*/
  MX_STYLE_ERROR_INVALID_FILE
} MxStyleError;

/**
 * MxStyle:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
struct _MxStyle
{
  /*< private >*/
  GObject parent_instance;

  MxStylePrivate *priv;
};

struct _MxStyleClass
{
  GObjectClass parent_class;

  void (* changed) (MxStyle *style);

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_style_get_type (void) G_GNUC_CONST;

MxStyle *mx_style_get_default (void);
MxStyle *mx_style_new         (void);

gboolean mx_style_load_from_file (MxStyle      *style,
                                  const gchar  *filename,
                                  GError      **error);
void     mx_style_get_property   (MxStyle      *style,
                                  MxStylable   *stylable,
                                  GParamSpec   *pspec,
                                  GValue       *value);
void     mx_style_get            (MxStyle      *style,
                                  MxStylable   *stylable,
                                  const gchar  *first_property_name,
                                  ...) G_GNUC_NULL_TERMINATED;
void     mx_style_get_valist     (MxStyle      *style,
                                  MxStylable   *stylable,
                                  const gchar  *first_property_name,
                                  va_list       va_args);

G_END_DECLS

#endif /* __MX_STYLE_H__ */
