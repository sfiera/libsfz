// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FILE_HPP_
#define SFZ_FILE_HPP_

#include <sfz/bytes.hpp>
#include <sfz/macros.hpp>
#include <sfz/string.hpp>

namespace sfz {

class ScopedFd {
  public:
    explicit ScopedFd(int fd);
    ~ScopedFd();

    int get();
    int release();

    void push(const BytesSlice& bytes);
    void push(size_t num, uint8_t byte);

  private:
    int _fd;

    DISALLOW_COPY_AND_ASSIGN(ScopedFd);
};

// Maps a file into memory in read-only mode.
//
// Uses the mmap(2) and munmap(2) system calls to map a file on disk into memory.  The file's
// content can be accessed as a BytesSlice.
class MappedFile {
  public:
    // Maps the given file into memory.
    //
    // @param [in] path     The path to the file, relative or absolute.
    explicit MappedFile(const StringSlice& path);

    // Unmaps the file.
    ~MappedFile();

    // @returns             The path to the mapped file.
    StringSlice path() const;

    // @returns             The block of data containing the file's contents.
    BytesSlice data() const;

  private:
    // The file descriptor of the open file.
    int _fd;

    // The path to the mapped file.
    const String _path;

    // The number of bytes in the mapped file.
    size_t _size;

    // A pointer to the mapped file data.
    const uint8_t* _data;

    DISALLOW_COPY_AND_ASSIGN(MappedFile);
};

}  // namespace sfz

#endif  // SFZ_FILE_HPP_
