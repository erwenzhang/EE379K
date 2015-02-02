// vector.h -- header file for vector data structure project

#pragma once
#ifndef _vector_h
#define _vector_h

#include <memory>
#include <cstdint>

namespace epl{

template <typename T>
class vector {
private:
	T* data;
	uint64_t length = 0;
	uint64_t capacity = 8;

	void amor_double(void) {
		T *tmp = data;
		capacity *= 2;
		data = (T*)operator new(sizeof(T) * capacity);
		for (uint64_t k = 0; k < length; k++) {
			new (data + k)T(std::move(tmp[k]));
			tmp[k].~T();
		}
		operator delete(tmp);
	}
public:
	vector(void)
	{
		data = (T*)operator new(sizeof(T) * capacity); 
	}

	explicit vector(uint64_t n) : length(0), capacity(n)
	{
		if (n == 0) capacity = 8;
		data = (T*)operator new(sizeof(T) * capacity); 
		for (uint64_t k = 0; k < length; k++) {
			new (data + k)T();
		}
	}

	~vector(void)
	{
		for (uint64_t k = 0; k < length; k++) {
			data[k].~T();
		}
		operator delete(data);
	}

	vector(const vector<T>& that)
	{
		capacity = that.capacity;
		length = that.length;
		data = (T*)operator new(sizeof(T) * capacity);
		for (uint64_t k = 0; k < length; k++) {
			new (data + k)T(that.data[k]);
		}
	}

	vector(const vector<T>&& that)
	{
		capacity = that.capacity;
		length = that.length;
		data = that.data;
	}

	vector<T>& operator=(const vector<T>& that)
	{
		std::swap(data, that.data);
		return *this;
	}

	vector<T>& operator=(const vector<T>&& that)
	{
		capacity = that.capacity;
		length = that.length;
		data = that.data;
		that.length = 0;
		that.data = 0;

		return *this;
	}

	T& operator[](uint64_t k)
	{
		if (k < length) {
			return data[k];
		}
		throw std::out_of_range{"index out of range"};
	}

	const T& operator[](uint64_t k) const {
		if (k < length) {
			return data[k];
		}
		throw std::out_of_range{"index out of range"};
	}

	uint64_t size(void) const {
		return length;
	}

	void push_back(const T& e)
	{
		if (length == capacity) {
			amor_double();
		}
		new (data + length) T(e);
		length++;
	}

	void push_back(const T&& e)
	{
		if (length == capacity) {
			amor_double();
		}
		new (data + length) T(std::move(e));
		length++;
	}

	void pop_back(void)
	{
		if (length == 0) {
			throw std::out_of_range{"index out of range"};
		}
		length--;
		data[length].~T();
	}


	void push_front(const T& e)
	{
		push_back(e);
	}

	void push_front(const T&& e)
	{
		push_back(e);
	}

	void pop_front(void)
	{
		pop_back();
	}
};

} //namespace epl

#endif /* _vector_h */
