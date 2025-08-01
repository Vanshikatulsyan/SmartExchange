#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>

using namespace std;

long getCurrentTimestamp() {
    return static_cast<long>(time(nullptr));
}

// --------------------- Base Order Class ---------------------
class Order {
protected:
    static int nextId;
    int id;
    string symbol;
    int quantity;
    double price;
    bool isBuy;  // true = buy, false = sell
    long timestamp;

public:
    Order(string sym, int qty, double pr, bool buy)
        : symbol(sym), quantity(qty), price(pr), isBuy(buy), timestamp(getCurrentTimestamp()) {
        id = nextId++;
    }

    virtual ~Order() {}

    virtual void display() const {
        cout << (isBuy ? "[Buy]  " : "[Sell] ") << "ID: " << id << " | "
             << "Symbol: " << symbol << " | Qty: " << quantity
             << " | Price: " << price << " | Time: " << timestamp << endl;
    }

    virtual bool match(Order* other) const {
        return isBuy != other->isBuy && symbol == other->symbol &&
               ((isBuy && price >= other->price) || (!isBuy && price <= other->price));
    }

    virtual double getPrice() const { return price; }
    int getQuantity() const { return quantity; }
    void setQuantity(int q) { quantity = q; }
    string getSymbol() const { return symbol; }
    long getTimestamp() const { return timestamp; }
    bool isBuyOrder() const { return isBuy; }
};

int Order::nextId = 1;

// --------------------- Order Comparator ---------------------
struct BuyOrderComparator {
    bool operator()(Order* a, Order* b) {
        return (a->getPrice() < b->getPrice()) || 
               (a->getPrice() == b->getPrice() && a->getTimestamp() > b->getTimestamp());
    }
};

struct SellOrderComparator {
    bool operator()(Order* a, Order* b) {
        return (a->getPrice() > b->getPrice()) ||
               (a->getPrice() == b->getPrice() && a->getTimestamp() > b->getTimestamp());
    }
};

// --------------------- OrderBook Class ---------------------
class OrderBook {
private:
    priority_queue<Order*, vector<Order*>, BuyOrderComparator> buyOrders;
    priority_queue<Order*, vector<Order*>, SellOrderComparator> sellOrders;

public:
    void addOrder(Order* order) {
        if (order->isBuyOrder()) {
            buyOrders.push(order);
        } else {
            sellOrders.push(order);
        }
        matchOrders();
    }

    void matchOrders() {
        while (!buyOrders.empty() && !sellOrders.empty()) {
            Order* buy = buyOrders.top();
            Order* sell = sellOrders.top();

            if (!buy->match(sell)) break;

            int tradedQty = min(buy->getQuantity(), sell->getQuantity());
            double tradedPrice = sell->getPrice(); // price = sell price in real world

            cout << "\n💥 Trade Executed: "
                 << tradedQty << " shares of " << buy->getSymbol()
                 << " at ₹" << fixed << setprecision(2) << tradedPrice << endl;

            buy->setQuantity(buy->getQuantity() - tradedQty);
            sell->setQuantity(sell->getQuantity() - tradedQty);

            if (buy->getQuantity() == 0) buyOrders.pop();
            if (sell->getQuantity() == 0) sellOrders.pop();
        }
    }

    void displayOrderBook() {
        cout << "\nOrder Book (Symbol: XYZ)\n-------------------\nBuy Orders:\n";
        priority_queue<Order*, vector<Order*>, BuyOrderComparator> tempBuy = buyOrders;
        while (!tempBuy.empty()) {
            tempBuy.top()->display();
            tempBuy.pop();
        }

        cout << "\nSell Orders:\n";
        priority_queue<Order*, vector<Order*>, SellOrderComparator> tempSell = sellOrders;
        while (!tempSell.empty()) {
            tempSell.top()->display();
            tempSell.pop();
        }
        cout << "-------------------\n";
    }
};

// --------------------- Exchange Class ---------------------
class Exchange {
private:
    map<string, OrderBook> books;

public:
    void placeOrder(Order* order) {
        books[order->getSymbol()].addOrder(order);
    }

    void display(string symbol) {
        books[symbol].displayOrderBook();
    }
};

// --------------------- Main ---------------------
int main() {
    Exchange nse;
    int choice;
    while (true) {
        cout << "\n📊 Stock Exchange Menu:\n1. Place Order\n2. View Order Book\n3. Exit\nEnter choice: ";
        cin >> choice;
        if (choice == 1) {
            string symbol;
            char side;
            int qty;
            double price;
            cout << "Enter symbol: "; cin >> symbol;
            cout << "Buy or Sell (B/S): "; cin >> side;
            cout << "Quantity: "; cin >> qty;
            cout << "Price: "; cin >> price;
            bool isBuy = (side == 'B' || side == 'b');
            Order* o = new Order(symbol, qty, price, isBuy);
            nse.placeOrder(o);
        } else if (choice == 2) {
            string symbol;
            cout << "Enter symbol: "; cin >> symbol;
            nse.display(symbol);
        } else {
            cout << "✅ Exiting...\n";
            break;
        }
    }

    return 0;
}
