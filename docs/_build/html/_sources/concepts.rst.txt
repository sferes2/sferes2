Main concepts
==============

Static object-oriented paradigm
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Object-oriented programming (OOP) has many advantages which are
desirable for complex high-level code. However, classic OOP (e.g. in
C++) heavily relies on virtual methods to implement abstraction and
polymorphism. Such an approach has a significant run-time overhead:

-  selecting the good method to call adds an indirection (the software
   have to check the virtual table)
-  abstract/virtual methods cannot be inlined, resulting in a overhead
   for very simple methods (e.g. setters/getters).

In a few word, we want the main benefits of OOP (abstraction,
re-usability) and the efficiency of low-level languages.

One solution in C++ is to use the "curiously recurring template pattern"
(`http://en.wikipedia.org/wiki/Curiously_Recurring_Template_Pattern <http://en.wikipedia.org/wiki/Curiously_Recurring_Template_Pattern>`__).
Here is the idea:

::

    template <class Derived> struct Base
    {
        void interface()
        {
            static_cast<Derived*>(this)->implementation();
        }
        void f()
        {
           for(int i = 0; i < 3; ++i) interface();
        }
    };

    struct Derived : public Base<Derived>
    {
        void implementation() { std::cout<<"impl"<<std::endl; }
    };

This code mimics a virtual call of an abstract method: in the abstract
class (Base), a generic algorithm can use methods which are only defined
in the derived classes. Extending this pattern for a full hierarchy of
classes requires more work; sferes2 relies on the method described in
the following paper:

