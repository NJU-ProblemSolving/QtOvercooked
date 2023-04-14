#pragma once

#include <random>
#include <vector>

#include "mixture.h"

class Order {
  public:
    Order(const Mixture &mixture, int price, int time)
        : mixture(mixture), price(price), time(time) {}

    Mixture mixture;
    int price;
    int time;
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
        time ++;
        timeCountdown --;
        for (auto &order : orders) {
            order.time--;
        }
        orders.erase(std::remove_if(orders.begin(), orders.end(),
                                    [](const Order &order) { return order.time <= 0; }), orders.end());
        for (int i = orders.size(); i < 4; i++) {
            generateOrder();
        }
    }

    int serveDish(const Mixture &mixture) {
        auto end = orders.end();
        auto order = std::find_if(orders.begin(), orders.end(), [&](const Order &order) {
            return order.mixture == mixture;
        });
        int price = 0;
        if (order != orders.end()) {
            price = order->price;
            orders.erase(order);
        }
        return price;
    }

    void generateOrder() {
        std::uniform_int_distribution<int> u(0, totalWeight);
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

    std::vector<Order> orders;
    std::vector<OrderTemplate> templates;
    int totalWeight = 0;

    std::default_random_engine e;
};
