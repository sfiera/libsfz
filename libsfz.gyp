{ "target_defaults":
  { "cxxflags":
    [ "-Wall"
    , "-Werror"
    ]
  , "include_dirs":
    [ "src/all"
    , "src/posix"
    ]
  , "conditions":
    [ [ "OS == 'win'"
      , { "sources/": [["exclude", "^src/posix/"]]
        , "include_dirs!":
          [ "src/posix"
          , "include/posix"
          ]
        , "direct_dependent_settings":
          { "include_dirs!": ["include/posix"]
          }
        }
      ]
    ]
  }

, "targets":
  [ { "target_name": "libsfz"
    , "type": "static_library"
    , "sources":
      [ "src/all/sfz/args.cpp"
      , "src/all/sfz/bytes.cpp"
      , "src/all/sfz/digest.cpp"
      , "src/all/sfz/encoding.cpp"
      , "src/all/sfz/endian.cpp"
      , "src/all/sfz/exception.cpp"
      , "src/all/sfz/format.cpp"
      , "src/all/sfz/json.cpp"
      , "src/all/sfz/print.cpp"
      , "src/all/sfz/read.cpp"
      , "src/all/sfz/string-utils.cpp"
      , "src/all/sfz/string.cpp"
      , "src/all/sfz/write.cpp"
      , "src/posix/sfz/file.cpp"
      , "src/posix/sfz/io.cpp"
      , "src/posix/sfz/os.cpp"
      , "src/posix/sfz/posix-format.cpp"
      , "src/posix/sfz/posix-format.hpp"
      ]
    , "include_dirs":
      [ "include/all"
      , "include/posix"
      ]
    , "direct_dependent_settings":
      { "include_dirs":
        [ "include/all"
        , "include/posix"
        ]
      }
    }

  , { "target_name": "libsfz-test"
    , "type": "static_library"
    , "dependencies":
      [ "libsfz"
      , "<(DEPTH)/ext/gmock-gyp/gmock.gyp:gmock_main"
      ]
    , "export_dependent_settings":
      [ "libsfz"
      , "<(DEPTH)/ext/gmock-gyp/gmock.gyp:gmock_main"
      ]
    }

  , { "target_name": "algorithm-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/algorithm.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "args-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/algorithm.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "bytes-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/bytes.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "digest-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/digest.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "encoding-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/encoding.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "format-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/format.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "io-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/io.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "json-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/json.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "optional-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/optional.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "os-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/os.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "print-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/print.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "read-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/read.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "string-utils-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/string-utils.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "string-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/string.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }

  , { "target_name": "write-test"
    , "type": "executable"
    , "sources": ["src/all/sfz/write.test.cpp"]
    , "dependencies": ["libsfz-test"]
    }
  ]
}
# -*- mode: python; tab-width: 2 -*-
