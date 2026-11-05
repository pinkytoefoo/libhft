#include <concepts>
#include <cstddef>
#include <new>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <format>

namespace hft
{
template<typename T, std::size_t Capacity>
class inplace_vector
{
    static_assert(Capacity > 0, "Capacity must be greater than 0");

public:
    // iterator
    // todo: add const_iterator
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

        pointer get() const noexcept {
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
            // using a seperate inlined function to avoid unecessarilly setting size_ to 0
            // probably would've been optimized out anyways, but still
            naive_clear_();
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
    constexpr void emplace(const iterator pos, Args&&... args)
        requires std::constructible_from<T, Args&&...>
    {
        std::construct_at(pos.get(), std::forward<Args>(args)...);
    }

    constexpr bool push_back(const T& value)
        noexcept(std::is_nothrow_copy_constructible_v<T>) 
    {
        if(size_ == Capacity) {
            return false;
        }
        
        std::construct_at(raw_ptr(size_), value);
        ++size_;
        return true;
    }

    constexpr bool push_back(T&& value)
        noexcept(std::is_nothrow_copy_constructible_v<T>) 
    {
        if(size_ == Capacity) {
            return false;
        }
        
        std::construct_at(raw_ptr(size_), std::move(value));
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

    // if the type is not trivially destructible,
    // then do the cleanup with destroy_at
    // however, if it is not, we simply set the size_ to 0
    // this has some implications
    // to start, any dangling references / iterators to once
    // active lifetimes are now invalid and considered UB if used
    // however, they may still seem to be valid due to the memory being intact.
    constexpr void clear()
        noexcept(std::is_nothrow_destructible_v<T>)
    {
        if(std::is_trivially_destructible_v<T>) {
            naive_clear_();
        } else {
            size_ = 0;
        }
    }

    constexpr pointer data() noexcept { return valid_ptr(0); }
    constexpr const_pointer data() const noexcept { return valid_ptr(0); }

    constexpr reference front() noexcept { return *valid_ptr(0); }
    constexpr reference back() noexcept { return *valid_ptr(size_ - 1); }
    
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

    constexpr reference operator[](size_type idx) noexcept { return *valid_ptr(idx); }
    constexpr const_reference operator[](size_type idx) const noexcept { return *valid_ptr(idx); }

    constexpr iterator begin() noexcept { return iterator(valid_ptr(0)); }
    constexpr iterator end() noexcept { return iterator(valid_ptr(size_)); }

private:
    void inline naive_clear_() {
        while(size_ > 0) {
            std::destroy_at(raw_ptr(--size_));
        }
    }

    T* raw_ptr(size_type idx) noexcept {
        return reinterpret_cast<T*>(storage_ + idx * sizeof(T));
    }

    T* valid_ptr(size_type idx) noexcept {
        return std::launder(raw_ptr(idx));
    }

    alignas(T) std::byte storage_[sizeof(T) * Capacity];
    size_type size_{};
};
}
