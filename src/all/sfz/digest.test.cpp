// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/digest.hpp"

#include <fcntl.h>
#include <cstring>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "sfz/ScopedFd.hpp"
#include "sfz/bytes.hpp"
#include "sfz/encoding.hpp"
#include "sfz/exception.hpp"
#include "sfz/foreach.hpp"
#include "sfz/format.hpp"
#include "sfz/os.hpp"
#include "sfz/print.hpp"
#include "sfz/read.hpp"
#include "sfz/write.hpp"

using testing::Eq;
using testing::Test;

namespace sfz {
namespace {

typedef Test Sha1Test;

BytesSlice string_bytes(const char* string) {
    return BytesSlice(reinterpret_cast<const uint8_t*>(string), strlen(string));
}

const Sha1::Digest kEmptyDigest = { 0xda39a3ee, 0x5e6b4b0d, 0x3255bfef, 0x95601890, 0xafd80709 };

// The empty string should have the given digest.
TEST_F(Sha1Test, Empty) {
    Sha1 sha;
    EXPECT_THAT(sha.digest(), Eq(kEmptyDigest));
}

// A short value should have the given digest.  This doesn't test anything particularly complex
// about the Sha1 implementation.
TEST_F(Sha1Test, Short) {
    Sha1 sha;
    write(&sha, "abc", 3);
    const Sha1::Digest expected = { 0xa9993e36, 0x4706816a, 0xba3e2571, 0x7850c26c, 0x9cd0d89d };
    EXPECT_THAT(sha.digest(), Eq(expected));
}

// If the input size is > 55, then a second block will have to be added, in order to append the
// input size to the end.
TEST_F(Sha1Test, ForceSecondBlock) {
    Sha1 sha;
    write(&sha, "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
    const Sha1::Digest expected = { 0x84983e44, 0x1c3bd26e, 0xbaae4aa1, 0xf95129e5, 0xe54670f1 };
    EXPECT_THAT(sha.digest(), Eq(expected));
}

// Test a long input, added in large chunks, which are not a multiple of 64 bytes (the block size).
TEST_F(Sha1Test, Long) {
    Bytes input;
    const char* kChars = "abcdefghijklm";
    foreach (int i, range(1000)) {
        input.append(1, kChars[i % 13]);
    }

    Sha1 sha;
    foreach (int i, range(1000)) {
        write(&sha, input);
    }

    const Sha1::Digest expected = { 0x2287c79b, 0xe65d2e85, 0x104e4c8e, 0xa704680a, 0x6ba68a75 };
    EXPECT_THAT(sha.digest(), Eq(expected));
}

// Adding the input in chunks of 512 bits (64 bytes) should work fine.
TEST_F(Sha1Test, EvenMultipleOf512Bits) {
    Bytes bytes;
    foreach (int i, range(8)) {
        write(&bytes, "01234567", 8);
    }
    ASSERT_THAT(bytes.size(), Eq<size_t>(64));

    Sha1 sha;
    foreach (int i, range(10)) {
        write(&sha, bytes);
    }

    const Sha1::Digest expected = { 0xdea356a2, 0xcddd90c7, 0xa7ecedc5, 0xebb56393, 0x4f460452 };
    EXPECT_THAT(sha.digest(), Eq(expected));
}

// Add each character from ' ' ('\x20') to '\x7f'.  Check the hash after adding each character.
TEST_F(Sha1Test, IncrementalDigest) {
    const Sha1::Digest expected[] = {
        { 0xb858cb28, 0x2617fb09, 0x56d96021, 0x5c8e84d1, 0xccf909c6 },
        { 0x3aecb474, 0x332ad132, 0xb04325b2, 0xc1c55e5d, 0x4ec74532 },
        { 0xddb5a749, 0x180a7abb, 0xd694994f, 0xcfd82ccc, 0x5054e9cb },
        { 0x1db37369, 0x2bee185f, 0x8fe3c40c, 0xdb8c539c, 0xe8ca08e5 },
        { 0x56d2b995, 0x23682ac4, 0xa684a7d5, 0xb917707c, 0xfaedc2ce },
        { 0x8c92c0f0, 0x93bca9fd, 0xf57a8a28, 0xca70e0d4, 0x98c5e8c1 },
        { 0xfd287331, 0x7df55f6d, 0x3cd19ba5, 0x7045dfaf, 0x5b7f6d13 },
        { 0xd147d76d, 0xdc5b7739, 0x53c6e81f, 0xb1b56e9e, 0x205c0976 },
        { 0x02178c57, 0x803f03b9, 0xca368fc3, 0xfcc3de0d, 0xacdb4383 },
        { 0x38a61348, 0x7ee9107e, 0x82dcd312, 0xdbfdbf44, 0x5581ccb2 },
        { 0x78c52f1e, 0x6dd8c9bd, 0xb3a338c0, 0xd9e97b05, 0xf2468ddf },
        { 0x33eb7260, 0x7f1447d0, 0x526020ce, 0xa8280dd4, 0x7a925d04 },
        { 0x327baa4b, 0x402b3058, 0x01c9c655, 0xe97b7e77, 0x85831890 },
        { 0x53b5a733, 0xcbd32346, 0xe923f2cc, 0x2b82303b, 0x47bab4bc },
        { 0x64afe078, 0xdbdc5a59, 0x629f935d, 0xdf07ef6b, 0x20daa9a2 },
        { 0x5c3f75dd, 0xa77eb61e, 0xf6d04b50, 0x45bdf661, 0xf4fa608c },
        { 0xddcf9ae9, 0xe65f0863, 0x5b424aa4, 0x9b5a80e4, 0x10d23d88 },
        { 0xf36eb8bf, 0x04ac8b3c, 0xb1f2735d, 0x972c42d7, 0xf4160d42 },
        { 0xc9e45751, 0x96829b57, 0xcded456c, 0x363f1550, 0xceb64f15 },
        { 0x8b08d580, 0xb0fcfdb4, 0x4343b8db, 0x2d24fc4a, 0xe7171c43 },
        { 0xe081c9cf, 0x08300e5e, 0xa2fda182, 0x1578d3aa, 0x7d2e7c63 },
        { 0x7e2a9446, 0x417b0194, 0x2f0a8a19, 0x0dbd13cb, 0x5e95821a },
        { 0xf321ed34, 0xddbea25a, 0x1528579d, 0xc80adf48, 0xe2f78de7 },
        { 0x20504f10, 0xbca86adb, 0x0f11058b, 0x8fcfc708, 0xdc8ce1cb },
        { 0x4631b568, 0x47c94c57, 0xf6c48d5b, 0x067c5955, 0x61f680b0 },
        { 0xef2dca9c, 0x9f02055e, 0x0b76e94f, 0xfa174544, 0xe123b063 },
        { 0x3de98e18, 0xd3b95ce3, 0xeb6a867a, 0xbb77e2b1, 0x6ed869f1 },
        { 0x254c0b10, 0x8948620d, 0xf6be1332, 0x63fc9b84, 0xcf285542 },
        { 0x54b74a5b, 0x3400c0bf, 0x015dcc02, 0xa3be53f7, 0x3b5564f3 },
        { 0x506b6876, 0x0f46b3db, 0x18931bf4, 0xe04e6fe8, 0x71f20b53 },
        { 0x44e8fc38, 0xc9b97043, 0x5a0c1d2b, 0x024c862b, 0x5d63802f },
        { 0x9502711a, 0x5b6468a0, 0x400d0954, 0x80515d96, 0x10f327ac },
        { 0x160a69c0, 0x884222a7, 0xb0ea863b, 0xad018f22, 0xf60664b3 },
        { 0x6b654289, 0x2b4eb68d, 0x784c342f, 0xbbb2b9a3, 0x39881027 },
        { 0x6fbf1f1d, 0x595cad13, 0x1a4086d6, 0xc76f13df, 0xa0ebc5af },
        { 0x6331ddba, 0x6fe05d3a, 0xce0affae, 0x592f9029, 0x111c4875 },
        { 0xc60d8092, 0xf6bb533f, 0x729a4406, 0x482df134, 0x9fa07eb0 },
        { 0x43bc4b4e, 0x62b2b289, 0xbf589627, 0xbc12c307, 0xb869842a },
        { 0xfe5104ee, 0x1cd13204, 0x121a6daa, 0x50ad9ad6, 0xd2699bb7 },
        { 0xc84880cb, 0x0eb19325, 0x2352467a, 0xa457f0a4, 0x79c34975 },
        { 0x3d55aea1, 0xc18896da, 0x6505feb3, 0xd9924771, 0x5f94a818 },
        { 0x611d52e6, 0x0c7fb365, 0xe3aeea18, 0x2878d2a2, 0x43a3d0c3 },
        { 0x36086c17, 0x1e3c3bbc, 0x8e09e63b, 0xa7497391, 0xd37d11c5 },
        { 0x25261e3d, 0x9f08e27c, 0x81cad52f, 0x5c08fcb1, 0x93685ef5 },
        { 0x888df783, 0xb40c072c, 0x840827bb, 0x5d54dd8f, 0x3543444c },
        { 0x0e9b5958, 0xc8e7ddcd, 0x5ed47934, 0x38205821, 0x28575a24 },
        { 0xf84e4efb, 0x3e8af81e, 0xed7cc76c, 0x01739ca5, 0xc4d88902 },
        { 0xfcd3e856, 0xd474e94e, 0xe65eb99d, 0x47e6d814, 0xb16a1ef5 },
        { 0x563d5cd1, 0xa2e7510d, 0x403551ca, 0xb7e811ef, 0x0866b9ee },
        { 0xbfd04468, 0x507bc972, 0x4e34d831, 0xcb980e8f, 0xaf2f110c },
        { 0x3ab74ad4, 0xe010e925, 0x1db2e003, 0x3c9e07ad, 0xef9a9aa2 },
        { 0x42234eeb, 0x0036a52e, 0xca9f80a8, 0x96241b54, 0xaa572e9f },
        { 0xe985e76a, 0xb9aad915, 0xd504acb3, 0xa171c606, 0x02b0ffd7 },
        { 0xac366322, 0x43257689, 0xb528976a, 0x30aa71a0, 0x06257276 },
        { 0x6b1a9531, 0xee7f0887, 0xaa3812b6, 0x93aa82a4, 0xbcab20db },
        { 0x10efb39d, 0x0fec45b0, 0x5f8a679d, 0xcf6328f1, 0xeb5fc469 },
        { 0xe1cfcd09, 0xcc8966ad, 0xc7b5608d, 0xa465f8f0, 0xe585fd15 },
        { 0xcbfa3bd9, 0x87dac669, 0xcfb0de30, 0xca04e378, 0x2498ca17 },
        { 0xcf636e3e, 0xc353551d, 0x44fbdd6a, 0x299e3ce2, 0x5ea5f12f },
        { 0x4d37c18d, 0x90d3d3c9, 0x635d6726, 0x38ab7c40, 0x41c1a15e },
        { 0x72ff9785, 0x6b74aa7e, 0xf325e836, 0x165b07a9, 0x818cd7dc },
        { 0xebde9988, 0xb4a5c550, 0xaf83b00b, 0xa29dfd6a, 0xe0f340f3 },
        { 0xfaf0e705, 0x90323070, 0x28dc94d0, 0xe97d1bfd, 0x5cb6a134 },
        { 0xd44e1fe0, 0xc1a7ac33, 0xe87a5102, 0x85e6419a, 0xfd9727e8 },
        { 0xe2acd1ef, 0x1f11d05e, 0x3955bb24, 0x4dba2115, 0x9de9621f },
        { 0x275d883e, 0x37eac6f2, 0xb5d996e0, 0xc0656eca, 0xbe65453f },
        { 0x7c7c7b6c, 0xd014d219, 0xfd72f626, 0x636fbb59, 0xc99aca58 },
        { 0xa3bb9cf7, 0xcc3ca408, 0x1f85acdb, 0xe1cc9105, 0x5b26c173 },
        { 0x6cfc9b15, 0xfe543569, 0xf675c4b7, 0x4fb7e036, 0xe8c24a8a },
        { 0xc3afc34c, 0x418dd1f6, 0xb4983795, 0xb2be0ac3, 0x09720a47 },
        { 0xe395cbd1, 0x8b968194, 0xe6e69e8a, 0x4fe86a6e, 0x4d96d9ca },
        { 0xc2fc032f, 0x01d9f026, 0x5e43e754, 0xc1f2f85e, 0xe4674296 },
        { 0xfcb774df, 0x95841689, 0xf7802b00, 0x21e546a4, 0x11e14fc0 },
        { 0xa5d632cf, 0xfa003261, 0x0bb6d1ce, 0x7c62913a, 0x1247c7c5 },
        { 0x6ff6f332, 0x70f7f6b6, 0x8bcca846, 0x9025be7e, 0xe961ff4d },
        { 0x2aad69a7, 0x50be6504, 0x76154d12, 0x84e4b515, 0x056eaafb },
        { 0xbd041ca5, 0x2212a111, 0xe3d2b9db, 0x52447172, 0xa05e1ce5 },
        { 0x59bc6623, 0x21db643d, 0xa590be5f, 0x3f96c132, 0x2492b9b1 },
        { 0x8e61ca1b, 0xa46bde4b, 0x1036d5f3, 0x280ceaca, 0x7787c7c1 },
        { 0xf990ab4f, 0xdad6f510, 0x4b777059, 0x3db916da, 0x0404e61d },
        { 0xbf36a16a, 0x965a0e96, 0x27b0978f, 0x713d96d8, 0x3b8e4d12 },
        { 0xc7039604, 0xec5bf96f, 0xb4359740, 0xfb3ab70e, 0x6bcff1fe },
        { 0xfbd5e6cd, 0xdfbe7685, 0x1223b029, 0x01fe7f11, 0xbaefce4f },
        { 0x07b9212f, 0x06f39d3e, 0x90a18f78, 0x1508c2e8, 0x97342467 },
        { 0x73f9c9b6, 0xa5275037, 0x4a128769, 0xa9294c27, 0x28a9dc56 },
        { 0x2581e10b, 0x2bba903b, 0x663e410d, 0x11293135, 0x09448fba },
        { 0x0b40b6af, 0x7097045d, 0x7a0d26f0, 0x2ab6c0ab, 0x12860dbd },
        { 0xbd6f4f4f, 0x8c411f0f, 0xbba2f6fe, 0x8f0154ba, 0x4b1d3a74 },
        { 0xd6ae7080, 0x981d00ad, 0x41773b7d, 0xff73fa55, 0x0a8e1b0a },
        { 0xf53b1c88, 0xd400edfa, 0x666a9815, 0x336b5062, 0x10e055e5 },
        { 0xd3cc63fe, 0x0137a146, 0x661ce4d5, 0xc8a65bad, 0x1e1d956e },
        { 0xe0197db2, 0x3a8981fa, 0x20925679, 0x88337843, 0x91d8cfd2 },
        { 0x0e8537c0, 0x824ef6e2, 0xcb930f54, 0xf0a820b3, 0x7aaa1868 },
        { 0x3530d5fe, 0xf61f5de6, 0xf09a67a1, 0x5b56af26, 0xe9b8245d },
        { 0xe4f8188c, 0xdca2a68b, 0x074005e2, 0xccab5b67, 0x842c6fc7 },
        { 0x4a4a4e9c, 0x646005da, 0x73117141, 0x7059d935, 0xc38e5ba1 },
    };

    Sha1 sha;
    foreach (uint8_t byte, range(' ', '\x80')) {
        write(&sha, byte);
        EXPECT_THAT(sha.digest(), Eq(expected[byte - ' ']));
    }

    sha.reset();
    EXPECT_THAT(sha.digest(), Eq(kEmptyDigest));
}

TEST_F(Sha1Test, ReadWrite) {
    const BytesSlice written(
            "\xda\x39\xa3\xee\x5e\x6b\x4b\x0d\x32\x55\xbf\xef\x95\x60\x18\x90\xaf\xd8\x07\x09");

    BytesSlice in(written);
    Sha1::Digest digest;
    read(&in, &digest);
    EXPECT_THAT(digest, Eq(kEmptyDigest));

    Bytes out;
    write(&out, kEmptyDigest);
    EXPECT_THAT(out, Eq(written));
}

TEST_F(Sha1Test, Print) {
    StringSlice printed("da39a3ee5e6b4b0d3255bfef95601890afd80709");

    String out;
    print(&out, kEmptyDigest);
    EXPECT_THAT(out, Eq(printed));
}

struct TreeData {
    const char* path;
    const char* data;
    Sha1::Digest digest;
};

const TreeData kTreeData[] = {
    {
        "beowulf",
        "Hwæt! We Gar‐Dena in gear‐dagum\n"
            "þeod‐cyninga þrym gefrunon,\n"
            "hu þa æðelingas ellen fremedon.\n",
        { 0x894a8e64, 0x9a526228, 0x452f0bd1, 0x48236c6d, 0xf50e88e8 },
    },
    {
        "rune-poem/æsc",
        "Æsc biþ oferheah, eldum dyre\n"
            "stiþ on staþule, stede rihte hylt,\n"
            "ðeah him feohtan on firas monige.\n",
        { 0xbb04d3ad, 0x414793d0, 0xaf29d691, 0x68bc8f98, 0xbe725301 },
    },
    {
        "rune-poem/wynn",
        "Wenne bruceþ, ðe can weana lyt\n"
            "sares and sorge and him sylfa hæfþ\n"
            "blæd and blysse and eac byrga geniht.\n",
        { 0xa97a4951, 0xfce250aa, 0x4584b0e0, 0xa7deafb6, 0xd9c522a8 },
    },
    {
        "rune-poem/þorn",
        "Ðorn byþ ðearle scearp; ðegna gehwylcum\n"
            "anfeng ys yfyl, ungemetum reþe\n"
            "manna gehwelcum, ðe him mid resteð.\n",
        { 0x41f05cfb, 0x597e0e2f, 0x594142e9, 0x3f72003f, 0x804cac79 },
    },
    {
        "rune-poem/yogh",
        "Gyfu gumena byþ gleng and herenys,\n"
            "wraþu and wyrþscype and wræcna gehwam\n"
            "ar and ætwist, ðe byþ oþra leas.\n",
        { 0xccdf48d1, 0x5528a2dc, 0xf9c65218, 0x7e9058a4, 0xef6f3a31 },
    },
};

const Sha1::Digest kTreeDigest = { 0x9ff59f85, 0x3ca5ef83, 0x62cf1fcd, 0x3f293716, 0x6730bb0d };

TEST_F(Sha1Test, TreeDigest) {
    TemporaryDirectory dir("sha1-test");

    foreach (const TreeData& tree_data, kTreeData) {
        String path(format("{0}/{1}", dir.path(), utf8::decode(tree_data.path)));
        String data(utf8::decode(tree_data.data));

        makedirs(path::dirname(path), 0700);
        ScopedFd fd(open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600));
        write(&fd, utf8::encode(data));

        EXPECT_THAT(file_digest(path), Eq(tree_data.digest));
    }
    EXPECT_THAT(tree_digest(dir.path()), Eq(kTreeDigest));
}

}  // namespace
}  // namespace sfz
