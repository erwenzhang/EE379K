// Vector.h -- header file for Vector data structure project

#pragma once
#ifndef _Vector_h
#define _Vector_h

namespace epl{

template <typename T>
class Vector {
	using Same = Vector<T>;
private:

public:
	Vector<T>(void) {
		// TODO: implement constructor
	}

	Vector<T>(uint64_t n) {
		// TODO: implement explicit constructor
	}

	~Vector<T>(void) {
		// TODO: implement destructor
	}

	Vector<T>(const Same& that) {
		// TODO: implement copy constructor
	}

	Vector<T>(const Same&& that) {
		// TODO: implement move constructor
	}

	Vector<T> operator=(const Same& that) {
		// TODO: implement copy operator=
	}

	Vector<T> operator=(const Same&& that) {
		// TODO: implement move operator=
	}

	T& operator[](uint64_t k) {
		// TODO: implement operator[]
	}

	const T& operator[](uint64_t k) const {
		// TODO: implement const operator[]
	}

	uint64_t size(void) const {
		// TODO: implement size
	}

	void push_back(const T&) {
		// TODO: implement push_back with copy
	}

	void push_back(const T&&) {
		// TODO: implement push_back with move
	}

	void push_front(const T&) {
		// TODO: implement push_front with copy
	}

	void push_front(const T&&) {
		// TODO: implement push_front with move
	}

	void pop_back(void) {
		// TODO: implement pop_back
	}

	void pop_front(void) {
		// TODO: implement pop_front
	}

};

} //namespace epl

#endif /* _Vector_h */
