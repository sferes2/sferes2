Reference manual
=================

Naming conventions
------------------

-  Names representing **classes** must be in mixed case starting with
   upper case: MyClass
-  **Variable and method names** must be in lower case, using
   underscores to separate words: my_variable`,
   my_method`\ ()
-  Names of **protected and private members** must start with an
   underscore: `_my_private_member`, `_my_private_method()`
-  **File names** must be in lower case, using underscores to separate
   words. A file which contains a class MyClass should be put in a file
   `my_class.hpp`
-  **File structure** mirrors namespace structure. For instance
   gen::MyClass is in the file `gen/my_class.hpp`
-  **Named constants** (including enumeration values) must be all
   uppercase using underscores to separate words
-  **Getters** should have the name of the attribute. For instance,
   `this->_objs` should be accessed using `this->objs()`
-  **Setters** should start with "set_" followed by the name of the
   attribute. For instance: `set_objs(const std::vector<float>&
   ov)`
-  The **public section** should be the first section of a class
-  Type names defined using **typedefs** should end with `_t` (e.g. `iterator_t`)
- The **default argument argument passing** is `const&` (except for `char/int/double` types), e.g., `void my_function(const std::vector<float>& x)`
- Methods should be declared as `const` except if there is a good reason for them to not be 
  const, e.g., `const std::vector<float>& get_objs() const { return _objs; }`

Full example:

::

    // filename: sferes/fit/my_test.hpp
    namespace sferes
    {
      namespace fit
      {
        class SFERES_CLASS(MyTest)
        {
        public:
          typedef float f_t;
          MyTest() {}
          void my_method() { _my_impl(); }
          float my_attr() const { return _my_attr;}
          float set_my_attr(float a) { _my_attr = v; }
        protected:
          float _my_attr;
          void _my_impl() {}
        };
      }
    }

Fitness
-------

.. _fitness-1:

Fitness
~~~~~~~

-  **File:**
   `sferes/fit/fitness.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/fit/fitness.hpp>`__
-  **Description:** Fitness is the base class of fitness functions in
   sferes2. It has two main attributes:

   -  `float _value` (setter/getter `Fitness::value()/Fitness::set_value()`) -> the value
      of the fitness for single-objective optimization
   -  `std::vector<float> _objs` (setter/getter `Fitness::objs()/Fitness::set_obj(i, x)`) ->
      the value of each objective, for multiobjective optimization

-  This class is abstract
- Note: in multi-objective algorithms, `this->_value` is ignored; and in singe-objective algorithms, `this->_objs` is ignored.


Defining your own fitness
~~~~~~~~~~~~~~~~~~~~~~~~~

In most sferes2 experiments, you will have to design your own fitness. A
fitness should:

-  be defined using `SFERES_FITNESS(MyFitness,
   sferes::fit::Fitness)`
-  define the `eval()` method, which takes an individual
-  attribute a fitness value in `this->_value` (single objective
   optimization) or `this->_objs` (multiobjective optimization)

   A fitness is a "normal" class and consequently you can add other
   methods or attributes to suit your needs.

Example:

::

    // single objective fitess
    SFERES_FITNESS(FitnessSingle, sferes::fit::fitness)
    {
      // (optional) constructor
      FitnessSingle()
        {
          this->_value = -1;
        }
      // evaluation
      template<typename Indiv>
      void eval(Indiv& indiv)
      {
        this->_value = -42;
      }
    };

    // two-objective fitness
    SFERES_FITNESS(FitnessMulti, sferes::fit::fitness)
    {
      // constructor
      FitnessMulti()
        {
          this->_objs.resize(2);   
        }
      // evaluation
      template<typename Indiv>
      void eval(Indiv& indiv)
      {
        this->_objs[0] = -42;
        this->_objs[1] = 42;
      }
    };

View mode
~~~~~~~~~

When loading a result file, it is often useful to slightly change the
fitness function, for instance to display data about the behavior of
the individual. This can be easily done in sferes2 using the boolean
`Fitness::mode()` than can takes two values:

-  `fit::mode::view` when you are viewing an individual via `--load`
-  `fit::mode::eval` when you are evaluting individuals (during the
   evolutionary process)

Example:

