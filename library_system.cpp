// ============================================================================
// Library Management System (Single-File)
// Demonstrates:
// - Multiple inheritance (Student + Staff → TeachingAssistant)
// - Virtual inheritance to resolve the diamond problem
// - Abstract base classes and interfaces
// - Simple object interactions (transactions, fees)
// ============================================================================

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <iomanip>
using namespace std;

// ============================================================================
// Interface: Identifiable
// Every class that needs a unique ID inherits from this.
// ============================================================================
class Identifiable {
public:
virtual ~Identifiable() = default;
virtual string id() const noexcept = 0; // returns unique identifier
};

// ============================================================================
// Base Class: Person (virtual inheritance)
// Represents any library system user.
// Virtual inheritance ensures only one Person object exists in TeachingAssistant.
// ============================================================================
class Person : public virtual Identifiable {
public:
Person(string personId, string name, string email, double balance = 0.0)
: personId_(move(personId)), name_(move(name)), email_(move(email)), balance_(balance) {}


// Implement ID access from Identifiable interface
string id() const noexcept override       { return personId_; }
string getName() const noexcept           { return name_; }
string getEmail() const noexcept          { return email_; }
double getBalance() const noexcept        { return balance_; }

// Add funds to the user's balance
void addFunds(double amount) noexcept {
    if (amount > 0) balance_ += amount;
}

// Deduct money for charges (never allow negative balance)
void deduct(double amount) noexcept {
    balance_ -= amount;
    if (balance_ < 0) balance_ = 0;
}

// Display user information (virtual to support polymorphism)
virtual void display() const {
    cout << name_ << " (" << personId_ << ") | Email: " << email_
         << " | Balance: " << balance_ << "\n";
}


protected:
string personId_;
string name_;
string email_;
double balance_;
};

// ============================================================================
// Class: Student (virtual Person)
// Adds borrowing limits and a fee discount.
// ============================================================================
class Student : public virtual Person {
public:
Student(string personId, string name, string email,
double balance, int maxConcurrentBorrows = 2, double discountFactor = 0.8)
: Person(move(personId), move(name), move(email), balance),
maxConcurrentBorrows_(maxConcurrentBorrows),
discountFactor_(discountFactor) {}


int getMaxConcurrentBorrows() const noexcept { return maxConcurrentBorrows_; }
double getDiscountFactor() const noexcept    { return discountFactor_; }

// Override display — include student details
void display() const override {
    Person::display();
    cout << "  Role: Student | MaxBorrows: " << maxConcurrentBorrows_
         << " | Discount: " << discountFactor_ << "\n";
}


protected:
int maxConcurrentBorrows_;
double discountFactor_;
};

// ============================================================================
// Class: Staff (virtual Person)
// Represents campus employees; optionally can approve purchases.
// ============================================================================
class Staff : public virtual Person {
public:
Staff(string personId, string name, string email, double balance,
bool canApprovePurchases = false)
: Person(move(personId), move(name), move(email), balance),
canApprovePurchases_(canApprovePurchases) {}


bool hasPurchaseApproval() const noexcept { return canApprovePurchases_; }

// Override display
void display() const override {
    Person::display();
    cout << "  Role: Staff | PurchaseApproval: "
         << (canApprovePurchases_ ? "Yes" : "No") << "\n";
}


protected:
bool canApprovePurchases_;
};

// ============================================================================
// Class: TeachingAssistant
// MULTIPLE INHERITANCE: Student + Staff
//
// Virtual inheritance ensures only ONE Person subobject is used.
// ============================================================================
class TeachingAssistant : public Student, public Staff {
public:
TeachingAssistant(string personId, string name, string email,
double balance, int maxConcurrentBorrows,
double discountFactor, bool canApprovePurchases)
: Person(move(personId), move(name), move(email), balance),
Student(personId_, name_, email_, balance, maxConcurrentBorrows, discountFactor),
Staff(personId_, name_, email_, balance, canApprovePurchases) {}


void display() const override {
    Person::display();
    cout << "  Role: TeachingAssistant"
         << " | MaxBorrows: " << maxConcurrentBorrows_
         << " | Discount: " << discountFactor_
         << " | PurchaseApproval: " << (canApprovePurchases_ ? "Yes" : "No")
         << "\n";
}


};

// ============================================================================
// Abstract Class: LibraryItem
// Base type for ANY library object that can be borrowed.
// ============================================================================
class LibraryItem : public Identifiable {
public:
LibraryItem(string id, string title, double lateFeePerDay)
: itemId_(move(id)), title_(move(title)), lateFeePerDay_(lateFeePerDay) {}


string id() const noexcept override       { return itemId_; }
string getTitle() const noexcept          { return title_; }
double lateFeePerDay() const noexcept     { return lateFeePerDay_; }

// Must be implemented by derived classes:
virtual string typeName() const noexcept = 0;
virtual double computeLateFee(int daysLate) const noexcept = 0;


protected:
string itemId_;
string title_;
double lateFeePerDay_;
};

