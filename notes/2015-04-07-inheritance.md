
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