::

    // single objective fitess
    SFERES_FITNESS(FitnessSingle, sferes::fit::fitness)
    {
      // evaluation
      template<typename Indiv>
       void eval(Indiv& indiv)
      {
        if (this->mode() == sferes::fit::mode::view)
          {
          std::ofstream ofs("fit.dat");
          ofs<<"this is a log file !"<<std::endl;
          }
        this->_value = -42;
      }
    };

Evolutionary algorithms
-----------------------

**Warning** Evolutionary algorithms *maximize* the fitness (whereas most
optimization algorithms minimize the cost function).

Shared parameters
~~~~~~~~~~~~~~~~~

All evolutionary algorithms (EA) use the following parameters:

-  `SFERES_CONST size_t Params::pop::size`: population
   size (e.g., 100)
-  `SFERES_CONST float Params::pop::initial_aleat`: during the
   random generation, `Params::pop::initial_aleat × Params::pop::size`
   individuals are generated and only the `Params::pop::size` best ones
   are kept (Note: some algorithms might ignore this parameters)
-  `SFERES_CONST int Params::pop::dump_period`: the period
   between each write of results, examples:

   -  `1`: statistics are written at each generation (`gen_0`,
      `gen_1`, `gen_2`, …)
   -  `-1`: statistics are never written (useful for unit tests)
   -  `10`: statistics are written each 10 generations (`gen_0`,
      `gen_10`, …)

Keep in mind that the `statitistics` classes are where final results are stored.

RankSimple
~~~~~~~~~~

-  **Status:** Deprecated (we keep this algorithm only as an example of an EA). If
   you need single-objective optimisation, use NSGA-2 with a single
   objective. It is equivalent to an elitist, rank-based algorithm.
-  **File:**
   `sferes/ea/rank_simple.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/ea/rank_simple.hpp>`__
-  **Unit test:**
   `tests/ea/rank_simple.cpp <https://github.com/sferes2/sferes2/blob/master/tests/ea/rank_simple.cpp>`__
-  **Typical typename:**:

::

    typedef sferes::ea::RankSimple<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;

