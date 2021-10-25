/**
 * @file
 *
 * @author xezon
 * 
 * @brief various Useful Standard Template Library functions
 *
 * @copyright Baseconfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_CLASSES_H
#define BASE_CLASSES_H

#ifdef __cplusplus

class NotCopyable
{
public:
    NotCopyable() = default;

private:
    NotCopyable(const NotCopyable &) = delete;
    NotCopyable &operator=(const NotCopyable &) = delete;
};

#endif // __cplusplus

#endif // BASE_CLASSES_H
