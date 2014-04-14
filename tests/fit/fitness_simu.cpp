//| This file is a part of the sferes2 framework.
//| Copyright 2009, ISIR / Universite Pierre et Marie Curie (UPMC)
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
#define BOOST_TEST_MODULE fitness_simu
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <sferes/simu/simu.hpp>
#include <sferes/fit/fitness_simu.hpp>
#include <sferes/ctrl.hpp>
#include <sferes/phen/indiv.hpp>

#include <iostream>
using namespace sferes::ctrl;

struct Params
{
  struct simu
  {
    static const float dt = 0.01f;
  };
};

BOOST_AUTO_TEST_CASE(fitness_simu_dummy)
{
  using namespace sferes;
  fit::FitnessSimuDummy<simu::SimuDummy<Params>, ctrl::AgentDummy<>, Params> fit;
  AgentDummy<> a;
  fit.eval(a);//dummy params !
}

SFERES_SIMU(SimuTest, sferes::simu::Simu)
{
 public:
  SimuTest() : nb_init(0), nb_refresh(0), nb_test(0), nb_test2(0) {}
  void init() { ++nb_init; }
  void refresh() { ++nb_refresh;  }
  void test() { ++nb_test;}
  void test2() { ++nb_test2;}
  int nb_init;
  int nb_refresh;
  int nb_test;
  int nb_test2;
};

SFERES_FITNESS_SIMU(FitnessSimuTest, sferes::fit::FitnessSimu)
{
 public:
  FitnessSimuTest() : 
    nb_exps(0), nb_steps(0), nb_evals(0), _this(this) 
    {}
  template<typename Phen>
    int refresh(Phen& p) 
  { 
    ++nb_steps;
    BOOST_CHECK(this == _this);
    return 0; 
  }
  template<typename Phen>
    void refresh_end_exp(Phen& p) { ++nb_exps; }
  template<typename Phen>
    void refresh_end_eval(Phen& p) { ++nb_evals ;}
  SFERES_SCHEDULER()
  { 
    EVERY(1, 5)	this->simu().test();

    NEW_EXP(0);
    AT(5)	this->simu().test2();
    END_EXP(9);

    NEW_EXP(10);
    END_EXP(19);
    
    NEW_EXP(20);
    END_EXP(29);
    END_EVAL(29);
  }
  int nb_exps;
  int nb_steps;
  int nb_evals;
  FitnessSimuTest* _this;
};

SFERES_AGENT(AgentTest, sferes::ctrl::Agent)
{
 public:
  AgentTest() : 
    nb_init(0), 
    nb_refresh_params(0), 
    nb_refresh_sensors(0), 
    nb_refresh_archs(0),
    nb_refresh_env(0)
      {}
  template<typename Indiv>
  void init(const Indiv&) { }
  template<typename Indiv>
    void refresh_params(const Indiv& i) { ++nb_refresh_params; }
  template<typename Simu>
    void refresh_sensors(const Simu& s) { ++ nb_refresh_sensors; }
  template<typename Simu>
    void refresh_arch(const Simu& s) { ++ nb_refresh_archs; }
  template<typename Simu>
    void refresh_env(Simu& s)
  { ++ nb_refresh_env; }

  int nb_init;
  int nb_refresh_params;
  int nb_refresh_sensors;
  int nb_refresh_archs;
  int nb_refresh_env;
};

void init_debug()
{
  // attach our streams
  dbg::init();
  dbg::enable(dbg::all, "fit", true);
  dbg::attach_ostream(dbg::info, "fit", std::cout);
  dbg::attach_ostream(dbg::tracing, "fit", std::cout);
}


BOOST_AUTO_TEST_CASE(fitness_simu_complex)
{
  init_debug();
  using namespace sferes;
  FitnessSimuTest<SimuTest<Params>, AgentTest<>, Params> fit;
  AgentTest<> a;
  fit.eval(a);//dummy arg! (should be an indiv)

  // fitness
  BOOST_CHECK_EQUAL(fit.nb_exps, 3);
  BOOST_CHECK_EQUAL(fit.nb_steps, 30);
  BOOST_CHECK_EQUAL(fit.nb_steps, 30);
  BOOST_CHECK_EQUAL(fit.nb_evals, 1);

  // simu
  BOOST_CHECK_EQUAL(fit.simu().nb_refresh, 30);
  BOOST_CHECK_EQUAL(fit.simu().nb_init, 3);
  BOOST_CHECK_EQUAL(fit.simu().nb_test, 6);

  // agent
  BOOST_CHECK_EQUAL(fit.agent().nb_refresh_params, 1);
  BOOST_CHECK_EQUAL(fit.agent().nb_refresh_sensors, 30);
  BOOST_CHECK_EQUAL(fit.agent().nb_refresh_archs, 30);
  BOOST_CHECK_EQUAL(fit.agent().nb_refresh_env, 30);

  std::cout<<"-- test done"<<std::endl;
}


SFERES_FITNESS_SIMU(FitnessSimuTest2, sferes::fit::FitnessSimu)
{
 public:
  FitnessSimuTest2() : nb_exps(0), nb_steps(0), nb_evals(0) {}
  template<typename Phen>
    int refresh(Phen& p) 
  { 
    ++nb_steps; 
    switch (this->_step)
      {
      case 9:
	return -1;
      case 229:
	return -1;
      case 449:
	return -1;
      }
    return 0; 
  }
  template<typename Phen>
    void refresh_end_exp(Phen& p) { ++nb_exps; }
  template<typename Phen>
    void refresh_end_eval(Phen& p) { ++nb_evals ;}
  template<typename Phen>
  void scheduler(Phen& p) 
  { 
    NEW_EXP(0);
    END_EXP(199);

    NEW_EXP(200);
    END_EXP(399);

    NEW_EXP(400);
    END_EXP(499);
    END_EVAL(499);
  }
  int nb_exps;
  int nb_steps;
  int nb_evals;
};



BOOST_AUTO_TEST_CASE(fitness_simu_complex2)
{
  init_debug();
  using namespace sferes;
  FitnessSimuTest2<SimuTest<Params>, AgentTest<>, Params> fit;
  AgentTest<> indiv;
  fit.eval(indiv);// fst should be an ea

  // fitness
  BOOST_CHECK_EQUAL(fit.nb_exps, 3);
  BOOST_CHECK_EQUAL(fit.nb_steps, 10 + 30 + 50);
  BOOST_CHECK_EQUAL(fit.nb_evals, 1);

  // simu
  BOOST_CHECK_EQUAL(fit.simu().nb_refresh, 10 + 30 + 50);
  BOOST_CHECK_EQUAL(fit.simu().nb_init, 3);
  BOOST_CHECK_EQUAL(fit.simu().nb_test, 0);

  // agent
  BOOST_CHECK_EQUAL(fit.agent().nb_refresh_params, 1);
  BOOST_CHECK_EQUAL(fit.agent().nb_refresh_sensors, 10 + 30 + 50);
  BOOST_CHECK_EQUAL(fit.agent().nb_refresh_archs, 10 + 30 + 50);
  BOOST_CHECK_EQUAL(fit.agent().nb_refresh_env, 10 + 30 + 50);
}



