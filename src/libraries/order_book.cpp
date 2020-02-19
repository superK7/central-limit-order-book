//
// Created by Vince on 16/2/20.
//

#include <set>
#include <limits>

#include "order_book.hpp"


order_book::order_book(price_precision_type precision, price_tick_type price_tick): _price_precision(precision), _price_tick(price_tick) {}
order_book::~order_book() = default;

void order_book::place_order(limit_order &order) {
    //do some basic check: quantity limit, price limit, etc.
    if (order.placed_quantity >= INT64_MAX ) {
        std::cout << "order quantity too large" << std::endl;
        return;
    }

    auto &by_id_idx = _table.get<by_order_id>();
    auto itr = by_id_idx.find(order.order_id);
    if (itr != by_id_idx.end()) {
        std::cout << "order exists, please amend instead" << std::endl;
    } else {
        if (order.price % _price_tick != 0) {
            std::cout << "price tick not match" << std::endl;
            return;
        }
        // match buy/sell orders
        match_orders(order);
        by_id_idx.emplace(order);
        std::cout << "order placed" << std::endl;
    }
}

void order_book::cancel_order(order_id_type id) {
    auto &by_id_idx = _table.get<by_order_id>();
    auto itr = by_id_idx.find(id);
    if (itr == by_id_idx.end()) {
        std::cout << "order does not exist" << std::endl;
    } else if (itr->status != open) {
            std::cout << "order has already been executed" << std::endl;
            return;
    } else if (_cancelled_orders.find(id) != _cancelled_orders.end())  {
        std::cout << "order has already been cancelled" << std::endl;
        return;
    } else {
        auto quantity = itr->placed_quantity;
        //adjust orders' positions
        update_positions(itr);
        by_id_idx.erase(itr);
        _cancelled_orders[itr->order_id] = std::to_string(id) + "," + get_order_status_string(cancelled) + "," +
                                           std::to_string(quantity) + "," + std::to_string(-1) + "\n";

        std::cout << "order canceled" << std::endl;
    }
}

void order_book::amend_order(order_id_type id, order_quantity_type quantity) {

    if (quantity >= INT64_MAX ) {
        std::cout << "order quantity too large" << std::endl;
        return;
    }

    auto &by_id_idx = _table.get<by_order_id>();
    auto itr = by_id_idx.find(id);
    if (itr == by_id_idx.end()) {
        std::cout << "order does not exist" << std::endl;
    } else if (itr->status == cancelled) {
        std::cout << "order has already been cancelled" << std::endl;
        return;
    } else if (itr->status != open) {
        std::cout << "order has already been executed" << std::endl;
        return;
    } else {
        by_id_idx.modify(itr, [&](auto &o) {
            if (quantity > itr->placed_quantity) {
                o.position = update_positions(itr);
            }
            o.placed_quantity = quantity;
            o.leave_quantity = quantity;
        });
        std::cout << "order amended" << std::endl;
    }
}

std::string order_book::query_ask(order_position_type price_level) {
    std::string result;
    order_price_type target_price = 0;
    uint128_t total_quantity = 0;
    uint64_t num_of_items = 0;

    auto prices = std::set<order_price_type>{};
    auto &by_ask_price_idx = _table.get<by_ask_price>();

    auto pitr = by_ask_price_idx.begin();
    while ( pitr != by_ask_price_idx.end()) {
        prices.emplace(pitr->price);
        if (prices.size() == price_level +1) {
            target_price = pitr->price;
            break;
        }
        ++pitr;
    }
    if (target_price != 0) {
        auto bounds = by_ask_price_idx.equal_range(std::make_tuple(sell_order, target_price));
        auto itr = bounds.first;
        while (itr != bounds.second) {
            total_quantity += itr->placed_quantity;
            num_of_items += 1;
            ++itr;
        }
        auto price = static_cast<double>(target_price) / _price_precision;
        result = "ask," + std::to_string(price_level) + "," + std::to_string(price) + "," +
                uint128_to_string(total_quantity) + "," + std::to_string(num_of_items) + "\n";
    } else {
        std::cout << "no matching order" << std::endl;
    }
    return result;
}

std::string order_book::query_bid(order_position_type price_level) {
    std::string result;
    order_price_type target_price = 0;
    uint128_t total_quantity = 0;
    uint64_t num_of_items = 0;
    auto prices = std::set<order_price_type>();
    auto &by_bid_price_idx = _table.get<by_bid_price>();

    auto pitr = by_bid_price_idx.begin();
    while ( pitr != by_bid_price_idx.end()) {
        prices.emplace(pitr->price);
        if (prices.size() == price_level +1) {
            target_price = pitr->price;
            break;
        }
        ++pitr;
    }
    if (target_price != 0) {
        auto bounds = by_bid_price_idx.equal_range(std::make_tuple(buy_order, target_price));
        auto itr = bounds.first;
        while (itr != bounds.second) {
            total_quantity += itr->placed_quantity;
            num_of_items += 1;
            ++itr;
        }
        auto price = static_cast<double>(target_price) / _price_precision;
        result = "bid," + std::to_string(price_level) + "," + std::to_string(price) + "," +
                uint128_to_string(total_quantity) + "," + std::to_string(num_of_items) + "\n";
    } else {
        std::cout << "no matching order" << std::endl;
    }
    return result;
}

std::string order_book::query_order_by_id(order_id_type id) {
    std::string result;
    if (_cancelled_orders.find(id) != _cancelled_orders.end()) {
        result = _cancelled_orders[id];
    } else {
        auto &by_id_idx = _table.get<by_order_id>();
        auto itr = by_id_idx.find(id);
        if (itr == by_id_idx.end()) {
            std::cout << "order does not exist" << std::endl;
        } else {
            result = std::to_string(id) + "," + get_order_status_string(itr->status) + "," +
                     std::to_string(itr->leave_quantity) + "," + std::to_string(itr->position) + "\n";
        }
    }
    return result;
}

