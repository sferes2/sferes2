//| This file is a part of the sferes2 framework.
//| Copyright 2016, ISIR / Universite Pierre et Marie Curie (UPMC)
//| Main contributor(s): Jean-Baptiste Mouret, mouret@isir.fr
//|
//| This software is a computer program whose purpose is to facilitate
//| experiments in evolutionary computation and evolutionary robotics.
//|
//| This software is governed by the CeCILL license under French law
//| and abiding by the rules of distribution of free software.  You
//| can use, modify and/ or redistribute the software under the terms
//| of the CeCILL license as circulated by CEA, CNRS and INRIA at the
//| following URL "http://www.cecill.info".
//|
//| As a counterpart to the access to the source code and rights to
//| copy, modify and redistribute granted by the license, users are
//| provided only with a limited warranty and the software's author,
//| the holder of the economic rights, and the successive licensors
//| have only limited liability.
//|
//| In this respect, the user's attention is drawn to the risks
//| associated with loading, using, modifying and/or developing or
//| reproducing the software by the user in light of its specific
//| status of free software, that may mean that it is complicated to
//| manipulate, and that also therefore means that it is reserved for
//| developers and experienced professionals having in-depth computer
//| knowledge. Users are therefore encouraged to load and test the
//| software's suitability as regards their requirements in conditions
//| enabling the security of their systems and/or data to be ensured
//| and, more generally, to use and operate it in the same conditions
//| as regards security.
//|
//| The fact that you are presently reading this means that you have
//| had knowledge of the CeCILL license and that you accept its terms.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE qd_archive

#include <cmath>
#include <iostream>

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/multi_array.hpp>

#include <boost/test/unit_test.hpp>

#include <sferes/eval/parallel.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/stat/state.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/phen/parameters.hpp>
#include <sferes/run.hpp>

#include <sferes/qd/quality_diversity.hpp>
#include <sferes/fit/fit_qd.hpp>
#include <sferes/qd/container/archive.hpp>
#include <sferes/qd/container/grid.hpp>
#include <sferes/qd/container/kdtree_storage.hpp>
#include <sferes/qd/container/sort_based_storage.hpp>
#include <sferes/qd/selector/tournament.hpp>
#include <sferes/qd/selector/uniform.hpp>
#include <sferes/qd/selector/population_based.hpp>
#include <sferes/qd/selector/score_proportionate.hpp>

#include <sferes/stat/best_fit.hpp>
#include <sferes/stat/qd_container.hpp>
#include <sferes/stat/qd_selection.hpp>
#include <sferes/stat/qd_progress.hpp>
#include <sferes/stat/state_qd.hpp>

using namespace sferes::gen::evo_float;

struct Params {
    // TODO: move to a qd::
    struct nov {
        SFERES_CONST size_t deep = 2;
        SFERES_CONST double l = 0.0075;
        SFERES_CONST double k = 24;
        SFERES_CONST double eps = 0.1;
    };
    // TODO: move to a qd::/ea
    struct pop {
        // number of initial random points
        SFERES_CONST size_t init_size = 1000;
        // size of a batch
        SFERES_CONST size_t size = 200;
        static size_t nb_gen;
        SFERES_CONST size_t dump_period = 100;
    };
    struct parameters {
        SFERES_CONST float min = -5;
        SFERES_CONST float max = 5;
    };
    struct evo_float {
        SFERES_CONST float cross_rate = 0.5f;
        SFERES_CONST float mutation_rate = 0.1f;
        SFERES_CONST float eta_m = 10.0f;
        SFERES_CONST float eta_c = 10.0f;
        SFERES_CONST mutation_t mutation_type = polynomial;
        SFERES_CONST cross_over_t cross_over_type = sbx;
    };
    struct qd {
      SFERES_CONST size_t behav_dim = 2; // number of feature dimensions
        SFERES_ARRAY(size_t, grid_shape, 100, 100); // for grid (map-elites)
        SFERES_CONST int n_niches = 10000; // for CVT

        struct cvt {
            // The following parameters are not used if you use a cached CVT:
            SFERES_CONST int n_samples = 10000;// number of samples for CVT (more than n_niches)
            SFERES_CONST int max_iterations = 100;// number of iterations of the CVT algorithm
            SFERES_CONST int n_restarts = 1; // number of restarts of the CVT algorithm
            SFERES_CONST double tolerance = 1e-8; // when to stop the CVT algorithm
        };
    };
};
size_t Params::pop::nb_gen = 500;

// Rastrigin
FIT_QD(Rastrigin){
    public : 
    template <typename Indiv> 
    void eval(Indiv & ind){
        float f = 10 * ind.size();
        for (size_t i = 0; i < ind.size(); ++i)
            f += ind.data(i) * ind.data(i) - 10 * cos(2 * M_PI * ind.data(i));
        this->_value = -f;

        std::vector<double> data = {ind.gen().data(0), ind.gen().data(1)};
        this->set_desc(data);
    }
};