-  **Description:** A rank-based single-objective evolutionary
   algorithm. Individuals are selected using the following formula:

   -  n = k_r × p
   -  κ = c^n + 1 - 1
   -  f = n / (*κ* + 1)
   -  i = p - f × log(r × *κ* + 1)
   -  where

      -  c = Params::pop::coeff
      -  k_r = Params::pop::keep_rate
      -  p = Params::pop::size
      -  r = a random number in [0,1[
      -  i = index of the selected individual

-  parameters:

   -  SFERES_CONST float Params::pop::coeff = a internal
      parameter (see the previous formula); typical value : 1.1
   -  SFERES_CONST float Params::pop::keep_rate = proportion
      of individuals kept from a generation to the next one; typical
      value: 0.2 to 0.6

Nsga2
~~~~~

-  **File:**
   `sferes/ea/nsga2.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/ea/nsga2.hpp>`__
-  **Unit test:**
   `tests/ea/nsga2.cpp <https://github.com/sferes2/sferes2/blob/master/tests/ea/nsga2.cpp>`__
-  Example:
   `examples/ex_nsga2.cpp <https://github.com/sferes2/sferes2/blob/master/sferes/examples/ex_nsga2.cpp>`__
-  **Typical typename:**:

::

    typedef sferes::ea::Nsga2<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;

-  **Description:** Implementation of the NSGA-II multiobjective
   evolutionary algorithm. See:

   -  Deb, K. (2001). Multi-Objective Optimization Using Evolutionary
      Algorithms. *Wiley*.
   -  Deb, K., Pratap, A., Agarwal, S. and Meyarivan, T. (2002). A fast
      and elitist multiobjective genetic algorithm: NSGA-II. In *IEEE
      transactions on evolutionary computation*, 6:2:182-197

-  **Parameters:** `pop_size` must be divisible by 4
-  **Note:** NSGA-II can be effective for a basic, single-objective optimization
   as it corresponds to a classic tournament-based evolutionary
   algorithm. In sferes2, a 1-sized `_objs` vector in the fitness
   can be employed (don't use `_value`).

EpsMOEA
~~~~~~~

-  **Status:** working but no user (we might remove it in the future
   because it is hard to trust the current implementation when nobody
   uses it).
-  **File:**
   `sferes/ea/eps_moea.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/ea/eps_moea.hpp>`__
-  **Unit test:**
   `tests/ea/eps_moea.cpp <https://github.com/sferes2/sferes2/blob/master/tests/ea/eps_moea.cpp>`__
-  **Typical typename:**:

::

    typedef sferes::ea::EpsMOEA<phen_t, eval_t, stat_t, modifier_t, Params> ea_t

-  **Description:** Implementation of the *ε*-MOEA multiobjective
   evolutionary algorithm. See:

   -  Deb, K., Mohan, M. and Mishra, S. (2005). Evaluating the
      *ε*-domination based multi-objective evolutionary algorithm for a
      quick computation of pareto-optimal solutions. In *Evolutionary
      Computation*, 13:4:501-525.

-  Parameters

   -  SFERES_ARRAY(float, eps, 0.0075f, 0.0075f) = values of
      *ε* for each objective. There should be as many values as
      objectives.
   -  SFERES_ARRAY(float, min_fit`, 0.0f, 0.0f) =
      minimum value of fitness for each objective.There should as many
      values as objectives
   -  SFERES_CONST size_t` grain = *ε*-MOEA is a
      steady-state algorithm which adds individuals one by one. A basic
      implementation would prevent the parallelization of individuals'
      evaluation. The parameter grain allows to generate and evaluate
      *grain* indidividuals in paralell at each generation. Typical
      value is size / 4.

-  **Notes:**

   -  *ε*-MOEA employs *ε*-dominance to sort individuals. This allows
      users to tune the precision of the pareto front for each objective
      and often results in better performance than NSGA-II.
   -  EpsMOEA should not be used with a modifier (because it uses an
      archive). You should always use fit::ModifierDummy

Defining your own EA
~~~~~~~~~~~~~~~~~~~~

To define your own EA, you should use the macro
SFERES_EA(Class, Parent). Mandatory methods:

-  random_pop(): fill this->_pop with random
   individuals
-  epoch(): main loop of the algorithm

Defined types (these types will be available in your class):

-  Phen: phenotype
-  Eval: evaluator
-  Stat: statistics vector
-  FitModifier: modifier
-  Params: parameters

::

    SFERES_EA(Test, Ea)
    {
     public:
     Test() {}
     void random_pop()
     { /* your code */ }
     void epoch()
     { /* your code */ }
    };

Multiobjective EA must also define a pareto_front() method,
see *ParetoFront*.

Statistics
----------

Statistics should be combined in a boost::fusion::vector (see:
http://www.boost.org/doc/libs/1_40_0/libs/fusion/doc/html/fusion/container/vector.html)
before being passed to a EA. For instance:

::

    typedef boost::fusion::vector<stat::BestFit<phen_t, Params>, stat::MeanFit<Params> >  stat_t;

(yes, it is a vector of *typenames*). You can put in the vector as many
statistics types as you desire.

BestFit
~~~~~~~

-  **File:**
   `sferes/stat/best_fit.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/stat/best_fit.hpp>`__
-  **Typical typename:**

::

    typedef boost::fusion::vector<sferes::ea::BestFit<phen_t, Params> > stat_t;

-  **Description:**

   -  stores the individual with the highest fitness in this->_best
      (getter: BestFit::best() ). *This assumes that the population is
      sorted* and consequently this statistics stores the first
      individual of the population.
   -  This class is designed for single objective optimization and
      consequently works using `Fitness::_value` (and not
      `Fitness::_objs`)
   -  writes the value of the best fitness in the file
      <result_directory>/bestfit.dat; this file should be easily
      plotted with gnuplot, matplotlib/numpy or R.

MeanFit
~~~~~~~

-  **File:**
   `sferes/stat/mean_fit.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/stat/mean_fit.hpp>`__
-  **Typical typename:**

::

    typedef boost::fusion::vector<sferes::ea::MeanFit<phen_t, Params> > stat_t;

-  **Description:** computes the mean fitness (single objective, based
   on Fitness::_value). This class is mainly aimed at providing
   a simple example of statistics.

ParetoFront
~~~~~~~~~~~

-  **File:**
   `sferes/stat/pareto_front.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/stat/pareto_front.hpp>`__
-  **Typical typename:**

::

    typedef boost::fusion::vector<sferes::ea::ParetoFront<phen_t, Params> > stat_t;

-  **Description:** stores the set of non-dominated individuals by
   copying the list returned by ea_t::pareto_front().
   ParetoFront does not compute the pareto front; this class assumes
   that the EA computed it. This implies that multiobjective EA classes
   should define the following method:

::

    SFERES_EA(MyEA, Ea)
    {
     public:
      std:vector<Indiv> &pareto_front() const { /* your code */ }
    };

-  **Note:** since this statistic can store many individuals, to load a
   result file you must give the identifier (the rank in the Pareto
   front) to your executable. For instance, assuming that ParetoFront in
   the first (0th) statistics in the stat vector, loading the 10th
   individual should look like:

.. code:: shell

    ./my_exp --load gen_100 -s 0 -n 10 -o output

Defining your own statistics
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    SFERES_STAT(MyStat, Stat)
    {
      // examines the population (via the EA) to update the statistics
      template<typename E> void refresh(const E& ea) { /* your code */ }
      // writes the content of the statistics in the stream; k is the number
      // of the individual in the statistics (e.g. in a Pareto front, the
      // statistics contains many individuals)
      void show(std::ostream& os, size_t k) const { /* your code */ }
      // serializes the statistics using boost::serialization
      template<class Archive> void serialize(Archive & ar, const unsigned int version)
      { /* your code */ }
    }

Genotypes
---------

BitString
~~~~~~~~~

-  **File:**
   `sferes/gen/bit_string.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/gen/bit_string.hpp>`__
-  **Unit test:**
   `sferes/gen/bit_string.hpp <../../tests/gen/bit_string.hpp>`__
-  **Description:** vector of bitstrings which can be used to represent
   a vector of integer parameters or a vector of discrete parameters.
   Cross-over is a simple one point cross-over (it always happens). The
   data can be easily converted to int (in the range 0..2^b,
   where b is the number of bits for each bitstring) via
   BitString<>::int_data(i) or to float (in the range 0..1)
   via BitString<>::data(i)
-  **Template parameter:** the number of bitstrings
-  **Parameters (struct bit_string`):**

   -  SFERES_CONST size_t nb_bits -> number of
      bits for each bitstring
   -  SFERES_CONST float mutation_rate -> the mutation
      rate for a bitstring (the bitstring i is mutated if a random real
      is belows `mutation_rate`)
   -  SFERES_CONST` float mutation_ratebit -> the
      mutation rate *for each bit*. In pseudo-code, here is the mutation
      procedure:

::

    foreach(bitstring b in genotype)
      if (rand(0, 1) < Params::bit_string::mutation_rate)
        foreach(bit i in b)
          if (rand(0, 1) < Params::bit_string::mutation_bit)
            flip(i)   

-  **Typical parameters:**

::

    struct Params
    {
      struct bit_string
      {
        SFERES_CONST size_t nb_bits = 8;
        SFERES_CONST float mutate_rate = 0.1f;
        SFERES_CONST float mutate_rate_bit = 0.1f;    
      };
    };

-  **Typical typename:**

::

    typedef BitString<10, Params> gen_t;

EvoFloat
~~~~~~~~

-  **File:**
   `sferes/gen/evo_float.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/gen/evo_float.hpp>`__
-  **Unit test:**
   `sferes/tests/gen/evo_float.cpp <https://github.com/sferes2/sferes2/blob/master/tests/gen/evo_float.cpp>`__
-  **Description:** This genotype is a n-sized vector of real (float)
   numbers. It is designed for real optimization.

-  Three mutation types are available:

   -  polynomial: polynomial mutation, as defined in Deb(200)1, p 124
   -  gaussian: adds a gaussian noise to the current value; the variance
      is a user-specified parameter
   -  uniform: adds a uniform noise to the current value; the range is a
      user-specified parameter

-  Two cross-over types are available (you can disable cross-over by
   using a cross_rate` equals to 0)

   -  recombination:

      -  a number k is randomly chosen
      -  the first child is made of the [0, k[ values of the first
         parent and of the [k, n-1[ values of the second parent
      -  the second child is made of the [0, k[ values of the second
         parent and of the [k, n-1] values of the first parent

   -  SBX (simulated binary cross-over, cf Deb (2001), p 113). Warning:
      the current implementation is copied from Deb's code and is
      slightly different from the original paper.

-  **Parameters:**

   -  Any variant:

      -  (`mutation_t` and `cross_over_t` are defined in the
         namespace sferes::gen::evo_float`)
      -  SFERES_CONST mutation_t mutation_type:
         polynomial, gaussian or uniform;
      -  SFERES_CONST cross_overt cross_overtype
         -> sbx or recombination;
      -  SFERES_CONST float mutation_rate -> the rate of
         mutation of one member of the vectorf
      -  SFERES_CONST float cross_rate -> the rate of
         cross-over (if no cross-over is applied, parents are copied to
         children)

   -  polynomial mutation:

      -  SFERES_CONST float eta_m -> η_m is a
         parameter which controls the polynomial distribution. See Deb's
         paper or book.

   -  gaussian mutation:

      -  SFERES_CONST float sigma -> the standard deviation of
         the gaussian

   -  uniform mutation:

      -  SFERES_CONST float max -> maximum change of the current
         value

   -  sbx:

      -  SFERES_CONST float eta_c -> see Deb's paper or
         book.

-  **Typical typename:**

::

    typedef sferes::gen::EvoFloat<10, Params1> gen_t;

-  **Typical parameters:**

::

    using namespace sferes::gen::evo_float;

    struct Params1
    {
      struct evo_float
      {
        SFERES_CONST float mutation_rate = 0.1f;
        SFERES_CONST float cross_rate = 0.1f;
        SFERES_CONST mutation_t mutation_type = polynomial;
        SFERES_CONST cross_over_t cross_over_type = sbx;
        SFERES_CONST float eta_m = 15.0f;
        SFERES_CONST float eta_c = 15.0f;
      };
    };

    struct Params2
    {
      struct evo_float
      {
        SFERES_CONST float mutation_rate = 0.1f;
        SFERES_CONST float cross_rate = 0.1f;
        SFERES_CONST mutation_t mutation_type = gaussian;
        SFERES_CONST cross_over_t cross_over_type = recombination;
        SFERES_CONST float sigma = 0.3f;
      };
    };


    struct Params3
    {
      struct evo_float
      {
        SFERES_CONST float mutation_rate = 0.1f;
        SFERES_CONST float cross_rate = 0.1f;
        SFERES_CONST mutation_t mutation_type = uniform;
        SFERES_CONST cross_over_t cross_over_type = sbx;
        SFERES_CONST float max = 0.3f;
        SFERES_CONST float eta_c = 15.0f;
      };
    };

Defining your own genotype
~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    #include <sferes/stc/stc.hpp>
    namespace sferes
    {
      namespace gen
      {  
        SFERES_CLASS(MyGenotype)
        {
        public:
          // generates a random genotype
          void random()
          {
          // your code
          }
          // mutates an individual
          void mutate() 
          { 
          // your code to mutate (you have to handle your own mutation_rate)
          }
          // WARNING: the arguments of this method are of the same type
          // than your genotype
          void cross(const MyGenotype& o, MyGenotype& c1, MyGenotype& c2)
          {
          // if you don't know what to do, simply use:
          if (misc::flip_coin())
            {
              c1 = *this;
              c2 = o;
              }
            else
              {
                c2 = *this;
                c1 = o;
              }   
          }
        protected:
          // your data here (your genotype)
        };
      }
    }

Phenotypes (Individuals)
------------------------

Phenotypes are the expression of the genotypes: it is often useful to
develop a genotype into a structure which is easier to evaluate. For
instance, a neural network can be encoded with a compact encoding
(genotype, e.g. a modular encoding which can repeat the same module
several times), then developped into a standard neural network
(phenotype).

Phenotypes are also the bridge between the fitness function and the
genotype. In sferes2, a phenotype is the composition of a `fit_t`
and a `gen_t`.

Indiv
~~~~~

-  **File:**
   `sferes/phen/indiv.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/indiv.hpp>`__
-  **Description:** The abstract base class for phenotypes.
-  **Main public methods:**

::

    // returns the fitness
    Fit& fit();
    const Fit& fit() const;
    // returns the genotype
    Gen& gen()  { return _gen; }
    const Gen& gen() const { return _gen; }
    // develops the genotype, called by the evalutar before evaluating
    // the individual
    void develop();
    // you can also override the following methods (although in most
    // cases, you should override their equivalent in the genotype)
    void mutate();
    void random();
    void cross(const boost::shared_ptr<Exact> i2, 
             boost::shared_ptr<Exact>& o1, 
             boost::shared_ptr<Exact>& o2);
    // this method is not implemented in Indiv but it must be in all the
    // concrete genotypes. It should describe this particular individual
    // in the stream os (see the Parameter source code for an example)
    void show(std::ostream& os) const

-  **Defined types:**

   -  Fit -> the fitness functor
   -  Gen -> genotype

Parameters
~~~~~~~~~~

-  **File:**
   `sferes/gen/parameters.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/phen/parameters.hpp>`__
-  **Unit test:**
   `sferes/tests/phen/parameters.cpp <https://github.com/sferes2/sferes2/blob/master/tests/phen/parameters.cpp>`__
-  **Description:** This phenotype transforms a list of parameters in
   [0, 1[ (typically a gen::Bitstring or a gen::EvoFloat) to a list of
   parameter in [min, max[. The genotype class must define a method:

::

    cont std::vector<float>& data() const ();

-  **Typical typename:**

::

    typedef eval::Parameters<gen_t, fit_t, Params> phen_t;

Defining your own phenotype
~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    #include <sferes/phen/indiv.hpp>
    namespace sferes
    {
      namespace phen
      {
        SFERES_INDIV(MyPhenotype, Indiv)
        {
        public:
          // constructor
          MyPhenotype() { /* your code */ }
          // develop the genotype to whatever you want
          void develop(){ /* your code */ }
          // describe this individual
          void show(std::ostream& os) const { /* your code */ }
        protected:
          // your data
        };
      }
    }

Evaluators
----------

Evaluators are in charge of evalutating a vector of individuals (i.e.
running phen_t::fit::eval() for each individual).

Eval
~~~~

-  **File:**
   `sferes/eval/eval.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/eval/eval.hpp>`__
-  A basic evaluator (no multiprocessing, no MPI, etc.), i.e. a basic
   loop.
-  **Typical typename:**

::

    typedef eval::Eval<Params> eval_t;

Parallel
~~~~~~~~

-  **File:**
   `sferes/eval/parallel.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/eval/parallel.hpp>`__
-  **Description:** A SMP/multicore evaluator based on TBB
   (http://www.threadingbuildingblocks.org/). **Your fitness should be
   thread-safe!**
-  **Typical typename:**

::

    typedef eval::Parallel<Params> eval_t;

Mpi
~~~

-  **File:** `sferes/eval/mpi.hpp <sferes/eval/mpi.hpp>`__
-  **Description:** A MPI evaluator based on boost::mpi. It works on
   clusters but keep in mind that every cluster is different!
-  **Typical typename:**

::

    typedef eval::Mpi<Params> eval_t;

Defining your own evaluator
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The only required method is eval. It should call
phen_t::develop() then phen_t::fit::eval() for each
individual.

::


    namespace sferes
    {
      namespace eval
      {
        SFERES_EVAL(Eval)
        {
        public:
          template<typename Phen>
            void eval(std::vector<boost::shared_ptr<Phen> >& pop, size_t begin, size_t end)
          {
            for (size_t i = begin; i < end; ++i)
              {
                pop[i]->develop();
                pop[i]->fit().eval(*pop[i]);
              }
          }
        protected:
        };
      }
    }

Modifiers
---------

Modifiers are run once all the individuals have been evaluated but
before any sorting. They are designed to allow to modify the fitness
values to implement niching strategies, diversity preservation
mechanisms, etc.

Dummy
~~~~~

-  **File:**
   `sferes/modif/dummy.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/modif/dummy.hpp>`__
-  **Description:** basic modifier which does nothing.

Diversity
~~~~~~~~~


Novelty
~~~~~~~

Misc
----

The misc namespace contains useful small functions for:

-  system access (e.g getpid())
-  random number generation
