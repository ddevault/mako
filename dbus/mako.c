#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbus.h"
#include "mako.h"
#include "notification.h"
#include "wayland.h"

static const char *service_path = "/fr/emersion/Mako";
static const char *service_interface = "fr.emersion.Mako";

static int handle_close_all_notifications(sd_bus_message *msg, void *data,
		sd_bus_error *ret_error) {
	struct mako_state *state = data;

	if (wl_list_empty(&state->notifications)) {
		goto done;
	}

	struct mako_notification *notif, *tmp;
	wl_list_for_each_safe(notif, tmp, &state->notifications, link) {
		close_notification(notif, MAKO_NOTIFICATION_CLOSE_DISMISSED);
	}
	send_frame(state);

done:
	return sd_bus_reply_method_return(msg, "");
}

static int handle_close_last_notification(sd_bus_message *msg, void *data,
		sd_bus_error *ret_error) {
	struct mako_state *state = data;

	if (wl_list_empty(&state->notifications)) {
		goto done;
	}

	struct mako_notification *notif =
		wl_container_of(state->notifications.next, notif, link);
	close_notification(notif, MAKO_NOTIFICATION_CLOSE_DISMISSED);
	send_frame(state);

done:
	return sd_bus_reply_method_return(msg, "");
}

static const sd_bus_vtable service_vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_METHOD("CloseAllNotifications", "", "", handle_close_all_notifications, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("CloseLastNotification", "", "", handle_close_last_notification, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_VTABLE_END
};

int init_dbus_mako(struct mako_state *state) {
	return sd_bus_add_object_vtable(state->bus, &state->mako_slot, service_path,
		service_interface, service_vtable, state);
}
