
## Move semantics
There are times when the copy mechanic fails us, namely, any instance when we
are *sure* that data can be reused instead of copied.

Most often used when you have a temporary object, an object with no name.

```
String s{"hello"};
String s{"world"};
String r = concat(s, t);
```

If you have failed to properly implement a copy constructor, you will likely
achieve the desired result, a variable `r` containing the result of the rvalue
expression. However, *this goes against C++ coding conventions*. In C++,
programmers are highly encouraged to use deep-copy semantics.

If you have properly implemented deep-copy, you still need a shallow-copy
operation for the above example. The move construct is the C++ solution to
shallow-copy.

The solution: a new type to allow designers to overload the assignment operator
in two ways (copy and move).

```
operator=(const T&&);
operator=(T&&);
```

Because the rvalue reference is a type, you can cast an object to force move
semantics.

```
r = (String&&)s;
```

When moving an object, the destructor of the rvalue reference is still called.
Therefore, when moving, you must ensure the destructor on the rvalue is
impotent.

```
void move(String&& that) {
    len = that.len;
    data = that.data;

    // required for impotent destructor
    that.data = nullptr;
}
```

## Note on Project1

When working with an object of an unknown type, only make assumptions specified
by the language (e.g. try not to make assumptions about `typename T`).

Play at home: create a `vector<vector<String>>` and observe allocations.

## LinkedList example
Say you have a linkedlist that you want to append to another linkedlist.
Additionally, this suffix list is an rvalue. We can optimize!

```
void appendList(LinkedList<T>& suffix) {
    for (const auto& x : suffix) {
        push_back(x);
    }
}
void appendist(LinkedList<T>&& suffix)
    // this example was rushed at the end of class
    // set head and tail pointers correctly
    // clean up suffix, make its destructor impotent
}
```

