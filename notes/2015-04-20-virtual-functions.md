# Pure Virtual

+ No implementation in Base class (i.e. no default behavior)

+ Forces the class to be abstract

+ Any concrete subclass must provide an implementation

# Event-Driven Simulation

We are simulating life in a petri dish. Eventually, we will be designing our own
"species" to navigate and survive in this environment.

Unlike the EE312 version of this assignment, we have fine-grained control over
movement, reproduction, and genetic variations.

```
   LifeForm    
    ^    ^     
   /      \    
Craig    Algae 
```

During simulation, we will treat all objects the same, as `LifeForm`s. Therefore,
all objects will be subclasses of `LifeForm`.

```
class LifeForm {
    virtual void species_name(void) const = 0;
private:
    double xpos;
    double ypos;
    LifeForm(void) { ... } // will be used by subclasses
}
```

## Do abstract base classes need virtual function tables?

Assume we have a constructor for our Base class object. In the construction of
a subclass, the Base class is constructed first, followed by the subclass.

What if we need to call a function from our constructor?

It is theoretically possible to call a virtual function `doit()` from the `Base`
class constructor. The language must decide between calling the `Base` class
`doit()` or the subclass version.

Invoking a function from the `constructor` or `destructor` of the `Base` class
will use the virtual function defined within the `Base` class.

```
class Base {
    virtual void doit(void) {
        // pass
    }
    Base(void) {
        doit();
    }
};

class Derived : Base {
    int* p;
    void doit(void) {
        *p = 42;
    }
    Derived(void) {
        p = new int;
    }
};

int main() {
    Derived d;
}
```

Without the above language decision, the above code would fail miserably.

The compiler knows when it is compiling a constructor, and can insert a direct
static invocation for all methods in the constructor (e.g. `Base::doit()`).
However, what if `doit()` calls another virtual function?

_Oof, this is getting messy._

At the start of the `Base` constructor, the compiler points the virtual function
table pointer to the `Base` class virtual function table. In the constructor for
the `Derived` class, we reassign the virtual function table pointer to the
`Derived` class virtual function table.

## Abstract Functions

We make functions abstract when there is no reasonable abstract and/or we want
to __force__ the `Derived` class to implement the function.

## Object-Oriented Design

In Object-Oriented design, you __always__ want your objects on the heap.
Otherwise, it becomes far to difficult to call the virtual functions of your
`Derived` class.

## Building a Simulator

In discrete time simulations, you update the state of the world at discrete time
steps (e.g. once per picosecond). If there is no coupling between the virtual
and the physical world, we likely prefer discrete time simulations (e.g.
circuit modeling, not a flight simulator).

The other option is a model of simulation based on events. We simply arrange our
events in chronological order and simulate them one at a time. These events can
be arbitrarily far apart in time, allowing us to be more efficient in our
simulation.

Our event-driven model looks like the following:
```
int main() {
    init_world() // create all lifeforms

    while (???) { // stop-point doesn't matter
        Event e = queue.pop(); // pop the next chronological event
        now = e.timestamp;     // jump our clock to the point the event happened
        e.handler();           // simulate the event, may add new events
    }
}
```

When lifeforms are far apart, we allow them to move more at a lower frequency.
When lifeforms are close together, we raise the frequency and reduce the
movement.

## Movement

In the past, we have used grid tiles to model the locations of lifeforms.
However, imagine we are simulating an enormous region of tiles. This creates
unnecessarily high memory pressure.

We will be creating a special data structure to hold our lifeforms. This data
structure will have an invariant such that two lifeforms must have a "boundary"
between each other. More on this next lecture.

