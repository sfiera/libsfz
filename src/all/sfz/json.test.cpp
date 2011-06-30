// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/json.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sfz/format.hpp>
#include <sfz/string-utils.hpp>

using std::make_pair;
using std::map;
using std::vector;
using testing::Eq;
using testing::InSequence;
using testing::Not;
using testing::StrictMock;

namespace sfz {

std::ostream& operator<<(std::ostream& ostr, const Json& json) {
    String str(json);
    CString c_str(str);
    return ostr << c_str.data();
}

namespace {

class MockJsonVisitor : public JsonVisitor {
  public:
    MOCK_CONST_METHOD0(enter_object, void());
    MOCK_CONST_METHOD1(object_key, void(const StringSlice&));
    MOCK_CONST_METHOD0(exit_object, void());
    void visit_object(const StringMap<Json>& value) const {
        enter_object();
        SFZ_FOREACH(const StringMap<Json>::value_type& item, value, {
            object_key(item.first);
            item.second.accept(*this);
        });
        exit_object();
    }

    MOCK_CONST_METHOD0(enter_array, void());
    MOCK_CONST_METHOD0(exit_array, void());
    void visit_array(const vector<Json>& value) const {
        enter_array();
        SFZ_FOREACH(const Json& item, value, {
            item.accept(*this);
        });
        exit_array();
    }

    MOCK_CONST_METHOD1(visit_string, void(const StringSlice& value));
    MOCK_CONST_METHOD1(visit_number, void(double value));
    MOCK_CONST_METHOD1(visit_bool, void(bool value));
    MOCK_CONST_METHOD0(visit_null, void());
};

typedef ::testing::Test JsonTest;

TEST_F(JsonTest, NullTest) {
    StrictMock<MockJsonVisitor> visitor;
    EXPECT_CALL(visitor, visit_null());
    Json().accept(visitor);
}

TEST_F(JsonTest, StringTest) {
    StrictMock<MockJsonVisitor> visitor;
    EXPECT_CALL(visitor, visit_string(Eq<StringSlice>("Hello, world!")));
    Json::string("Hello, world!").accept(visitor);
}

TEST_F(JsonTest, NumberTest) {
    StrictMock<MockJsonVisitor> visitor;
    EXPECT_CALL(visitor, visit_number(1.0));
    Json::number(1.0).accept(visitor);
}

TEST_F(JsonTest, BoolTest) {
    StrictMock<MockJsonVisitor> visitor;
    EXPECT_CALL(visitor, visit_bool(true));
    Json::boolean(true).accept(visitor);
}

// []
TEST_F(JsonTest, EmptyArrayTest) {
    StrictMock<MockJsonVisitor> visitor;
    {
        InSequence s;
        EXPECT_CALL(visitor, enter_array());
        EXPECT_CALL(visitor, exit_array());
    }
    vector<Json> a;
    Json::array(a).accept(visitor);
}

// [1.0, 2.0, 3.0]
TEST_F(JsonTest, NonEmptyArrayTest) {
    StrictMock<MockJsonVisitor> visitor;
    {
        InSequence s;
        EXPECT_CALL(visitor, enter_array());
        EXPECT_CALL(visitor, visit_number(1.0));
        EXPECT_CALL(visitor, visit_number(2.0));
        EXPECT_CALL(visitor, visit_number(3.0));
        EXPECT_CALL(visitor, exit_array());
    }
    vector<Json> a;
    a.push_back(Json::number(1.0));
    a.push_back(Json::number(2.0));
    a.push_back(Json::number(3.0));
    Json::array(a).accept(visitor);
}

// {}
TEST_F(JsonTest, EmptyObjectTest) {
    StrictMock<MockJsonVisitor> visitor;
    {
        InSequence s;
        EXPECT_CALL(visitor, enter_object());
        EXPECT_CALL(visitor, exit_object());
    }
    StringMap<Json> o;
    Json::object(o).accept(visitor);
}

// {
//   "one", 1.0,
//   "two", 2.0,
//   "three", 3.0
// }
TEST_F(JsonTest, NonEmptyObjectTest) {
    StrictMock<MockJsonVisitor> visitor;
    {
        InSequence s;
        EXPECT_CALL(visitor, enter_object());
        EXPECT_CALL(visitor, object_key(Eq<StringSlice>("one")));
        EXPECT_CALL(visitor, visit_number(1.0));
        EXPECT_CALL(visitor, object_key(Eq<StringSlice>("three")));
        EXPECT_CALL(visitor, visit_number(3.0));
        EXPECT_CALL(visitor, object_key(Eq<StringSlice>("two")));
        EXPECT_CALL(visitor, visit_number(2.0));
        EXPECT_CALL(visitor, exit_object());
    }
    StringMap<Json> o;
    o.insert(make_pair("one", Json::number(1.0)));
    o.insert(make_pair("two", Json::number(2.0)));
    o.insert(make_pair("three", Json::number(3.0)));
    Json::object(o).accept(visitor);
}

// {
//   "album": "Hey Everyone",
//   "artist": "Dananananaykroyd",
//   "compilation": false,
//   "tracks": [
//     {
//       "title": "Hey Everyone",
//       "length": 151
//     },
//     {
//       "title": "Watch This!",
//       "length": 213
//     },
//     {
//       "title": "The Greater Than Symbol & The Hash",
//       "length": 281
//     }
//   ]
// }
struct Album {
    struct Track {
        const StringSlice title;
        double length;
    };

