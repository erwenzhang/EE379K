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

#include <vector>

using std::vector; // during development and testing
// using epl::vector; // after submission

namespace epl {

template <typename T>
struct valarray;

/*
 * type alias to detect numeric types
 */
template <typename T> struct is_complex : public std::false_type {};
template <typename T> struct is_complex<std::complex<T>> : public std::true_type {};

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

template <class T>
using ValueType = typename T::value_type;

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
			typename std::conditional<
				std::is_same<T, const std::complex<long long>>::value ||
				std::is_same<U, const std::complex<long long>>::value,
				const std::complex<long long>,
				typename std::conditional<
					std::is_same<T, const std::complex<long>>::value ||
					std::is_same<U, const std::complex<long>>::value,
					const std::complex<long>,
					T
				>::type
			>::type
		>::type
	>::type
>::type;

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

template <class T, class U>
using CondComp = ConditionalComplex<ValueType<T>, ValueType<U>>;

template <class T>
struct UnaryVal {
	using value_type = T;
	T v;
	UnaryVal(const T v) : v(v) {}
	T operator[](size_t k) const { return v; }
	size_t size() const { return SIZE_MAX; }
};

template <class Op, class Lhs>
struct UnaryOp {
	using value_type = CondComp<Lhs, Lhs>;
	const Op op;
	const Ref<Lhs> lhs;
	UnaryOp(const Op& op, const Lhs& lhs) : op(op), lhs(const_cast<Lhs&>(lhs)) {}
	CondComp<Lhs, Lhs> operator[](size_t k) const { return op(lhs[k]); }
	size_t size() const { return lhs.size(); }
};

template <class Op, class Lhs, class Rhs>
struct BinaryOp {
	const Op op;
	const Ref<Lhs> lhs;
	const Ref<Rhs> rhs;
	using value_type = decltype(op(lhs[0], rhs[0]));
	BinaryOp(const Op& op, const Lhs& lhs, const Rhs& rhs) : op(op), lhs(const_cast<Lhs&>(lhs)), rhs(const_cast<Rhs&>(rhs)) {}
	auto operator[](size_t k) const -> decltype(op(lhs[k], rhs[k])) { return op(lhs[k], rhs[k]); }
	size_t size() const { return std::min(lhs.size(), rhs.size()); }
};

template <class T>
struct unary_negate : std::unary_function<ValueType<T>, ValueType<T>> {
	ValueType<T> operator()(const ValueType<T>& x) const { return -x; }
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

template<class VExpr>
struct vexpr {
	using value_type = typename VExpr::value_type;
	Ref<VExpr> v;
	vexpr(VExpr v) : v(v) {}
	vexpr(valarray<VExpr> v) : v(v) {}
	value_type operator[](size_t k) const { return v[k]; }
	size_t size() const { return v.size(); }
};

template<class VExpr>
struct is_vexpr : std::false_type {};
template<class VExpr>
struct is_vexpr<vexpr<VExpr>> : std::true_type {};
template<typename U>
using is_easy_vexpr = typename std::enable_if<is_vexpr<U>::value, U>::type;

template<template<class, class> class Op, class A, class B>
using BinOp = typename std::enable_if<is_vexpr<A>::value && is_vexpr<B>::value,
	vexpr<BinaryOp<Op<A, B>, A, B>>>::type;

template<template<class> class Op, class A>
using UnOp = typename std::enable_if<is_vexpr<A>::value, vexpr<UnaryOp<Op<A>, A>>>::type;

template <typename T>
using UnVal = vexpr<UnaryVal<T>>;

/* Basic declaration of valarray (inherits everything from vector) */
template <typename T>
struct valarray : public vector<T> {
	using value_type = T;
	valarray() : vector<T>(){}
	valarray(uint64_t n) : vector<T>(n) {}
	valarray(std::initializer_list<T> il) : vector<T>(il) {}

	virtual valarray& operator=(T x) {
		this->clear();
		for (int k = 0; k < this->capacity(); k++) {
			this->push_back(x);
		}
		return *this;
	}

	template <typename U, typename = is_easy_vexpr<U>>
	valarray& operator=(U v) {
		if (this->capacity() == 0) {
			this->resize(v.size());
		}
		for (int k = 0; k < this->capacity(); k++) {
			this->at(k) = v[k];
		}
		return *this;
	}

	UnOp<unary_negate, valarray<T>> operator-();
	template <typename U>
	BinOp<addition, valarray<T>, vexpr<U>> operator+(vexpr<U> o);
	template <typename U>
	BinOp<subtraction, valarray<T>, vexpr<U>> operator-(vexpr<U> o);
	template <typename U>
	BinOp<multiplication, valarray<T>, vexpr<U>> operator*(vexpr<U> o);
	template <typename U>
	BinOp<division, valarray<T>, vexpr<U>> operator/(vexpr<U> o);
};

template<typename T>
struct is_vexpr<valarray<T>> : std::true_type {};

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

}

#endif /* _Valarray_h */

