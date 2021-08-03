Reference manual
=================
.. highlight:: c++


The QD framework introduces 4 new types to consider:

-  `qd_t` : type of Quality Diversity (QD) algorithm (e.g. qd::QualityDiversity, which is itself a subclass of ea::Ea).
-  `selector_t` : type of selector, used for selecting individuals from the container (e.g. qd::selector::Uniform)
-  `container_t` : type of QD container (e.g. qd::container::Grid). The container may rely on a specific storage for its individuals.
-  `storage_t` : type of storage; may be used for organising the individuals of the container. Some containers require such a storage (e.g. unstructured containers: qd::container::Archive).

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

The container is used to gather a collection of diverse and high-performing solutions.
Each type of container presents its own rules for preserving the behavioural diversity and local performance of the solutions.

Grid
~~~~

-  **File:**
`sferes/qd/container/grid.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/container/grid.hpp>`__

-  **Description:**
    - This type of container creates a grid container (e.g. as seen in  MAP Elites). The idea is to discretize the Behavioural Descriptor space in evenly sized cells and then fill these up with a qd algorithm.
-  **Example:**
    - `sferes/qd/examples/ex_map_elites.cpp <https://github.com/sferes2/sferes2/blob/qd/examples/ex_map_elites.cpp>`__
    - `AIRL/examples/cpp/standard/robotdart-example/src/dart_exp.cpp <https://gitlab.doc.ic.ac.uk/AIRL/examples/cpp/standard/robotdart-example/-/blob/master/src/dart_exp.cpp>`__
    - `tests/qd/test_qd.cpp <https://github.com/sferes2/sferes2/blob/qd/tests/qd/test_qd.cpp>`__

-  **Typical typename:**

::

 typedef qd::container::Grid<phen_t, Params> container_t;



-  **Parameters:**
    - `Params::qd::grid_shape`: size of the grid in each dimension
    - `Params::qd::behav_dim`: Dimensions of the Behavioral Descriptor
    - `Params::nov::deep`:  this parameter defines how deep we should go around our cell to calculate the novelty

-  **Notes:**

CVT
~~~

-  **File:**
`sferes/qd/container/cvt.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/container/cvt.hpp>`__

-  **Description:**
    - This type of container discretises the feature space with a Centroidal Voronoi Tessellation (CVT), e.g. as seen in `CVT-MAP-Elites <https://ieeexplore.ieee.org/document/8000667>`__. That CVT splits the Behavioural Descriptor space in evenly sized cells and then fill these up with a QD algorithm.
-  **Example:**
    - `tests/qd/test_qd.cpp <https://github.com/sferes2/sferes2/blob/qd/tests/qd/test_qd.cpp>`__

-  **Typical typename:**

::

 typedef qd::container::CVT<phen_t, storage_t, Params> container_t;



-  **Parameters:**
    - `Params::qd::n_niches` (e.g, 10000): number of niches
    - `Params::qd::behav_dim` (e.g, 2): number of feature dimensions
    - `Params::qd::cvt::n_samples` (e.g, 100000): number of samples for CVT (more than n_niches); ignored if using a cached CVT
    - `Params::qd::cvt::max_iterations` (e.g, 100): number of iterations of the CVT algorithm; ignored if using a cached CVT
    - `Params::qd::cvt::n_restarts` (e.g, 1):  number of restarts of the CVT algorithm; ignored if using a cached CVT
    - `Params::qd::cvt::tolerance` (e.g, 1e-8):  when to stop the CVT algorithm; ignored if using a cached CVT
-  **Notes:**
    - During initialisation, the presence of cached centroids `cache_centroids_<dim>_<n_niches>.bin` is checked.
        - If that file is not present, then the file `sferes/qd/container/compute_cvt.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/container/compute_cvt.hpp>`__ is used to compute the CVT that partitions the feature space.
        - If that file is present, then the CVT centroids are directly loaded from it. The parameters in `Params::qd::cvt::` are then ignored.
    - The centroids are stored in a specific storage of type <storage_t>, which facilitates the computation of k-Nearest Neighbours (kNN). The different types of possible storages are detailed below.

Archive
~~~~~~~

-  **File:**
`sferes/qd/container/archive.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/container/archive.hpp>`__