void order_book::match_orders(limit_order& order) {
    if (order.type == sell_order) {
        auto &by_ask_idx = _table.get<by_ask_price>();
        auto ask_range = by_ask_idx.equal_range(std::make_tuple(sell_order, order.price));

        order.position = std::distance(ask_range.first, ask_range.second);

        auto &by_bid_idx = _table.get<by_bid_price>();
        auto bid_range = by_bid_idx.equal_range(std::make_tuple(buy_order, order.price));
        auto pitr = bid_range.first;
        while (pitr != bid_range.second) {
            if (pitr->leave_quantity > 0) {
                auto match_quantity = (order.leave_quantity > pitr->leave_quantity) ? pitr->leave_quantity
                                                                                    : order.leave_quantity;
                by_bid_idx.modify(pitr, [&](auto &o) {
                    o.leave_quantity -= match_quantity;
                    if (o.leave_quantity == 0) {
                        o.status = fully_filled;
                    } else if (o.status == open) {
                        o.status = partially_filled;
                    }
                });
                order.leave_quantity -= match_quantity;
                if (order.leave_quantity == 0) {
                    order.status = fully_filled;
                    break;
                } else if (order.status == open) {
                    order.status = partially_filled;
                }
            }
            ++pitr;
        }
    } else {
        auto &by_bid_idx = _table.get<by_bid_price>();
        auto bid_range = by_bid_idx.equal_range(std::make_tuple(buy_order, order.price));
        order.position = std::distance(bid_range.first, bid_range.second);

        auto &by_ask_idx = _table.get<by_ask_price>();
        auto ask_range = by_ask_idx.equal_range(std::make_tuple(sell_order, order.price));
        auto pitr = ask_range.first;
        while (pitr != ask_range.second) {
            if (pitr->leave_quantity > 0) {
                auto match_quantity = (order.leave_quantity > pitr->leave_quantity) ? pitr->leave_quantity
                                                                                    : order.leave_quantity;
                by_ask_idx.modify(pitr, [&](auto &o) {
                    o.leave_quantity -= match_quantity;
                    if (o.leave_quantity == 0) {
                        o.status = fully_filled;
                    } else if (o.status == open) {
                        o.status = partially_filled;
                    }
                });
                order.leave_quantity -= match_quantity;
                if (order.leave_quantity == 0) {
                    order.status = fully_filled;
                    break;
                } else if (order.status == open) {
                    order.status = partially_filled;
                }
            }
            ++pitr;
        }
    }
}

order_position_type order_book::update_positions(order_table::index<by_order_id>::type::iterator itr) {
    order_position_type size = 0;
    if (itr->type == buy_order) {
        auto &by_position_idx = _table.get<by_bid_price>();
        auto bounds = by_position_idx.equal_range(std::make_tuple(buy_order, itr->price));
        auto pitr = bounds.first;
        while (pitr != bounds.second) {
            if (size > itr->position) {
                by_position_idx.modify(pitr, [&](auto &e) { e.position -= 1; });
            }
            ++size;
            ++pitr;
        }
    } else {
        auto &by_position_idx = _table.get<by_ask_price>();
        auto bounds = by_position_idx.equal_range(std::make_tuple(sell_order, itr->price));
        auto pitr = bounds.first;
        while (pitr != bounds.second) {
            if (size > itr->position) {
                by_position_idx.modify(pitr, [&](auto &e) { e.position -= 1; });
            }
            ++size;
            ++pitr;
        }
    }
    return size-1;
}


std::string order_book::get_order_status_string(order_status status) {
    if (status == open) return "open";
    if (status == partially_filled) return "partially filled";
    if (status == fully_filled) return "fully filled";
    if (status == cancelled) return "cancelled";
    return "unknown";
}

void order_book::print_all() {
    auto &by_id_idx = _table.get<by_order_id>();
    auto itr = by_id_idx.begin();
    while (itr != by_id_idx.end()) {
        std::cout << itr->position << " " << itr->order_id << " " << itr->type << " " <<
                  static_cast<uint64_t>(itr->price) << " " << itr->placed_quantity << " " << itr->status << std::endl;
        itr++;
    }
}

uint64_t order_book::get_order_book_size() {
    return _table.size();
}

int64_t order_book::get_order_position(order_id_type id) {
    //use int64_t here for simplicity, assuming no more than MAX_INT64 orders will fall on the same price.
    auto &by_id_idx = _table.get<by_order_id>();
    auto itr = by_id_idx.find(id);
    return (itr == by_id_idx.end()) ? -1 : itr->position;
}

order_status order_book::get_order_status(order_id_type id) {
    auto &by_id_idx = _table.get<by_order_id>();
    auto itr = by_id_idx.find(id);
    if (itr == by_id_idx.end()) {
        if (_cancelled_orders.find(id) != _cancelled_orders.end()) {
            return cancelled;
        } else {
            return unknown;
        }
    }
    return itr->status;
}

int64_t order_book::get_order_leave_quantity(order_id_type id) {
    auto &by_id_idx = _table.get<by_order_id>();
    auto itr = by_id_idx.find(id);
    return (itr == by_id_idx.end()) ? -1 : itr->leave_quantity;
}

std::string order_book::uint128_to_string(uint128_t number) {
    std::string result;
    if (!number) {
        result = "0";
    } else {
        while (number) {
            result += number % 10 + '0';
            number /= 10;
        }
        reverse(result.begin(), result.end());
    }
    return result;
}