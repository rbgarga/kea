// Copyright (C) 2009  Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

// $Id$

#include <gtest/gtest.h>
#include <boost/foreach.hpp>
#include <boost/assign/std/vector.hpp>

#include <data.h>

using namespace isc::data;

#include <iostream>
using std::oct;
#include <iomanip>
using std::setfill;
using std::setw;


TEST(Element, type) {
    // this tests checks whether the getType() function returns the
    // correct type
    IntElement int_el = IntElement(1);
    EXPECT_EQ(int_el.getType(), Element::integer);
    DoubleElement double_el = DoubleElement(1.0);
    EXPECT_EQ(double_el.getType(), Element::real);
    BoolElement bool_el = BoolElement(true);
    EXPECT_EQ(bool_el.getType(), Element::boolean);
    StringElement str_el = StringElement("foo");
    EXPECT_EQ(str_el.getType(), Element::string);
    std::vector<ElementPtr> v;
    ListElement list_el = ListElement(v);
    EXPECT_EQ(list_el.getType(), Element::list);
    std::map<std::string, ElementPtr> m;
    MapElement map_el = MapElement(m);
    EXPECT_EQ(map_el.getType(), Element::map);
}

TEST(Element, from_and_to_str) {
    // this test checks whether the str() method returns the same
    // string that was used for creation
    ElementPtr el;
    std::vector<std::string> sv;

    sv.push_back("12");
    sv.push_back("1.1");
    sv.push_back("True");
    sv.push_back("False");
    sv.push_back("\"asdf\"");
    sv.push_back("[ 1, 2, 3, 4 ]");
    sv.push_back("{\"name\": \"foo\", \"value\": 47806}");

    BOOST_FOREACH(std::string s, sv) {
        // also test << operator, which uses Element::str()
        std::ostringstream stream;
        el = Element::createFromString(s);
        stream << el;
        EXPECT_EQ(stream.str(), s);
    }

    // some parse errors
    try {
        Element::createFromString("{1}");
    } catch (isc::data::ParseError pe) {
        std::string s = std::string(pe.what());
        EXPECT_EQ(s, "String expected in <unknown>:1:3");
    }
    
    sv.clear();
    sv.push_back("{1}");
    //ElementPtr ep = Element::createFromString("\"aaa\nbbb\"err");
    //std::cout << ep << std::endl;
    sv.push_back("\n\nTru");
    sv.push_back("{ \n \"aaa\nbbb\"err:");
    sv.push_back("{ \t\n \"aaa\nbbb\"\t\n\n:\n True, \"\\\"");
    sv.push_back("{ \"a\": None}");
    sv.push_back("");
    BOOST_FOREACH(std::string s, sv) {
        
        EXPECT_THROW(el = Element::createFromString(s), isc::data::ParseError);
    }

}

TEST(Element, create_and_value_throws) {
    // this test checks whether elements throw exceptions if the
    // incorrect type is requested
    ElementPtr el;

    el = Element::create(1);
    EXPECT_THROW(el->doubleValue(), TypeError);
    EXPECT_THROW(el->boolValue(), TypeError);
    EXPECT_THROW(el->stringValue(), TypeError);
    EXPECT_THROW(el->listValue(), TypeError);
    EXPECT_THROW(el->mapValue(), TypeError);

    el = Element::create(1.1);
    EXPECT_THROW(el->intValue(), TypeError);
    EXPECT_THROW(el->boolValue(), TypeError);
    EXPECT_THROW(el->stringValue(), TypeError);
    EXPECT_THROW(el->listValue(), TypeError);
    EXPECT_THROW(el->mapValue(), TypeError);

    el = Element::create(true);
    EXPECT_THROW(el->intValue(), TypeError);
    EXPECT_THROW(el->doubleValue(), TypeError);
    EXPECT_THROW(el->stringValue(), TypeError);
    EXPECT_THROW(el->listValue(), TypeError);
    EXPECT_THROW(el->mapValue(), TypeError);

    el = Element::create("foo");
    EXPECT_THROW(el->intValue(), TypeError);
    EXPECT_THROW(el->doubleValue(), TypeError);
    EXPECT_THROW(el->boolValue(), TypeError);
    EXPECT_THROW(el->listValue(), TypeError);
    EXPECT_THROW(el->mapValue(), TypeError);

    std::vector<ElementPtr> v;
    el = Element::create(v);
    EXPECT_THROW(el->intValue(), TypeError);
    EXPECT_THROW(el->doubleValue(), TypeError);
    EXPECT_THROW(el->boolValue(), TypeError);
    EXPECT_THROW(el->stringValue(), TypeError);
    EXPECT_THROW(el->mapValue(), TypeError);

    std::map<std::string, ElementPtr> m;
    el = Element::create(m);
    EXPECT_THROW(el->intValue(), TypeError);
    EXPECT_THROW(el->doubleValue(), TypeError);
    EXPECT_THROW(el->boolValue(), TypeError);
    EXPECT_THROW(el->stringValue(), TypeError);
    EXPECT_THROW(el->listValue(), TypeError);

}