-  **Description:**
    - The Archive corresponds to an unstructured container, here called `Archive` (e.g. as introduced in the `QD framework paper <https://ieeexplore.ieee.org/document/7959075>`__). The idea is to avoid discretising the Behavioural Descriptor (BD) space. Instead, solutions may be added depending on the distance between their BDs and their kNNs.
    - More precisely, a solution `i` is added to the container if the distance between its BD and its nearest neighbour is inferior to `Params::nov::l`.
    - Also, a solution `i` may replace its nearest neighbour `nn` if the following conditions hold:
        - `N(i) >= (1-eps) N(nn)`, where `N(.)` represents the novelty score (average distance to kNNs).
        - `F(i) >= (1-eps) F(nn)`, where `F(.)` represents the fitness score (supposed positive in this description).
        - `(N(i) - N(nn)) / N(nn) > -(F(i) - F(nn)) / F(nn)`

-  **Example:**
    - see `qd_archive_sortbased` and `qd_archive_kdtree` in `tests/qd/test_qd.cpp <https://github.com/sferes2/sferes2/blob/qd/tests/qd/test_qd.cpp>`__

-  **Typical typename:**

::

 typedef qd::container::Archive<phen_t, storage_t, Params> container_t;



-  **Parameters:**
    - `Params::nov::k`: number of nearest neighbours to consider for computing the novelty score.
    - `Params::nov::l`: minimal distance for adding a new individual to the container.
    - `Params::nov::eps`: used to set the conditions for epsilon dominance, as detailed above.

-  **Notes:**
    - The individuals are stored in a specific storage of type <storage_t>, which facilitates the computation of k-Nearest Neighbours (kNN) and novelty scores. The different types of possible storages are detailed below.
    - When the container is updated (at each iteration of the QD algorithm), the novelty scores and local quality scores are re-computed for all individuals.

Defining your own container
~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    template <typename Phen, typename Params>
    class CustomContainer {
    public:
        typedef boost::shared_ptr<Phen> indiv_t;
        typedef typename std::vector<indiv_t> pop_t;

        CustomContainer() {}

        // Adds all the individuals/solutions of the container to the population `content`
        void get_full_content(pop_t& content) const
        { /* your code */ }

        // Adds an individual `i1` to the container if all the container conditions are verified
        bool add(indiv_t i1)
        { /* your code */ }

        // Adds an individual `i1` to the container regardless of the container conditions
        void direct_add(const indiv_t& i1)
        { /* your code */ }

        // Updates attributes of the container, and of individuals from `offspring` and `parents`
        // such attributes may include the novelty score, and the local quality score
        void update(pop_t& offspring, pop_t& parents)
        { /* your code */ }
    };

Storage
-------

The purpose of the storage is to provide an interface facilitating the computation of k-Nearest Neighbours (kNN) and novelty scores.

SortBasedStorage
~~~~~~~~~~~~~~~~

-  **File:**
`sferes/qd/container/sort_based_storage.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/container/sort_based_storage.hpp>`__

-  **Description:**
    - Relies on `std::partial_sort` to compute the k-nearest neighbors.

-  **Example:**
    - `tests/qd/test_qd.cpp <https://github.com/sferes2/sferes2/blob/qd/tests/qd/test_qd.cpp>`__

-  **Typical typename:**

::

typedef sferes::qd::container::SortBasedStorage<boost::shared_ptr<phen_t>> storage_t;


-  **Parameters:**
    - None

-  **Notes:**
    - With the above typename, the storage stores pairs `(bd, indiv_t)` where `indiv_t = boost::shared_ptr<phen_t>`
    - This is not effective in low-dimensional spaces (dim(bd) < 10); but it works better than a kd-tree for more than 10-dimensional spaces


KdtreeStorage
~~~~~~~~~~~~~


-  **File:**
`sferes/qd/container/kdtree_storage.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/container/kdtree_storage.hpp>`__

-  **Description:**
    - Computes the k-nearest neighbors using a `kd-tree <https://en.wikipedia.org/wiki/K-d_tree>`__ (optional external library).

-  **Example:**
    - `tests/qd/test_qd.cpp <https://github.com/sferes2/sferes2/blob/qd/tests/qd/test_qd.cpp>`__

-  **Typical typename:**

::

