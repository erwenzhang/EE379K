// Valarray.h

/* Put your solution in this file, we expect to be able to use
 * your epl::valarray class by simply saying #include "Valarray.h"
 *
 * We will #include "Vector.h" to get the epl::vector<T> class 
 * before we #include "Valarray.h". You are encouraged to test
 * and develop your class using std::vector<T> as the base class
 * for your epl::valarray<T>
 * you are required to submit your project with epl::vector<T>
 * as the base class for your epl::valarray<T>
 */

#ifndef _Valarray_h
#define _Valarray_h

#include <cmath>

// #include <vector>
// using std::vector; // during development and testing
#include "Vector.h"
using epl::vector; // after submission

namespace epl {

/* declare these up front */
template <typename T>
struct valarray;
template <typename T>
struct vexpr;

/* type alias to detect numeric types */
template <typename T> struct is_complex : public std::false_type {};
template <typename T> struct is_complex<std::complex<T>> : public std::true_type {};

/*
 * return true for all math-y types, could be more restrictive,
 * but I like the idea of allowing unsigned long long
 */
template <typename T>
using is_easy_math = typename std::enable_if<std::is_integral<T>::value || is_complex<T>::value>::type;

/*
 * Proxy should store a reference to a vector but a copy of a Proxy.
 * to_ref and Ref<T> allow us to choose between the two
 */
template<typename T>
struct to_ref { using type = T; };
template<typename T>
struct to_ref<vector<T>> { using type = vector<T>&; };
template<typename T>
struct to_ref<valarray<T>> { using type = valarray<T>&; };
template<typename T>
using Ref = typename to_ref<T>::type;

/* alias to grab a type's value_type */
template <class T>
using ValueType = typename T::value_type;

/*
 * given two types, try our best to make the right type choice
 * this becomes tricky with complex types, which is why we use
 * is_same so many times
 */
template <typename T, typename U>
using SameSame = typename std::conditional<
	std::is_same<T, const std::complex<double>>::value ||
	std::is_same<U, const std::complex<double>>::value,
	const std::complex<double>,
	typename std::conditional<
		(std::is_same<T, double>::value && is_complex<U>::value) ||
		(std::is_same<U, double>::value && is_complex<T>::value),
		const std::complex<double>,
		typename std::conditional<
			std::is_same<T, const std::complex<float>>::value ||
			std::is_same<U, const std::complex<float>>::value,
			const std::complex<float>,
			T
		>::type
	>::type
>::type;

/* given two types, choose the best promotion */
template <class T, class U>
using ConditionalComplex = typename std::conditional<
	is_complex<T>::value,
	SameSame<T, U>,
	typename std::conditional<
		is_complex<U>::value,
		SameSame<U, T>,
		T
	>::type
>::type;

/* shortcut for ConditionalComplex */
template <class T, class U>
using CondComp = ConditionalComplex<ValueType<T>, ValueType<U>>;

/* a very special vexpr, returns the same value no matter what */
template <class T>
struct UnaryVal {
	using value_type = T;
	T v;
	UnaryVal(const T v) : v(v) {}
	T operator[](size_t k) const { return v; }
	size_t len() const { return SIZE_MAX; }
	size_t size() const { return this->len(); }
};

/* unary function vexpr (used for things like negation, sqrt, etc.) */
template <class Op, class Lhs>
struct UnaryOp {
	using value_type = CondComp<Lhs, Lhs>;
	const Op op;
	const Ref<Lhs> lhs;
	UnaryOp(const Op& op, const Lhs& lhs) : op(op), lhs(const_cast<Lhs&>(lhs)) {}
	CondComp<Lhs, Lhs> operator[](size_t k) const { return op(lhs[k]); }
	size_t len() const { return lhs.len(); }
	size_t size() const { return this->len(); }
};

/* binary function vexpr (used for things like addition, subtraction, etc.) */
template <class Op, class Lhs, class Rhs>
struct BinaryOp {
	const Op op;
	const Ref<Lhs> lhs;
	const Ref<Rhs> rhs;
	using value_type = decltype(op(lhs[0], rhs[0]));
	BinaryOp(const Op& op, const Lhs& lhs, const Rhs& rhs) : op(op), lhs(const_cast<Lhs&>(lhs)), rhs(const_cast<Rhs&>(rhs)) {}
	auto operator[](size_t k) const -> decltype(op(lhs[k], rhs[k])) { return op(lhs[k], rhs[k]); }
	size_t len() const { return (lhs.len() < rhs.len()) ? lhs.len() : rhs.len(); }
	size_t size() const { return this->len(); }
};

/* unary function vexpr (used for things like apply) */
template <template <class> class Op, class T, class Lhs>
struct UnaryFunction {
	const Op<T> op;
	const Ref<Lhs> lhs;
	using value_type = typename Op<T>::result_type;
	UnaryFunction(const Op<T>& op, const Lhs& lhs) : op(op), lhs(const_cast<Lhs&>(lhs)) {}
	value_type operator[](size_t k) const { return op(static_cast<T>(lhs[k])); }
	size_t len() const { return lhs.len(); }
	size_t size() const { return this->len(); }
};

/* 
 * unary and binary functions to use in our operator
 * could have used std library functions except for
 * the weird complex conversions.
 */
template <class T>
struct unary_negate : std::unary_function<ValueType<T>, ValueType<T>> {
	ValueType<T> operator()(const ValueType<T>& x) const { return -x; }
};
template <class T>
struct unary_sqrt : std::unary_function<T, T> {
	T operator()(const T& x) const { return sqrt(x); }
};
template <class T, class U>
struct addition : std::binary_function<ValueType<T>, ValueType<U>, ValueType<T>> {
	using A = CondComp<T, U>;
	using B = CondComp<U, T>;
	auto operator()(const ValueType<T>& x, const ValueType<U>& y) const -> decltype(A(x) + B(y)) {
		return A(x) + B(y);
	}
};
template <class T, class U>
struct subtraction : std::binary_function<ValueType<T>, ValueType<U>, ValueType<T>> {
	using A = CondComp<T, U>;
	using B = CondComp<U, T>;
	auto operator()(const ValueType<T>& x, const ValueType<U>& y) const -> decltype(A(x) - B(y)) {
		return A(x) - B(y);
	}
};
template <class T, class U>
struct multiplication : std::binary_function<ValueType<T>, ValueType<U>, ValueType<T>> {
	using A = CondComp<T, U>;
	using B = CondComp<U, T>;
	auto operator()(const ValueType<T>& x, const ValueType<U>& y) const -> decltype(A(x) * B(y)) {
		return A(x) * B(y);
	}
};
template <class T, class U>
struct division : std::binary_function<ValueType<T>, ValueType<U>, ValueType<T>> {
	using A = CondComp<T, U>;
	using B = CondComp<U, T>;
	auto operator()(const ValueType<T>& x, const ValueType<U>& y) const -> decltype(A(x) / B(y)) {
		return A(x) / B(y);
	}
};

/* type aliases to verify template arguments are actually valarrays */
template<class VExpr>
struct is_vexpr : std::false_type {};
template<class VExpr>
struct is_vexpr<vexpr<VExpr>> : std::true_type {};
template<typename T>
struct is_vexpr<valarray<T>> : std::true_type {};
template<typename U>
using is_easy_vexpr = typename std::enable_if<is_vexpr<U>::value, U>::type;

/* type aliases to help determine return types (and verify we have a valarray) */
template<template<class, class> class Op, class A, class B>
using BinOp = typename std::enable_if<is_vexpr<A>::value && is_vexpr<B>::value,
	vexpr<BinaryOp<Op<A, B>, A, B>>>::type;
template<template<class> class Op, class A>
using UnOp = typename std::enable_if<is_vexpr<A>::value, vexpr<UnaryOp<Op<A>, A>>>::type;
template <typename T>
using UnVal = vexpr<UnaryVal<T>>;
template <template<class> class Op, typename A, typename U>
using UnFun = typename std::enable_if<is_vexpr<A>::value, vexpr<UnaryFunction<Op, U, A>>>::type;

/* valarray expression definition */
template<class VExpr>
struct vexpr {
	using value_type = typename VExpr::value_type;
	Ref<VExpr> v;
	vexpr(VExpr v) : v(v) {}
	vexpr(valarray<VExpr> v) : v(v) {}
	value_type operator[](size_t k) const { return v[k]; }
	size_t len() const { return v.len(); }
	size_t size() const { return this->len(); }

