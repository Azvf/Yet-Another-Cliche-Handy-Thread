#pragma once

#include <type_traits>
#include <utility>
#include <cassert>

#include "template_util.h"

namespace ustl {
	struct nullopt_t {
		constexpr explicit nullopt_t() = default;
	};
#if _HAS_CXX17  
	inline nullopt_t nullopt{};
#else
	static nullopt_t nullopt{};
#endif

	struct in_place_t {
		constexpr explicit in_place_t() = default;
	};
#if _HAS_CXX17  
	inline in_place_t in_place{};
#else
	static in_place_t in_place{};
#endif

	template<class T>
	class optional;

	template<class T, class... Ts>
	static optional<T> make_optional(Ts&&... ts) {
		return optional<T>(in_place, std::forward<Ts>(ts)...);
	}

	template<class T>
	class optional final {
		bool has_value_;
		union {
			T value_;
		};

		template <class Ty2>
		using allow_direct_conversion = 
			std::bool_constant<
				std::conjunction_v<
					std::negation<std::is_same<ustl::remove_cvref_t<Ty2>, optional>>,
					std::negation<std::is_same<ustl::remove_cvref_t<Ty2>, in_place_t>>,
					std::negation<
						std::conjunction<
							std::is_same<std::remove_cv_t<T>, bool>, 
							ustl::is_specialization<ustl::remove_cvref_t<Ty2>, optional>
						>
					>,
					std::is_constructible<T, Ty2>
				>
			>;

		template <class _Ty2>
		struct allow_unwrapping : 
			std::bool_constant<
				std::disjunction_v<
					std::is_same<std::remove_cv_t<T>, bool>,
					std::negation<
						std::disjunction<
							//std::is_same<T, _Ty2>, 
							std::is_constructible<T, optional<_Ty2>&>,
							std::is_constructible<T, const optional<_Ty2>&>, 
							std::is_constructible<T, const optional<_Ty2>>,
							std::is_constructible<T, optional<_Ty2>>, 
							std::is_convertible<optional<_Ty2>&, T>,
							std::is_convertible<const optional<_Ty2>&, T>, 
							std::is_convertible<const optional<_Ty2>, T>,
							std::is_convertible<optional<_Ty2>, T>
						>
					>
				>
			> {};

	public:
		optional() : has_value_(false) {}

		optional(nullopt_t) : has_value_(false) {}

		template <
			class Ty2 = T, 
			std::enable_if_t<allow_direct_conversion<Ty2>::value, int> = 0
		>
		optional(Ty2&& val) : has_value_(true), value_(std::forward<Ty2>(val)) {}

		template <class Ty2,
			std::enable_if_t<
				std::conjunction_v<
					allow_unwrapping<Ty2>/*, 
					std::is_constructible<T, const Ty2&>*/
				>
			, int> = 0
		>
		optional(const optional<Ty2>& other) : has_value_(other.has_value_) {
			if (has_value_) {
				new (&this->value_) T(other.value_);
			}
		}

		template <class Ty2,
			std::enable_if_t<
				std::conjunction_v<
					allow_unwrapping<Ty2>/*,
					std::is_constructible<T, const Ty2&>*/
				>
			, int> = 0
		>
		optional(optional<Ty2>&& other) : has_value_(other.has_value_) {
			if (has_value_) {
				new (&this->value_) T(std::move(other.value_));
			}
		}

		template<class... Ts>
		explicit optional(in_place_t, Ts&&... ts) : has_value_(true), value_(std::forward<Ts>(ts)...) {}

		template<class... Ts>
		explicit optional(in_place_t) : has_value_(false) {}

		~optional() {
			if (has_value_) {
				value_.~T();
			}
		}

	public:
		optional& operator=(const optional& other) noexcept {
			if (this == &other) {
				return *this;
			}

			if (has_value_) {
				value_.~T();
				has_value_ = false;
			}

			has_value_ = other.has_value_;
			if (has_value_) {
				new (&value_) T(other.value_);
			}

			return *this;
		}

