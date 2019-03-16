#include "logic/sensor.hpp"

#include <catch.hpp>

TEST_CASE("logic/Trigger/default_constructor")
{
    Trigger t;
    CHECK(!t.inverted);
    CHECK(std::isnan(t.low_threshold));
    CHECK(std::isnan(t.high_threshold));
    CHECK(!t.falling_edge);
    CHECK(!t.rising_edge);
    CHECK(!t.firing);
    CHECK(!t.is_firing());
}


TEST_CASE("logic/Trigger/update/does_not_break_unconfigured")
{
    Trigger t;
    t.low_threshold = 10;
    t.high_threshold = 20;

    t.update(0);
    t.update(11);
    t.update(19);
    t.update(21);
    t.update(19);
    t.update(11);
    t.update(0);
}


TEST_CASE("logic/Trigger/update/emits_always_for_unconfigured_thresholds")
{
    unsigned firing_called = 0;
    unsigned rising_edge_called = 0;
    unsigned falling_edge_called = 0;
    auto firing_func = [&firing_called](){ firing_called += 1; };
    auto rising_edge_func =
            [&rising_edge_called](){ rising_edge_called += 1; };
    auto falling_edge_func =
            [&falling_edge_called](){ falling_edge_called += 1; };

    Trigger t;
    t.falling_edge = falling_edge_func;
    t.rising_edge = rising_edge_func;
    t.firing = firing_func;

    t.update(0);
    t.update(11);
    t.update(19);
    t.update(21);
    t.update(19);
    t.update(11);
    t.update(0);

    CHECK(!falling_edge_called);
    CHECK(rising_edge_called == 1);
    CHECK(firing_called == 7);
    CHECK(t.is_firing());
}


TEST_CASE("logic/Trigger/update/rising_edge")
{
    bool called = false;
    auto func = [&called](){ called = true; };

    Trigger t;
    t.low_threshold = 10;
    t.rising_edge = func;

    t.update(0);
    CHECK(!called);

    t.update(11);
    CHECK(called);
}


TEST_CASE("logic/Trigger/update/falling_edge")
{
    bool called = false;
    auto func = [&called](){ called = true; };

    Trigger t;
    t.low_threshold = 10;
    t.falling_edge = func;

    t.update(0);
    CHECK(!called);

    t.update(11);
    CHECK(!called);

    t.update(9);
    CHECK(called);
}


TEST_CASE("logic/Trigger/update/firing")
{
    unsigned called = 0;
    auto func = [&called](){ called += 1; };

    Trigger t;
    t.low_threshold = 10;
    t.firing = func;

    t.update(0);
    CHECK(!called);
    CHECK(!t.is_firing());

    t.update(11);
    CHECK(called == 1);
    CHECK(t.is_firing());

    t.update(9);
    CHECK(called == 1);
    CHECK(!t.is_firing());

    t.update(11);
    CHECK(called == 2);
    CHECK(t.is_firing());

    t.update(11);
    CHECK(called == 3);
    CHECK(t.is_firing());

    t.update(110);
    CHECK(called == 4);
    CHECK(t.is_firing());
}


TEST_CASE("logic/Trigger/update/inverted_rising_edge")
{
    bool called = false;
    auto func = [&called](){ called = true; };

    Trigger t;
    t.inverted = true;
    t.low_threshold = 10;
    t.rising_edge = func;

    t.update(11);
    CHECK(!called);

    t.update(0);
    CHECK(called);
}


TEST_CASE("logic/Trigger/update/inverted_falling_edge")
{
    bool called = false;
    auto func = [&called](){ called = true; };

    Trigger t;
    t.inverted = true;
    t.low_threshold = 10;
    t.falling_edge = func;

    t.update(19);
    CHECK(!called);

    t.update(0);
    CHECK(!called);

    t.update(11);
    CHECK(called);
}


TEST_CASE("logic/Trigger/update/inverted_firing")
{
    unsigned called = 0;
    auto func = [&called](){ called += 1; };

    Trigger t;
    t.inverted = true;
    t.low_threshold = 10;
    t.firing = func;

    t.update(0);
    CHECK(called == 1);
    CHECK(t.is_firing());

    t.update(11);
    CHECK(called == 1);
    CHECK(!t.is_firing());

    t.update(9);
    CHECK(called == 2);
    CHECK(t.is_firing());

    t.update(11);
    CHECK(called == 2);
    CHECK(!t.is_firing());

    t.update(2);
    CHECK(called == 3);
    CHECK(t.is_firing());

    t.update(-100);
    CHECK(called == 4);
    CHECK(t.is_firing());
}
