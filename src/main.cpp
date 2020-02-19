#include "libraries/order_book.hpp"

#include <fstream>
#include <sstream>

constexpr price_precision_type precision = 100;
constexpr price_tick_type price_tick = 5;

int main() {

    std::ifstream infile("input.txt");
    std::string line;

    std::ofstream outfile("output.txt");
    if (!outfile.is_open()) { return -1; }
    auto central_limit_order_book = order_book(precision, price_tick);

    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        std::string action;
        if (!(iss >> action)) { break; }
        if (action == "order") {
            order_id_type id;
            std::string order_type_string;
            order_quantity_type quantity;
            double float_price;
            if (!(iss >> id >> order_type_string >> quantity >> float_price)) { break; }
            order_type type;
            if (order_type_string == "sell") {
                type = order_type::sell_order;
            } else if (order_type_string == "buy") {
                type = order_type::buy_order;
            } else {
                break;
            }
            order_price_type price;
            price = static_cast<uint128_t>(float_price * precision);
            auto order = limit_order(id, type, quantity, price);
            central_limit_order_book.place_order(order);
        } else if (action == "cancel") {
            order_id_type id;
            if (!(iss >> id)) { break; }
            central_limit_order_book.cancel_order(id);
        } else if (action == "amend") {
            order_id_type id;
            order_quantity_type quantity;
            if (!(iss >> id >> quantity)) { break; }
            central_limit_order_book.amend_order(id, quantity);
        } else if (action == "q") {
            std::string sub_act;
            if (!(iss >> sub_act)) { break; }
            if (sub_act == "level") {
                std::string type;
                uint64_t level;
                if (!(iss >> type >> level)) { break; }
                if (type == "bid") {
                    outfile << central_limit_order_book.query_bid(level);
                } else if (type == "ask") {
                    outfile << central_limit_order_book.query_ask(level);
                } else {
                    break;
                }
            } else if (sub_act == "order") {
                order_id_type id;
                if (!(iss >> id)) { break; }
                outfile << central_limit_order_book.query_order_by_id(id);
            } else {
                break;
            }
        }
    }
    return 0;
}