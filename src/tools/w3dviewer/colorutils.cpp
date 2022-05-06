/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View color utils
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "colorutils.h"

void DrawSunkenRect(BYTE *bits, RECT *rect, int stride)
{
    int width = rect->right - rect->left;
    int height = rect->bottom - rect->top;
    BYTE color = (BYTE)GetSysColor(COLOR_3DSHADOW);

    if (rect->top < height) {
        BYTE *b = &bits[3 * rect->left + rect->top * stride];
        for (int i = 0; i < height - rect->top; i++) {
            b[0] = color;
            b[1] = color;
            b[2] = color;
            b += stride;
        }
    }

    color = (BYTE)GetSysColor(COLOR_3DHIGHLIGHT);

    if (rect->top < height) {
        BYTE *b = &bits[3 * rect->right - 3 + rect->top * stride];
        for (int i = 0; i < height - rect->top; i++) {
            b[0] = color;
            b[1] = color;
            b[2] = color;
            b += stride;
        }
    }

    color = (BYTE)GetSysColor(COLOR_3DSHADOW);

    if (rect->left < width) {
        BYTE *b = &bits[3 * rect->left + rect->top * stride];
        for (int i = width - rect->left; i; i--) {
            b[0] = color;
            b[1] = color;
            b[2] = color;
            b += 3;
        }
    }

    color = (BYTE)GetSysColor(COLOR_3DHIGHLIGHT);

    if (rect->left < width) {
        BYTE *b = &bits[3 * rect->left + (rect->bottom - 1) * stride];
        for (int i = width - rect->left; i; i--) {
            b[0] = color;
            b[1] = color;
            b[2] = color;
            b += 3;
        }
    }
}

void DrawRaisedRect(BYTE *bits, RECT *rect, int stride)
{
    int width = rect->right - rect->left;
    int height = rect->bottom - rect->top;
    BYTE color = (BYTE)GetSysColor(COLOR_3DHIGHLIGHT);

    if (rect->top < height) {
        BYTE *b = &bits[3 * rect->left + rect->top * stride];
        for (int i = 0; i < height - rect->top; i++) {
            b[0] = color;
            b[1] = color;
            b[2] = color;
            b += stride;
        }
    }

    color = (BYTE)GetSysColor(COLOR_3DSHADOW);

    if (rect->top < height) {
        BYTE *b = &bits[3 * rect->right - 3 + rect->top * stride];
        for (int i = 0; i < height - rect->top; i++) {
            b[0] = color;
            b[1] = color;
            b[2] = color;
            b += stride;
        }
    }

    color = (BYTE)GetSysColor(COLOR_3DHIGHLIGHT);

    if (rect->left < width) {
        BYTE *b = &bits[3 * rect->left + rect->top * stride];
        for (int i = width - rect->left; i; i--) {
            b[0] = color;
            b[1] = color;
            b[2] = color;
            b += 3;
        }
    }

    color = (BYTE)GetSysColor(COLOR_3DSHADOW);

    if (rect->left < width) {
        BYTE *b = &bits[3 * rect->left + (rect->bottom - 1) * stride];
        for (int i = width - rect->left; i; i--) {
            b[0] = color;
            b[1] = color;
            b[2] = color;
            b += 3;
        }
    }
}

void DrawOutlinedRect(BYTE *bits, RECT *rect, int color, int stride)
{
    BYTE *b = &bits[3 * rect->left + rect->top * stride];

    for (int i = rect->left; i < rect->right; i++) {
        b[0] = (BYTE)color;
        b[1] = (BYTE)color;
        b[2] = (BYTE)color;
        b += 3;
    }

    b = &bits[3 * rect->left + (rect->bottom - 1) * stride];

    for (int i = rect->left; i < rect->right; i++) {
        b[0] = (BYTE)color;
        b[1] = (BYTE)color;
        b[2] = (BYTE)color;
        b += 3;
    }

    b = &bits[3 * rect->left + rect->top * stride];

    for (int i = rect->top; i < rect->bottom; i++) {
        b[0] = (BYTE)color;
        b[1] = (BYTE)color;
        b[2] = (BYTE)color;
        b += stride;
    }

    b = &bits[3 * rect->right - 3 + rect->top * stride];

    for (int i = rect->top; i < rect->bottom; i++) {
        b[0] = (BYTE)color;
        b[1] = (BYTE)color;
        b[2] = (BYTE)color;
        b += stride;
    }
}
