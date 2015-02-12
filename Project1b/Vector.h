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

	void amor_double(const std::function <void (void)>& insert_element)
	{
		T* hold = head;
		T* told = tail;
		capacity *= 2;
		head = (T*)operator new(sizeof(T) * capacity);
		tail = head + capacity - 1;
		insert_element();
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
		capacity = that.capacity;
		head = that.head;
		tail = that.tail;
		hsize = that.hsize;
		tsize = that.tsize;
		that.head = nullptr;
		that.tail = nullptr;
		that.capacity = 0;
		that.hsize = 0;
		that.tsize = 0;
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
		if (this != &that) {
			destroy();
			copy(that);
		}
		return *this;
	}

	vector<T>& operator=(vector<T>&& that)
	{
		if (this != &that) {
			destroy();
			move(std::move(that));
		}
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
		auto insert_element = [&](void) { new (head + hsize) T(e); };
		if (hsize + tsize == capacity) {
			amor_double(insert_element);
		} else {
			insert_element();
		}
		hsize++;
	}

	void push_back(T&& e)
	{
		auto insert_element = [&](void) { new (head + hsize) T(std::move(e)); };
		if (hsize + tsize == capacity) {
			amor_double(insert_element);
		} else {
			insert_element();
		}
		hsize++;
	}

	void push_front(const T& e)
	{
		auto insert_element = [&](void) { new (tail - tsize) T(e); };
		if (hsize + tsize == capacity) {
			amor_double(insert_element);
		} else {
			insert_element();
		}
		tsize++;
	}

	void push_front(T&& e)
	{
		auto insert_element = [&](void) { new (tail - tsize) T(std::move(e)); };
		if (hsize + tsize == capacity) {
			amor_double(insert_element);
		} else {
			insert_element();
		}
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
