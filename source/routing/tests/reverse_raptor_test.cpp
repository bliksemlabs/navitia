
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_ed
#include <boost/test/unit_test.hpp>

#include "routing/raptor.h"
#include "ed/build_helper.h"

using namespace navitia;
using namespace routing;


BOOST_AUTO_TEST_CASE(direct){
    ed::builder b("20120614");
    b.vj("A")("stop1", 8000, 8050)("stop2", 9100, 9150);
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop2"], 9200, 0, DateTimeUtils::min, false);

    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    auto res = res1.back();

    BOOST_REQUIRE_EQUAL(res.items.size(), 1);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].departure), 8050);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].arrival), 9100);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].departure), 0);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].arrival), 0);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop2"], 9200, 0, DateTimeUtils::set(0,8050-1000), false);

    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();

    BOOST_REQUIRE_EQUAL(res.items.size(), 1);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].departure), 8050);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].arrival), 9100);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].departure), 0);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].arrival), 0);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop2"], 9200, 0, DateTimeUtils::set(0,(8050)), false);

    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();

    BOOST_REQUIRE_EQUAL(res.items.size(), 1);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].departure), 8050);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].arrival), 9100);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].departure), 0);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].arrival), 0);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop2"], 9200, 0, DateTimeUtils::set(0,(8050)+1), false);

    BOOST_REQUIRE_EQUAL(res1.size(), 0);
}

BOOST_AUTO_TEST_CASE(change){
    ed::builder b("20120614");
    b.vj("A")("stop1", 8000, 8050)("stop2", 8200, 8250)("stop3", 8400, 8450);
    b.vj("B")("stop4", 9000, 9050)("stop2", 9500, 9550)("stop5", 10000,10050);
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop5"], 13000, 0, DateTimeUtils::min, false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    auto res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[2].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[1], d.stop_areas_map["stop5"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].departure), 8050);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].arrival), 8200);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].departure), 8200);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].arrival), 8320);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[2].departure), 8320);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[2].arrival), 9550);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[3].departure), 9550);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[3].arrival), 10000);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].arrival), 0);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[1].arrival), 0);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop5"], 13000, 0, DateTimeUtils::set(0, 8050-1000), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[2].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[1], d.stop_areas_map["stop5"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].departure), 8050);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].arrival), 8200);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].departure), 8200);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].arrival), 8320);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[2].departure), 8320);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[2].arrival), 9550);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[3].departure), 9550);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[3].arrival), 10000);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].arrival), 0);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[1].arrival), 0);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop5"], 13000, 0, DateTimeUtils::set(0, 8050), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[2].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[1], d.stop_areas_map["stop5"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].departure), 8050);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].arrival), 8200);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].departure), 8200);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].arrival), 8320);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[2].departure), 8320);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[2].arrival), 9550);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[3].departure), 9550);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[3].arrival), 10000);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].arrival), 0);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[1].arrival), 0);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop5"], 13000, 0, DateTimeUtils::set(0, 8050 +1), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 0);
}

BOOST_AUTO_TEST_CASE(passe_minuit){
    ed::builder b("20120614");
    b.vj("A")("stop1", 23*3600)("stop2", 24*3600 + 5*60);
    b.vj("B")("stop2", 10*60)("stop3", 20*60);
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 22*3600, 1, DateTimeUtils::min, false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    auto res = res1.back();

    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[2].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[1], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].departure), 0);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[1].arrival), 1);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 22*3600, 1, DateTimeUtils::set(0, 23*3600-1000), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[2].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[1], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].departure), 0);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[1].arrival), 1);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 22*3600, 1, DateTimeUtils::set(0, 23*3600), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[2].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[1], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].departure), 0);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[1].arrival), 1);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 22*3600, 1, DateTimeUtils::set(0, 23*3600+1), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 0);
}