// ============================================================================
// Class: Book
// Represents a borrowable book (fee: 1.0 per day).
// ============================================================================
class Book : public LibraryItem {
public:
Book(string itemId, string title)
: LibraryItem(move(itemId), move(title), 1.0) {}


string typeName() const noexcept override { return "Book"; }

double computeLateFee(int daysLate) const noexcept override {
    return daysLate * lateFeePerDay_;
}


};

// ============================================================================
// Class: Magazine
// Represents a magazine (fee: 0.5 per day).
// ============================================================================
class Magazine : public LibraryItem {
public:
Magazine(string itemId, string title)
: LibraryItem(move(itemId), move(title), 0.5) {}


string typeName() const noexcept override { return "Magazine"; }

double computeLateFee(int daysLate) const noexcept override {
    return daysLate * lateFeePerDay_;
}


};

// ============================================================================
// Class: DVD
// Represents a DVD (fee: 2.0 per day).
// ============================================================================
class DVD : public LibraryItem {
public:
DVD(string itemId, string title)
: LibraryItem(move(itemId), move(title), 2.0) {}


string typeName() const noexcept override { return "DVD"; }

double computeLateFee(int daysLate) const noexcept override {
    return daysLate * lateFeePerDay_;
}


};

// ============================================================================
// Class: BorrowTransaction
// Represents an instance of a borrower returning an item late.
// Handles fee calculation, discounts, and balance deduction.
// ============================================================================
class BorrowTransaction {
public:
BorrowTransaction(Person& borrower, LibraryItem& item, int daysLate = 0)
: borrower_(&borrower), item_(&item),
daysLate_(daysLate), isOpen_(true), lateFeeCost_(0.0) {}


// Process the late fees:
// - Compute fee based on item type
// - Apply student discount (if applicable)
// - Deduct from user balance
double process() {
    if (!isOpen_) return lateFeeCost_;

    // Base cost calculated from the LibraryItem
    double cost = item_->computeLateFee(daysLate_);

    // If borrower is a Student (or derived type), apply discount
    if (auto* s = dynamic_cast<Student*>(borrower_))
        cost *= s->getDiscountFactor();

    // Deduct cost from the user's balance
    borrower_->deduct(cost);

    // Store the final cost and close the transaction
    lateFeeCost_ = cost;
    isOpen_ = false;
    return lateFeeCost_;
}

// Accessors:
string getUserId() const noexcept { return borrower_->id(); }
string getItemId() const noexcept { return item_->id(); }
bool isOpened() const noexcept    { return isOpen_; }
double getLateFeeCost() const noexcept { return lateFeeCost_; }


private:
Person* borrower_;
LibraryItem* item_;
int daysLate_;
bool isOpen_;
double lateFeeCost_;
};

// ============================================================================
// MAIN PROGRAM
// Demonstrates:
// - Polymorphic user display
// - Fund management
// - Borrowing and fee deduction
// ============================================================================
int main() {


// ------------------------------------------------------------
// 1. Create Users (stored polymorphically as Person*)
// ------------------------------------------------------------
vector<unique_ptr<Person>> users;
users.emplace_back(make_unique<Student>("S100","Amina","amina@uni.edu",50.0,2,0.8));
users.emplace_back(make_unique<Staff>("ST200","Omar","omar@uni.edu",75.0,true));
users.emplace_back(make_unique<TeachingAssistant>("TA300","Lina","lina@uni.edu",60.0,2,0.85,true));

cout << "=== Users ===\n";
for (const auto& u : users)
    u->display();

// ------------------------------------------------------------
// 2. Add funds to users
// ------------------------------------------------------------
users[0]->addFunds(20);
users[1]->addFunds(10);
users[2]->addFunds(5);

cout << "\n=== Users After Adding Funds ===\n";
for (const auto& u : users)
    u->display();

// ------------------------------------------------------------
// 3. Create Library Items
// ------------------------------------------------------------
vector<unique_ptr<LibraryItem>> items;
items.emplace_back(make_unique<Book>("B001","Effective C++"));
items.emplace_back(make_unique<Magazine>("M010","Tech Monthly"));
items.emplace_back(make_unique<DVD>("D100","C++ Patterns"));

cout << "\n=== Library Items ===\n";
for (const auto& it : items)
    cout << it->id() << " | " << it->getTitle()
         << " | " << it->typeName()
         << " | fee/day: " << it->lateFeePerDay() << "\n";

// ------------------------------------------------------------
// 4. Simulate a Borrow Transaction
// Student Amina returns a book 5 days late
// ------------------------------------------------------------
Person& borrower       = *users[0];
LibraryItem& borrowed  = *items[0];

BorrowTransaction tx(borrower, borrowed, 5);
double finalFee = tx.process();

// ------------------------------------------------------------
// 5. Display transaction summary
// ------------------------------------------------------------
cout << "\n=== Transaction Summary ===\n";
cout << "User: " << tx.getUserId() << " | Item: " << tx.getItemId() << "\n";
cout << "Days late: 5 | Fee charged: " << fixed << setprecision(2) << finalFee << "\n";
cout << "Remaining balance: " << borrower.getBalance() << "\n";
cout << "Transaction open: " << (tx.isOpened() ? "Yes" : "No") << "\n";

return 0;

}
