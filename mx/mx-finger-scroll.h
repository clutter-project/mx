/* mx-finger-scroll.h: Finger scrolling container actor
 *
 * Copyright (C) 2008 OpenedHand
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
 *
 * Written by: Chris Lord <chris@openedhand.com>
 */

#ifndef __MX_FINGER_SCROLL_H__
#define __MX_FINGER_SCROLL_H__

#include <glib-object.h>
#include <mx/mx-bin.h>

G_BEGIN_DECLS

#define MX_TYPE_FINGER_SCROLL            (mx_finger_scroll_get_type())
#define MX_FINGER_SCROLL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_FINGER_SCROLL, MxFingerScroll))
#define MX_IS_FINGER_SCROLL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_FINGER_SCROLL))
#define MX_FINGER_SCROLL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_FINGER_SCROLL, MxFingerScrollClass))
#define MX_IS_FINGER_SCROLL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_FINGER_SCROLL))
#define MX_FINGER_SCROLL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_FINGER_SCROLL, MxFingerScrollClass))

/**
 * MxFingerScrollMode:
 * @MX_FINGER_SCROLL_MODE_PUSH: Non-kinetic scrolling
 * @MX_FINGER_SCROLL_MODE_KINETIC: Kinetic scrolling
 *
 * Type of scrolling.
 */
typedef enum {
  MX_FINGER_SCROLL_MODE_PUSH,
  MX_FINGER_SCROLL_MODE_KINETIC
} MxFingerScrollMode;

typedef struct _MxFingerScroll          MxFingerScroll;
typedef struct _MxFingerScrollPrivate   MxFingerScrollPrivate;
typedef struct _MxFingerScrollClass     MxFingerScrollClass;

struct _MxFingerScroll
{
  /*< private >*/
  MxBin                  parent_instance;

  MxFingerScrollPrivate *priv;
};

struct _MxFingerScrollClass
{
  MxBinClass parent_class;
};

GType mx_finger_scroll_get_type (void) G_GNUC_CONST;

ClutterActor *mx_finger_scroll_new  (MxFingerScrollMode mode);

void          mx_finger_scroll_stop (MxFingerScroll *scroll);

void          mx_finger_scroll_set_use_captured (MxFingerScroll *scroll,
                                                 gboolean        use_captured);
gboolean      mx_finger_scroll_get_use_captured (MxFingerScroll *scroll);

void          mx_finger_scroll_set_mouse_button (MxFingerScroll *scroll,
                                                 guint32         button);
guint32       mx_finger_scroll_get_mouse_button (MxFingerScroll *scroll);

G_END_DECLS

#endif /* __MX_FINGER_SCROLL_H__ */