Nicolas Burrus, Alexandre Duret-Lutz, Thierry Géraud, David Lesage and
Raphaël Poss. A Static C++ Object-Oriented Programming (SCOOP) Paradigm
Mixing Benefits of Traditional OOP and Generic Programming. In
*Proceedings of the Workshop on Multiple Paradigm with OO Languages
(MPOOL'03)*, 2003. http://www.lrde.epita.fr/dload/papers/mpool03.pdf

This mechanics is hidden in sferes2 behind some macros (see section
*macros*)

Static parameters
~~~~~~~~~~~~~~~~~

Evolutionary algorithms have a lot of parameters (e.g. population size,
mutation rate, etc.) and we need a convenient way to set them. A
configuration file (e.g. an XML or JSON or YML file) is often used; 
however, such a
method has two drawbacks:

-  some code to read the files has to be written and kept synchronized
   with the objects;
-  parameters are unknown at compile time so some checks (e.g. if
   (mutation_type = x) {} else { } …) have to be done many times
   whereas they are useless.

In sferes2, parameters are defined at compile time using a structure
which contains only constants. This structure is passed to all sferes2
classes so they can access to the parameters. This method makes it very 
fast to add parameters. It also allows the compiler to propagate constants, 
check the validty of the code, and settings in the whole source code, 
resulting in an executable optimized for the specific parameters.

The main issue is that a recompilation is needed for each parameter change.
We believe it is a minor issue (given the benefits above) for algorithm 
developpers who are working on their algorithms (since they are recompiling 
very often anyway). This would be a bigger issue for users who wish to use 
sferes as a "black-box" evolutionary algorithm and simply play with the parameters.


Macros and special functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To simplify the writing of "static virtual functions" and the definition
of static parameters, sferes2 provides several macros which are defined
in
`stc.hpp <https://github.com/jbmouret/sferes2/blob/master/sferes/stc.hpp>`__.

Classes
-------

   -  `SFERES_CLASS(Name)`: defines the base class Name ; this
      class uses the "static" inheritance scheme and takes a Param
      template parameter.
   -  `SFERES_CLASSD(Name, Parent)`: derives the class Name from
      Parent
   -  `SFERES_PARENT(Name, Parent)::method()`: calls `method()` in
      the parent class
   -  `stc::exact(this)->method()`: calls an abstract `method()` which is
      defined in the derived classes

Moreover, sferes2 provides special macros to define specific classes
(`SFERES_FITNESS`, `SFERES_INDIV`, `SFERES_EA`, etc.). See *ref*.

   Example:

   ::

       SFERES_CLASS(X)
       {
         X() { std::cout<<"X"<<std::endl; }
         void test() { stc::exact(this)->blah(); }
         void test2() { std::cout<<"test2"<<std::endl; }
       };
       // Y derives from X
       SFERES_CLASS_D(Y, X)
       {
         Y() { std::cout<<"Y"<<std::endl; }
         void blah() 
         { 
           std::cout<<"param::test::x"<<Param::test::x<<std::endl;
           SFERES_PARENT(Y, X)::test2();
         }
       };
       // Z derives from Y
       SFERES_CLASS_D(Z, Y)
       {
         Z() { std::cout<<"Z"<<std::endl; }
         void blah() 
         { 
           std::cout<<"Z"<<std::endl;
         }
       };
       // sample parameter (see next subsection)
       struct Params_test
       {
         struct test
         {
           SFERES_CONST float x = 0.1f;
         };
       };
       // to instantiate Y and Z (X is abstract):
       int main()
       {
         // Y uses a parameter
         Y<Param_test> y;
         // Z doesn't use any parameter, these two forms are possible:
         Z<> z1;
         Z<Param_test> z2;
         return 0;
       }

Parameter handling
---------------------

The basic parameter format employs `SFERES_CONST` members in nested structures. For instance:

   ::

       struct Param
       {
         struct pop
         {
           SFERES_CONST size_t size = 100;
           SFERES_CONST float x = 0.2f;
         };
       };

However, c++ does not allow to define `SFERES_CONST` strings or
arrays in these structures. To solve this problem, sferes2 provides 
two macros:

   -  `SFERES_ARRAY (type, name, val1, val2, …)`
   -  `SFERES_STRING(name, "string")`

   ::

       struct Param
       {
         struct test
         {
           SFERES_ARRAY(float, my_array, 1.0f, 2.0f, 3.0f);
           SFERES_STRING(my_string, "hello world");
         };
       };

These parameters can then be retrieved in the source code in the following way:

   ::

       ...
       // size
       size_t s = Params::test::my_array_size();
       // 2nd element
       float v = Param::test::my_array(2);
       // string
       const char* c = Params::test::my_string();

More information
~~~~~~~~~~~~~~~~

A general introduction of the concepts is the Sferes2 paper:

Mouret, J.-B. and Doncieux, S. (2010). SFERESv2: Evolvin' in the
Multi-Core World. *WCCI 2010 IEEE World Congress on Computational
Intelligence, Congress on Evolutionary Computation (CEC)*. Pages
4079–4086. `{pdf} <http://www.isir.upmc.fr/files/2010ACTI1524.pdf>`__

Mastering boost
---------------

Sferes2 heavily relies on boost libraries (see http://www.boost.org). To
implement your own algorithms in the Sferes_v2 framework, you should be
familiar enough with the following libraries:

-  `boost::shared_ptr`: http://www.boost.org/doc/libs/1_40_0/libs/smart_ptr/shared_ptr.html
-  `boost::serialization` (useful to define your own genotypes and
   statistics): http://www.boost.org/doc/libs/1_40_0/libs/serialization/doc/index.html

The following libraries are used in sferes2 but you might never notice
them:

-  `BOOST_FOREACH` (to simplify loops):
   http://www.boost.org/doc/libs/1_40_0/doc/html/foreach.html
-  `boost::filesystem` (to create directories, etc.):
   http://www.boost.org/doc/libs/1_40_0/libs/filesystem/doc/index.htm
-  `boost::fusion` (list of statistics and list of modifiers):
   http://www.boost.org/doc/libs/1_40_0/libs/fusion/doc/html/index.html
-  `boost::mpi` (MPI evaluator, optional):
   http://www.boost.org/doc/libs/1_40_0/doc/html/mpi.html

