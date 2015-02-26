// vector.h -- header file for vector data structure project

#pragma once
#ifndef _vector_h
#define _vector_h

#include <stdexcept>
#include <utility>
#include <memory>
#include <cstdint>

//Utility gives std::rel_ops which will fill in relational
//iterator operations so long as you provide the
//operators discussed in class.  In any case, ensure that
//all operations listed in this website are legal for your
//iterators:
//http://www.cplusplus.com/reference/iterator/RandomAccessIterator/
using namespace std::rel_ops;

namespace epl {

class invalid_iterator {
public:
	enum SeverityLevel {SEVERE,MODERATE,MILD,WARNING};
	SeverityLevel level;

	invalid_iterator(SeverityLevel level = SEVERE){ this->level = level; }
	virtual const char* what() const {
		switch (level) {
		case WARNING:   return "Warning"; // not used in Spring 2015
		case MILD:      return "Mild";
		case MODERATE:  return "Moderate";
		case SEVERE:    return "Severe";
		default:        return "ERROR"; // should not be used
		}
	}
};

template <typename T>
class vector {
private:
	T* head = nullptr;
	T* tail = nullptr;
	uint64_t hsize = 0;
	uint64_t tsize = 0;
	uint64_t capacity = 8;

	void amor_double(const std::function <void (void)>& insert_element) {
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
		modver++;
	}

	void destroy(void) {
		for (uint64_t k = 0; k < hsize; k++) {
			head[k].~T();
		}
		for (uint64_t k = 0; k < tsize; k++) {
			tail[-k].~T();
		}
		operator delete(head);
	}

