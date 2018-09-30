#include <iostream>
#include <sferes/stat/best_fit.hpp>
#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/ea/nsga2.hpp>
#include <sferes/modif/diversity.hpp>
#include <sferes/run.hpp>
#include <boost/program_options.hpp>

#ifdef GRAPHIC
#define NO_PARALLEL
#endif

#ifndef NO_PARALLEL
#include <sferes/eval/parallel.hpp>
#else
#include <sferes/eval/eval.hpp>
#endif

using namespace sferes;
using namespace sferes::gen::evo_float;

// for details about behavioral diversity, please see:
// Mouret JB, Doncieux S. Encouraging behavioral diversity in evolutionary robotics: An empirical study. Evolutionary computation. 2012 Mar;20(1):91-133.


struct Params {
  struct evo_float {
    SFERES_CONST float cross_rate = 0.5f;
    SFERES_CONST float mutation_rate = 0.1f;
    SFERES_CONST float eta_m = 15.0f;
    SFERES_CONST float eta_c = 10.0f;
    SFERES_CONST mutation_t mutation_type = polynomial;
    SFERES_CONST cross_over_t cross_over_type = sbx;
  };
  struct pop {
    SFERES_CONST unsigned size = 300;
    SFERES_CONST unsigned nb_gen = 500;
    SFERES_CONST int dump_period = 50;
    SFERES_CONST int initial_aleat = 1;
  };
  struct parameters {
    SFERES_CONST float min = 0.0f;
    SFERES_CONST float max = 1.0f;
  };
};


template<typename Indiv>
float _g(const Indiv &ind) {
  float g = 0.0f;
  assert(ind.size() == 30);
  for (size_t i = 1; i < 30; ++i)
    g += ind.data(i);
  g = 9.0f * g / 29.0f;
  g += 1.0f;
  return g;
}

SFERES_FITNESS(FitDiv, sferes::fit::Fitness) {
public:
  FitDiv()  {}
  template<typename Indiv>
  void eval(Indiv& ind) {
    // we need two objectives: (1) the fitness, (2) the diversity
    this->_objs.resize(2);

    // compute the fitness (here \sum_x x^4)
    float v = 0;
    for (unsigned i = 0; i < ind.size(); ++i) {
      float p = ind.data(i);
      v += p * p * p * p;
    }
    // we need this->_value for some statistics
    this->_value = v;
    this->_objs[0] = v;
    // this->_objs[1] will be the diversity score
    // we set the behavior descriptors (to be used by the dist() method)
    // here we use the genotype (genotypic diversity) but it is better
    // to use the trajectory of the robot or something related to its behavior
    this->_behavior.resize(2);
    this->_behavior[0] = ind.data(0);
    this->_behavior[1] = ind.data(1);
  }

  // for behavioral diversity we need a dist() method
  // this method can use whatever you saved from the fitness evaluation
  template<typename Indiv>
  float dist(const Indiv& other) const {
      // we compute a simple Euclidean distance between the behavior vectors
      double d = 0;
      for (size_t i = 0; i < _behavior.size(); ++i)
        d += (_behavior[i] - other.fit()._behavior[i]) * (_behavior[i] - other.fit()._behavior[i]);
      return d;
  }
protected:
  std::vector<float> _behavior;
};

int main(int argc, char **argv) {
  std::cout<<"running "<<argv[0]<<" ... try --help for options (verbose)"<<std::endl;

#ifndef NO_PARALLEL
  typedef eval::Parallel<Params> eval_t;
#else
  typedef eval::Eval<Params> eval_t;
#endif

  typedef gen::EvoFloat<10, Params> gen_t;
  typedef phen::Parameters<gen_t, FitDiv<Params>, Params> phen_t;
  typedef boost::fusion::vector<sferes::stat::BestFit<phen_t, Params> >  stat_t;
  typedef modif::Diversity<Params> modifier_t;
  typedef ea::Nsga2<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
  ea_t ea;

  run_ea(argc, argv, ea);

  return 0;
}
