/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Cross platform message box invokation function.
 *
 * @copyright CaptainsLog is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "captnmessage.h"
#include <assert.h>
#include <string.h>
#include <gtk/gtk.h>

int captainslog_messagebox(const char *message)
{
    assert(gtk_init_check(NULL, NULL));

    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, "%s", message);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "_Abort", GTK_RESPONSE_OK);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "_Retry", GTK_RESPONSE_CANCEL);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "_Ignore", GTK_RESPONSE_HELP);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    while (gtk_events_pending()) {
        gtk_main_iteration();
    }

    switch (result) {
        case GTK_RESPONSE_OK:
            return CAPTMSG_RET;
        case GTK_RESPONSE_HELP:
            return CAPTMSG_IGN;
        default:
            return CAPTMSG_ABRT;
    }
}
