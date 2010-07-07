{
    'target_defaults': {
        'include_dirs': [
            'include/all',
            'include/mac',
            'include/posix',
        ],
    },
    'targets': [
        {
            'target_name': 'libsfz',
            'type': '<(library)',
            'sources': [
                'src/all/Bytes.cpp',
                'src/all/CString.cpp',
                'src/all/Encoding.cpp',
                'src/all/Exception.cpp',
                'src/all/Format.cpp',
                'src/all/Formatter.cpp',
                'src/posix/Io.cpp',
                'src/posix/MappedFile.cpp',
                'src/all/NetworkBytes.cpp',
                'src/posix/Os.cpp',
                'src/posix/PosixFormatter.cpp',
                'src/all/PrintItem.cpp',
                'src/all/ReadItem.cpp',
                'src/mac/ReferenceCounted.cpp',
                'src/posix/ScopedFd.cpp',
                'src/all/Sha1.cpp',
                'src/all/String.cpp',
                'src/all/StringUtilities.cpp',
                'src/all/WriteItem.cpp',
            ],
            'dependencies': [
                ':check-deps',
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    'include/all',
                    'include/mac',
                    'include/posix',
                ],
            },
        },
        {
            'target_name': 'libsfz-tests',
            'type': 'executable',
            'sources': [
                'src/all/Encoding.test.cpp',
                'src/all/Formatter.test.cpp',
                'src/all/Os.test.cpp',
                'src/all/PrintItem.test.cpp',
                'src/all/ReadItem.test.cpp',
                'src/all/Sha1.test.cpp',
                'src/all/String.test.cpp',
                'src/all/WriteItem.test.cpp',
            ],
            'dependencies': [
                ':check-deps',
                ':libsfz',
                '<(DEPTH)/ext/googlemock/googlemock.gyp:gmock_main',
            ],
        },
    ],
}
