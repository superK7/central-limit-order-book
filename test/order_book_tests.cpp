//
// Created by Vince on 16/2/20.
//
#define BOOST_TEST_MODULE OrderBookTests

#include <boost/test/unit_test.hpp>
#include "libraries/order_book.hpp"


BOOST_AUTO_TEST_SUITE(order_book_tests)
    constexpr price_precision_type precision = 100;
    constexpr price_tick_type price_tick = 5;

    BOOST_AUTO_TEST_CASE(place_order_test)
    {
        auto ob = order_book(precision, price_tick);

        auto order1 = limit_order(1001, buy_order, 100, 100);
        ob.place_order(order1);
        BOOST_REQUIRE_EQUAL(ob.get_order_book_size(), 1);

        auto order2 = limit_order(1002, buy_order, 100, 99);
        ob.place_order(order2);//this order does not match the price tick.
        BOOST_REQUIRE_EQUAL(ob.get_order_book_size(), 1);

        auto order3 = limit_order(1001, sell_order, 200, 100);
        ob.place_order(order3);//this order contains a duplicate order_id.
        BOOST_REQUIRE_EQUAL(ob.get_order_book_size(), 1);

        auto order4 = limit_order(1002, sell_order, 100, 100);
        ob.place_order(order4);
        BOOST_REQUIRE_EQUAL(ob.get_order_book_size(), 2);
    }

    BOOST_AUTO_TEST_CASE(match_test)
    {
        auto ob = order_book(precision, price_tick);

        auto order1 = limit_order(1001, buy_order, 100, 100);
        ob.place_order(order1);

        auto order2 = limit_order(1002, buy_order, 200, 100);
        ob.place_order(order2);

        auto order3 = limit_order(1003, sell_order, 140, 100);
        ob.place_order(order3);
        BOOST_REQUIRE_EQUAL(ob.get_order_book_size(), 3);
        BOOST_REQUIRE_EQUAL(ob.get_order_status(1001), fully_filled);
        BOOST_REQUIRE_EQUAL(ob.get_order_status(1002), partially_filled);
        BOOST_REQUIRE_EQUAL(ob.get_order_leave_quantity(1002), 160);
        BOOST_REQUIRE_EQUAL(ob.get_order_status(1003), fully_filled);
    }

    BOOST_AUTO_TEST_CASE(cancel_order_test)
    {
        auto ob = order_book(precision, price_tick);

        auto order1 = limit_order(1001, buy_order, 100, 100);
        ob.place_order(order1);

        auto order2 = limit_order(1002, buy_order, 200, 100);
        ob.place_order(order2);

        auto order3 = limit_order(1003, sell_order, 140, 100);
        ob.place_order(order3);

        auto order4 = limit_order(1004, buy_order, 100, 150);
        ob.place_order(order4);

        ob.cancel_order(1002); //order has been executed partially, nothing will happen
        BOOST_REQUIRE_EQUAL(ob.get_order_book_size(), 4);
        BOOST_REQUIRE_EQUAL(ob.get_order_position(1002), 1);
        BOOST_REQUIRE_EQUAL(ob.get_order_status(1002), partially_filled);

        ob.cancel_order(1004);
        BOOST_REQUIRE_EQUAL(ob.get_order_book_size(), 3);
        BOOST_REQUIRE_EQUAL(ob.get_order_position(1004), -1);
        BOOST_REQUIRE_EQUAL(ob.get_order_status(1004), cancelled);
    }

    BOOST_AUTO_TEST_CASE(amend_order_test)
    {
        auto ob = order_book(precision, price_tick);

        auto order1 = limit_order(1001, buy_order, 100, 100);
        ob.place_order(order1);

        auto order2 = limit_order(1002, buy_order, 50, 100);
        ob.place_order(order2);

        auto order3 = limit_order(1003, buy_order, 150, 100);
        ob.place_order(order3);

        ob.amend_order(1002, 200);
        BOOST_REQUIRE_EQUAL(ob.get_order_book_size(), 3);
        BOOST_REQUIRE_EQUAL(ob.get_order_position(1001), 0);
        BOOST_REQUIRE_EQUAL(ob.get_order_position(1002), 2);
        BOOST_REQUIRE_EQUAL(ob.get_order_position(1003), 1);

        ob.amend_order(1003, 25);
        BOOST_REQUIRE_EQUAL(ob.get_order_book_size(), 3);
        BOOST_REQUIRE_EQUAL(ob.get_order_position(1001), 0);
        BOOST_REQUIRE_EQUAL(ob.get_order_position(1002), 2);
        BOOST_REQUIRE_EQUAL(ob.get_order_position(1003), 1);
    }

    BOOST_AUTO_TEST_CASE(query_test)
    {
        auto ob = order_book(precision, price_tick);

        auto order1 = limit_order(1001, buy_order, 100, 100);
        ob.place_order(order1);

        auto order2 = limit_order(1002, buy_order, 50, 100);
        ob.place_order(order2);

        auto order3 = limit_order(1003, buy_order, 150, 150);
        ob.place_order(order3);

        ob.amend_order(1002, 200);
        ob.amend_order(1003, 25);

        auto order4 = limit_order(1004, sell_order, 140, 100);
        ob.place_order(order4);

        auto order5 = limit_order(1005, sell_order, 100, 150);
        ob.place_order(order5);

        auto q_result1 = ob.query_bid(1);
        BOOST_REQUIRE_EQUAL(q_result1, "bid,1,1.000000,300,2\n");

        auto q_result2 = ob.query_ask(0);
        BOOST_REQUIRE_EQUAL(q_result2, "ask,0,1.000000,140,1\n");

        auto q_result3 = ob.query_order_by_id(1002);
        BOOST_REQUIRE_EQUAL(q_result3, "1002,partially filled,160,1\n");
    }
BOOST_AUTO_TEST_SUITE_END()