BOOST_AUTO_TEST_CASE(passe_minuit_2){
    ed::builder b("20120614");
    b.vj("A")("stop1", 23*3600)("stop2", 23*3600 + 59*60);
    b.vj("B")("stop4", 23*3600 + 10*60)("stop2", 10*60)("stop3", 20*60);
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 22*3600, 1, DateTimeUtils::min, false);

    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    auto res = res1.back();

    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[2].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[1], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].departure), 0);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[2].arrival), 1);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 22*3600, 1, DateTimeUtils::set(0, 23*3600 - 1000), false);

    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();

    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[2].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[1], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].departure), 0);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[2].arrival), 1);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 22*3600, 1, DateTimeUtils::set(0, 23*3600), false);

    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();

    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[2].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[0], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[1], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].departure), 0);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[2].arrival), 1);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 22*3600, 1, DateTimeUtils::set(0, 23*3600 + 1), false);

    BOOST_REQUIRE_EQUAL(res1.size(), 0);
}

BOOST_AUTO_TEST_CASE(passe_minuit_interne){
    ed::builder b("20120614");
    b.vj("A")("stop1", 23*3600)("stop2", 23*3600 + 30*60, 24*3600 + 30*60)("stop3", 24*3600 + 40*60);
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 50*60, 1, DateTimeUtils::min, false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    auto res = res1.back();

    BOOST_REQUIRE_EQUAL(res.items.size(), 1);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[2], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].arrival), 1);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].departure), 0);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"],  50*60, 1, DateTimeUtils::set(0, 23*3600 - 1000), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();

    BOOST_REQUIRE_EQUAL(res.items.size(), 1);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[2], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].arrival), 1);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].departure), 0);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 50*60, 1, DateTimeUtils::set(0, 23*3600), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();

    BOOST_REQUIRE_EQUAL(res.items.size(), 1);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[0], d.stop_areas_map["stop1"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[1], d.stop_areas_map["stop2"]);
    BOOST_CHECK_EQUAL(res.items[0].stop_points[2], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].arrival), 1);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[0].departure), 0);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 50*60, 1, DateTimeUtils::set(0, 23*3600 + 1), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 0);
}

BOOST_AUTO_TEST_CASE(validity_pattern){
    ed::builder b("20120614");
    b.vj("D", "00", "", true)("stop1", 8000)("stop2", 8200);
    b.vj("C", "10", "", true)("stop1", 9000)("stop2", 9200);
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop2"], 7000, 1, DateTimeUtils::min, false);
    BOOST_REQUIRE_EQUAL(res.size(), 0);
}


BOOST_AUTO_TEST_CASE(marche_a_pied_milieu){
    ed::builder b("20120614");
    b.vj("A", "11111111", "", true)("stop1", 8000,8050)("stop2", 8200,8250);
    b.vj("B", "11111111", "", true)("stop3", 10000, 19050)("stop4", 19200, 19250);
    b.connection("stop2", "stop3", 10*60);
    b.connection("stop3", "stop2", 10*60);
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop4"], 27900, 0, DateTimeUtils::min, false);

    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    auto res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[3].arrival), 19200);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop4"], 27900, 0, DateTimeUtils::set(0, 8050 - 1000), false);

    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[3].arrival), 19200);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop4"], 27900, 0, DateTimeUtils::set(0, 8050), false);

    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[3].arrival), 19200);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop4"], 27900, 0, DateTimeUtils::set(0, 8050 + 1), false);

    BOOST_REQUIRE_EQUAL(res1.size(), 0);
}


BOOST_AUTO_TEST_CASE(marche_a_pied_fin){
    ed::builder b("20120614");
    b.vj("A", "11111111", "", true)("stop1", 8000)("stop2", 8200);
    b.vj("B", "11111111", "", true)("stop3", 30000)("stop4",40000);
    b.connection("stop2", "stop3", 10*60);
    b.connection("stop3", "stop2", 10*60);

    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 8200+10*60, 0, DateTimeUtils::min, false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    auto res = res1.back();

    BOOST_REQUIRE_EQUAL(res.items.size(), 2);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].arrival), 8200);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].arrival), 8800);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].departure), 8200);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 8200+10*60, 0, DateTimeUtils::set(0, 8000 - 1000), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 2);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].arrival), 8200);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].arrival), 8800);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].departure), 8200);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 8200+10*60, 0, DateTimeUtils::set(0, 8000), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 2);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[0].arrival), 8200);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].arrival), 8800);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].departure), 8200);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 8200+10*60, 0, DateTimeUtils::set(0, 8000 + 1), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 0);
}

