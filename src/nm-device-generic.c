/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* NetworkManager -- Network link manager
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright 2013 Red Hat, Inc.
 */

#include "config.h"

#include "nm-device-generic.h"
#include "nm-device-private.h"
#include "nm-enum-types.h"
#include "nm-properties-changed-signal.h"
#include "nm-utils.h"

#include "nm-device-generic-glue.h"

G_DEFINE_TYPE (NMDeviceGeneric, nm_device_generic, NM_TYPE_DEVICE)

#define NM_DEVICE_GENERIC_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), NM_TYPE_DEVICE_GENERIC, NMDeviceGenericPrivate))

typedef struct {
	int dummy;
} NMDeviceGenericPrivate;

enum {
	PROPERTIES_CHANGED,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

#define NM_DEVICE_GENERIC_ERROR (nm_device_generic_error_quark ())

static GQuark
nm_device_generic_error_quark (void)
{
	static GQuark quark = 0;
	if (!quark)
		quark = g_quark_from_static_string ("nm-device-generic-error");
	return quark;
}

/**************************************************************/

static guint32
get_generic_capabilities (NMDevice *dev)
{
	return NM_DEVICE_CAP_NM_SUPPORTED;
}

static gboolean
is_available (NMDevice *device)
{
	return TRUE;
}

static gboolean
check_connection_compatible (NMDevice *device,
                             NMConnection *connection,
                             GError **error)
{
	NMSettingConnection *s_con;

	if (!NM_DEVICE_CLASS (nm_device_generic_parent_class)->check_connection_compatible (device, connection, error))
		return FALSE;

	if (!nm_connection_is_type (connection, NM_SETTING_GENERIC_SETTING_NAME)) {
		g_set_error (error,
		             NM_DEVICE_GENERIC_ERROR, NM_DEVICE_GENERIC_ERROR_CONNECTION_NOT_GENERIC,
		             "The connection was not a generic connection.");
		return FALSE;
	}

	s_con = nm_connection_get_setting_connection (connection);
	if (!nm_setting_connection_get_interface_name (s_con)) {
		g_set_error (error,
		             NM_DEVICE_GENERIC_ERROR, NM_DEVICE_GENERIC_ERROR_CONNECTION_INVALID,
		             "The connection did not specify an interface name.");
		return FALSE;
	}

	return TRUE;
}

/**************************************************************/

NMDevice *
nm_device_generic_new (const char *udi,
                       const char *iface,
                       const char *driver)
{
	g_return_val_if_fail (udi != NULL, NULL);

	return (NMDevice *) g_object_new (NM_TYPE_DEVICE_GENERIC,
	                                  NM_DEVICE_UDI, udi,
	                                  NM_DEVICE_IFACE, iface,
	                                  NM_DEVICE_DRIVER, driver,
	                                  NM_DEVICE_TYPE_DESC, "Generic",
	                                  NM_DEVICE_DEVICE_TYPE, NM_DEVICE_TYPE_GENERIC,
	                                  NULL);
}

static void
nm_device_generic_init (NMDeviceGeneric *self)
{
	nm_device_set_default_unmanaged (NM_DEVICE (self), TRUE);
}

static void
nm_device_generic_class_init (NMDeviceGenericClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	NMDeviceClass *parent_class = NM_DEVICE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (NMDeviceGenericPrivate));

	parent_class->get_generic_capabilities = get_generic_capabilities;
	parent_class->is_available = is_available;
	parent_class->check_connection_compatible = check_connection_compatible;

	/* signals */
	signals[PROPERTIES_CHANGED] =
		nm_properties_changed_signal_new (object_class,
		                                  G_STRUCT_OFFSET (NMDeviceGenericClass, properties_changed));

	dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass),
	                                 &dbus_glib_nm_device_generic_object_info);

	dbus_g_error_domain_register (NM_DEVICE_GENERIC_ERROR, NULL, NM_TYPE_DEVICE_GENERIC_ERROR);
}