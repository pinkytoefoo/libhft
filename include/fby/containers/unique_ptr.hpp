// https://godbolt.org/z/T8rcGWhYc

#pragma once

#include <memory>
#include <new>
#include <utility>
#include <iostream>
#include <type_traits>

namespace fby
{
    template<typename T>
    struct default_deleter {
        constexpr void operator()(T* ptr) const noexcept {
            delete ptr; // deleting nullptr is safe since C++98
        }
    };

    // TODO: create array specialization
    template<typename T, typename Deleter = default_deleter<T>>
    class unique_ptr
    {
        static_assert(
            std::is_nothrow_invocable_v<Deleter&, T*>,
            "Deleter must be nothrow invocable"
        );

    public:
        constexpr unique_ptr() = default;
        constexpr unique_ptr(std::nullptr_t)
        {
        }

        explicit constexpr unique_ptr(T* ptr, const Deleter& deleter = Deleter())
            noexcept(std::is_nothrow_constructible_v<Deleter, const Deleter&>)
            : ptr_{ ptr }
            , deleter_{ deleter }
        {
        }

        constexpr ~unique_ptr() noexcept {
            reset();
        }

        unique_ptr(const unique_ptr&) = delete;
        unique_ptr& operator=(const unique_ptr&) = delete;

        constexpr unique_ptr(unique_ptr&& other)
            noexcept(std::is_nothrow_move_constructible_v<Deleter>)
            : ptr_{ std::exchange(other.ptr_, nullptr) }
            , deleter_{ std::move(other.deleter_) }
        {
        }

        constexpr unique_ptr& operator=(unique_ptr&& rhs)
            noexcept(std::is_nothrow_move_assignable_v<Deleter>)
        {
            if(this == &rhs) {
                return *this;
            }

            reset();
            deleter_ = std::move(rhs.deleter_);
            ptr_ = std::exchange(rhs.ptr_, nullptr);

            return *this;
        }

        constexpr unique_ptr& operator=(std::nullptr_t) noexcept
        {
            reset();
            return *this;
        }

        // note: calling reset(get()) is UB
        constexpr void reset(T* new_ptr = nullptr) noexcept
        {
            deleter_(ptr_);
            ptr_ = new_ptr;
        }

        [[nodiscard]] constexpr T* release() noexcept {
            return std::exchange(ptr_, nullptr);
        }

        [[nodiscard]] constexpr T* operator->() const noexcept { return ptr_; }
        [[nodiscard]] constexpr T& operator*() const noexcept { return *ptr_; }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return ptr_ != nullptr; }
        [[nodiscard]] constexpr T* get() const noexcept { return ptr_; }

        constexpr auto operator<=>(const unique_ptr& other) const {
            return ptr_ <=> other.ptr_;
        }

        constexpr bool operator ==(const unique_ptr& other) const {
            return ptr_ == other.ptr_;
        }

        constexpr void swap(unique_ptr& other)
            noexcept(std::is_nothrow_swappable_v<Deleter>)
        {
            using std::swap;
            swap(ptr_, other.ptr_);
            swap(deleter_, other.deleter_);
        }

        constexpr friend void swap(unique_ptr& first, unique_ptr& second)
            noexcept(std::is_nothrow_swappable_v<Deleter>)
        {
            using std::swap;
            first.swap(second);
        }

    private:
        T* ptr_{ nullptr };
        [[no_unique_address]] Deleter deleter_{ Deleter() };
    };

    template<typename T, typename Deleter = default_deleter<T>, typename ...Args>
    constexpr unique_ptr<T, Deleter> make_unique(Args&&... args) {
        return unique_ptr<T, Deleter>(new T(std::forward<Args>(args)...));
    }
}
