# -*- mode: python -*-

APPNAME = "libsfz"
VERSION = "0.1.0"

DEFAULTS = {
    "cxxflags": "-Wall -Werror",
    "arch": "x86_64 i386 ppc",
}

def common(ctx):
    ctx.load("compiler_cxx")
    ctx.load("externals", "ext/waf-sfiera")
    ctx.load("platforms", "ext/waf-sfiera")
    ctx.load("test", "ext/waf-sfiera")
    ctx.external("googlemock")

def options(opt):
    common(opt)

def configure(cnf):
    common(cnf)

def build(bld):
    common(bld)

    bld.stlib(
        target="libsfz/libsfz",
        source=[
            "src/all/sfz/bytes.cpp",
            "src/all/sfz/digest.cpp",
            "src/all/sfz/encoding.cpp",
            "src/all/sfz/endian.cpp",
            "src/all/sfz/exception.cpp",
            "src/all/sfz/format.cpp",
            "src/all/sfz/json.cpp",
            "src/all/sfz/memory.cpp",
            "src/all/sfz/print.cpp",
            "src/all/sfz/read.cpp",
            "src/all/sfz/string-utils.cpp",
            "src/all/sfz/string.cpp",
            "src/all/sfz/write.cpp",
            "src/mac/sfz/ref-count.cpp",
            "src/posix/sfz/file.cpp",
            "src/posix/sfz/io.cpp",
            "src/posix/sfz/os.cpp",
            "src/posix/sfz/posix-format.cpp",
        ],
        includes=[
            "./include/all",
            "./include/mac",
            "./include/posix",
            "./src/all",
            "./src/mac",
            "./src/posix",
        ],
        export_includes=[
            "./include/all",
            "./include/mac",
            "./include/posix",
        ],
        **DEFAULTS
    )

    bld.stlib(
        target="libsfz/test-main",
        source="",
        use=[
            "libsfz/libsfz",
            "googlemock/gmock_main",
        ],
        **DEFAULTS
    )

    bld.test(
        target="libsfz/bytes-test",
        source="src/all/sfz/bytes.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/digest-test",
        source="src/all/sfz/digest.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/encoding-test",
        source="src/all/sfz/encoding.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/foreach-test",
        source="src/all/sfz/foreach.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/format-test",
        source="src/all/sfz/format.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/io-test",
        source="src/all/sfz/io.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/json-test",
        source="src/all/sfz/json.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/memory-test",
        source="src/all/sfz/memory.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/os-test",
        source="src/all/sfz/os.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/print-test",
        source="src/all/sfz/print.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/read-test",
        source="src/all/sfz/read.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/string-test",
        source="src/all/sfz/string.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/string-utils-test",
        source="src/all/sfz/string-utils.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )

    bld.test(
        target="libsfz/write-test",
        source="src/all/sfz/write.test.cpp",
        use="libsfz/test-main",
        **DEFAULTS
    )