		optional& operator=(optional&& other) noexcept {
			if (this == &other) {
				return *this;
			}

			if (has_value_) {
				value_.~T();
				has_value_ = false;
			}

			has_value_ = other.has_value_;
			if (has_value_) {
				new (&value_) T(std::move(other.value_));
			}

			return *this;
		}

		optional& operator=(const T& val) noexcept {
			new (&value_) T(val);
			has_value_ = true;
			return *this;
		}

		template <class T2 = T>
		optional& operator=(T2&& val) noexcept {
			new (&value_) T(std::move(val));
			has_value_ = true;
			return *this;
		}

		optional& operator=(nullopt_t) noexcept {
			return optional<T>();
		}

		bool operator==(const T& val) noexcept {
			if (has_value_) {
				return value_ == val;
			}
			return false;
		}

		bool operator==(T&& val) noexcept {
			if (has_value_) {
				return value_ == val;
			}
			return false;
		}

		bool operator!=(const T& val) noexcept {
			if (has_value_) {
				return value_ != val;
			}
			return false;
		}

		bool operator!=(T&& val) noexcept {
			if (has_value_) {
				return value_ != val;
			}
			return false;
		}

		T* operator->() noexcept {
			return const_cast<T*>(&value());
		}

		explicit operator bool() noexcept {
			return has_value_;
		}

		T& operator*() noexcept {
			return value_;
		}

		const T& operator*() const noexcept {
			return value_;
		}

	public:
		bool has_value() { return has_value_; }

		constexpr const T& value() const& {
			if (!has_value_) {
				throw std::runtime_error("optional no value");
			}
			return value_; 
		}
		
		constexpr T& value() & {
			if (!has_value_) {
				throw std::runtime_error("optional no value");
			}
			return value_;
		}

		template<class... Ts>
		optional& emplace(Ts&&... ts) {
			new (&value_) T(std::forward<Ts>(ts)...);
			has_value_ = true;
			return *this;
		}

		template<
			class T2/*,
			std::enable_if_t<std::is_convertible_v<T, T2>, int> = 0*/
		>
		T value_or(T2&& default_value) const& {
			if (!has_value_) {
				return default_value;
			}
			return value_;
		}

		template<
			class T2/*,
			std::enable_if_t<std::is_convertible_v<T, T2>, int> = 0*/
		>
		T value_or(T2&& default_value) && {
			if (!has_value_) {
				return std::move(default_value);
			}
			return value_;
		}

		void reset() {
			*this = optional<T>();
		}

		void reset(const optional& other) {
			*this = other;
		}

		void reset(optional&& other) {
			*this = std::move(other);
		}

		void reset(const T& other) {
			new (&value_) T(other);
			has_value_ = true;
		}

		void reset(T&& other) {
			new (&value_) T(std::move(other));
			has_value_ = true;
		}

		template<class F>
		optional or_else(F&& f) const& {
			if (has_value_) {
				return *this;
			}
			return std::invoke(std::forward<F>(f));
		}

		template<class F>
		optional or_else(F&& f) && {
			if (has_value_) {
				return std::move(*this);
			}
			return std::invoke(std::forward<F>(f));
		}

		template<class F>
		optional and_then(F&& f) const& {
			if (has_value_) {
				return std::invoke(std::forward<F>(f));
			}
			return *this;
		}

		template<class F>
		optional and_then(F&& f)&& {
			if (has_value_) {
				return std::invoke(std::forward<F>(f));
			}
			return std::move(*this);
		}
	};
}

namespace ustl {
	template<class Px>
	class shared_ptr;

	template<class Dx>
	class default_deleter {
	public:
		constexpr default_deleter() = default;
		void operator()(Dx* dx) {
			static_assert(sizeof(Dx) > 0);
			delete dx;
		}
	};

