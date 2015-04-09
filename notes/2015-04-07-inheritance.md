
# Inheritance

```
Base    b;
Base    *pb = &b;
Derived d;
Derived *pd = &d;

Base    *q = &d; // would work!
Derived *r = &b; // erm... not really
Derived *r = q;  // based on static type info, this doesn't work
Derived *r = (Derived *) q; // approximated dynamic_cast, don't use though
Derived *r = static_cast<Derived *>(q);  // much better
Derived *r = dynamic_cast<Derived *>(q); // does a Java style cast, runtime
```

The main idea of inheritance is the way the compiler generates an object that is
a Derived class. It simply appends all the fields to the Base class. The
compiler does NOT recompile the Base class.

## Virtual Function Table

One table for class (i.e. an implicit static data member). This table is created
by the compiler to hold an array of pointers to functions. Java uses a Map where
C++ uses an Array (indexed by offsets, not by name).

Each object holds a pointer to this vtable. This avoids object bloat as you
increase the number of functions.

