/*
 * mx-actor-manager: Actor life-cycle manager object
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

#ifndef _MX_ACTOR_MANAGER_H
#define _MX_ACTOR_MANAGER_H

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define MX_TYPE_ACTOR_MANAGER mx_actor_manager_get_type()

#define MX_ACTOR_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MX_TYPE_ACTOR_MANAGER, MxActorManager))

#define MX_ACTOR_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_ACTOR_MANAGER, MxActorManagerClass))

#define MX_IS_ACTOR_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MX_TYPE_ACTOR_MANAGER))

#define MX_IS_ACTOR_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_ACTOR_MANAGER))

#define MX_ACTOR_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_ACTOR_MANAGER, MxActorManagerClass))

typedef struct _MxActorManager MxActorManager;
typedef struct _MxActorManagerClass MxActorManagerClass;
typedef struct _MxActorManagerPrivate MxActorManagerPrivate;

typedef ClutterActor * (*MxActorManagerCreateFunc) (MxActorManager *manager,
                                                    gpointer        userdata);

typedef enum
{
  MX_ACTOR_MANAGER_CONTAINER_DESTROYED,
  MX_ACTOR_MANAGER_ACTOR_DESTROYED,
  MX_ACTOR_MANAGER_CREATION_FAILED,
  MX_ACTOR_MANAGER_UNKNOWN_OPERATION
} MxActorManagerError;

struct _MxActorManager
{
  GObject parent;

  MxActorManagerPrivate *priv;
};

struct _MxActorManagerClass
{
  GObjectClass parent_class;

  /* signals */
  void (*actor_created) (MxActorManager *manager,
                         gulong          id,
                         ClutterActor   *actor);
  void (*actor_added) (MxActorManager   *manager,
                       gulong            id,
                       ClutterContainer *container,
                       ClutterActor     *actor);
  void (*actor_removed) (MxActorManager   *manager,
                         gulong            id,
                         ClutterContainer *container,
                         ClutterActor     *actor);

  void (*actor_finished) (MxActorManager *manager,
                          ClutterActor   *actor);

  void (*operation_completed) (MxActorManager *manager,
                               gulong          id);
  void (*operation_cancelled) (MxActorManager *manager,
                               gulong          id);
  void (*operation_failed) (MxActorManager *manager,
                            gulong          id,
                            GError         *error);

  /* padding for future expansion */
  void (*_padding_0) (void);
  void (*_padding_1) (void);
  void (*_padding_2) (void);
  void (*_padding_3) (void);
  void (*_padding_4) (void);
};

GType mx_actor_manager_get_type (void) G_GNUC_CONST;

MxActorManager *mx_actor_manager_new (ClutterStage *stage);

MxActorManager *mx_actor_manager_get_for_stage (ClutterStage *stage);

ClutterStage *mx_actor_manager_get_stage (MxActorManager *manager);

gulong mx_actor_manager_create_actor (MxActorManager           *manager,
                                      MxActorManagerCreateFunc  create_func,
                                      gpointer                  userdata,
                                      GDestroyNotify            destroy_func);

gulong mx_actor_manager_add_actor (MxActorManager   *manager,
                                   ClutterContainer *container,
                                   ClutterActor     *actor);

gulong mx_actor_manager_remove_actor (MxActorManager   *manager,
                                      ClutterContainer *container,
                                      ClutterActor     *actor);

void mx_actor_manager_remove_container (MxActorManager   *manager,
                                        ClutterContainer *container);

void mx_actor_manager_cancel_operation (MxActorManager *manager,
                                        gulong          id);

void mx_actor_manager_cancel_operations (MxActorManager *manager,
                                         ClutterActor   *actor);

void  mx_actor_manager_set_time_slice (MxActorManager *manager,
                                       guint           msecs);
guint mx_actor_manager_get_time_slice (MxActorManager *manager);

guint mx_actor_manager_get_n_operations (MxActorManager *manager);

G_END_DECLS

#endif /* _MX_ACTOR_MANAGER_H */
