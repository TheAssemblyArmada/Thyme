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
#ifndef LOG_CAPTNMESSAGE_H
#define LOG_CAPTNMESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    CAPTMSG_IGN,
    CAPTMSG_RET,
    CAPTMSG_ABRT,
} CaptainMessageResult;

/** 
 * Opens a message box.
 */
int captainslog_messagebox(const char *message);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* LOG_CAPTNMESSAGE_H */