	/* a silly little iterator to make vexpr iterable */
	class iterator {
	public:
		using value_type = typename VExpr::value_type;
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = ptrdiff_t;
		using reference = value_type;
		using pointer = value_type*;
		Ref<VExpr> v;
		uint64_t k;

		iterator(VExpr b, uint64_t n) : v(b), k(n) {}
		iterator(const iterator& it) : v(it.v), k(it.k) {}
		iterator& operator=(const iterator& it) { v = it.v; k = it.k; return *this; }
		bool operator==(const iterator& it) const { return k == it.k; }
		bool operator!=(const iterator& it) const { return ! this->operator==(it); }
		bool operator<(const iterator& it)  const { return k < it.k; }
		bool operator>(const iterator& it)  const { return k > it.k; }
		bool operator<=(const iterator& it) const { return k <= it.k; }
		bool operator>=(const iterator& it) const { return k >= it.k; }
		iterator& operator++() { k++; return *this; }
		iterator& operator--() { k--; return *this; }
		iterator operator++(int) { iterator t{*this}; this->operator++(); return t; }
		iterator operator--(int) { iterator t{*this}; this->operator--(); return t; }
		iterator& operator+=(uint64_t d) { k += d; return *this; }
		iterator& operator-=(uint64_t d) { k -= d; return *this; }
		iterator operator+(uint64_t d) const { iterator t{*this}; t.k += d; return t; }
		iterator operator-(uint64_t d) const { iterator t{*this}; t.k -= d; return t; }
		int64_t operator-(iterator it) const { return k - it.k; }
		value_type operator*() const { return v.operator[](k); }
		value_type operator[](size_t n) const { return v.operator[](k + n); }
	};
	iterator begin() { return iterator(v, 0); }
	iterator end() { return iterator(v, this->len()); }