typedef sferes::qd::container::KdtreeStorage<boost::shared_ptr<phen_t>, Params::qd::behav_dim> storage_t;


-  **Parameters:**
    - None

-  **Notes:**
    - With the above typename, the storage stores pairs `(bd, indiv_t)` where `indiv_t = boost::shared_ptr<phen_t>`
    - This is effective in low-dimensional spaces (dim(bd) < 10); but not in high-dimensional spaces (dim(bd) > 10)


Selector
--------

Uniform
~~~~~~~

-  **File:** `sferes/qd/selector/uniform.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/selector/uniform.hpp>`__

-  **Description:** This selector selects individuals from the population of the algorithm with uniform probability. All individuals from the population have the exact same probability to be selected.

-  **Example:** `sferes/qd/examples/ex_qd.cpp <https://github.com/sferes2/sferes2/blob/qd/examples/ex_qd.cpp>`__

-  **Typical typename:**
::

 typedef qd::selector::Uniform<phen_t, Params> select_t;

NoSelection
~~~~~~~~~~~

-  **File:** `sferes/qd/selector/noselection.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/selector/noselection.hpp>`__

-  **Description:** This selector returns a population of random individuals. In other words, it is not performing any selection from the existing algorithm population, but returns new individuals.

-  **Example:** None

-  **Typical typename:**
::

 typedef qd::selector::NoSelection<phen_t, Params> select_t;

ParetoBased
~~~~~~~~~~~

-  **File:** `sferes/qd/selector/pareto_based.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/selector/pareto_based.hpp>`__

-  **Description:** This selector selects individuals in the joint parents and offspring populations based on pareto-dominance. It selects individuals from this joint-population uniformly, and then performs tournament to keep the bests according to pareto-dominance. The template parameters ObjSelector determines which objectives defined in the fitness class are used to compute the pareto-dominance. 

-  **Notes:** This selector relies on the definition of the different objectives in the fitness class. It also relies on multiple classes define in `sferes/ea/crowd.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/ea/crowd.hpp>`__ to approximate the pareto front. 

-  **Parameters:** `Params::pareto::genoDiv`: boolean to choose if the crowding distance should be based on the distance in genotype space (true) or in objective space (false).

-  **Example:** None

-  **Typical typename:**
::

 typedef qd::selector::ParetoBased<phen_t, objselector_t, Params> select_t;

ValueSelector
~~~~~~~~~~~~~

-  **File:** `sferes/qd/selector/value_selector.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/selector/value_selector.hpp>`__

-  **Description:** Some selectors, described later in this section, are based on a score allocated to each individual. ValueSelector structs allow to choose which score is used by these selectors. One can choose among the followings:
    - **getFitness:** use the fitness of each individual given by the evaluation.
    - **getNovelty:** use the novelty of each individual with respect to the task.
    - **getCuriosity:** use the curiosity of each individual, computed based on the performance of its offspring.
    - **getLocalQuality:** use the local quality of each individual with respect to the task.

ValueSelector-based: ScoreProportionate
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  **File:** `sferes/qd/selector/noselection.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/selector/noselection.hpp>`__

-  **Description:** This selector selects individuals in the algorithm population with a probability proportional to a given score. The score used for this selection is determined by the ValueSelector given as template parameters. The individual of the population with the lower score has a 0 probability to be chosen, and all other individuals have consequent probability based on their score values.

-  **Notes:** This selector is based on ValueSelector, see previous sub-section "ValueSelector" for more information.

-  **Example:** None

-  **Typical typename:**
::

 typedef qd::selector::ScoreProportionate<phen_t, valueselector_t, Params> select_t;

ValueSelector-based: Tournament
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  **File:** `sferes/qd/selector/tournament.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/selector/tournament.hpp>`__

-  **Description:** This selector selects individuals in the algorithm population with tournaments based on a given score. The score is determined by the ValueSelector given as template parameters. To perform tournaments, the selector uniformly selects two individuals from the population and keeps the one that has the highest score.

-  **Notes:** This selector is based on ValueSelector, see previous sub-section "ValueSelector" for more information.

-  **Example:** None

-  **Typical typename:**
::

 typedef qd::selector::Tournament<phen_t, valueselector_t, Params> select_t;

Meta-Selector: PopulationBased
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  **File:** `sferes/qd/selector/population_based.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/qd/selector/population_based.hpp>`__