TEST(Element, ListElement) {
    // this function checks the specific functions for ListElements
    ElementPtr el = Element::createFromString("[ 1, \"bar\", 3 ]");
    EXPECT_EQ(el->get(0)->intValue(), 1);
    EXPECT_EQ(el->get(1)->stringValue(), "bar");
    EXPECT_EQ(el->get(2)->intValue(), 3);

    el->set(0, Element::createFromString("\"foo\""));
    EXPECT_EQ(el->get(0)->stringValue(), "foo");

    el->add(Element::create(47806));
    EXPECT_EQ(el->get(3)->intValue(), 47806);

    el->remove(1);
    el->remove(1);
    EXPECT_EQ(el->str(), "[ \"foo\", 47806 ]");

    // hmm, it errors on EXPECT_THROW(el->get(3), std::out_of_range)
    EXPECT_ANY_THROW(el->get(3));

    el->add(Element::create(32));
    EXPECT_EQ(el->get(2)->intValue(), 32);
}

TEST(Element, MapElement) {
    // this function checks the specific functions for ListElements
    ElementPtr el = Element::createFromString("{ \"name\": \"foo\", \"value1\": \"bar\", \"value2\": { \"number\": 42 } }");
    ElementPtr el2;
    
    EXPECT_EQ(el->get("name")->stringValue(), "foo");
    EXPECT_EQ(el->get("value2")->getType(), Element::map);

    EXPECT_TRUE(isNull(el->get("value3")));

    el->set("value3", Element::create(47806));
    EXPECT_EQ(el->get("value3")->intValue(), 47806);

    el->remove("value3");
    EXPECT_TRUE(isNull(el->get("value3")));

    EXPECT_EQ(el->find("value2/number")->intValue(), 42);
    EXPECT_TRUE(isNull(el->find("value2/nothing/")));
   
    EXPECT_EQ(el->find("value1")->stringValue(), "bar");
    EXPECT_EQ(el->find("value1/")->stringValue(), "bar");
    
    EXPECT_TRUE(el->find("value1", el2));
    EXPECT_FALSE(el->find("name/error", el2));
}

TEST(Element, to_and_from_wire) {
    ElementPtr el, decoded_el;
    std::string wire;
    std::vector<std::string> sv;
    std::vector<std::string> sw;
    std::stringstream bigstring, bigstring2;
    std::stringstream bigwire, bigwire2;

    sv.push_back("{\"name\": \"foo\"}");
    sw.push_back("Skan\004name\050\003foo");
    sv.push_back("{\"value2\": {\"number\": 42}}");
    sw.push_back("Skan\006value2\042\013\006number\046\00242");
    sv.push_back("{\"bool\": False, \"bool2\": True, \"real\": 2.34, \"string\": \"foo\"}");
    sw.push_back("Skan\004bool\045\0010\005bool2\045\0011\004real\047\0042\05634\006string\050\003foo");
    sv.push_back("{\"list\": [ 1, 2, 3, 4 ]}");
    sw.push_back("Skan\004list\043\014\046\0011\046\0012\046\0013\046\0014");

    // some big ones

    bigstring << "{\"bigstring\": \"";
    bigwire << "Skan\011bigstring\030\001\001";
    for (size_t i = 0; i < 257; i++) {
        bigstring << "x";
        bigwire << "x";
    }
    bigstring << "\"}";
    sv.push_back(bigstring.str());
    sw.push_back(bigwire.str());


    bigstring2 << "{\"bigstring2\": \"";
    bigwire2 << "Skan\012bigstring2\010";
    bigwire2 << '\000' << '\001' << '\000' << '\001';
    for (size_t i = 0; i < 65537; i++) {
        bigstring2 << "x";
        bigwire2 << "x";
    }
    bigstring2 << "\"}";
    sv.push_back(bigstring2.str());
    sw.push_back(bigwire2.str());


    BOOST_FOREACH(std::string s, sv) {
        // also test << operator, which uses Element::str()
        el = Element::createFromString(s);
        EXPECT_EQ(s, el->str());
        wire = el->toWire();
        /*
        std::cout << "Encoded wire format:" << std::endl;
        my_print(wire);
        std::cout << "Expecting:" << std::endl;
        my_print(sw.at(0));
        */
        EXPECT_EQ(sw.at(0), wire);
        sw.erase(sw.begin());
        decoded_el = Element::fromWire(wire);
        EXPECT_EQ(s, decoded_el->str());
    }
    
    //EXPECT_THROW(Element::fromWire("Skan\004name\050\003foo"), DecodeError);
    EXPECT_THROW(Element::fromWire("Skan\004name\050"), DecodeError);
    EXPECT_THROW(Element::fromWire("Skan\004na"), DecodeError);
    EXPECT_THROW(Element::fromWire("Skan\004name\050\003fo"), DecodeError);
    EXPECT_NO_THROW(Element::fromWire("Skan\004name\041\003foo"));
    EXPECT_THROW(Element::fromWire("Skan\004name\041\003fo"), DecodeError);
    EXPECT_NO_THROW(Element::fromWire("Skan\004name\044\001a"));
    EXPECT_THROW(Element::fromWire("Skab\004name\050\003foo"), DecodeError);

    //EXPECT_EQ("\047\0031.2", Element::create(1.2)->toWire(0));
    EXPECT_EQ("\046\0011", Element::createFromString("[ 1 ]")->toWire(1));
}

