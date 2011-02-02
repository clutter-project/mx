/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-fade-effect.h: A border-fading offscreen effect
 *
 * Copyright 2011 Intel Corporation
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

#ifndef _MX_FADE_EFFECT_H
#define _MX_FADE_EFFECT_H

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MX_TYPE_FADE_EFFECT mx_fade_effect_get_type()

#define MX_FADE_EFFECT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_FADE_EFFECT, MxFadeEffect))

#define MX_FADE_EFFECT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_FADE_EFFECT, MxFadeEffectClass))

#define MX_IS_FADE_EFFECT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_FADE_EFFECT))

#define MX_IS_FADE_EFFECT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_FADE_EFFECT))

#define MX_FADE_EFFECT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_FADE_EFFECT, MxFadeEffectClass))

typedef struct _MxFadeEffect MxFadeEffect;
typedef struct _MxFadeEffectClass MxFadeEffectClass;
typedef struct _MxFadeEffectPrivate MxFadeEffectPrivate;

struct _MxFadeEffect
{
  ClutterOffscreenEffect parent;

  MxFadeEffectPrivate *priv;
};

struct _MxFadeEffectClass
{
  ClutterOffscreenEffectClass parent_class;
};

GType mx_fade_effect_get_type (void) G_GNUC_CONST;

ClutterEffect *mx_fade_effect_new (void);

void mx_fade_effect_set_border (MxFadeEffect *effect,
                                guint         top,
                                guint         right,
                                guint         bottom,
                                guint         left);
void mx_fade_effect_get_border (MxFadeEffect *effect,
                                guint        *top,
                                guint        *right,
                                guint        *bottom,
                                guint        *left);

void mx_fade_effect_set_bounds (MxFadeEffect *effect,
                                gint          x,
                                gint          y,
                                guint         width,
                                guint         height);
void mx_fade_effect_get_bounds (MxFadeEffect *effect,
                                gint         *x,
                                gint         *y,
                                guint        *width,
                                guint        *height);

void mx_fade_effect_set_color (MxFadeEffect       *effect,
                               const ClutterColor *color);
void mx_fade_effect_get_color (MxFadeEffect       *effect,
                               ClutterColor       *color);

G_END_DECLS

#endif /* _MX_FADE_EFFECT_H */
