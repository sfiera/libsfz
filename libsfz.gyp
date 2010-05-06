{
    'target_defaults': {
        'include_dirs': [
            'include',
            '<(DEPTH)/ext/googlemock/include',
            '<(DEPTH)/ext/googletest/include',
        ],
        'xcode_settings': {
            'GCC_TREAT_WARNINGS_AS_ERRORS': 'YES',
            'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES',
            'SDKROOT': 'macosx10.4',
            'GCC_VERSION': '4.0',
            'ARCHS': 'ppc x86_64 i386',
            'WARNING_CFLAGS': [
                '-Wall',
                '-Wendif-labels',
            ],
        },
    },
    'targets': [
        {
            'target_name': 'check-deps',
            'type': 'none',
            'actions': [
                {
                    'action_name': 'check-deps',
                    'inputs': [ ],
                    'outputs': [ ],
                    'action': [
                        './scripts/check-deps.sh',
                        '<(DEPTH)',
                    ],
                },
            ],
        },
        {
            'target_name': 'libsfz',
            'type': '<(library)',
            'sources': [
                'src/sfz/Bytes.cpp',
                'src/sfz/Encoding.cpp',
                'src/sfz/Exception.cpp',
                'src/sfz/Format.cpp',
                'src/sfz/Formatter.cpp',
                'src/sfz/MappedFile.cpp',
                'src/sfz/NetworkBytes.cpp',
                'src/sfz/PosixFormatter.cpp',
                'src/sfz/PrintItem.cpp',
                'src/sfz/ReadItem.cpp',
                'src/sfz/ReferenceCounted.cpp',
                'src/sfz/ScopedFd.cpp',
                'src/sfz/Sha1.cpp',
                'src/sfz/String.cpp',
                'src/sfz/StringUtilities.cpp',
                'src/sfz/WriteItem.cpp',
            ],
        },
        {
            'target_name': 'libsfz-tests',
            'type': 'executable',
            'sources': [
                'src/sfz/Formatter.test.cpp',
                'src/sfz/PrintItem.test.cpp',
                'src/sfz/ReadItem.test.cpp',
                'src/sfz/Sha1.test.cpp',
                'src/sfz/String.test.cpp',
                'src/sfz/WriteItem.test.cpp',
            ],
            'dependencies': [
                ':check-deps',
                ':libsfz',
                '<(DEPTH)/ext/googlemock/googlemock.gyp:gmock_main',
            ],
        },
    ],
}
