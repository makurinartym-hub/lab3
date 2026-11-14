#pragma once

#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <typename T>
concept Numeric =
    (std::integral<T> || std::floating_point<T>)
    && !std::same_as<T, bool>
    && !std::same_as<T, char>
    && !std::same_as<T, wchar_t>
    && !std::same_as<T, char8_t>
    && !std::same_as<T, char16_t>
    && !std::same_as<T, char32_t>
    && !std::is_const_v<T>
    && !std::is_volatile_v<T>;

template <Numeric T>
class Vector {
private:
    T* data_;
    std::size_t size_;
    std::size_t capacity_;
    std::allocator<T> allocator_;

    void grow() {
        const auto limit =
            std::allocator_traits<std::allocator<T>>::max_size(allocator_);

        if (capacity_ >= limit) {
            throw std::length_error("Vector is too large");
        }

        if (capacity_ == 0) {
            reserve(1);
        } else if (capacity_ > limit / 2) {
            reserve(limit);
        } else {
            reserve(capacity_ * 2);
        }
    }

public:
    Vector() : data_(nullptr), size_(0), capacity_(0) {}

    ~Vector() {
        clear();

        if (data_ != nullptr) {
            allocator_.deallocate(data_, capacity_);
        }
    }

    Vector(const Vector& other) : Vector() {
        reserve(other.size_);

        for (std::size_t i = 0; i < other.size_; ++i) {
            push_back(other[i]);
        }
    }

    Vector(Vector&& other) noexcept : Vector() {
        swap(other);
    }

    Vector& operator=(const Vector& other) {
        Vector temp(other);
        swap(temp);
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        Vector temp(std::move(other));
        swap(temp);
        return *this;
    }

    void swap(Vector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    std::size_t size() const {
        return size_;
    }

    std::size_t capacity() const {
        return capacity_;
    }

    bool empty() const {
        return size_ == 0;
    }

    void reserve(std::size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }

        const auto limit =
            std::allocator_traits<std::allocator<T>>::max_size(allocator_);

        if (new_capacity > limit) {
            throw std::length_error("Vector is too large");
        }

        T* new_data = allocator_.allocate(new_capacity);
        std::size_t constructed = 0;

        try {
            for (; constructed < size_; ++constructed) {
                std::construct_at(
                    new_data + constructed,
                    std::move_if_noexcept(data_[constructed])
                );
            }
        } catch (...) {
            for (std::size_t i = 0; i < constructed; ++i) {
                std::destroy_at(new_data + i);
            }

            allocator_.deallocate(new_data, new_capacity);
            throw;
        }

        for (std::size_t i = 0; i < size_; ++i) {
            std::destroy_at(data_ + i);
        }

        if (data_ != nullptr) {
            allocator_.deallocate(data_, capacity_);
        }

        data_ = new_data;
        capacity_ = new_capacity;
    }

    void push_back(const T& value) {
        if (size_ == capacity_) {
            T saved(value);
            grow();
            std::construct_at(data_ + size_, saved);
        } else {
            std::construct_at(data_ + size_, value);
        }

        ++size_;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) {
            T saved(std::move(value));
            grow();
            std::construct_at(data_ + size_, std::move(saved));
        } else {
            std::construct_at(data_ + size_, std::move(value));
        }

        ++size_;
    }

    void pop_back() {
        if (empty()) {
            return;
        }

        std::destroy_at(data_ + size_ - 1);
        --size_;
    }

    void clear() {
        for (std::size_t i = 0; i < size_; ++i) {
            std::destroy_at(data_ + i);
        }

        size_ = 0;
    }

    void resize(std::size_t new_size) {
        if (new_size < size_) {
            while (size_ > new_size) {
                pop_back();
            }
        } else {
            reserve(new_size);

            while (size_ < new_size) {
                std::construct_at(data_ + size_);
                ++size_;
            }
        }
    }

    T& operator[](std::size_t index) {
        return data_[index];
    }

    const T& operator[](std::size_t index) const {
        return data_[index];
    }

    T& at(std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Vector index out of range");
        }

        return data_[index];
    }

    const T& at(std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Vector index out of range");
        }

        return data_[index];
    }

    T& front() {
        return data_[0];
    }

    const T& front() const {
        return data_[0];
    }

    T& back() {
        return data_[size_ - 1];
    }

    const T& back() const {
        return data_[size_ - 1];
    }

    class Iterator {
    private:
        T* ptr_;

    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        Iterator() : ptr_(nullptr) {}

        explicit Iterator(T* ptr) : ptr_(ptr) {}

        T& operator*() const {
            return *ptr_;
        }

        T* operator->() const {
            return ptr_;
        }

        Iterator& operator++() {
            ++ptr_;
            return *this;
        }

        Iterator operator++(int) {
            Iterator old = *this;
            ++(*this);
            return old;
        }

        Iterator& operator--() {
            --ptr_;
            return *this;
        }

        Iterator operator--(int) {
            Iterator old = *this;
            --(*this);
            return old;
        }

        Iterator& operator+=(difference_type offset) {
            if (offset != 0) {
                ptr_ += offset;
            }

            return *this;
        }

        Iterator& operator-=(difference_type offset) {
            if (offset != 0) {
                ptr_ -= offset;
            }

            return *this;
        }

        Iterator operator+(difference_type offset) const {
            Iterator result = *this;
            result += offset;
            return result;
        }

        Iterator operator-(difference_type offset) const {
            Iterator result = *this;
            result -= offset;
            return result;
        }

        difference_type operator-(const Iterator& other) const {
            if (ptr_ == other.ptr_) {
                return 0;
            }

            return ptr_ - other.ptr_;
        }

        bool operator==(const Iterator& other) const {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const Iterator& other) const {
            return ptr_ != other.ptr_;
        }

        bool operator<(const Iterator& other) const {
            return ptr_ < other.ptr_;
        }

        bool operator>(const Iterator& other) const {
            return ptr_ > other.ptr_;
        }

        bool operator<=(const Iterator& other) const {
            return ptr_ <= other.ptr_;
        }

        bool operator>=(const Iterator& other) const {
            return ptr_ >= other.ptr_;
        }

        T& operator[](difference_type offset) const {
            return *(ptr_ + offset);
        }

        friend Iterator operator+(
            difference_type offset,
            const Iterator& it
        ) {
            return it + offset;
        }
    };

    Iterator begin() {
        return Iterator(data_);
    }

    Iterator end() {
        if (empty()) {
            return Iterator(data_);
        }

        return Iterator(data_ + size_);
    }
};