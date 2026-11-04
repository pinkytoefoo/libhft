#pragma once

#include <concepts>
#include <cstddef>
#include <new>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <format>
#include <utility>

namespace fby
{
template<typename T, std::size_t Capacity>
class inplace_vector
{
    static_assert(Capacity > 0, "Capacity must be greater than 0");

public:
    // iterator
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        iterator(pointer ptr) : ptr_{ptr} {}

        reference operator*() const { 
            return *ptr_; 
        }

        pointer operator->() { 
            return ptr_; 
        }

        iterator& operator++() {
            ptr_++;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ptr_++;
            return tmp;
        }

        friend bool operator==(const iterator& a, const iterator& b) { 
            return a.ptr_ == b.ptr_; 
        };
        
        friend bool operator!=(const iterator& a, const iterator& b) { 
            return a.ptr_ != b.ptr_; 
        };

    private:
        pointer ptr_;
    };

public:
    // nested types
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;

public:
    constexpr inplace_vector() noexcept = default;
    constexpr ~inplace_vector() {
        if(!std::is_trivially_destructible_v<T>) {
            while(size_ > 0) {
                std::destroy_at(raw_ptr(--size_));
            }
        }
    }
    
    constexpr explicit inplace_vector(size_t count)
        requires std::is_default_constructible_v<T>
    {
        if(count > Capacity) {
            throw std::length_error{std::format("inplace_vector count ({}) must be <= Capacity ({})!", count, Capacity)};
        }

        try {
            for(; size_ < count; ++size_) {
                std::construct_at(raw_ptr(size_));
            }
        } catch(...) {
            clear();
            throw;
        }
    }

    template<typename ...Args>
    constexpr inplace_vector(size_t count, Args&&... args)
        requires std::constructible_from<T, Args&&...>
    {
        if(count > Capacity) {
            throw std::length_error{std::format("inplace_vector count ({}) must be <= Capacity ({})!", count, Capacity)};
        }

        try {
            for(; size_ < count; ++size_) {
                std::construct_at(raw_ptr(size_), std::forward<Args>(args)...);
            }
        } catch(...) {
            clear();
            throw;
        }
    }

    inplace_vector(const inplace_vector&) = delete;
    inplace_vector& operator=(const inplace_vector&) = delete;

    inplace_vector(inplace_vector&&) = delete;
    inplace_vector& operator=(inplace_vector&&) = delete;

    template<typename ...Args>
    requires std::constructible_from<T, Args&&...>
    constexpr void emplace(const iterator pos, Args&&... args) {
        std::construct_at(pos, std::forward<Args>(args)...);
    }

    constexpr bool push_back(const T& value)
        noexcept(std::is_nothrow_copy_constructible_v<T>) 
    {
        if(size_ == Capacity) {
            return false;
        }
        
        std::construct_at(ptr(size_), value);
        ++size_;
        return true;
    }

    constexpr bool push_back(T&& value)
        noexcept(std::is_nothrow_copy_constructible_v<T>) 
    {
        if(size_ == Capacity) {
            return false;
        }
        
        std::construct_at(ptr(size_), std::move(value));
        ++size_;
        return true;
    }
     
    // in C++26's version of inplace_vector
    // calling pop_back() with no active elements
    // is either a contract violation, or UB
    // here, it is simply UB
    // to safely pop the last element,
    // use try_pop_back
    constexpr void pop_back()
        noexcept(std::is_nothrow_destructible_v<T>)
    {
        std::destroy_at(raw_ptr(--size_));
    }

    constexpr bool try_pop_back()
        noexcept(std::is_nothrow_destructible_v<T>)
    {
        if(empty()) {
            return false;
        }

        pop_back();
        return true;
    }

    constexpr void clear()
        noexcept(std::is_nothrow_destructible_v<T>)
    {
        while(size_ > 0) {
            std::destroy_at(raw_ptr(--size_));
        }
    }

    constexpr pointer data() noexcept { return ptr(0); }
    constexpr const_pointer data() const noexcept { return ptr(0); }

    constexpr reference front() noexcept { return *ptr(0); }
    constexpr const_reference back() const noexcept { return *ptr(size_ - 1); }
    
    [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
    [[nodiscard]] constexpr size_type capacity() const noexcept { return Capacity; }
    [[nodiscard]] constexpr size_type max_size() const noexcept { return Capacity; }

    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] constexpr bool full() const noexcept { return size_ == Capacity; }

    constexpr reference at(size_type idx) {
        if (idx >= size_) {
            throw std::out_of_range{"inplace_vector index out of range"};
        }

        return (*this)[idx];
    }

    constexpr reference operator[](size_type idx) noexcept { return *ptr(idx); }
    constexpr const_reference operator[](size_type idx) const noexcept { return *ptr(idx); }

    constexpr iterator begin() noexcept { return iterator(ptr(0)); }
    constexpr iterator end() noexcept { return iterator(ptr(size_)); }

private:
    T* raw_ptr(size_type idx) noexcept {
        return reinterpret_cast<T*>(storage_ + idx * sizeof(T));
    }
    
    T* ptr(size_type idx) noexcept {
        return std::launder(raw_ptr(idx));
    }

    alignas(T) std::byte storage_[sizeof(T) * Capacity];
    size_type size_{};
};
} // namespace fby
