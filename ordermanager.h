#pragma once

#include <algorithm>
#include <random>
#include <vector>

#include "mixture.h"

class Order {
  public:
    Order(const Mixture &mixture, int price, int time)
        : mixture(mixture), price(price), countdown(time), totalTime(time) {}

    Mixture mixture;
    int price;
    int countdown;
    int totalTime;
};

class OrderTemplate {
  public:
    OrderTemplate(const Mixture &mixture, int price, int time, int weight)
        : mixture(mixture), price(price), time(time), weight(weight) {}

    Order generate() { return Order(mixture, price, time); }

    Mixture mixture;
    int price;
    int time;
    int weight;
};

class OrderManager {
  public:
    OrderManager() {}

    int getFrame() { return time; }
    int getTimeCountdown() { return timeCountdown; }
    void setTimeCountdown(int time) { timeCountdown = time; }
    int getFund() { return fund; }
    void addFund(int fund) { this->fund += fund; }

    void addOrderTemplates(OrderTemplate orderTemplate) {
        templates.push_back(orderTemplate);
        totalWeight += orderTemplate.weight;
    }

    const std::vector<Order> &getOrders() { return orders; }

    void setRandomizeSeed(int seed) { e.seed(seed); }

    void step() {
        time++;
        timeCountdown--;
        for (auto &order : orders) {
            order.countdown--;
            if (order.countdown <= 0) {
                tipFactor = 0;
            }
        }
        orders.erase(std::remove_if(orders.begin(), orders.end(),
                                    [](const Order &order) {
                                        return order.countdown <= 0;
                                    }),
                     orders.end());
        for (auto i = orders.size(); i < 4; i++) {
            generateOrder();
        }
    }

    int serveDish(const Mixture &mixture) {
        auto orderPos = orders.size();
        for (auto i = 0; i < orders.size(); i++) {
            if (orders[i].mixture == mixture) {
                if (orderPos == orders.size()) {
                    orderPos = i;
                } else {
                    if (orders[i].countdown < orders[orderPos].countdown) {
                        orderPos = i;
                    }
                }
            }
        }
        if (orderPos == orders.size()) {
            tipFactor = 0;
            return 0;
        }

        auto order = &orders[orderPos];
        int tip = 8;
        if (order->countdown < order->totalTime / 3) {
            tip = 3;
        } else if (order->countdown < order->totalTime * 2 / 3) {
            tip = 5;
        } else {
            tip = 8;
        }
        int price = order->price + tipFactor * tip;
        if (orderPos == 0) {
            tipFactor++;
            if (tipFactor > 4) {
                tipFactor = 4;
            }
        } else {
            tipFactor = 0;
        }
        orders.erase(orders.begin() + orderPos);
        return price;
    }

    void generateOrder() {
        std::uniform_int_distribution<int> u(0, totalWeight - 1);
        int r = u(e);
        for (auto &orderTemplate : templates) {
            if (r < orderTemplate.weight) {
                orders.push_back(orderTemplate.generate());
                break;
            }
            r -= orderTemplate.weight;
        }
    }

  private:
    int time = 0;
    int timeCountdown = 0;
    int fund = 0;
    int tipFactor = 0;

    std::vector<Order> orders;
    std::vector<OrderTemplate> templates;
    int totalWeight = 0;

    std::default_random_engine e;
};