template <typename TQualityDiversity>
void test_resume() {
    // Run algorithm
    std::cout<<"Running the algorithm for 501 generations"<<std::endl;
    Params::pop::nb_gen = 501;
    TQualityDiversity qd_1;
    qd_1.run();

    // General test
    std::cout<<qd_1.template stat<0>().best()->fit().value()<<" "<<qd_1.template stat<1>().archive().size()<<std::endl;
    BOOST_CHECK(qd_1.template stat<1>().archive().size() > 8500);
    BOOST_CHECK(qd_1.template stat<0>().best()->fit().value() > -50);


    // Resume for same number of gen to see similarities
    std::cout<<"Resuming it for same number of gen"<<std::endl;
    TQualityDiversity qd_2;
    qd_2.resume(qd_1.res_dir() + "/gen_500");

    // General test
    std::cout<<qd_2.template stat<0>().best()->fit().value()<<" "<<qd_2.template stat<1>().archive().size()<<std::endl;
    BOOST_CHECK(qd_2.template stat<1>().archive().size() > 8500);
    BOOST_CHECK(qd_2.template stat<0>().best()->fit().value() > -50);

    // Test similarity to previous algo
    BOOST_CHECK(qd_2.template stat<1>().archive().size() == qd_1.template stat<1>().archive().size());
    BOOST_CHECK(qd_2.template stat<0>().best()->fit().value() == qd_1.template stat<0>().best()->fit().value());

    BOOST_CHECK(qd_2.parents().size() == qd_1.parents().size());
    BOOST_CHECK(qd_2.offspring().size() == qd_1.offspring().size());


    // Resume for higher number of generations
    std::cout<<"Resuming it for double number of gen"<<std::endl;
    Params::pop::nb_gen = 1001;
    TQualityDiversity qd_3;
    qd_3.resume(qd_1.res_dir() + "/gen_500");
    // General test
    std::cout<<qd_3.template stat<0>().best()->fit().value()<<" "<<qd_3.template stat<1>().archive().size()<<std::endl;
    BOOST_CHECK(qd_3.template stat<1>().archive().size() > 8500);
    BOOST_CHECK(qd_3.template stat<0>().best()->fit().value() > -50);
}


BOOST_AUTO_TEST_CASE(resume_map_elites)
{
    using namespace sferes;

    typedef Rastrigin<Params> fit_t;
    typedef gen::EvoFloat<10, Params> gen_t;
    typedef phen::Parameters<gen_t, fit_t, Params> phen_t;

    typedef eval::Parallel<Params> eval_t;

    typedef boost::fusion::vector<
        stat::BestFit<phen_t, Params>, 
        stat::QdContainer<phen_t, Params>, 
        stat::QdProgress<phen_t, Params>, 
        stat::QdSelection<phen_t, Params>>
        stat_t; 
    typedef modif::Dummy<> modifier_t;
    typedef qd::MapElites<phen_t, eval_t, stat_t, modifier_t, Params>
        qd_t;

    test_resume<qd_t>();
}

BOOST_AUTO_TEST_CASE(resume_cvt_map_elites)
{
    using namespace sferes;

    typedef Rastrigin<Params> fit_t;
    typedef gen::EvoFloat<10, Params> gen_t;
    typedef phen::Parameters<gen_t, fit_t, Params> phen_t;

    typedef eval::Parallel<Params> eval_t;

    typedef boost::fusion::vector<
    stat::BestFit<phen_t, Params>,
    stat::QdContainer<phen_t, Params>,
    stat::QdProgress<phen_t, Params>,
    stat::QdSelection<phen_t, Params>>
    stat_t;

    typedef modif::Dummy<> modifier_t;
    typedef qd::CvtMapElites<phen_t, eval_t, stat_t, modifier_t, Params> qd_t;

    test_resume<qd_t>();
}

BOOST_AUTO_TEST_CASE(resume_archive)
{
    using namespace sferes;

    typedef Rastrigin<Params> fit_t;
    typedef gen::EvoFloat<10, Params> gen_t;
    typedef phen::Parameters<gen_t, fit_t, Params> phen_t;

    typedef eval::Parallel<Params> eval_t;

    typedef boost::fusion::vector<stat::BestFit<phen_t, Params>, stat::QdContainer<phen_t, Params>,
        stat::QdProgress<phen_t, Params>, stat::QdSelection<phen_t, Params>>
        stat_t;
    typedef modif::Dummy<> modifier_t;
    
    typedef qd::selector::Uniform<phen_t, Params> selector_t;
    typedef qd::container::SortBasedStorage< boost::shared_ptr<phen_t> > storage_t;
    typedef qd::container::Archive<phen_t, storage_t, Params> container_t;

    typedef qd::QualityDiversity<phen_t, eval_t, stat_t, modifier_t, selector_t, container_t,
        Params>
        qd_t;

    test_resume<qd_t>();
}

#ifdef USE_KDTREE

BOOST_AUTO_TEST_CASE(resume_archive_kdtree)
{
    using namespace sferes;

    typedef Rastrigin<Params> fit_t;
    typedef gen::EvoFloat<10, Params> gen_t;
    typedef phen::Parameters<gen_t, fit_t, Params> phen_t;

    typedef eval::Parallel<Params> eval_t;

    typedef boost::fusion::vector<stat::BestFit<phen_t, Params>, stat::QdContainer<phen_t, Params>,
        stat::QdProgress<phen_t, Params>, stat::QdSelection<phen_t, Params>>
        stat_t;
    typedef modif::Dummy<> modifier_t;
    
    typedef qd::selector::Uniform<phen_t, Params> selector_t;
    typedef qd::container::KdtreeStorage< boost::shared_ptr<phen_t>, Params::qd::behav_dim > storage_t;    
    typedef qd::container::Archive<phen_t, storage_t, Params> container_t;


    typedef qd::QualityDiversity<phen_t, eval_t, stat_t, modifier_t, selector_t, container_t,
        Params>
        qd_t;

    test_resume<qd_t>();

}
#endif