BOOST_AUTO_TEST_CASE(marche_a_pied_pam){
    ed::builder b("20120614");
    b.vj("A")("stop1", 8000)("stop2", 23*3600+59*60);
    b.vj("B")("stop3", 2*3600)("stop4",2*3600+20);
    b.connection("stop2", "stop3", 10*60);
    b.connection("stop3", "stop2", 10*60);

    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop4"], 17900, 1, DateTimeUtils::set(0, 8000), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    auto res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[1], d.stop_areas_map["stop4"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[3].arrival), 2*3600+20);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[3].arrival), 1);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop4"], 17900, 1, DateTimeUtils::set(0, 8000 - 1000), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[1], d.stop_areas_map["stop4"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[3].arrival), 2*3600+20);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[3].arrival), 1);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop4"], 17900, 1, DateTimeUtils::set(0, 8000), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 4);
    BOOST_CHECK_EQUAL(res.items[3].stop_points[1], d.stop_areas_map["stop4"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[3].arrival), 2*3600+20);
    BOOST_CHECK_EQUAL(DateTimeUtils::date(res.items[3].arrival), 1);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop4"], 17900, 1, DateTimeUtils::set(0, 8000 + 1), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 0);
}


BOOST_AUTO_TEST_CASE(marche_a_pied_debut) {
    ed::builder b("20120614");
    b.vj("A")("stop1", 8000)("stop20", 8200);
    b.vj("B")("stop2", 30000)("stop3",40000);
    b.connection("stop1", "stop2", 10*60);
    b.connection("stop2", "stop1", 10*60);

    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 50900, 0, DateTimeUtils::min, false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    auto res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 2);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].arrival), 40000);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 50900, 0, DateTimeUtils::set(0, 8000 - 1000), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 2);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1],d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].arrival), 40000);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 50900, 0, DateTimeUtils::set(0, 8000), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res = res1.back();
    BOOST_REQUIRE_EQUAL(res.items.size(), 2);
    BOOST_CHECK_EQUAL(res.items[1].stop_points[1], d.stop_areas_map["stop3"]);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res.items[1].arrival), 40000);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 50900, 0, DateTimeUtils::set(0, (30000 - 10*60) + 1), false);
    for(auto r : res1)
        r.print(d);
    BOOST_REQUIRE_EQUAL(res1.size(), 0);
}

BOOST_AUTO_TEST_CASE(sn_fin) {
    ed::builder b("20120614");
    b.vj("A")("stop1", 8*3600)("stop2", 8*3600 + 20*60);
    b.vj("B")("stop1", 9*3600)("stop2", 9*3600 + 20*60);

    std::vector<std::pair<navitia::type::idx_t, double>> departs, destinations;
    departs.push_back(std::make_pair(0, 0));
    destinations.push_back(std::make_pair(1, 10 * 60));
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);


    auto res1 = raptor.compute_all(departs, destinations, DateTimeUtils::set(0, 9*3600 + 20 * 60));

    BOOST_REQUIRE_EQUAL(res1.size(), 1);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res1.back().items[0].departure), 8*3600);
}

BOOST_AUTO_TEST_CASE(prolongement_service) {
    ed::builder b("20120614");
    b.vj("A", "1111111", "", true)("stop1", 8*3600)("stop2", 8*3600+10*60);
    b.vj("B", "1111111", "", true)("stop4", 8*3600+15*60)("stop3", 8*3600 + 20*60);
    b.journey_pattern_point_connection("A", "B");
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 8*3600 + 25*60, 0, DateTimeUtils::min, false);

    BOOST_REQUIRE_EQUAL(res1.size(), 1);
}

