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
#include <sferes/modif/dummy.hpp>
#include <sferes/phen/parameters.hpp>
#include <sferes/run.hpp>
#include <sferes/stat/best_fit.hpp>
#include <sferes/stat/qd_container.hpp>
#include <sferes/stat/qd_selection.hpp>
#include <sferes/stat/qd_progress.hpp>


#include <sferes/fit/fit_qd.hpp>
#include <sferes/qd/container/archive.hpp>
#include <sferes/qd/container/grid.hpp>
#include <sferes/qd/container/kdtree_storage.hpp>
#include <sferes/qd/container/sort_based_storage.hpp>
#include <sferes/qd/quality_diversity.hpp>
#include <sferes/qd/selector/tournament.hpp>
#include <sferes/qd/selector/uniform.hpp>

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
        SFERES_CONST size_t nb_gen = 500;
        SFERES_CONST size_t dump_period = -1;
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

BOOST_AUTO_TEST_CASE(qd_archive_sortbased)
{
    using namespace sferes;

    typedef Rastrigin<Params> fit_t;
    typedef gen::EvoFloat<10, Params> gen_t;
    typedef phen::Parameters<gen_t, fit_t, Params> phen_t;

    typedef qd::container::SortBasedStorage< boost::shared_ptr<phen_t> > storage_t;
    typedef qd::container::Archive<phen_t, storage_t, Params> container_t;

    container_t archive;
    // archive initially empty
    BOOST_CHECK(archive.archive().size() == 0);
    container_t::indiv_t indiv(new phen_t);

    std::vector<double> data = {0.5,0.5};
    indiv->fit().set_desc(data);
    indiv->fit().set_value(0);
    // add indiv when archive empty, always works
    BOOST_CHECK(archive.add(indiv));

    // add the same indiv twice, never works
    BOOST_CHECK(!archive.add(indiv));

    
    container_t::indiv_t indiv2(new phen_t);
    std::vector<double> data2 = data;
    data2[0]+=0.5*Params::nov::l;
    indiv2->fit().set_desc(data2);
    indiv2->fit().set_value(0);
    // add new indiv too close from already existing one
    BOOST_CHECK(!archive.add(indiv2));
    
    data2[0]=0.5+1.1*Params::nov::l;
    indiv2->fit().set_desc(data2);
    // add new indiv far enough from already existing one, should work
    BOOST_CHECK(archive.add(indiv2));
    BOOST_CHECK(archive.archive().size() == 2);

    container_t::indiv_t indiv3(new phen_t);
    std::vector<double> data3 = data;
    data3[0]=0.5 +0.9*Params::nov::l;
    indiv3->fit().set_desc(data3);
    indiv3->fit().set_value(0);
    // add indiv very close to NN, but not far enough from second NN. 
    BOOST_CHECK(!archive.add(indiv3));
    
    data3[0]=0.5 +1.1*Params::nov::l;
    indiv3->fit().set_desc(data3);
    indiv3->fit().set_value(1);
    // add indiv identical to NN, but with better quality, should replace it
    BOOST_CHECK(archive.add(indiv3));
    BOOST_CHECK(archive.archive().size() == 2);

    container_t::indiv_t indiv4(new phen_t);
    std::vector<double> data4 = data;
    data4[0]=0.5 +1.4*Params::nov::l;
    indiv4->fit().set_desc(data4);
    indiv4->fit().set_value(1-Params::nov::eps*0.5);
    // add indiv slightly worse than NN, but significantly further (i.e. with higher noverlty), should be added (and replace NN)
    BOOST_CHECK(archive.add(indiv4));
    BOOST_CHECK(archive.archive().size() == 2);
    
    
}


#ifdef USE_KDTREE

BOOST_AUTO_TEST_CASE(qd_archive_kdtree)
{
    using namespace sferes;

    typedef Rastrigin<Params> fit_t;
    typedef gen::EvoFloat<10, Params> gen_t;
    typedef phen::Parameters<gen_t, fit_t, Params> phen_t;

    typedef qd::container::KdtreeStorage< boost::shared_ptr<phen_t>, Params::qd::behav_dim > storage_t;
    typedef qd::container::Archive<phen_t, storage_t, Params> container_t;


    container_t archive;
    // archive initially empty
    BOOST_CHECK(archive.archive().size() == 0);
    container_t::indiv_t indiv(new phen_t);

    std::vector<double> data = {0.5,0.5};
    indiv->fit().set_desc(data);
    indiv->fit().set_value(0);
    // add indiv when archive empty, always works
    BOOST_CHECK(archive.add(indiv));

    // add the same indiv twice, never works
    BOOST_CHECK(!archive.add(indiv));

    
    container_t::indiv_t indiv2(new phen_t);
    std::vector<double> data2 = data;
    data2[0]+=0.5*Params::nov::l;
    indiv2->fit().set_desc(data2);
    indiv2->fit().set_value(0);
    // add new indiv too close from already existing one
    BOOST_CHECK(!archive.add(indiv2));
    
    data2[0]=0.5+1.1*Params::nov::l;
    indiv2->fit().set_desc(data2);
    // add new indiv far enough from already existing one, should work
    BOOST_CHECK(archive.add(indiv2));
    BOOST_CHECK(archive.archive().size() == 2);

    container_t::indiv_t indiv3(new phen_t);
    std::vector<double> data3 = data;
    data3[0]=0.5 +0.9*Params::nov::l;
    indiv3->fit().set_desc(data3);
    indiv3->fit().set_value(0);
    // add indiv very close to NN, but not far enough from second NN. 
    BOOST_CHECK(!archive.add(indiv3));
    
    data3[0]=0.5 +1.1*Params::nov::l;
    indiv3->fit().set_desc(data3);
    indiv3->fit().set_value(1);
    // add indiv identical to NN, but with better quality, should replace it
    BOOST_CHECK(archive.add(indiv3));
    BOOST_CHECK(archive.archive().size() == 2);

    container_t::indiv_t indiv4(new phen_t);
    std::vector<double> data4 = data;
    data4[0]=0.5 +1.4*Params::nov::l;
    indiv4->fit().set_desc(data4);
    indiv4->fit().set_value(1-Params::nov::eps*0.5);
    // add indiv slightly worse than NN, but significantly further (i.e. with higher noverlty), should be added (and replace NN)
    BOOST_CHECK(archive.add(indiv4));
    BOOST_CHECK(archive.archive().size() == 2);
    
}
#endif