	/* apply and accumulate template functions */
	template <template <class> class Func, typename T>
	auto accumulate(Func<T> f) -> typename decltype(f)::result_type {
		typename decltype(f)::result_type acc(this->operator[](0));
		for (int k = 1; k < this->len(); k++) {
			acc = f(acc, this->operator[](k));
		}
		return acc;
	}
	template <template <class> class Func, typename U>
	UnFun<Func, vexpr<VExpr>, U> apply(Func<U> f) {
		using Op = UnaryFunction<Func, U, vexpr<VExpr>>;
		return vexpr<Op>(Op(f, *this));
	}
	auto sqrt() -> decltype(this->apply(unary_sqrt<value_type>())) { return this->apply(unary_sqrt<value_type>()); }
	auto sum() -> decltype(this->accumulate(std::plus<value_type>())) { return this->accumulate(std::plus<value_type>()); }
};

/* Basic declaration of valarray (inherits everything from vector) */
template <typename T>
struct valarray : public vector<T> {
	using value_type = T;
	valarray() : vector<T>() {}
	explicit valarray(size_t n) : vector<T>(n) {}
	valarray(std::initializer_list<T> il) : vector<T>(il) {}
	size_t len() const { return this->size(); }

	/* create a valarray from a vexpr */
	template <typename U, typename = is_easy_vexpr<U>>
	valarray(U v) {
		this->operator=(v);
	}

	/* not technically correct, but close enough */
	valarray& operator=(T x) {
		while (this->len() != 0) {
			this->pop_back();
		}
		for (int k = 0; k < this->len(); k++) {
			this->push_back(x);
		}
		return *this;
	}

	/* create a valarray from a vexpr */
	template <typename U, typename = is_easy_vexpr<U>>
	valarray& operator=(U v) {
		for (int k = 0; k < this->len(); k++) {
			this->operator[](k) = v[k];
		}
		for (int k = this->len(); k < v.len(); k++) {
			this->push_back(v[k]);
		}
		return *this;
	}