BOOST_AUTO_TEST_CASE(itl) {
    ed::builder b("20120614");
    b.vj("A")("stop1",8*3600+10*60, 8*3600 + 10*60,1)("stop2",8*3600+15*60,8*3600+15*60,1)("stop3", 8*3600+20*60);
    b.vj("B")("stop1",9*3600)("stop2",10*3600);
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop2"], 9*3600+15*60, 0, DateTimeUtils::min, false);

    BOOST_REQUIRE_EQUAL(res1.size(), 0);


    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 8*3600+20*60, 0, DateTimeUtils::min, false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);
    BOOST_CHECK_EQUAL(DateTimeUtils::hour(res1.back().items[0].departure), 8*3600+10*60);

}


BOOST_AUTO_TEST_CASE(mdi) {
    ed::builder b("20120614");

    b.vj("B")("stop1",17*3600, 17*3600,std::numeric_limits<uint32_t>::max(), true, false)("stop2", 17*3600+15*60)("stop3",17*3600+30*60, 17*3600+30*60,std::numeric_limits<uint32_t>::max(), true, true);
    b.vj("C")("stop4",16*3600, 16*3600,std::numeric_limits<uint32_t>::max(), true, true)("stop5", 16*3600+15*60)("stop6",16*3600+30*60, 16*3600+30*60,std::numeric_limits<uint32_t>::max(), false, true);
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop3"], 17*3600+30*60, 0, DateTimeUtils::min, false);
    BOOST_CHECK_EQUAL(res1.size(), 0);
    res1 = raptor.compute(d.stop_areas_map["stop2"], d.stop_areas_map["stop3"], 17*3600+30*60, 0, DateTimeUtils::min, false);
    BOOST_CHECK_EQUAL(res1.size(), 1);

    res1 = raptor.compute(d.stop_areas_map["stop4"], d.stop_areas_map["stop6"], 17*3600+30*60, 0, DateTimeUtils::min, false);
    BOOST_CHECK_EQUAL(res1.size(), 0);
    res1 = raptor.compute(d.stop_areas_map["stop4"], d.stop_areas_map["stop5"], 17*3600+30*60, 0, DateTimeUtils::min, false);
    BOOST_CHECK_EQUAL(res1.size(), 1);
}

BOOST_AUTO_TEST_CASE(max_duration){
    ed::builder b("20120614");
    b.vj("A")("stop1", 8000, 8050)("stop2", 8100,8150);
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop2"], 8200, 0, DateTimeUtils::min, false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop2"], 8200, 0, DateTimeUtils::set(0, 8049), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 1);

    res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop2"], 8200, 0, DateTimeUtils::set(0, 8051), false);
    BOOST_REQUIRE_EQUAL(res1.size(), 0);
}

BOOST_AUTO_TEST_CASE(max_transfers){
    ed::builder b("20120614");
    b.vj("A")("stop1", 8000, 8050)("stop2", 81000,81500);
    b.vj("B")("stop1",8000)("stop3",8500);
    b.vj("C")("stop3",9000)("stop2",11000);
    b.vj("D")("stop3",9000)("stop4",9500);
    b.vj("E")("stop4",10000)("stop2",10500);
    b.data.pt_data.index();
    b.data.build_raptor();
    b.data.build_uri();
    RAPTOR raptor(b.data);
    type::PT_Data & d = b.data.pt_data;

    for(uint32_t nb_transfers=0; nb_transfers<=2;++nb_transfers) {
//        type::Properties p;
        auto res1 = raptor.compute(d.stop_areas_map["stop1"], d.stop_areas_map["stop2"], 86400, 0, DateTimeUtils::inf, true, type::AccessibiliteParams()/* p*/, nb_transfers);
        BOOST_REQUIRE(res1.size()>=1);
        for(auto r : res1) {
            BOOST_REQUIRE(r.nb_changes <= nb_transfers);
        }
    }
}
