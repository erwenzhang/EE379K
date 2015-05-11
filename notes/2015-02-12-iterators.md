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
```cpp
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
```cpp
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

```cpp
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

This solution does not work for base classes. For instance, an `int*` is a
valid iterator, but `I::iterator_category` makes no sense on a base type.

The standard library provides an `iterator_traits` class to determine the
type of an iterator. The reason for a class instead of a function is the
inability to `return a type` in C++. Using a class allows us to use a feature of
C++ called _template specialization_.

### `iterator_traits`

```cpp
/* by itself, this is useless... we're going to specialize though */
/* this is an example of how the std library is implemented */
template <typename T>
struct iterator_traits {
    using value_type = T::value_type;
    using iterator_category = T::iterator_category;
}

/* the following specialization matches pointers */
template <template T>
struct iterator_traits<T*> {
    using value_type = T;
    using iterator_category = std::random_access_iterator_tag;
}

/* we can modify our distance function from above to use this concept */
template <typename I>
uint64_t distance(I b, I e) {
    /* template metaprogramming - oh boy! */
    iterator_traits<I>::iterator_category x{};
    return distance(b, e, x);
}
```

### A simple annoyance

When the compiler first checks our distance function template, the compiler must
choose how to interpret `iterator_traits<I>::iterator_category`. It might be a
nested type, it might be a static variable, it might even be a function. The
default assumption is that `::iterator_category` is a static variable. So the
first time the compiler checks our function, it sees a syntax error.

As a result... `typename`!
```cpp
template <typename I>
uint64_t distance(I b, I e) {
    /* let the compiler know what's happening */
    typename std::iterator_traits<I>::iterator_category x{};
    return distance(b, e, x);
}
```
And another!
```cpp
template <typename T>
struct iterator_traits {
    using value_type = T::value_type;
    using iterator_category = typename T::iterator_category;
}
```

### `value_type`

If we have a function that operates on iterators, we may want to dereference our
iterator at some point.
```cpp
template <typename I>
I partition(I b, I e) {
    auto piv = *b;
    /* etc. */
}
```
Using the typename `auto` is the easiest way to do this, but it's still useful
to understand the alternative (if you're using something older than C++11).
```cpp
template <typename I>
I partition(I b, I e) {
    using T = typename std::iterator_traits<I>::value_type;
    T piv = *b;
    /* etc. */
}
```

Template metaprogramming allows you to operate with and make queries about
types.


