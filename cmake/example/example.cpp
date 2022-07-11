//| This file is a part of the sferes2 framework.
//| Copyright 2022, Inria
//| Main contributor(s): Jean-Baptiste Mouret, jean-baptiste.mouret@inria.fr
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

#include <iostream>

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
#include <sferes/qd/quality_diversity.hpp>
#include <sferes/qd/selector/uniform.hpp>

using namespace sferes::gen::evo_float;

struct Params {
    struct nov {
        SFERES_CONST size_t deep = 2;
        SFERES_CONST double l = 1; // TODO value ???
        SFERES_CONST double k = 8; // TODO right value?
        SFERES_CONST double eps = 0.01;// TODO right value??
    };
  
    // TODO: move to a qd::
    struct pop {
        // number of initial random points
        SFERES_CONST size_t init_size = 1000;
        // size of a batch
        SFERES_CONST size_t size = 2000;
        SFERES_CONST size_t nb_gen = 10001;
        SFERES_CONST size_t dump_period = 1000;
    };
    struct parameters {
        SFERES_CONST float min = -5;
        SFERES_CONST float max = 5;
    };
    struct evo_float {
        SFERES_CONST float cross_rate = 0.75f;
        SFERES_CONST float mutation_rate = 0.1f;
        SFERES_CONST float eta_m = 10.0f;
        SFERES_CONST float eta_c = 10.0f;
        SFERES_CONST mutation_t mutation_type = polynomial;
        SFERES_CONST cross_over_t cross_over_type = sbx;
    };
    struct qd {
        SFERES_CONST size_t behav_dim = 2;
        SFERES_ARRAY(size_t, grid_shape, 128, 128);
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

int main(int argc, char **argv) 
{
    using namespace sferes;

    typedef Rastrigin<Params> fit_t;
    typedef gen::EvoFloat<10, Params> gen_t;
    typedef phen::Parameters<gen_t, fit_t, Params> phen_t;

    typedef eval::Parallel<Params> eval_t;

    typedef boost::fusion::vector<
        stat::BestFit<phen_t, Params>, 
        stat::QdContainer<phen_t, Params>, 
        stat::QdProgress<phen_t, Params>
        >
        stat_t; 
    typedef modif::Dummy<> modifier_t;
    typedef qd::MapElites<phen_t, eval_t, stat_t, modifier_t, Params>
        qd_t;

    qd_t qd;
    run_ea(argc, argv, qd);
    std::cout<<"best fitness:" << qd.stat<0>().best()->fit().value() << std::endl;
    std::cout<<"archive size:" << qd.stat<1>().archive().size() << std::endl;
    return 0;
}
