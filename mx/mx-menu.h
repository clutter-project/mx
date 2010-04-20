/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * mx-menu.c: menu class
 *
 * Copyright (c) 2009, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#if !defined(MX_H_INSIDE) && !defined(MX_COMPILATION)
#error "Only <mx/mx.h> can be included directly.h"
#endif

#ifndef _MX_MENU_H
#define _MX_MENU_H

#include "mx-floating-widget.h"
#include "mx-action.h"

G_BEGIN_DECLS

#define MX_TYPE_MENU mx_menu_get_type()

#define MX_MENU(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_MENU, MxMenu))

#define MX_MENU_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_MENU, MxMenuClass))

#define MX_IS_MENU(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_MENU))

#define MX_IS_MENU_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_MENU))

#define MX_MENU_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_MENU, MxMenuClass))

/* The following is defined in mx-widget.h to avoid recursion */
/* typedef struct _MxMenu MxMenu; */
typedef struct _MxMenuClass MxMenuClass;
typedef struct _MxMenuPrivate MxMenuPrivate;

/**
 * MxMenu:
 *
 * The contents of this structure are private and should only be accessed
 * through the public API.
 */
struct _MxMenu
{
  /*< private >*/
  MxFloatingWidget parent;

  MxMenuPrivate *priv;
};

struct _MxMenuClass
{
  MxFloatingWidgetClass parent_class;

  void (*action_activated) (MxMenu   *menu,
                            MxAction *action);

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_menu_get_type (void);

ClutterActor *mx_menu_new           (void);

void          mx_menu_add_action         (MxMenu   *menu,
                                          MxAction *action);
void          mx_menu_remove_action      (MxMenu   *menu,
                                          MxAction *action);
void          mx_menu_remove_all         (MxMenu *menu);
void          mx_menu_show_with_position (MxMenu *menu,
                                          gfloat  x,
                                          gfloat  y);

G_END_DECLS

#endif /* _MX_MENU_H */
