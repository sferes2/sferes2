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




#ifndef RANK_SIMPLE_HPP_
#define RANK_SIMPLE_HPP_

#ifdef EIGEN2_ENABLED

 #include <algorithm>
 #include <boost/foreach.hpp>
 #include <sferes/stc.hpp>
 #include <sferes/ea/ea.hpp>
 #include <sferes/fit/fitness.hpp>
 #include <sferes/parallel.hpp>

namespace sferes
{
  namespace ea
  {
    namespace cmaes
    {
      template<typename Phen>
      struct IndexSort
      {
        IndexSort(const std::vector<boost::shared_ptr<Phen> >& pop) : _pop(pop) {
        }
        bool operator() (size_t x, size_t y) const
        {
          return _pop[x]->fit().value() > _pop[y]->fit().value();
        }
        const std::vector<boost::shared_ptr<Phen> >& _pop;
      };
    }
    SFERES_EA(Cmaes, Ea)
    {
      public:
        Cmaes() :
          _xmean(vector_t::Random()),
          _sigma(Params::cmaes::sigma),
          _lambda(4 + floorf(3 * logf(N))),
          _mu(floorf(_lambda / 2)),
          _weights(Eigen::VectorXf::Constant(_mu, logf(_mu + 1))),
          _cc(4.0f / (N + 4.0f)),
          _pc(vector_t::Zero()),
          _ps(vector_t::Zero()),
          _B(matrix_t::Identity()),
          _D(matrix_t::Identity()),
          _C(_B * _D * (_B * _D)),
          _chi_n(powf(N, 0.5f) * (1.0f - 1.0f / (4.0f * N) + 1.0f / (21.0f * N * N))),
          _count_eval(0),
          _done(false)
        {
           // weights
          for (size_t i = 0; i < _mu; ++i)
            _weights[i] -= logf(i + 1);
          float sw = _weights.sum();
          _weights = _weights / sw;
          sw = _weights.sum();
          _mueff = sw * sw / (_weights.cwise() * _weights).sum();

           // adaptation
          _cs = (_mueff + 2) / (N + _mueff + 3);
          _mucov = _mueff;
          _ccov = (1.0f / _mucov) * 2 / powf(N + 1.4f, 2.0f)
                  + (1.0f - 1.0f / _mucov)
                  * ((2 * _mucov - 1) / (powf((N + 2.0f), 2) + 2 * _mucov));
          _damps = 1.0f + 2.0f * std::max(0.0f, sqrtf((_mueff - 1) / (N + 1)) - 1) + _cs;

        }
        void random_pop()
        {
          this->_pop.resize(_lambda);
          BOOST_FOREACH(boost::shared_ptr<Phen>&indiv, this->_pop)
          {
            indiv = boost::shared_ptr<Phen>(new Phen());
            indiv->random();
          }
        }

        void epoch()
        {
          if (_done)
            return;
          USING_PART_OF_NAMESPACE_EIGEN

          assert(this->_pop.size());
           // mutate population (generate new individuals)
          for (size_t i = 0; i < this->_pop.size(); ++i)
            this->_pop[i]->gen().mutate(_xmean, _sigma, _B, _D);
           // evaluate all the population
          this->_eval.eval(this->_pop, 0, this->_pop.size());
          this->apply_modifier();
          _count_eval += this->_pop.size();


           // adapt using CMA-ES
          std::vector<float> arindex = _rank_v<std::vector<float> >(_lambda);
          std::sort(arindex.begin(), arindex.end(),
                    cmaes::IndexSort<Phen>(this->_pop));

          MatrixXf x_tmp = MatrixXf::Zero(N, _mu),
                   z_tmp = MatrixXf::Zero(N, _mu);
          for (size_t i = 0; i < _mu; ++i)
          {
            x_tmp.col(i) = this->_pop[arindex[i]]->gen().data();
            z_tmp.col(i) = this->_pop[arindex[i]]->gen().arz();
          }
          _xmean = x_tmp * _weights;

          vector_t zmean = z_tmp * _weights;
          _ps = (1 - _cs) * _ps + sqrtf(_cs * (2.0f - _cs) * _mueff) * (_B * zmean);

          int hsig = _ps.norm() / sqrtf(1.0f - powf((1.0f - _cs),
                                                    (2.0f * _count_eval / _lambda))
                                        ) / _chi_n
                     < 1.4f + 2.0f / (N + 1.0f);

          _pc = (1.0f - _cc) * _pc + hsig *sqrtf(_cc * (2 - _cc) *_mueff) * (_B * _D * zmean);

          _C = (1 - _ccov) * _C
               + _ccov * (1.0f / _mucov) * (_pc * _pc.transpose()
                                            + (1 - hsig) * _cc * (2 - _cc) * _C)
               + _ccov * (1 - 1.0f / _mucov)
               * (_B * _D * z_tmp)
               * _weights.asDiagonal() * (_B * _D * z_tmp).transpose();
          _sigma = _sigma * expf((_cs / _damps) * (_ps.norm() / _chi_n - 1));

          _C = _C.template part<Eigen::UpperTriangular>()
               + _C.template part<Eigen::StrictlyUpperTriangular>().transpose();
          Eigen::EigenSolver<MatrixXf> eigen_solver(_C);

          matrix_t B = eigen_solver.eigenvectors().real();
          _B = eigen_solver.eigenvectors().real();
           // we add a almost useless abs() to ensure to take the square
           // root of a positive number
          _D = eigen_solver.eigenvalues().real().cwise().abs().cwise().sqrt().asDiagonal();


           // sort again
          std::sort(this->_pop.begin(), this->_pop.end(), fit::compare());
          dbg::out(dbg::info, "ea") << "best fitness: "
                                    << this->_pop[0]->fit().value()
                                    << std::endl;
          if (this->_pop[0]->fit().value() > Params::cmaes::max_value)
            _done = true;

        }
      protected:
        SFERES_CONST size_t N = Phen::gen_t::es_size;
        typedef Eigen::Matrix<float, N, 1> vector_t;
        typedef Eigen::Matrix<float, N, N> matrix_t;

        vector_t _xmean;
        float _sigma;

         // Strategy parameter setting: Selection
        float _lambda, _mu, _mueff;
        Eigen::VectorXf _weights;

         // Strategy parameter setting: Adaptation
        float _cc, _cs, _mucov, _ccov, _damps;

         // Dynamic (internal) strategy parameters and constants
        vector_t _pc, _ps;
        matrix_t _B, _D, _C;
        float _chi_n;

        size_t _count_eval;
        bool _done;
        template<typename V>
        V _rank_v(size_t N)
        {
          V v(N);
          for (size_t i = 0; i < N; ++i)
            v[i] = i;
          return v;
        }

    };
  }
}
#else
 #warning Eigen2 is disabled -> no CMAES
#endif
#endif
