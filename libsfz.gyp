{
    'target_defaults': {
        'include_dirs': [
            'include',
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
            'target_name': 'libsfz',
            'type': '<(library)',
            'sources': [
                'src/sfz/BinaryReader.cpp',
                'src/sfz/BinaryWriter.cpp',
                'src/sfz/Bytes.cpp',
                'src/sfz/Encoding.cpp',
                'src/sfz/Exception.cpp',
                'src/sfz/Format.cpp',
                'src/sfz/Formatter.cpp',
                'src/sfz/MappedFile.cpp',
                'src/sfz/NetworkBytes.cpp',
                'src/sfz/PosixFormatter.cpp',
                'src/sfz/ReferenceCounted.cpp',
                'src/sfz/String.cpp',
                'src/sfz/StringUtilities.cpp',
            ],
        },
    ],
}
