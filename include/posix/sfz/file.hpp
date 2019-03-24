// Copyright (c) 2009-2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FILE_HPP_
#define SFZ_FILE_HPP_

#include <pn/data>
#include <pn/file>
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
    pn::data_view   data() const { return pn::data_view{_data, static_cast<int>(_size)}; }
    pn::string_view string() const { return data().as_string(); }

  private:
    struct fd {
        int no;
        fd(const pn::string& path);
        ~fd();
    };

    const pn::string _path;
    fd               _fd;
    uint8_t*         _data;
    size_t           _size;
};

}  // namespace sfz

#endif  // SFZ_FILE_HPP_
