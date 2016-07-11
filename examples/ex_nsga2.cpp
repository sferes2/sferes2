#include <iostream>
#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/ea/nsga2.hpp>
#include <sferes/eval/eval.hpp>
#include <sferes/stat/pareto_front.hpp>
#include <sferes/modif/dummy.hpp>
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

SFERES_FITNESS(FitZDT2, sferes::fit::Fitness) {
public:
  FitZDT2()  {}
  template<typename Indiv>
  void eval(Indiv& ind) {
    this->_objs.resize(2);
    float f1 = ind.data(0);
    float g = _g(ind);
    float h = 1.0f - pow((f1 / g), 2.0);
    float f2 = g * h;
    this->_objs[0] = -f1;
    this->_objs[1] = -f2;
  }
};

namespace sferes {
    namespace stat {
        SFERES_STAT(ParetoSamples, Stat)
        {
        public:
            typedef std::vector<boost::shared_ptr<Phen>> pareto_t;
            // assume a ea.pareto_front() method
            template <typename E>
            void refresh(const E& ea)
            {
                _pareto_front = ea.pareto_front();
                parallel::sort(_pareto_front.begin(), _pareto_front.end(),
                    fit::compare_objs_lex());
                this->_create_log_file(ea, "pareto_samples.dat");
                if (ea.dump_enabled())
                    show_all(*(this->_log_file), ea.gen(), ea.nb_evals());
                //this->_log_file->close();
            }
            void show(std::ostream & os, size_t k) const
            {
                os << "log format : gen nb_evals id sample_1 ... sample_n" << std::endl;
                show_all(os, 0);
                _pareto_front[k]->develop();
                _pareto_front[k]->show(os);
                _pareto_front[k]->fit().set_mode(fit::mode::view);
                _pareto_front[k]->fit().eval(*_pareto_front[k]);
                os << "=> displaying individual " << k << std::endl;
                os << "samples:";
                for (unsigned j = 0; j < _pareto_front[k]->data().size(); ++j)
                    os << _pareto_front[k]->data(j) << " ";
                os << std::endl;
                assert(k < _pareto_front.size());
            }
            const pareto_t& pareto_front() const
            {
                return _pareto_front;
            }
            template <class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                ar& BOOST_SERIALIZATION_NVP(_pareto_front);
            }

            void show_all(std::ostream & os, size_t gen = 0, size_t nb_evals = 0) const
            {
                for (unsigned i = 0; i < _pareto_front.size(); ++i) {
                    os << gen << " " << nb_evals << " " << i << " ";
                    for (unsigned j = 0; j < _pareto_front[i]->data().size(); ++j)
                        os << _pareto_front[i]->data(j) << " ";
                    os << std::endl;
                }
            }

        protected:
            pareto_t _pareto_front;
        };
    }
}

int main(int argc, char **argv) {
  std::cout<<"running "<<argv[0]<<" ... try --help for options (verbose)"<<std::endl;

#ifndef NO_PARALLEL
  typedef eval::Parallel<Params> eval_t;
#else
  typedef eval::Eval<Params> eval_t;
#endif

  typedef gen::EvoFloat<30, Params> gen_t;
  typedef phen::Parameters<gen_t, FitZDT2<Params>, Params> phen_t;
  typedef boost::fusion::vector<stat::ParetoFront<phen_t, Params>, stat::ParetoSamples<phen_t, Params> >  stat_t;
  typedef modif::Dummy<> modifier_t;
  typedef ea::Nsga2<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
  ea_t ea;

  run_ea(argc, argv, ea);

  return 0;
}
