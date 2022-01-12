/**
 * @file
 *
 * @author xezon
 *
 * @brief File Reference class to use with File class. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "file.h"
#include "utility/shared_ptr.h"

class File;

namespace Thyme
{

// #FEATURE Reference counted file class that automatically closes the assigned file when all references are destroyed.
template<typename Counter> class FileRefT
{
private:
    struct FileWrapper
    {
        File *file = nullptr;
    };

    struct FileWrapperDeleter
    {
        void operator()(FileWrapper *instance) const
        {
            if (instance->file != nullptr) {
                instance->file->Delete_On_Close(true);
                instance->file->Close();
            }
            delete instance;
        }
    };

public:
    FileRefT() : m_wrapper(nullptr) {}

    ~FileRefT() {}

    FileRefT(File *file) { Assign(file); }

    FileRefT(const FileRefT &other) { *this = other; }

    FileRefT &operator=(File *file)
    {
        Assign(file);
        return *this;
    }

    FileRefT &operator=(const FileRefT &other)
    {
        m_wrapper = other.m_wrapper;
        return *this;
    }

    File *Get_File() { return m_wrapper ? m_wrapper->file : nullptr; }

    const File *Get_File() const { return m_wrapper ? m_wrapper->file : nullptr; }

    // Returns whether or not file is open. Unopened file must not be used.
    bool Is_Open() const { return m_wrapper && m_wrapper->file && m_wrapper->file->m_access; }

    void Close()
    {
        m_wrapper->file->Delete_On_Close(true);
        m_wrapper->file->Close();
        m_wrapper->file = nullptr;
        m_wrapper.reset();
    }

    int Read(void *dst, int bytes) { return m_wrapper->file->Read(dst, bytes); }

    int Write(void const *src, int bytes) { return m_wrapper->file->Write(src, bytes); }

    int Seek(int offset, File::SeekMode mode) { return m_wrapper->file->Seek(offset, mode); }

    void Next_Line(char *dst, int bytes) { m_wrapper->file->Next_Line(dst, bytes); }

    bool Scan_Int(int &integer) { return m_wrapper->file->Scan_Int(integer); }

    bool Scan_Real(float &real) { return m_wrapper->file->Scan_Real(real); }

    bool Scan_String(Utf8String &string) { return m_wrapper->file->Scan_String(string); }

    template<typename... Args> void Print(const char *format, Args... args) { m_wrapper->file->Print(format, args...); }

    int Size() { return m_wrapper->file->Size(); }

    int Position() { return m_wrapper->file->Position(); }

    void *Read_All_And_Close()
    {
        m_wrapper->file->Delete_On_Close(true);
        void *data = m_wrapper->file->Read_All_And_Close();
        m_wrapper->file = nullptr;
        m_wrapper.reset();
        return data;
    }

    bool To_RAM_File()
    {
        bool converted = false;

        m_wrapper->file->Delete_On_Close(true);
        File *old_file = m_wrapper->file;
        File *new_file = m_wrapper->file->Convert_To_RAM_File();
        converted = (old_file != new_file);
        m_wrapper->file = new_file;
        m_wrapper->file->Delete_On_Close(false);

        return converted;
    }

    const Utf8String &Get_File_Name() const { return m_wrapper->file->Get_File_Name(); }

    int Get_File_Mode() const { return m_wrapper->file->Get_Access(); }

private:
    void Assign(File *file)
    {
        if (m_wrapper.get() == nullptr || m_wrapper->file != file) {
            if (file == nullptr) {
                m_wrapper.reset();
            } else {
                m_wrapper.reset(new FileWrapper);
                m_wrapper->file = file;
                m_wrapper->file->Delete_On_Close(false);
            }
        }
    }

    rts::shared_ptr_t<FileWrapper, FileWrapperDeleter, Counter> m_wrapper;
};

using FileRef = FileRefT<rts::shared_counter>;
using FileRefAtomic = FileRefT<rts::atomic_shared_counter>;

} // namespace Thyme
