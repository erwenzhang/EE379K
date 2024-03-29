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
	T* head = nullptr;
	T* tail = nullptr;
	uint64_t hsize = 0;
	uint64_t tsize = 0;
	uint64_t capacity = 8;

	void amor_double(void)
	{
		T* hold = head;
		T* told = tail;
		capacity *= 2;
		head = (T*)operator new(sizeof(T) * capacity);
		tail = head + capacity - 1;
		for (uint64_t k = 0; k < hsize; k++) {
			new (head + k)T(std::move(hold[k]));
		}
		for (uint64_t k = 0; k < tsize; k++) {
			new (tail - k)T(std::move(told[-k]));
		}
		operator delete(hold);
	}

	void destroy(void)
	{
		for (uint64_t k = 0; k < hsize; k++) {
			head[k].~T();
		}
		for (uint64_t k = 0; k < tsize; k++) {
			tail[-k].~T();
		}
		operator delete(head);
	}

	void copy(const vector<T>& that)
	{
		if (this != &that) {
			destroy();
			capacity = that.capacity;
			hsize = that.hsize;
			tsize = that.tsize;
			head = (T*)operator new(sizeof(T) * capacity);
			tail = head + capacity - 1;
			for (uint64_t k = 0; k < hsize; k++) {
				new (head + k)T(that.head[k]);
			}
			for (uint64_t k = 0; k < tsize; k++) {
				new (tail - k)T(that.tail[-k]);
			}
		}
	}

	void move(vector<T>&& that)
	{
		std::swap(capacity, that.capacity);
		std::swap(head, that.head);
		std::swap(tail, that.tail);
		std::swap(hsize, that.hsize);
		std::swap(tsize, that.tsize);
	}

	T& lookup(uint64_t k)
	{
		if (k < tsize) return tail[k - tsize + 1];
		else if (k < hsize + tsize) return head[k - tsize];
		else throw std::out_of_range{"index out of range"};
	}
public:
	vector(void)
	{
		head = (T*)operator new(sizeof(T) * capacity);
		tail = head + capacity - 1;
	}

	explicit vector(uint64_t n) : hsize(n), capacity(n)
	{
		if (n == 0) capacity = 8;
		head = (T*)operator new(sizeof(T) * capacity);
		tail = head + capacity - 1;
		for (uint64_t k = 0; k < hsize; k++) {
			new (head + k)T();
		}
	}

	~vector(void)
	{
		destroy();
	}

	vector(const vector<T>& that)
	{
		copy(that);
	}

	vector(vector<T>&& that)
	{
		*this = std::move(that);
	}

	vector<T>& operator=(vector<T>& that)
	{
		copy(that);
		return *this;
	}

	vector<T>& operator=(vector<T>&& that)
	{
		move(std::move(that));
		return *this;
	}

	T& operator[](uint64_t k)
	{
		return lookup(k);
	}

	const T& operator[](uint64_t k) const
	{
		return lookup(k);
	}

	uint64_t size(void) const
	{
		return hsize + tsize;
	}

	void push_back(const T& e)
	{
		if (hsize + tsize == capacity) {
			amor_double();
		}
		new (head + hsize) T(e);
		hsize++;
	}

	void push_back(const T&& e)
	{
		if (hsize + tsize == capacity) {
			amor_double();
		}
		new (head + hsize) T(std::move(e));
		hsize++;
	}

	void push_front(const T& e)
	{
		if (hsize + tsize == capacity) {
			amor_double();
		}
		new (tail - tsize) T(e);
		tsize++;
	}

	void push_front(const T&& e)
	{
		if (hsize + tsize == capacity) {
			amor_double();
		}
		new (tail - tsize) T(std::move(e));
		tsize++;
	}

	void pop_back(void)
	{
		if (hsize == 0) {
			throw std::out_of_range{"index out of range"};
		}
		hsize--;
		head[hsize].~T();
	}

	void pop_front(void)
	{
		if (tsize == 0) {
			throw std::out_of_range{"index out of range"};
		}
		tsize--;
		tail[-tsize].~T();
	}
};

} //namespace epl

#endif /* _vector_h */
