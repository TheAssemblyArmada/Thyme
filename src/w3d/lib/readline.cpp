/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Reads a line of text from a file or straw.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "readline.h"
#include "filestraw.h"
#include <cstring>

using std::strlen;

int Read_Line(FileClass &file, char *line, int length, bool &eof)
{
    FileStraw fstraw(&file);
    return Read_Line(fstraw, line, length, eof);
}

int Read_Line(Straw &straw, char *buffer, int length, bool &eof)
{
    if (buffer && length > 0) {
        int i = 0;
        char c;

        while (true) {
            if (straw.Get(&c, sizeof(c)) != sizeof(c)) {
                eof = true;
                break;
            }

            if (c == '\n') {
                break;
            }

            if (c != '\r' && i + 1 < length) {
                buffer[i++] = c;
            }
        }

        // insert null char at the end of the line and trim it of whitespace.
        buffer[i] = '\0';
        strtrim(buffer);

        return strlen(buffer);
    }

    return 0;
}
