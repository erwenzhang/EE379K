# Template Specialization

Allows us to create a default implementation for a template, but _also_ specify
alternative implementations given specific types.

```
template <typename T>
class vector {
    T* data;
    int len;
public:
    /* etc. */
}

/* template specialization for a bit vector */
template<>
class vector<bool> {
    uint64_t* data;
    int len;
public:
    vector(uint64_t x) {
        /* etc. */
    }
    /* etc. */
};
```

In the above example, when the client instansiates a vector, the compiler
searches for any matching templates. The generic `typename T` template is the
broadest category, and will be matched last.

One of the early arguments for this feature is the following:
```
template <>
class vector<void*> {
    /* etc. */
};

template <>
class vector<Student*> : public vector<void*> {
public:
    Student* operator[](uint32_t k) {
        return (Student*) vector<void*>::operator[](k);
    }
};
vector<Student*> x;
vector<Professor*> y;
```

Normally, the two vector instansiations would both generate new machine code.
One vector for type `Student*` and one vector for type `Professor*`. We can
avoid this excess by creating a template specialization for `void*` and
`Student*` (which inherits from `void*`).

### Partial Specialization.

This itself grows tedious though. We can do _one more thing_.
```
template <typename T>
class vector<T*> : public vector<void*> {
public:
    T* operator[](uint32_t k) {
        return (T*) vector<void*>::operator[](k);
    }
};
```

### Summary

Generally, we want to solve the following problem.
```
template <typename t>
void doit(T p) {
    if (T is powerful)
        method 1
    else
        method 2
}
```
__The above solution does not work.__

Instead, we use function overloading (for the compiler's benefit).
```
template <typename T>
void doit(T p) {
    T::dependent_type x{};
    method(x); // where overloading decides
}
```
This becomes an `if` at compile time.