	template <template <class> class Func, typename U>
	auto accumulate(Func<U> f) -> typename decltype(f)::result_type {
		using V = typename decltype(f)::result_type;
		if (this->size() == 0) {
			return V{};
		}
		V acc(this->operator[](0));
		for (int k = 1; k < this->len(); k++) {
			acc = f(acc, static_cast<V>(this->operator[](k)));
		}
		return acc;
	}
	template <template <class> class Func, typename U>
	UnFun<Func, valarray<T>, U> apply(Func<U> f) {
		using Op = UnaryFunction<Func, U, valarray<T>>;
		return vexpr<Op>(Op(f, *this));
	}
	auto sqrt() -> decltype(this->apply(unary_sqrt<T>())) { return this->apply(unary_sqrt<T>()); }
	auto sum() -> decltype(this->accumulate(std::plus<T>())) { return this->accumulate(std::plus<T>()); }
};

/* the actual operators between valarrays */
template<class Expr1>
UnOp<unary_negate, Expr1> operator-(const Expr1& x) {
	using Op = UnaryOp<unary_negate<Expr1>, Expr1>;
	return vexpr<Op>(Op(unary_negate<Expr1>(), x));
}
template<class Expr1, class Expr2>
BinOp<addition, Expr1, Expr2> operator+(const Expr1& x, const Expr2& y) {
	using Op = BinaryOp<addition<Expr1, Expr2>, Expr1, Expr2>;
	return vexpr<Op>(Op(addition<Expr1, Expr2>(), x, y));
}
template<class Expr1, class Expr2>
BinOp<subtraction, Expr1, Expr2> operator-(const Expr1& x, const Expr2& y) {
	using Op = BinaryOp<subtraction<Expr1, Expr2>, Expr1, Expr2>;
	return vexpr<Op>(Op(subtraction<Expr1, Expr2>(), x, y));
}
template<class Expr1, class Expr2>
BinOp<multiplication, Expr1, Expr2> operator*(const Expr1& x, const Expr2& y) {
	using Op = BinaryOp<multiplication<Expr1, Expr2>, Expr1, Expr2>;
	return vexpr<Op>(Op(multiplication<Expr1, Expr2>(), x, y));
}
template<class Expr1, class Expr2>
BinOp<division, Expr1, Expr2> operator/(const Expr1& x, const Expr2& y) {
	using Op = BinaryOp<division<Expr1, Expr2>, Expr1, Expr2>;
	return vexpr<Op>(Op(division<Expr1, Expr2>(), x, y));
}

/* handle operations between valarrays and math-y numbers */
template <typename T, typename U, typename = is_easy_math<U>>
BinOp<addition, T, UnVal<U>> operator+(const T& a, const U& b) { return a + UnVal<U>(UnaryVal<U>(b)); }
template <typename T, typename U, typename = is_easy_math<U>>
BinOp<addition, UnVal<U>, T> operator+(const U& b, const T& a) { return UnVal<U>(UnaryVal<U>(b)) + a; }
template <typename T, typename U, typename = is_easy_math<U>>
BinOp<subtraction, T, UnVal<U>> operator-(const T& a, const U& b) { return a - UnVal<U>(UnaryVal<U>(b)); }
template <typename T, typename U, typename = is_easy_math<U>>
BinOp<subtraction, UnVal<U>, T> operator-(const U& b, const T& a) { return UnVal<U>(UnaryVal<U>(b)) - a; }
template <typename T, typename U, typename = is_easy_math<U>>
BinOp<multiplication, T, UnVal<U>> operator*(const T& a, const U& b) { return a * UnVal<U>(UnaryVal<U>(b)); }
template <typename T, typename U, typename = is_easy_math<U>>
BinOp<multiplication, UnVal<U>, T> operator*(const U& b, const T& a) { return UnVal<U>(UnaryVal<U>(b)) * a; }
template <typename T, typename U, typename = is_easy_math<U>>
BinOp<division, T, UnVal<U>> operator/(const T& a, const U& b) { return a / UnVal<U>(UnaryVal<U>(b)); }
template <typename T, typename U, typename = is_easy_math<U>>
BinOp<division, UnVal<U>, T> operator/(const U& b, const T& a) { return UnVal<U>(UnaryVal<U>(b)) / a; }

/* allow the user to print valarrays */
template <typename T, typename = is_easy_vexpr<T>>
std::ostream& operator<<(std::ostream& stream, const T& x) {
	char sep = '[';
	for (int i = 0; i < x.size(); i++) {
		stream << sep;
		stream << x[i];
		sep = ',';
	}
	stream << ']';
	return stream;
}

}

#endif /* _Valarray_h */

