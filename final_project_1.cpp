#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <queue>
#include <climits>  // Include for INT_MAX

using namespace std;

// Struct to hold individual order details (unchanged)
struct Order {
    int id;
    int clientType;  // 1 for old customer, 0 for new
    int price;
    int prepTime;
    string item;

    Order(int cid, int type, int p, int time, const string& it)
        : id(cid), clientType(type), price(p), prepTime(time), item(it) {}
};

// Comparison logic for comparing customers based on total cost, max prep time, and client type
struct CompareCustomer {
    bool operator()(const pair<int, pair<int, int>>& a, const pair<int, pair<int, int>>& b) {
        // a.first is customer ID, a.second.first is the total cost, a.second.second is max prep time
        // b.first is customer ID, b.second.first is the total cost, b.second.second is max prep time

        // 1. Higher total cost comes first
        if (a.second.first != b.second.first) return a.second.first < b.second.first;
        // 2. If total cost is the same, prioritize the lower max prep time (shorter time is better)
        if (a.second.second != b.second.second) return a.second.second > b.second.second;
        // 3. If cost and max prep time are the same, prioritize old customer (clientType == 1 for old)
        return a.first < b.first;  // Prioritize old customer (lower customer ID is old)
    }
};

class OrderManager {
private:
    unordered_map<string, int> registeredCustomers;  // Registered customers mapped by name (string)
    unordered_map<int, pair<int, int>> customerOrders;  // Maps customer ID -> (total cost, max prep time)
    unordered_map<string, pair<int, int>> menu = {
        {"Steak", {25, 30}}, 
        {"Burger", {15, 20}}, 
        {"Salad", {12, 10}}, 
        {"Lobster", {30, 40}}, 
        {"Pizza", {20, 25}}, 
        {"Pasta", {18, 20}},
        // New items added to the menu
        {"Sushi", {22, 15}},       // Sushi: $22, 15 mins to prepare
        {"Tacos", {10, 10}},       // Tacos: $10, 10 mins to prepare
        {"Soup", {8, 5}},          // Soup: $8, 5 mins to prepare
        {"Steak Fries", {12, 15}}, // Steak Fries: $12, 15 mins to prepare
        {"Ice Cream", {5, 5}},     // Ice Cream: $5, 5 mins to prepare
        {"Chicken Wings", {18, 20}}, // Chicken Wings: $18, 20 mins to prepare
        {"Caesar Salad", {14, 12}}, // Caesar Salad: $14, 12 mins to prepare
        {"Grilled Cheese", {10, 8}}, // Grilled Cheese: $10, 8 mins to prepare
        {"Spaghetti", {16, 25}},    // Spaghetti: $16, 25 mins to prepare
        {"Spring Rolls", {9, 10}},  // Spring Rolls: $9, 10 mins to prepare
        {"BBQ Ribs", {28, 35}}      // BBQ Ribs: $28, 35 mins to prepare
    };

    // Priority queue to handle customer priority based on new comparison
    priority_queue<pair<int, pair<int, int>>, vector<pair<int, pair<int, int>>>, CompareCustomer> customerPriorityQueue;

    // Helper functions for customer file handling
    void loadCustomersFromFile() {
        ifstream inFile("customers.txt");
        if (inFile.is_open()) {
            string name;
            int id;
            while (inFile >> name >> id) {
                registeredCustomers[name] = id;  // Populate map from file
            }
            inFile.close();
        }
    }

    void saveCustomerToFile(const string& customerName, int customerId) {
        ofstream outFile("customers.txt", ios::app);  // Append mode to avoid overwriting
        if (outFile.is_open()) {
            outFile << customerName << " " << customerId << endl;  // Save new customer to file
            outFile.close();
        }
    }

public:
    OrderManager() {
        srand(static_cast<unsigned int>(time(0)));  // Initialize random seed
        loadCustomersFromFile();  // Load existing customers from file
    }

    // Add a customer to the registered list (if new) or return the customer ID if existing
    int getCustomerId(const string& customerName) {
        int customerId;
        if (registeredCustomers.find(customerName) == registeredCustomers.end()) {
            // New customer
            customerId = rand() % 9000 + 1000; // Generate a new ID
            registeredCustomers[customerName] = customerId;
            saveCustomerToFile(customerName, customerId);  // Save the new customer to file
            cout << "Welcome new customer " << customerName << "! Your unique ID: " << customerId << "\n";
        } else {
            // Returning customer
            customerId = registeredCustomers[customerName];
            cout << "Welcome back " << customerName << "! Your unique ID: " << customerId << "\n";
        }
        return customerId;
    }

    // Process a customer's order
    void processCustomer(const string& customerName, const vector<string>& foodItems) {
        int customerId = getCustomerId(customerName);
        int totalCost = 0;
        int maxPrepTime = 0;  // Start with a very low value for the maximum preparation time

        // Process each food item
        for (const string& item : foodItems) {
            if (menu.find(item) == menu.end()) {
                cout << "Menu item " << item << " is unavailable.\n";
                continue;
            }
            int price = menu[item].first;
            int prepTime = menu[item].second;

            totalCost += price;
            maxPrepTime = max(maxPrepTime, prepTime);  // Track the maximum preparation time

            cout << "Order placed for customer " << customerName << ": " << item 
                 << " ($" << price << ", " << prepTime << " mins).\n";
        }

        // Add customer to the priority queue with the new calculated total cost and max prep time
        customerPriorityQueue.push({customerId, {totalCost, maxPrepTime}});
    }

    // Process orders based on customer priority
    void processOrders() {
        while (!customerPriorityQueue.empty()) {
            auto customer = customerPriorityQueue.top();
            customerPriorityQueue.pop();

            int customerId = customer.first;
            int totalCost = customer.second.first;
            int maxPrepTime = customer.second.second;

            // Find the customer name by ID
            for (const auto& entry : registeredCustomers) {
                if (entry.second == customerId) {
                    cout << "Processing order for " << entry.first << ": Total cost $" 
                         << totalCost << ", Max prep time " << maxPrepTime << " mins.\n";
                    break;
                }
            }
        }
    }

    // Show the menu to the user (only once)
    void showMenu() const {
        cout << "\nMenu:\n";
        for (const auto& item : menu) {
            cout << item.first << ": $" << item.second.first 
                 << ", " << item.second.second << " minutes to prepare\n";
        }
    }
};

int main() {
    OrderManager manager;

    // Show the menu once at the start of the program
    manager.showMenu();

    while (true) {
        cout << "\nEnter customer name: ";
        string customerName;
        cin >> customerName;

        vector<string> foodItems;
        string foodList;
        cout << "Enter items (comma separated): ";
        cin.ignore();
        getline(cin, foodList);

        // Split the input into individual items
        size_t pos = 0;
        while ((pos = foodList.find(',')) != string::npos) {
            foodItems.push_back(foodList.substr(0, pos));
            foodList.erase(0, pos + 1);
        }
        if (!foodList.empty()) foodItems.push_back(foodList);

        manager.processCustomer(customerName, foodItems);

        cout << "\nContinue? (yes/no): ";
        string decision;
        cin >> decision;
        if (decision == "no") break;
    }

    cout << "\nProcessing orders based on priority...\n";
    manager.processOrders();

    return 0;
}
 