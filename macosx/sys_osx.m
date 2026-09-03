#include "sys_osx.h"

#import <Cocoa/Cocoa.h>	/* NSRunCriticalAlertPanel() */

#ifndef DEDICATED_ONLY
#ifndef MAC_OS_X_VERSION_10_12
#define NSAlertStyleCritical NSCriticalAlertStyle
#endif
/* Display message from Sys_Error() on a window: */
void Cocoa_ErrorMessage (const char *errorMsg)
{
#if (MAC_OS_X_VERSION_MIN_REQUIRED < 1040)	/* ppc builds targeting 10.3 and older */
    NSString* msg = [NSString stringWithCString:errorMsg];
#else
    NSString* msg = [NSString stringWithCString:errorMsg encoding:NSASCIIStringEncoding];
#endif
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1030
    NSRunCriticalAlertPanel (@"Quake II Error", msg, @"OK", nil, nil);
#else
    NSAlert *alert = [[[NSAlert alloc] init] autorelease];
    alert.alertStyle = NSAlertStyleCritical;
    alert.messageText = @"Quake II Error";
    alert.informativeText = msg;
    [alert runModal];
#endif
}
#endif
