#include <iostream>
#include <optional>
#include <string>
#include <vector>

class Thing {
    public:
    ~Thing() {
        std::cout << "destructor" << "\n";
    }

    Thing() {
        std::cout << "default constructor" << "\n";
    }

    Thing(const Thing& rhs) {
        std::cout << "copy constructor" << "\n";
    }

    Thing(Thing&& rhs) {
        std::cout << "move constructor" << "\n";
    }

};

void things() {
    std::cout << "T1:\n";
    Thing t1;

    std::cout << "T2:\n";
    Thing t2 = t1;

    std::cout << "T3:\n";
    Thing t3(t1);

    std::cout << "T4:\n";
    Thing t4(std::move(t1));

    std::cout << "T5:\n";
    Thing t5 = t1;

    std::cout << "FIN\n";
}

void optional_strings() {
    std::optional<std::string> t1("hello");

    std::cout << t1.has_value() << "\n";

    std::optional<std::string> t2 = t1;

    std::cout << t1.has_value() << ", " << t2.has_value() << "\n";
    std::cout << *t1 << ", " << *t2 << "\n";

    std::optional<std::string> t3(std::move(t1));

    std::cout << t1.has_value() << ", " << t2.has_value() << ", " << t3.has_value() << "\n";
    std::cout << *t1 << ", " << *t2 << ", " << *t3 << "\n";
}

int main(void) {
    std::cout << "Hello World.\n";

    things();
    optional_strings();
    return 0;
}