	class ref_counter_base {
		uint32_t uses;
	public:
		ref_counter_base() : uses(1) {}
		~ref_counter_base() = default;

		virtual void destroy() = 0;

		uint32_t use_count() { return uses; }

		virtual void* get_deleter(const type_info& info) { return nullptr; }

		void incr() { ++uses; }

		void decr() {
			if (!(--uses)) {
				destroy();
			}
		}
	};
	template<class Px>
	class ptr_base {
	public:
		using elem_type = std::remove_extent_t<Px>;
	protected:
		elem_type* raw_ptr = nullptr;
		ref_counter_base* rep = nullptr;
	public:
		ptr_base(elem_type* px) : raw_ptr(px) {}
		ptr_base(const ptr_base& other) = delete;
		ptr_base& operator=(const ptr_base& other) = delete;

		uint32_t use_count() {
			return rep ? rep->use_count() : 0;
		}

		elem_type* get() { return raw_ptr; }

	protected:
		constexpr ptr_base() = default;

		void set_ptr_rep(elem_type* px, ref_counter_base* rx) {
			raw_ptr = px;
			rep = rx;
		}

		void incr() {
			if (rep) {
				rep->incr();
			}
		}

		void decr() {
			if (rep) {
				rep->decr();
			}
		}

		template<class Px2>
		void copy_construct_from(const shared_ptr<Px2>& other) {
			if (other.rep) {
				other.rep->incr();
			}

			raw_ptr = other.raw_ptr;
			rep = other.rep;
		}

		template<class Px2>
		void move_construct_from(ptr_base<Px2>&& other) {
			raw_ptr = other.raw_ptr;
			rep = other.rep;

			other.raw_ptr = nullptr;
			other.rep = nullptr;
		}

		void swap(ptr_base& other) {
			std::swap(other.raw_ptr, raw_ptr);
			std::swap(other.rep, rep);
		}

	private:
		template<class PxN>
		friend class ptr_base;
	};

	template<class Px>
	class ref_counter : public ref_counter_base {
		Px* raw_ptr;
	public:
		explicit ref_counter(Px* px) : ref_counter_base(), raw_ptr(px) {}
		~ref_counter() = default;

	private:
		void destroy() override {
			delete raw_ptr;
		}
	};

	template<class Resource, class Dx>
	class ref_counter_resouce : public ref_counter_base {
		std::pair<Dx, Resource> my_pair;
	public:
		ref_counter_resouce(Resource res, Dx dt)
			: ref_counter_base(), my_pair(std::move(dt), res) {}

	public:
		void set_ref_counter(Resource res, Dx dt) {
			resource = res;
			deleter = std::move(dt);
		}

		void destroy() override {
			my_pair.first()(my_pair.second());
		}

		void* get_deleter(const type_info& info) override {
			if (info == typeid(Dx)) {
				return const_cast<Dx*>(std::addressof(my_pair.first()));
			}
			return nullptr;
		}

	};

	template<
		class Px,
		class... Args
	>
	static shared_ptr<Px> make_shared(Args&&... args) {
		return shared_ptr<Px>(new Px(std::forward<Args>(args)...));
	}

	template<class Px>
	class shared_ptr : public ptr_base<Px> {
		template<
			class Px,
			class... Args
		>
		friend shared_ptr<Px> make_shared(Args&&... args);

	public:
		shared_ptr(Px* px) {
			setp(px, std::is_array<Px>{});
		}

		shared_ptr(const shared_ptr& other) {
			this->copy_construct_from(other);
		}

		shared_ptr(shared_ptr&& right) {
			this->move_construct_from(std::move(right));
		}

		~shared_ptr() {
			this->decr();
		}

		shared_ptr& operator=(const shared_ptr& other) {
			shared_ptr(other).swap(*this);
			return *this;
		}

