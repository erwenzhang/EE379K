## Final Exam

+ Dynamic polymorphism

+ Inheritance

+ Multiple inheritance

+ Given a choice between static and dynamic polymorphism, how do we decide?

+ Dynamic polymorphism using virtual functions

+ Dynamic casts

+ How this-> works in inheritance (virtual function tables)

+ Multiple inheritance, two base classes

```cpp
    // pointer to member function of LifeForm with no parameters and no return
    void LifeForm::* p)(void) = &LifeForm::border_cross;
    void LifeForm::* q)(void) = &LifeForm::species_name; // virtual func
    Craig x;
    (x.*p)(); // &x equivalent to this, jsr *p
    (x.*q)(); // implicit param this works, jsr to virtual funct
              // must resolve to indexing vtable to find func
              // add info to type (is_virtual, offset / addr)
    // what if we have multiple inheritance?
    // Craig is a FooBar, LifeForm, and Craig
    Craig c;
    LifeForm& x = c;
    // when running a method on the derived class, this must
    // point to the beginning of the derived object
    
    // in our case, we must add a negative offset to x until
    // x points to the beginning of c
```

+ Array of objects

```cpp
    LifeForm *p = new Craig[10]; // very very bad
    delete[] p; // unable to delete
```

+ Tradeoffs between static / dynamic polymorphism

+ Factory method in Project3

+ Reboot question from exam one, change type of object at runtime

```cpp
    LifeForm *p = new Craig;
    (*p) <= Algae // we want to change type
                  // we have no idea how many pointers are to p
    // type is set by constructor, rerunning constructor changes type
```

+ Take a look at function case-study (especially virtual copy)