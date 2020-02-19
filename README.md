# central-limit-order-book

## Structure
```
.
├── build/                              # Build directory
│   └── order_book                      # Built main executable
│   └── input.txt                       # Input file
│   └── test/                           # 
│       └── order_book_test             # Built unittest executable
├── src/                                # Source code directory
│   ├── libraries/                      # Library classes
│   └── main.cpp                        # Main entry of the application
├── test/                               # Test directory
│   ├── order_book_tests.cpp            # Unittest application
│   └── CMakeLists.txt                  # 
└── CMakeLists.txt                      # 
```

## General

* The `order_book` executable will load `input.txt` within the same directory, and generate the `output.txt` in place.  
* The default price tick is 0.05 and price precision is 100, this can only be modified in `main.cpp` for simplicity.
* A `relative position` is the position of a non-cancelled order among all orders with the same price.

## Functions

 Functions         | Standard Input /Output     | Explanation
-------------------| ---------------------------|--------------
`place_order`      | *order 1001 buy 100 12.30* | `action`,`order_id`,`order_type`,`quantity`,`price`. Inserts valid order into order book, and executes all matching orders. Every order will be assigned a relative position of the same price level. Orders that are `fully_filled` will be kept in the order book for future query.   
`cancel_order`     | *cancel 1004*              | `action`,`order_id`. Erase an `open` order from the order book, the relative position will be changed to `-1`. Only open orders are able to be cancelled.
`amend_order`      | *amend 1004 600*           | `action`,`order_id`,`quantity`. Amend an `open` order's quantity. If the quantity is amended up, this order will move to the back at the same price level.
`query_bid`        | *q level bid 1* / *bid,1,12.150000,600,1* | `action`,`level`/`action`,`level`,`price`,`quantity_sum`,`num_items`.
`query_ask`        | *q level ask 0* / *ask,0,12.200000,300,2* | `action`,`level`/`action`,`level`,`price`,`quantity_sum`,`num_items`.
`query_order_by_id`| *q order 1004* / *1004,open,600,0* | `action`,`order_id`/`order_id`,`order_status`,`leave_quantity`,`relative_position`,`num_items`.
