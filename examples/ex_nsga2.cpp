#include <iostream>
#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/ea/nsga2.hpp>
#include <sferes/eval/eval.hpp>
#include <sferes/stat/pareto_front.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/run.hpp>
#include <boost/program_options.hpp>

using namespace sferes;
using namespace sferes::gen::evo_float;

struct Params
{
  struct evo_float
  {
    static const float cross_rate = 0.5f;
    static const float mutation_rate = 0.1f;
    static const float eta_m = 15.0f;
    static const float eta_c = 10.0f;
    static const mutation_t mutation_type = polynomial;
    static const cross_over_t cross_over_type = sbx;
  };
  struct pop
  {
    static const unsigned size = 300;
    static const unsigned nb_gen = 500;
    static const int dump_period = 50;
    static const int initial_aleat = 1;
  };
  struct parameters
  {
    static const float min = 0.0f;
    static const float max = 1.0f;
  };
};


template<typename Indiv>
float _g(const Indiv &ind)
{
  float g = 0.0f;
  assert(ind.size() == 30);
  for (size_t i = 1; i < 30; ++i)
    g += ind.data(i);
  g = 9.0f * g / 29.0f;
  g += 1.0f;
  return g;
}

SFERES_FITNESS(FitZDT2, sferes::fit::Fitness)
{ 
 public:
  FitZDT2()  {}
  template<typename Indiv>
    void eval(Indiv& ind) 
  {
    this->_objs.resize(2);
    float f1 = ind.data(0);
    float g = _g(ind);
    float h = 1.0f - pow((f1 / g), 2.0);
    float f2 = g * h;
    this->_objs[0] = -f1;
    this->_objs[1] = -f2;
  }
};




int main(int argc, char **argv)
{
  std::cout<<"running "<<argv[0]<<" ... try --help for options (verbose)"<<std::endl;

  typedef gen::EvoFloat<30, Params> gen_t;
  typedef phen::Parameters<gen_t, FitZDT2<Params>, Params> phen_t;
  typedef eval::Eval<Params> eval_t;
  typedef boost::fusion::vector<stat::ParetoFront<phen_t, Params> >  stat_t;
  typedef modif::Dummy<> modifier_t;
  typedef ea::Nsga2<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
  ea_t ea;

  run_ea(argc, argv, ea);

  return 0;
}
