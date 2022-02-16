// Copyright (c) 2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FILE_HPP_
#define SFZ_FILE_HPP_

#pragma push_macro("WIN32_LEAN_AND_MEAN")
#pragma push_macro("NOMINMAX")

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#pragma pop_macro("WIN32_LEAN_AND_MEAN")
#pragma pop_macro("NOMINMAX")


#include <pn/data>
#include <pn/string>

namespace sfz {

// Maps a file into memory in read-only mode.
//
// Uses the mmap(2) and munmap(2) system calls to map a file on disk into memory.  The file's
// content can be accessed as a pn::data_view or pn::string_view.
class mapped_file {
  public:
    // Maps the given file into memory.
    //
    // @param [in] path     The path to the file, relative or absolute.
    explicit mapped_file(pn::string_view path);
    mapped_file(const mapped_file&) = delete;

    // Unmaps the file.
    ~mapped_file();

    // @returns             The path to the mapped file.
    pn::string_view path() const;

    // @returns             The block of data containing the file's contents.
    pn::data_view   data() const;
    pn::string_view string() const { return data().as_string(); }

  private:
    struct handle {
        HANDLE h;
        handle(pn::string_view path, HANDLE h);
        ~handle();
    };

    struct view_of_file {
        LPVOID ptr;
        view_of_file(pn::string_view path, LPVOID ptr);
        ~view_of_file();
    };

    const pn::string _path;
    handle           _file;
    LONGLONG         _size;
    handle           _file_mapping;
    view_of_file     _view_of_file;
};

}  // namespace sfz

#endif  // SFZ_FILE_HPP_
