# Iterators

### C++ Iterators:

 * handle to the positions of a data structure.
 * pseudo-pointers
 * all data structures work like arrays
 * use ranges to describe contains
 * end is out of bounds
 * no inheritance (no OOP in standard library)

### STL has certain requirements:

 * Generic with respect to element type
 * Generic with respect to container type
 * Predictable time complexity ( usually O(1) )
 * Efficient, on par with hand written code

### Typicall Pointer Operations:

 * dereference: `*p`
 * advance: `++p`
 * retreat: `--p`
 * equivalence: `p == q`
 * random access: `p+k` (aka `p[k]`)
 * subtraction: `p-q` (number of elements)

### Three broad categories of iterator:

 * `forward_iterator` (dereference, advance, equivalence)
 * `bidirectional_iterator` (`forward_iterator` with retreat)
 * `rand_access_iterator` (all six operations)

## Conventions

The iterator will be a nested type named `iterator` (ability to
change) or a `const_iterator` (cannot modify values).

The data struct will provide two functions:

 * `iterator begin(void)`
 * `iterator end(void)`
 * all functions specified by the iterator type

Pre- and post-increment operators are sort of funky.
```
// pre-increment
iterator& operator++(void) {
    position = position->next;
    return *this;
}
// post-increment
iterator operator++(int) {
    iterator t{*this};  // copy self
    this->operator++(); // increment self
    return t;           // return copy of old value
}
```

"If you return a reference to a local variable, you are going to die."

### Checking type of iterator

Note: `std::distance` provides a means of calculating the number of elements between two iterators.

The following code does not work!
```
template <typename I>
uint64_t distance(I b, I e) {
    if (I::iterator_category >= RANDOM_ACCESS) {
        return e - b;
    } else {
        // forward_iterator distance
        uint64_t len;
        while (b != e) {
            len++;
            ++b;
        }
        return len;
    }
}
```
If we had used function overloading, this would have worked. But we can't use
overloading to distinguish between an iterator of type `RANDOM_ACCESS` and an
iterator of type `FORWARD_ITERATOR`. In order to make the above code work, we
define two functions as follows:

```
template <typename I>
uint64_t distance(I b, I e, std::forward_iterator_tag t) {
    uint64_t len;
    while (b != e) {
        len += 1;
        ++b;
    }
    return len;
}

template <typename I>
uint64_t distance(I b, I e, std::random_access_iterator_tag t) {
    return e - b;
}

template <typename I>
uint64_t distance(I b, I e) {
    I::iterator_category x{};
    return distance(b, e, x);
}
```