		shared_ptr& operator=(shared_ptr&& other) {
			shared_ptr(std::move(other)).swap(*this);
			return *this;
		}
		
		shared_ptr& operator=(nullopt_t) {
			shared_ptr().swap(*this);
			return *this;
		}

		auto operator->() const& {
			return raw_ptr;
		}
		
		auto operator->() && {
			return raw_ptr;
		}

		auto operator*() const& {
			return *raw_ptr;
		}

		auto operator*() && {
			return *raw_ptr;
		}

	private:
		// true_type indicates array ptr or not
		void setp(Px* px, std::true_type) {
			// todo
			assert(0);
		}

		void setp(Px* px, std::false_type) {
			try {
				this->set_ptr_rep(px, new ref_counter<Px>(px));
			}
			catch (const std::exception& e) {
				throw;
			}
		}

		template<class Dx>
		void setpd(Px*, Dx dt) {
			try {
				this->set_ptr_rep(px, new ref_counter_resouce<Px, Dx>(px, std::move(dt)));
			}
			catch (const std::exception& e) {
				throw;
			}
		}

	};

}

namespace ustl {
	template<class T, class Alloc = std::allocator<T>>
	class Vector {
		T* m_data;
		size_t m_size;
		size_t m_cap;
		Alloc m_alloc;

	public:
		Vector() : m_data(nullptr), m_size(0), m_cap(0) {}

		explicit Vector(size_t n,  Alloc& alloc = Alloc()) : m_alloc(alloc) {
			m_data = alloc.allocate(n);
			m_size = m_cap = n;
			for (int i = 0; i < m_size; i++) {
				new (&m_data[i]) T();
			}
		}

		Vector(size_t n, const T& val,  Alloc& alloc = Alloc()) : m_alloc(alloc) {
			m_data = alloc.allocate(n);
			m_size = m_cap = n;
			for (int i = 0; i < m_size; i++) {
				new (&m_data[i]) T(val);
			}
		}

		~Vector() {
			clear();
		}

	public:
		T& operator[](size_t i) const {
			return m_data[i];
		}

		T& operator[](size_t i) {
			return m_data[i];
		}

	public:
		void clear() {
			for (int i = 0; i < m_size; i++) {
				m_data[i].~T();
			}
			m_size = 0;
		}

		void resize(size_t n) {
			reserve(n);

			if (n < m_size) {
				for (int i = n; i < m_size; i++) {
					m_data[i].~T();
				}
			}
			else if (n > m_size) {
				for (int i = m_size; i < n; i++) {
					new (&m_data[i]) T();
				}
			}

			m_size = n;
		}

		void reserve(size_t n) {
			if (n < m_cap) {
				return;
			}

			T* old_data = m_data;
			size_t old_size = m_size;
			size_t old_cap = m_cap;

			// alloc new sapce
			m_cap = std::max(n, m_cap * 2);

			m_data = m_alloc.allocate(m_cap);

			for (int i = 0; i < old_size; i++) {
				new (&m_data[i]) T(std::move(old_data[i]));
			}
			for (int i = old_size; i < m_cap; i++) {
				new (&m_data[i]) T();
			}

			// dealloc old space
			for (int i = 0; i < old_cap; i++) {
				old_data[i].~T();
			}
			m_alloc.deallocate(old_data, old_cap);
		}

		size_t capacity() { return m_cap; }

		size_t size() { return m_size; }

		void push_back(const T& val) {
			reserve(m_size + 1);
			new (&m_data[m_size++]) T(val);
		}

		void push_back(T&& val) {
			reserve(m_size + 1);
			new (&m_data[m_size++]) T(std::move(val));
		}

		template<class... Ts>
		void emplace_back(Ts&&... ts) {
			reserve(m_size + 1);
			new (&m_data[m_size++]) T(std::forward<Ts>(ts)...);
		}

		T& back() {
			assert(m_size > 0);
			return m_data[m_size - 1];
		}

	};

}