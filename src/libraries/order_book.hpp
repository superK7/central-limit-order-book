//
// Created by Vince on 16/2/20.
//

#pragma once

#include <string>
#include <iostream>
#include <ctime>
#include <map>
#include <limits>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>

typedef __uint128_t uint128_t;
typedef __int128_t int128_t;
typedef uint64_t order_id_type;
typedef uint64_t order_quantity_type;
typedef uint64_t order_position_type;
typedef uint128_t order_price_type;
typedef uint16_t price_tick_type;
typedef uint16_t price_precision_type;

using namespace boost::multi_index;

enum order_type {
    sell_order,
    buy_order
};

enum order_status {
    open,
    partially_filled,
    fully_filled,
    cancelled,
    unknown
};

enum action {
    order,
    cancel,
    q_level_ask,
    q_level_bid,
    q_order,
    unexpected
};

struct limit_order {
    order_id_type        order_id;
    order_type           type;
    order_quantity_type  placed_quantity;
    order_quantity_type  leave_quantity = placed_quantity;
    order_price_type     price;
    order_status         status = order_status::open;
    order_position_type  position = 0;

    limit_order(order_id_type id, order_type type, order_quantity_type quantity, order_price_type price): order_id(id), type(type), placed_quantity(quantity), price(price){}
    bool operator<(const limit_order& o) const { return order_id < o.order_id; }
};

struct by_order_id;
struct by_ask_price;
struct by_bid_price;

typedef multi_index_container<
        limit_order,
        indexed_by<
                ordered_unique<tag<by_order_id>, member<limit_order, order_id_type, &limit_order::order_id> >,
                ordered_non_unique<
                        tag<by_bid_price>,
                        composite_key<limit_order,
                                member<limit_order, order_type, &limit_order::type>,
                                member<limit_order, order_price_type, &limit_order::price>,
                                member<limit_order, order_position_type, &limit_order::position>
                        >,
                        composite_key_compare<std::greater<>, std::greater<>, std::less<>>
                >,
                ordered_non_unique<
                        tag<by_ask_price>,
                        composite_key<limit_order,
                                member<limit_order, order_type, &limit_order::type>,
                                member<limit_order, order_price_type, &limit_order::price>,
                                member<limit_order, order_position_type, &limit_order::position>
                        >,
                        composite_key_compare<std::less<>, std::less<>, std::less<>>
                >
        >
> order_table;

class order_book {
public:
    order_book(price_precision_type precision, price_tick_type price_tick);
    ~order_book();

    void place_order(limit_order &order);

    void cancel_order(order_id_type id);

    void amend_order(order_id_type id, order_quantity_type quantity);

    std::string query_ask(order_position_type price_level);

    std::string query_bid(order_position_type price_level);

    std::string query_order_by_id(order_id_type id);

    void print_all();

    uint64_t get_order_book_size();

    int64_t get_order_position(order_id_type id);

    order_status get_order_status(order_id_type id);

    int64_t get_order_leave_quantity(order_id_type id);
private:
    order_table                          _table;
    price_precision_type                 _price_precision;
    price_tick_type                      _price_tick;
    std::map<order_id_type, std::string> _cancelled_orders;

    void match_orders(limit_order& order);
    order_position_type update_positions(order_table::index<by_order_id>::type::iterator itr);
    std::string get_order_status_string(order_status status);
    std::string uint128_to_string(uint128_t number);
};