    const StringSlice album;
    const StringSlice artist;
    bool compilation;
    Track tracks[3];
};

TEST_F(JsonTest, ComplexObjectTest) {
    const Album kAlbum = {
        "Hey Everyone",
        "Dananananaykroyd",
        false,
        {
            { "Hey Everyone", 151 },
            { "Watch This!", 213 },
            { "The Greater Than Symbol & The Hash", 281 },
        },
    };

    StrictMock<MockJsonVisitor> visitor;
    {
        InSequence s;
        EXPECT_CALL(visitor, enter_object());

        EXPECT_CALL(visitor, object_key(Eq<StringSlice>("album")));
        EXPECT_CALL(visitor, visit_string(kAlbum.album));

        EXPECT_CALL(visitor, object_key(Eq<StringSlice>("artist")));
        EXPECT_CALL(visitor, visit_string(kAlbum.artist));

        EXPECT_CALL(visitor, object_key(Eq<StringSlice>("compilation")));
        EXPECT_CALL(visitor, visit_bool(kAlbum.compilation));

        EXPECT_CALL(visitor, object_key(Eq<StringSlice>("tracks")));
        EXPECT_CALL(visitor, enter_array());
        SFZ_FOREACH(const Album::Track& track, kAlbum.tracks, {
            EXPECT_CALL(visitor, enter_object());
            EXPECT_CALL(visitor, object_key(Eq<StringSlice>("length")));
            EXPECT_CALL(visitor, visit_number(track.length));
            EXPECT_CALL(visitor, object_key(Eq<StringSlice>("title")));
            EXPECT_CALL(visitor, visit_string(track.title));
            EXPECT_CALL(visitor, exit_object());
        });
        EXPECT_CALL(visitor, exit_array());

        EXPECT_CALL(visitor, exit_object());
    }

    vector<Json> tracks;
    SFZ_FOREACH(const Album::Track& track, kAlbum.tracks, {
        StringMap<Json> object;
        object.insert(make_pair("title", Json::string(track.title)));
        object.insert(make_pair("length", Json::number(track.length)));
        tracks.push_back(Json::object(object));
    });

    StringMap<Json> album;
    album.insert(make_pair("album", Json::string(kAlbum.album)));
    album.insert(make_pair("artist", Json::string(kAlbum.artist)));
    album.insert(make_pair("compilation", Json::boolean(kAlbum.compilation)));
    album.insert(make_pair("tracks", Json::array(tracks)));

    Json::object(album).accept(visitor);
}

typedef ::testing::Test SerializeTest;

MATCHER_P(SerializesTo, expected, "") {
    sfz::String actual(arg);
    CString actual_c_str(actual);
    CString expected_c_str(expected);
    *result_listener
        << "actual " << actual_c_str.data() << " vs. expected " << expected_c_str.data();
    return actual == expected;
}

MATCHER(Parses, "") {
    Json actual;
    if (!string_to_json(arg, actual)) {
        CString arg_c_str(arg);
        *result_listener << "couldn't parse " << arg_c_str.data();
        return false;
    }
    *result_listener << "parsed to " << actual;
    return true;
}

MATCHER_P(ParsesTo, expected, "") {
    Json actual;
    if (!string_to_json(arg, actual)) {
        CString arg_c_str(arg);
        *result_listener << "couldn't parse " << arg_c_str.data();
        return false;
    }
    *result_listener << "actual " << actual << " vs. expected " << expected;
    return actual == expected;
}

void RoundTripBetween(const Json& json, PrintItem text) {
    String string(text);
    EXPECT_THAT(json, SerializesTo<StringSlice>(string));
    EXPECT_THAT(string, ParsesTo(json));
}

TEST_F(SerializeTest, NullTest) {
    RoundTripBetween(Json(), "null");
}

TEST_F(SerializeTest, StringTest) {
    RoundTripBetween(Json::string(""), "\"\"");
    RoundTripBetween(Json::string("Hello, world!"), "\"Hello, world!\"");
    RoundTripBetween(Json::string("Multiple\nLines"), "\"Multiple\\nLines\"");
    RoundTripBetween(Json::string("Control\001Chars"), "\"Control\\u0001Chars\"");

    RoundTripBetween(Json::string("\""), "\"\\\"\"");
    RoundTripBetween(Json::string("\\"), "\"\\\\\"");
    RoundTripBetween(Json::string("/"), "\"\\/\"");
    RoundTripBetween(Json::string("\b"), "\"\\b\"");
    RoundTripBetween(Json::string("\f"), "\"\\f\"");
    RoundTripBetween(Json::string("\n"), "\"\\n\"");
    RoundTripBetween(Json::string("\r"), "\"\\r\"");
    RoundTripBetween(Json::string("\t"), "\"\\t\"");
}

TEST_F(SerializeTest, NumberTest) {
    RoundTripBetween(Json::number(1.0), 1.0);
    RoundTripBetween(Json::number(2.0), 2.0);
    RoundTripBetween(Json::number(3.0), 3.0);
}

TEST_F(SerializeTest, BoolTest) {
    RoundTripBetween(Json::boolean(true), "true");
    RoundTripBetween(Json::boolean(false), "false");
}

TEST_F(SerializeTest, EmptyArrayTest) {
    vector<Json> a;
    RoundTripBetween(Json::array(a), "[]");
}

TEST_F(SerializeTest, NonEmptyArrayTest) {
    vector<Json> a;
    a.push_back(Json::number(1.0));
    a.push_back(Json::number(2.0));
    a.push_back(Json::number(3.0));
    RoundTripBetween(Json::array(a), format("[{0},{1},{2}]", 1.0, 2.0, 3.0));
}

TEST_F(SerializeTest, EmptyObjectTest) {
    StringMap<Json> o;
    RoundTripBetween(Json::object(o), "{}");
}

TEST_F(SerializeTest, NonEmptyObjectTest) {
    StringMap<Json> o;
    o.insert(make_pair("one", Json::number(1.0)));
    o.insert(make_pair("two", Json::number(2.0)));
    o.insert(make_pair("three", Json::number(3.0)));
    RoundTripBetween(Json::object(o), format(
                "{{"
                    "\"one\":{0},"
                    "\"three\":{1},"
                    "\"two\":{2}"
                "}}",
                1.0, 3.0, 2.0));
}

TEST_F(SerializeTest, ComplexObjectTest) {
    const Album kAlbum = {
        "Hey Everyone",
        "Dananananaykroyd",
        false,
        {
            { "Hey Everyone", 151 },
            { "Watch This!", 213 },
            { "The Greater Than Symbol & The Hash", 281 },
        },
    };

    vector<Json> tracks;
    SFZ_FOREACH(const Album::Track& track, kAlbum.tracks, {
        StringMap<Json> object;
        object.insert(make_pair("title", Json::string(track.title)));
        object.insert(make_pair("length", Json::number(track.length)));
        tracks.push_back(Json::object(object));
    });

    StringMap<Json> album;
    album.insert(make_pair("album", Json::string(kAlbum.album)));
    album.insert(make_pair("artist", Json::string(kAlbum.artist)));
    album.insert(make_pair("compilation", Json::boolean(kAlbum.compilation)));
    album.insert(make_pair("tracks", Json::array(tracks)));

    RoundTripBetween(Json::object(album), format(
                "{{"
                    "\"album\":\"Hey Everyone\","
                    "\"artist\":\"Dananananaykroyd\","
                    "\"compilation\":false,"
                    "\"tracks\":["
                        "{{"
                            "\"length\":{0},"
                            "\"title\":\"Hey Everyone\""
                        "}},"
                        "{{"
                            "\"length\":{1},"
                            "\"title\":\"Watch This!\""
                        "}},"
                        "{{"
                            "\"length\":{2},"
                            "\"title\":\"The Greater Than Symbol & The Hash\""
                        "}}"
                    "]"
                "}}",
                151.0, 213.0, 281.0));
}

TEST_F(SerializeTest, ParseWhitespace) {
    StringSlice text =
        "  {  \n"
        "    \"\"  :  {  }  ,  "
        "    \" \"  :  {  \"\": \"\"  }  ,  "
        "    \"  \"  :  [  ]  ,  "
        "    \"   \"  :  [  \"\"  ]  ,  "
        "    \"    \"  :  \" \"  ,  "
        "    \"     \"  :  0  ,  "
        "    \"      \"  :  true  ,  "
        "    \"       \"  :  null  "
        "  }  ";
    StringMap<Json> object;

    StringMap<Json> inner_object;
    object.insert(make_pair("", Json::object(inner_object)));
    inner_object.insert(make_pair("", Json::string("")));
    object.insert(make_pair(" ", Json::object(inner_object)));

    vector<Json> inner_array;
    object.insert(make_pair("  ", Json::array(inner_array)));
    inner_array.push_back(Json::string(""));
    object.insert(make_pair("   ", Json::array(inner_array)));

    EXPECT_TRUE(object.insert(make_pair("    ", Json::string(" "))).second);
    EXPECT_TRUE(object.insert(make_pair("     ", Json::number(0))).second);
    EXPECT_TRUE(object.insert(make_pair("      ", Json::boolean(true))).second);
    EXPECT_TRUE(object.insert(make_pair("       ", Json())).second);

    EXPECT_THAT(object.size(), Eq<size_t>(8));
    EXPECT_THAT(text, ParsesTo(Json::object(object)));
}

TEST_F(SerializeTest, ParseFailure) {
    ASSERT_THAT("", Not(Parses()));
    ASSERT_THAT(",", Not(Parses()));
    ASSERT_THAT("@", Not(Parses()));
    ASSERT_THAT("1,", Not(Parses()));
    ASSERT_THAT(",1", Not(Parses()));

    ASSERT_THAT("{", Not(Parses()));
    ASSERT_THAT("{a:1}", Not(Parses()));
    ASSERT_THAT("{\"a\"}", Not(Parses()));
    ASSERT_THAT("{\"a\":", Not(Parses()));
    ASSERT_THAT("{\"a\":,", Not(Parses()));
    ASSERT_THAT("{\"a\":@", Not(Parses()));
    ASSERT_THAT("{\"a\":1", Not(Parses()));
    ASSERT_THAT("{\"a\":1,", Not(Parses()));
    ASSERT_THAT("{\"a\":1,}", Not(Parses()));

    ASSERT_THAT("[", Not(Parses()));
    ASSERT_THAT("[a]", Not(Parses()));
    ASSERT_THAT("[\"a\"", Not(Parses()));
    ASSERT_THAT("[\"a\",", Not(Parses()));
    ASSERT_THAT("[\"a\",]", Not(Parses()));
    ASSERT_THAT("[\"a\"@", Not(Parses()));

    ASSERT_THAT("\"", Not(Parses()));
    ASSERT_THAT("\"\\", Not(Parses()));
    ASSERT_THAT("\"\\\"", Not(Parses()));
    ASSERT_THAT("\"\\@\"", Not(Parses()));
    ASSERT_THAT("\"\\u1\"", Not(Parses()));
    ASSERT_THAT("\"\\u12\"", Not(Parses()));
    ASSERT_THAT("\"\\u123\"", Not(Parses()));

    ASSERT_THAT("--1", Not(Parses()));
    ASSERT_THAT(".0", Not(Parses()));
    ASSERT_THAT(".0.", Not(Parses()));
    ASSERT_THAT("1u", Not(Parses()));
    ASSERT_THAT("1..", Not(Parses()));
    ASSERT_THAT("1.0.", Not(Parses()));
    ASSERT_THAT("1.0.0", Not(Parses()));
    ASSERT_THAT("1.0ee", Not(Parses()));
    ASSERT_THAT("1.0ee", Not(Parses()));
    ASSERT_THAT("1.0e+-2", Not(Parses()));
    ASSERT_THAT("1.0e-e", Not(Parses()));

    ASSERT_THAT("t", Not(Parses()));
    ASSERT_THAT("talse", Not(Parses()));
    ASSERT_THAT("truer", Not(Parses()));
    ASSERT_THAT("falser", Not(Parses()));
    ASSERT_THAT("maybe", Not(Parses()));
    ASSERT_THAT("NULL", Not(Parses()));
}

}  // namespace
}  // namespace sfz
