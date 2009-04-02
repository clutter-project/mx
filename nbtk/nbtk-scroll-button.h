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
 * Written by: Robert Staudinger <robsta@openedhand.com>.
 *
 */

#ifndef __NBTK_SCROLL_BUTTON_H__
#define __NBTK_SCROLL_BUTTON_H__

#include <nbtk/nbtk-button.h>

G_BEGIN_DECLS

#define NBTK_TYPE_SCROLL_BUTTON              (nbtk_scroll_button_get_type ())
#define NBTK_SCROLL_BUTTON(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), NBTK_TYPE_SCROLL_BUTTON, NbtkScrollButton))
#define NBTK_IS_SCROLL_BUTTON(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NBTK_TYPE_SCROLL_BUTTON))
#define NBTK_SCROLL_BUTTON_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), NBTK_TYPE_SCROLL_BUTTON, NbtkScrollButtonClass))
#define NBTK_IS_SCROLL_BUTTON_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NBTK_TYPE_SCROLL_BUTTON))
#define NBTK_SCROLL_BUTTON_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), NBTK_TYPE_SCROLL_BUTTON, NbtkScrollButtonClass))

typedef struct _NbtkScrollButton              NbtkScrollButton;
typedef struct _NbtkScrollButtonPrivate       NbtkScrollButtonPrivate;
typedef struct _NbtkScrollButtonClass         NbtkScrollButtonClass;

struct _NbtkScrollButton
{
  /*< private >*/
  NbtkButton parent;

  NbtkScrollButtonPrivate *priv;
};

struct _NbtkScrollButtonClass
{
  NbtkButtonClass parent;
};

GType nbtk_scroll_button_get_type (void) G_GNUC_CONST;

NbtkWidget * nbtk_scroll_button_new (void);

G_END_DECLS

#endif /* __NBTK_SCROLL_BUTTON_H__ */
