Reference manual
=================
.. highlight:: c++


The QD framework introduces 4 new types to consider:
-  `qd_t` : type of Quality Diversity (QD) algorithm (e.g. qd::QualityDiversity, which is itself a subclass of ea::Ea).
-  `selector_t` : type of selector, used for selecting individuals from the container (e.g. qd::selector::Uniform)
-  `container_t` : type of QD container (e.g. qd::container::Grid).
The container may rely on a specific storage for its individuals.
-  `storage_t` : type of storage; may be used for organising the individuals of the container.
Some containers require such a storage (e.g. unstructured containers: qd::container::Archive).

The QD framework relies on a Fitness definition which is specific to QD (FitQD).
Also, some additional statistics were defined to generate logs for QD algorithms.

QualityDiversity
----------------

-  **File:**
   `sferes/qd/quality_diversity.hpp <https://github.com/sferes2/sferes2/blob/master/sferes/qd/quality_diversity.hpp>`__

-  **Description:** General implementation of a Quality Diversity algorithm. At each iteration:
    1) Parents are selected from the Container based by using the Selector
    2) The offspring is generated via cross-overs and mutations of parents
    3) The offspring is evaluated
    4) We apply the Modifiers.
    5) We attempt to add the individuals in the offspring to the Container.

-  **Example:**
   `sferes/qd/quality_diversity.hpp <https://github.com/sferes2/sferes2/blob/master/examples/ex_qd.cpp>`__

-  **Typical typename:**

::

    typedef qd::QualityDiversity<phen_t, eval_t, stat_t, modifier_t, selector_t, container_t, Params> qd_t;



-  **Parameters:**
    - `SFERES_CONST size_t Params::pop::size`: Number of individuals selected from the container at each QD iteration (also used as an initial population size)


Container
---------

Grid
~~~~

-  **File:**
`sferes/qd/container/grid.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/container/grid.hpp>`__

-  **Description:**
    - This type of container creates a grid container (e.g. as seen in  MAP Elites). The idea is to discretize the Behavioural Descriptor space in evenly sized cells and then fill these up with a qd algorithm.
-  **Example:**
    - `sferes/qd/examples/ex_map_elites.cpp <https://github.com/sferes2/sferes2/blob/qd/examples/ex_map_elites.cpp>`__

    - `AIRL/examples/cpp/standard/robotdart-example/src/dart_exp.cpp <https://gitlab.doc.ic.ac.uk/AIRL/examples/cpp/standard/robotdart-example/-/blob/master/src/dart_exp.cpp>`__

-  **Typical typename:**

::

 typedef qd::container::Grid<phen_t, Params> container_t;



-  **Parameters:**
    - `Params::qd::grid_shape : size of the grid in each dimension`
    -  `Params::nov::deep :  this parameter defines how deep we should go around our cell to calculate the novelty`
-  **Notes:**

CVT
~~~

Archive
~~~~~~~



Defining your own container
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. Description

Example:

.. Example




Storage
-------

SortBasedStorage
~~~~~~~~~~~~~~~~

-  **File:**
-  **Description:**
-  **Example:**
-  **Typical typename:**
-  **Parameters:**
-  **Notes:**


KdtreeStorage
~~~~~~~~~~~~~




Selector
--------

Uniform
~~~~~~~

-  **File:** `sferes/qd/selector/uniform.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/selector/uniform.hpp>`__

-  **Description:** This selector selects individuals from the population of the algorithm with uniform probability. 

-  **Example:** `sferes/qd/examples/ex_qd.cpp <https://github.com/sferes2/sferes2/blob/qd/examples/ex_qd.cpp>`__

-  **Typical typename:**
::

 typedef qd::selector::Uniform<phen_t, Params> select_t;

NoSelection
~~~~~~~~~~~

-  **File:** `sferes/qd/selector/noselection.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/selector/noselection.hpp>`__

-  **Description:** This selector returns a population of random individuals. In other words, it is not performing any selection from the existing algorithm population.

-  **Example:** None

-  **Typical typename:**
::

 typedef qd::selector::NoSelection<phen_t, Params> select_t;

ValueSelector
~~~~~~~~~~~~~

-  **File:** `sferes/qd/selector/value_selector.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/selector/value_selector.hpp>`__

-  **Description:** Some selectors, described later in this section, are based on a score allocated to each individual such as fitness or novelty. ValueSelector structs allow to choose which score is used by these selectors. One can choose among the followings:
   - **getFitness:** use the fitness of each individual given by the evaluation.
   - **getNovelty:** use the novelty of each individual with respect to the task.
   - **getCuriosity:** use the curiosity of each individual, computed based on the performance of its offspring.
   - **getLocalQuality:** use the local quality of each individual with respect to the task.

ValueSelector-based: ScoreProportionate
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  **File:** `sferes/qd/selector/noselection.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/selector/noselection.hpp>`__

-  **Description:** This selector selects individuals in the algorithm population with a probability proportional to a given score. The score used for this selection is determined by the ValueSelector given as template parameters. The individual of the population with the lower score has a 0 probability to be chosen and all other individuals have consequent probability based on their score values.

-  **Example:** None

-  **Typical typename:**
::

 typedef qd::selector::ScoreProportionate<phen_t, valueselector_t, Params> select_t;

-  **Notes:** This selector is based on ValueSelector, see previous sub-section "ValueSelector" for more information.

ValueSelector-based: Tournament
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  **File:** `sferes/qd/selector/tournament.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/selector/tournament.hpp>`__

-  **Description:** This selector selects individuals in the algorithm population with tournaments based on a given score. The score is determined by the ValueSelector given as template parameters. The selector uniformly selects two individuals from the population and keeps the one that has the highest score.

-  **Example:** None

-  **Typical typename:**
::

 typedef qd::selector::Tournament<phen_t, valueselector_t, Params> select_t;

-  **Notes:** This selector is based on ValueSelector, see previous sub-section "ValueSelector" for more information.

Meta-Selector: ParetoBased
~~~~~~~~~~~~~~~~~~~~~~~~~~

Meta-Selector: PopulationBased
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Defining your own selector
~~~~~~~~~~~~~~~~~~~~~~~~~~




QD Fitness
----------

FitQD
~~~~~

-  **File:**
`sferes/fit/fit_qd.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/fit/fit_qd.hpp>`__

-  **Description:**
    - The fitness function has the purpose of evaluating your individual and attributing it a Behavioral Descriptor and a fitness value (if needed). 
    You can define what you want in the fitness function but the eval function needs to be defined since it is used after each offspring generation to evaluate the new individuals.
-  **Example:**
`sferes/qd/examples/ex_qd.hpp <https://github.com/sferes2/sferes2/blob/qd/examples/ex_qd.cpp>`__

-  **Typical typename:**

::

    typedef Fitness_Function_Name<Params> fit_t;
    


-  **Parameters:**
    - `Params::qd::behav_dim` : Dimensions of the Behavioral Descriptor 
-  **Notes:**

Defining your own QD Fitness
~~~~~~~~~~~~~~~~~~~~~~~~~~~~




QD Statistics
-------------

QdContainer
~~~~~~~~~~~

-  **File:**
-  **Description:**
-  **Example:**
-  **Typical typename:**
-  **Parameters:**
-  **Notes:**

QdProgress
~~~~~~~~~~

QdSelection
~~~~~~~~~~~









