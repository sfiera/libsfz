# -*- mode: python -*-

APPNAME = "libsfz"
VERSION = "0.1.0"

def common(ctx):
    ctx.default_sdk = "10.7"
    ctx.default_compiler = "clang"
    ctx.cxx_std = "c++11"
    ctx.load("compiler_cxx")
    ctx.load("core externals test", "ext/waf-sfiera")
    ctx.external("gmock-waf")

def options(opt):
    common(opt)

def configure(cnf):
    common(cnf)

def build(bld):
    common(bld)

    bld.stlib(
        target="libsfz/libsfz",
        features="universal",
        source=[
            "src/all/sfz/args.cpp",
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
        ],
        cxxflags="-Wall -Werror",
        includes="./include/all ./src/all",
        export_includes="./include/all",
    )

    bld.platform(
        target="libsfz/libsfz",
        platform="darwin linux",
        source=[
            "src/posix/sfz/file.cpp",
            "src/posix/sfz/io.cpp",
            "src/posix/sfz/os.cpp",
            "src/posix/sfz/posix-format.cpp",
        ],
        includes="./include/posix ./src/posix",
        export_includes="./include/posix",
    )

    def test(name):
        bld.test(
            target="libsfz/%s-test" % name,
            features="universal",
            source="src/all/sfz/%s.test.cpp" % name,
            cxxflags="-Wall -Werror",
            defines="GTEST_USE_OWN_TR1_TUPLE=1",
            use=[
                "libsfz/libsfz",
                "gmock/gmock-main",
            ],
        )

    test("algorithm")
    test("args")
    test("bytes")
    test("digest")
    test("encoding")
    test("foreach")
    test("format")
    test("io")
    test("json")
    test("memory")
    test("optional")
    test("os")
    test("print")
    test("read")
    test("string")
    test("string-utils")
    test("write")