	void copy(const vector<T>& that) {
		if (this != &that) {
			destroy();
			capacity = that.capacity;
			hsize = that.hsize;
			tsize = that.tsize;
			ver = that.ver;
			modver = that.modver;
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

	void move(vector<T>&& that) {
		if (this != &that) {
			destroy();
			capacity = that.capacity;
			head = that.head;
			tail = that.tail;
			hsize = that.hsize;
			tsize = that.tsize;
			ver = that.ver;
			modver = that.modver;
			that.head = nullptr;
			that.tail = nullptr;
			that.capacity = 0;
			that.hsize = 0;
			that.tsize = 0;
			that.ver += 1;
		}
	}

	T& lookup(uint64_t k) const {
		if (k < tsize) return tail[k - tsize + 1];
		else if (k < hsize + tsize) return head[k - tsize];
		else throw std::out_of_range{"index out of range"};
	}

	template <typename I>
	void construct(I b, I e, std::forward_iterator_tag t, T v) {
	    head = (T*)operator new(sizeof(T) * capacity);
		tail = head + capacity - 1;
		for (auto it = b; it != e; ++it) {
			push_back(*it);
		}
	}

	template <typename I>
	void construct(I b, I e, std::random_access_iterator_tag t, T v) {
		capacity = e - b;
		hsize = capacity;
	    head = (T*)operator new(sizeof(T) * capacity);
		tail = head + capacity - 1;
		for (uint64_t k = 0; k < hsize; k++) {
			new (head + k)T(b[k]);
		}
	}


public:
	uint64_t ver = rand();
	uint64_t modver = rand();

	class iterator {
	private:
		void validate(bool deref=false, uint64_t access=0) const {
			if (v == nullptr) {
				throw invalid_iterator{invalid_iterator::SEVERE};
			}
			if (deref && (k >= v->size())) {
				throw invalid_iterator{invalid_iterator::SEVERE};
			}
			if (deref && (this->modver != v->modver)) {
				if (this->ver != v->ver || access >= dirty_size || dirty[access]) {
					throw invalid_iterator{invalid_iterator::MODERATE};
				}
			}
			if (this->ver != v->ver) {
				throw invalid_iterator{invalid_iterator::MILD};
			}
		}
	public:
		using value_type = T;
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = ptrdiff_t;
		using reference = T;
		using pointer = T*;

		vector* v;
		uint64_t k;
		uint64_t ver;
		uint64_t modver;
		bool *dirty;
		uint64_t dirty_size;

		iterator(vector* b, uint64_t n) {
			v = b;
			k = n;
			ver = b->ver;
			modver = b->modver;
			dirty_size = v->size();
			dirty = new bool[dirty_size]();
		}
		iterator(const iterator& it) {
			v = it.v;
			k = it.k;
			ver = it.ver;
			modver = it.modver;
			dirty_size = v->size();
			dirty = new bool[dirty_size]();
			std::copy(it.dirty, it.dirty + it.dirty_size, dirty);
		}
		~iterator() {
			delete[] dirty;
		}
		iterator& operator=(const iterator& it) {
			it.validate();
			v = it.v;
			k = it.k;
			ver = it.ver;
			modver = it.modver;
			dirty_size = it.dirty_size;
			dirty = new bool[dirty_size]();
			std::copy(it.dirty, it.dirty + it.dirty_size, dirty);
			return *this;
		}
		bool operator==(const iterator& it) const { validate(); it.validate(); return k == it.k; }
		bool operator!=(const iterator& it) const { validate(); it.validate(); return ! this->operator==(it); }
		bool operator<(const iterator& it)  const { validate(); it.validate(); return k < it.k; }
		bool operator>(const iterator& it)  const { validate(); it.validate(); return k > it.k; }
		bool operator<=(const iterator& it) const { validate(); it.validate(); return k <= it.k; }
		bool operator>=(const iterator& it) const { validate(); it.validate(); return k >= it.k; }
		iterator& operator++() {
			validate();
			k++;
			return *this;
		}
		iterator operator++(int) {
			validate();
			iterator t{*this};
			this->operator++();
			return t;
		}
		iterator& operator--() {
			validate();
			k--;
			return *this;
		}
		iterator operator--(int) {
			validate();
			iterator t{*this};
			this->operator--();
			return t;
		}
		iterator& operator+=(uint64_t offset) {
			validate();
			k += offset;
			return *this;
		}
		iterator operator+(uint64_t offset) const {
			validate();
			iterator t{*this};
			t.k += offset;
			return t;
		}
		iterator& operator-=(uint64_t offset) {
			validate();
			k -= offset;
			return *this;
		}
		iterator operator-(uint64_t offset) const {
			validate();
			iterator t{*this};
			t.k -= offset;
			return t;
		}
		int64_t operator-(iterator it) const {
			validate();
			return k - it.k;
		}

		T& operator*() const {
			validate(true, k);
			if (k < dirty_size) dirty[k] = true;
			return v->operator[](k);
		}
		T& operator[](uint64_t n) const {
			validate(true, k + n);
			if (k < dirty_size) dirty[k] = true;
			return v->operator[](k + n);
		}
	};

	class const_iterator {
	private:
		void validate(bool deref=false, uint64_t access=0) const {
			if (v == nullptr) {
				throw invalid_iterator{invalid_iterator::SEVERE};
			}
			if (deref && (k >= v->size())) {
				throw invalid_iterator{invalid_iterator::SEVERE};
			}
			if (deref && (this->modver != v->modver)) {
				if (this->ver != v->ver || access >= dirty_size || dirty[access]) {
					throw invalid_iterator{invalid_iterator::MODERATE};
				}
			}
			if (this->ver != v->ver) {
				throw invalid_iterator{invalid_iterator::MILD};
			}
		}
	public:
		using value_type = T;
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = ptrdiff_t;
		using reference = T;
		using pointer = const T*;

		const vector* v;
		uint64_t k;
		uint64_t ver;
		uint64_t modver;
		bool *dirty;
		uint64_t dirty_size;

		const_iterator(const vector* b, uint64_t n) {
			v = b;
			k = n;
			ver = b->ver;
			modver = b->modver;
			dirty_size = v->size();
			dirty = new bool[dirty_size]();
		}
		const_iterator(const const_iterator& it) {
			v = it.v;
			k = it.k;
			ver = it.ver;
			modver = it.modver;
			dirty_size = v->size();
			dirty = new bool[dirty_size]();
			std::copy(it.dirty, it.dirty + it.dirty_size, dirty);
		}
		const_iterator(const iterator& it) {
			v = it.v;
			k = it.k;
			ver = it.ver;
			modver = it.modver;
			dirty_size = v->size();
			dirty = new bool[dirty_size]();
			std::copy(it.dirty, it.dirty + it.dirty_size, dirty);
		}
		~const_iterator() {
			delete[] dirty;
		}

		const_iterator& operator=(const const_iterator& it) {
			it.validate();
			v = it.v;
			k = it.k;
			ver = it.ver;
			modver = it.modver;
			dirty_size = it.dirty_size;
			dirty = new bool[dirty_size]();
			std::copy(it.dirty, it.dirty + it.dirty_size, dirty);
			return *this;
		}
		bool operator==(const const_iterator& it) const { validate(); it.validate(); return k == it.k; }
		bool operator!=(const const_iterator& it) const { validate(); it.validate(); return ! this->operator==(it); }
		bool operator<(const const_iterator& it)  const { validate(); it.validate(); return k < it.k; }
		bool operator>(const const_iterator& it)  const { validate(); it.validate(); return k > it.k; }
		bool operator<=(const const_iterator& it) const { validate(); it.validate(); return k <= it.k; }
		bool operator>=(const const_iterator& it) const { validate(); it.validate(); return k >= it.k; }
		const_iterator& operator++() {
			validate();
			k++;
			return *this;
		}
		const_iterator operator++(int) {
			validate();
			const_iterator t{*this};
			this->operator++();
			return t;
		}
		const_iterator& operator--() {
			validate();
			k--;
			return *this;
		}
		const_iterator operator--(int) {
			validate();
			const_iterator t{*this};
			this->operator--();
			return t;
		}
		const_iterator& operator+=(uint64_t offset) {
			validate();
			k += offset;
			return *this;
		}
		const_iterator operator+(uint64_t offset) const {
			validate();
			const_iterator t{*this};
			t.k += offset;
			return t;
		}
		const_iterator& operator-=(uint64_t offset) {
			validate();
			k -= offset;
			return *this;
		}
		const_iterator operator-(uint64_t offset) const {
			validate();
			const_iterator t{*this};
			t.k -= offset;
			return t;
		}
		int64_t operator-(const_iterator it) const {
			validate();
			return k - it.k;
		}
		const T& operator*() const {
			validate(true, k);
			if (k < dirty_size) dirty[k] = true;
			return v->operator[](k);
		}
		const T& operator[](uint64_t n) const {
			validate(true, k + n);
			if (k < dirty_size) dirty[k] = true;
			return v->operator[](k + n);
		}
	};

	vector(void) {
		head = (T*)operator new(sizeof(T) * capacity);
		tail = head + capacity - 1;
	}

	explicit vector(uint64_t n) : hsize(n), capacity(n) {
		if (n == 0) capacity = 8;
		head = (T*)operator new(sizeof(T) * capacity);
		tail = head + capacity - 1;
		for (uint64_t k = 0; k < hsize; k++) {
			new (head + k)T();
		}
	}

	~vector(void) {
		destroy();
	}

	vector(const vector<T>& that) {
		copy(that);
	}

	vector(vector<T>&& that) {
		*this = std::move(that);
	}

	vector(std::initializer_list<T> c) {
		head = (T*)operator new(sizeof(T) * capacity);
		tail = head + capacity - 1;
		for (auto it = c.begin(); it != c.end(); ++it) {
			push_back(*it);
		}
	}

	template <typename I>
	vector(I b, I e) {
	    typename std::iterator_traits<I>::iterator_category tag{};
	    typename std::iterator_traits<I>::value_type val{};
	    construct(b, e, tag, val);
	}

	vector(const_iterator& b, const_iterator& e) {
		std::random_access_iterator_tag tag{};
		T val{};
		construct(b, e, tag, val);
	}

	iterator begin() {
		return iterator(this, 0);
	}

	iterator end() {
		return iterator(this, this->size());
	}

	const_iterator begin() const {
		return const_iterator(this, 0);
	}

	const_iterator end() const {
		return const_iterator(this, this->size());
	}

	vector<T>& operator=(vector<T>& that) {
		copy(that);
		return *this;
	}

	vector<T>& operator=(vector<T>&& that) {
		move(std::move(that));
		return *this;
	}

	T& operator[](uint64_t k) {
		modver++;
		return lookup(k);
	}

	const T& operator[](uint64_t k) const {
		return lookup(k);
	}

	uint64_t size(void) const {
		return hsize + tsize;
	}

	void push_back(const T& e) {
		auto insert_element = [&](void) { new (head + hsize) T(e); };
		if (hsize + tsize == capacity) {
			amor_double(insert_element);
		} else {
			insert_element();
		}
		ver++;
		hsize++;
	}

	void push_back(T&& e) {
		auto insert_element = [&](void) { new (head + hsize) T(std::move(e)); };
		if (hsize + tsize == capacity) {
			amor_double(insert_element);
		} else {
			insert_element();
		}
		ver++;
		hsize++;
	}

	void push_front(const T& e) {
		auto insert_element = [&](void) { new (tail - tsize) T(e); };
		if (hsize + tsize == capacity) {
			amor_double(insert_element);
		} else {
			insert_element();
		}
		ver++;
		tsize++;
	}

	void push_front(T&& e) {
		auto insert_element = [&](void) { new (tail - tsize) T(std::move(e)); };
		if (hsize + tsize == capacity) {
			amor_double(insert_element);
		} else {
			insert_element();
		}
		ver++;
		tsize++;
	}

	void pop_back(void) {
		if (tsize + hsize == 0) {
			throw std::out_of_range{"index out of range"};
		}
		if (tsize == 0) {
			hsize--;
			head[hsize].~T();
		} else {
			tsize--;
			tail[-tsize].~T();
		}
		ver++;
	}

	void pop_front(void) {
		if (tsize + hsize == 0) {
			throw std::out_of_range{"index out of range"};
		}
		if (tsize == 0) {
			head[0].~T();
			T* old = head;
			head = (T*)operator new(sizeof(T) * capacity);
			tail = head + capacity - 1;
			hsize--;
			for (uint64_t k = 0; k < hsize; k++) {
				new (head + k)T(std::move(old[k + 1]));
			}
			operator delete(old);
			modver++;
		} else {
			tsize--;
			tail[-tsize].~T();
		}
		ver++;
	}
};

} //namespace epl

#endif /* _vector_h */
