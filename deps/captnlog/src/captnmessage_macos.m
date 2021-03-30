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
#include <AppKit/AppKit.h>
#include <Availability.h>

int captainslog_messagebox(const char *message)
{
	@autoreleasepool {

	NSWindow *key_window = [[NSApplication sharedApplication] keyWindow];

	NSAlert *alert = [[NSAlert alloc] init];

#ifdef __MAC_10_12
    [alert setAlertStyle:NSAlertStyleCritical];
#else
    [alert setAlertStyle:NSCriticalAlertStyle];
#endif
	[alert addButtonWithTitle:@"Abort"];
	[alert addButtonWithTitle:@"Retry"];
	[alert addButtonWithTitle:@"Ignore"];

	NSString *msg_string = [NSString stringWithUTF8String:message];
	[alert setMessageText:msg_string];
	NSInteger button = [alert runModal];
	[key_window makeKeyAndOrderFront:nil];

    switch(button) {
        case NSAlertFirstButtonReturn:
            return CAPTMSG_ABRT;
        case NSAlertThirdButtonReturn:
            return CAPTMSG_IGN;
        default:
            return CAPTMSG_RET;
    }

	} /* @autoreleasepool */
}