-  **Description:** This meta-selector allows to select from the joint offspring and parents populations of the algorithm, instead of its current population. The selector that is applied on this joint-population is given as the second template parameter.

-  **Notes:** This selector is a meta-selector: it takes as template argument another selector and acts as a wrapper around it.

-  **Example:** None

-  **Typical typename:**
::

 typedef qd::selector::PopulationBased<phen_t, sub_select_t, Params> select_t;

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

::

    FIT_QD(CustomFitnessQD)
    {
        // (optional) constructor
        CustomFitnessQD()
        { /* your code */ }

        // evaluation
        template<typename Indiv>
        void eval(Indiv& indiv)
        {
            this->_value = -42; // Set fitness score
            this->set_desc({0.3, 0.7, 0.1}); // Set behavioural descriptor, supposing Params::qd::behav_dim == 3
        }
    };


QD Statistics
-------------

QdContainer
~~~~~~~~~~~


-  **File:**
`sferes/stat/qd_container.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/stat/qd_container.hpp>`__

-  **Description:**
    - Every `dump_period`, writes a file `archive_<gen>.dat` (where `gen` is the generation number), with some information related to all individuals present in the archive at generation `gen`. Every line of such a file presents some information related to one individual. For each individual, the following values are saved (in order):
        - Index
        - Behavioural Descriptor
        - Fitness Score
        - Genotype

-  **Example:**
    - `examples/ex_qd.cpp <https://github.com/sferes2/sferes2/blob/qd/examples/ex_qd.cpp>`__

-  **Typical typename:**

::

    typedef boost::fusion::vector<
                // Other Stats, ...,
                stat::QdContainer<phen_t, Params>
            >
            stat_t;

-  **Parameters:**
    - `Params::pop::dump_period` : Generation period for writing a file `archive_<gen>.dat`


QdProgress
~~~~~~~~~~


-  **File:**
`sferes/stat/qd_container.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/stat/qd_container.hpp>`__

-  **Description:**
    - Every `dump_period`, writes a line at the end of the file `progress.dat` with some information related to the state of the container at generation `gen`. For each line, the following values are saved (in order):
        - Generation number
        - Archive size
        - Maximum fitness score from the archive
        - Sum of the fitness scores from the archive (also known as `QD score`)
        - Sum of the novelty scores from the archive
        - Variance of the novelty scores from the archive

-  **Example:**
    - `examples/ex_qd.cpp <https://github.com/sferes2/sferes2/blob/qd/examples/ex_qd.cpp>`__

-  **Typical typename:**

::

    typedef boost::fusion::vector<
                // Other Stats, ...,
                stat::QdProgress<phen_t, Params>
            >
            stat_t;

-  **Parameters:**
    - `Params::pop::dump_period` : Generation period for writing a file `archive_<gen>.dat`



QdSelection
~~~~~~~~~~~

-  **File:**
`sferes/stat/qd_selection.hpp <https://github.com/sferes2/sferes2/blob/qd/sferes/stat/qd_selection.hpp>`__

-  **Description:**
    - Every generation `gen`, writes several lines at the end of the file `selection.dat` with some information related to the state of the parents and offspring populations at generation `gen`. At each generation, `n` lines are written. For each `i` between `0` and `n`, the following values are saved (in order):
        - Generation number
        - Behavioural Descriptor of individual having index `i` in the parents population
        - Fitness score of parent `i`
        - Novelty score of parent `i`
        - Local Quality score of parent `i`
        - Curiosity score of parent `i`
        - Behavioural Descriptor of individual having index `i` in the offspring population
        - Fitness score of offspring individual `i`
        - Novelty score of offspring individual `i`
        - Local Quality score of offspring individual `i`
        - Curiosity score of offspring individual `i`
        - Boolean indicating if offspring individual `i` was added to the container

-  **Typical typename:**

::

    typedef boost::fusion::vector<
                // Other Stats, ...,
                stat::QdSelection<phen_t, Params>
            >
            stat_t;

-  **Parameters:**
    - `Params::qd::behav_dim`: Dimensions of the Behavioral Descriptor

-  **Notes:**
    - the populations `ea.parents()`, `ea.offspring()` and the vector of booleans `ea.added()` need to have the same size.

