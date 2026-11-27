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

template <Numeric T, typename Allocator = std::allocator<T>>
class Vector {
private:
    using Traits = std::allocator_traits<Allocator>;

    static_assert(
        std::same_as<typename Traits::value_type, T>,
        "Allocator value_type must match T"
    );

    static_assert(
        std::same_as<typename Traits::pointer, T*>,
        "Allocator must use ordinary pointers"
    );

    static_assert(
        Traits::is_always_equal::value,
        "Allocator instances must be interchangeable"
    );

    T* data_;
    std::size_t size_;
    std::size_t capacity_;
    Allocator allocator_;

    void grow() {
        const auto limit = Traits::max_size(allocator_);

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
    Vector()
        : data_(nullptr), size_(0), capacity_(0) {}

    ~Vector() {
        clear();

        if (data_ != nullptr) {
            Traits::deallocate(allocator_, data_, capacity_);
        }
    }

    Vector(const Vector& other) : Vector() {
        reserve(other.size_);

        for (std::size_t i = 0; i < other.size_; ++i) {
            push_back(other[i]);
        }
    }

    Vector(Vector&& other)
        noexcept(std::is_nothrow_default_constructible_v<Allocator>)
        : Vector() {
        swap(other);
    }

    Vector& operator=(const Vector& other) {
        Vector temp(other);
        swap(temp);
        return *this;
    }

    Vector& operator=(Vector&& other)
        noexcept(std::is_nothrow_default_constructible_v<Allocator>) {
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

        const auto limit = Traits::max_size(allocator_);

        if (new_capacity > limit) {
            throw std::length_error("Vector is too large");
        }

        T* new_data = Traits::allocate(allocator_, new_capacity);
        std::size_t constructed = 0;

        try {
            for (; constructed < size_; ++constructed) {
                Traits::construct(
                    allocator_,
                    new_data + constructed,
                    std::move_if_noexcept(data_[constructed])
                );
            }
        } catch (...) {
            for (std::size_t i = 0; i < constructed; ++i) {
                Traits::destroy(allocator_, new_data + i);
            }

            Traits::deallocate(
                allocator_,
                new_data,
                new_capacity
            );

            throw;
        }

        for (std::size_t i = 0; i < size_; ++i) {
            Traits::destroy(allocator_, data_ + i);
        }

        if (data_ != nullptr) {
            Traits::deallocate(allocator_, data_, capacity_);
        }

        data_ = new_data;
        capacity_ = new_capacity;
    }

    void push_back(const T& value)


{
        if (size_ == capacity_) {
            T saved(value);
            grow();

            Traits::construct(
                allocator_,
                data_ + size_,
                saved
            );
        } else {
            Traits::construct(
                allocator_,
                data_ + size_,
                value
            );
        }

        ++size_;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) {
            T saved(std::move(value));
            grow();

            Traits::construct(
                allocator_,
                data_ + size_,
                std::move(saved)
            );
        } else {
            Traits::construct(
                allocator_,
                data_ + size_,
                std::move(value)
            );
        }

        ++size_;
    }

    void pop_back() {
        if (empty()) {
            return;
        }

        Traits::destroy(allocator_, data_ + size_ - 1);
        --size_;
    }

    void clear() {
        for (std::size_t i = 0; i < size_; ++i) {
            Traits::destroy(allocator_, data_ + i);
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
                Traits::construct(allocator_, data_ + size_);
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

    template <bool IsConst>
    class BasicIterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        using pointer =
            std::conditional_t<IsConst, const T*, T*>;

        using reference =
            std::conditional_t<IsConst, const T&, T&>;

        using iterator_category =
            std::random_access_iterator_tag;

    private:
        pointer ptr_ = nullptr;

    public:
        BasicIterator() = default;

        explicit BasicIterator(pointer ptr) : ptr_(ptr) {}

        reference operator*() const {
            return *ptr_;
        }

        pointer operator->() const {
            return ptr_;
        }

        BasicIterator& operator++() {
            ++ptr_;
            return *this;
        }

        BasicIterator operator++(int) {
            BasicIterator old = *this;
            ++(*this);
            return old;
        }

        BasicIterator& operator--() {
            --ptr_;
            return *this;
        }

        BasicIterator operator--(int) {
            BasicIterator old = *this;
            --(*this);
            return old;
        }

        BasicIterator& operator+=(difference_type offset) {
            if (offset != 0) {
                ptr_ += offset;
            }

            return *this;
        }

        BasicIterator& operator-=(difference_type offset) {
            if (offset != 0) {
                ptr_ -= offset;
            }

            return *this;
        }

        BasicIterator operator+(difference_type offset) const {
            BasicIterator result = *this;
            result += offset;
            return result;
        }

        BasicIterator operator-(difference_type


offset) const {
            BasicIterator result = *this;
            result -= offset;
            return result;
        }

        difference_type operator-(const BasicIterator& other) const {
            if (ptr_ == other.ptr_) {
                return 0;
            }

            return ptr_ - other.ptr_;
        }

        bool operator==(const BasicIterator& other) const {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const BasicIterator& other) const {
            return ptr_ != other.ptr_;
        }

        bool operator<(const BasicIterator& other) const {
            return ptr_ < other.ptr_;
        }

        bool operator>(const BasicIterator& other) const {
            return ptr_ > other.ptr_;
        }

        bool operator<=(const BasicIterator& other) const {
            return ptr_ <= other.ptr_;
        }

        bool operator>=(const BasicIterator& other) const {
            return ptr_ >= other.ptr_;
        }

        reference operator[](difference_type offset) const {
            return *(ptr_ + offset);
        }

        friend BasicIterator operator+(
            difference_type offset,
            const BasicIterator& it
        ) {
            return it + offset;
        }
    };

    using Iterator = BasicIterator<false>;
    using ConstIterator = BasicIterator<true>;

    Iterator begin() {
        return Iterator(data_);
    }

    Iterator end() {
        return Iterator(empty() ? data_ : data_ + size_);
    }

    ConstIterator begin() const {
        return ConstIterator(data_);
    }

    ConstIterator end() const {
        return ConstIterator(empty() ? data_ : data_ + size_);
    }

    ConstIterator cbegin() const {
        return begin();
    }

    ConstIterator cend() const {
        return end();
    }
};