#include "vector.hpp"

#include <algorithm>
#include <iostream>
#include <utility>

void print(Vector<int>& v) {
    for (auto it = v.begin(); it != v.end(); ++it) {
        std::cout << *it << ' ';
    }

    std::cout << '\n';
}

int main() {
    Vector<bool> invalid;
    Vector<int> v;

    v.push_back(30);
    v.push_back(10);
    v.push_back(20);

    std::cout << "Original: ";
    print(v);

    std::cout << "Size: " << v.size()
              << ", capacity: " << v.capacity() << '\n';

    std::cout << "First: " << v.front()
              << ", last: " << v.back()
              << ", at(1): " << v.at(1) << '\n';

    v[0] = 40;

    std::sort(v.begin(), v.end());
    std::cout << "Sorted: ";
    print(v);

    auto it = std::find(v.begin(), v.end(), 20);
    if (it != v.end()) {
        std::cout << "Found: " << *it << '\n';
    }

    std::for_each(v.begin(), v.end(), [](int& x) {
        x *= 2;
    });

    std::cout << "Multiplied by 2: ";
    print(v);

    Vector<int> copy(v);
    copy[0] = 999;

    std::cout << "Copy: ";
    print(copy);

    std::cout << "Original unchanged: ";
    print(v);

    Vector<int> moved(std::move(copy));
    std::cout << "Moved: ";
    print(moved);

    copy = v;
    moved = std::move(copy);

    v.pop_back();
    std::cout << "After pop_back: ";
    print(v);

    v.reserve(10);
    v.resize(4);
    std::cout << "After reserve and resize: ";
    print(v);

    v.clear();
    std::cout << "Empty after clear: " << v.empty() << '\n';
}