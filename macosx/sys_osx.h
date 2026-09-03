/* Mac OS X specifics needed by common sys_unix.c : */

#ifndef SYS_OSX_H
#define SYS_OSX_H

#ifndef DEDICATED_ONLY
void Cocoa_ErrorMessage (const char *errorMsg);
#define Sys_ErrorMessage	Cocoa_ErrorMessage
#endif

#endif /* SYS_OSX_H */

