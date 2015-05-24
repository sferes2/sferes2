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

#ifndef MODIFIER_NOVELTY_HPP
#define MODIFIER_NOVELTY_HPP

#include <Eigen/Core>
#include "sferes/parallel.hpp"

namespace sferes {
  namespace modif {
    namespace novelty {

      // compute the matrix of distances
      template<typename Phen>
      struct _distance_f {
        typedef std::vector<boost::shared_ptr<Phen> > pop_t;
        const pop_t& _pop;
        const pop_t& _archive;
        Eigen::MatrixXf& distances;

        ~_distance_f() { }
        _distance_f(const pop_t& pop, const pop_t& archive, Eigen::MatrixXf& d) :
          _pop(pop), _archive(archive), distances(d) {}
        _distance_f(const _distance_f& ev) :
          _pop(ev._pop), _archive(ev._archive), distances(ev.distances) {}

        void operator() (const parallel::range_t& r) const {
          for (size_t i = r.begin(); i != r.end(); ++i) {
            for (size_t j = 0; j < _archive.size(); ++j)
              distances(i, j) = _pop[i]->fit().dist(*_archive[j]);
          }
        }
      };
    }

    // The novelty score will be stored in the last objective 'slot'
    // If there is only one objective (this->objs().size() == 1), then
    // this modifiers is a standard Novelty search algorithm [1]
    // otherwise, it is a "novelty-based multi-objectivization" [2]
    // See [2] for more explanations of the differences between
    // novelty search, multi-objective novelty search, and behavioral diversity
    //
    // * References
    // [1] Lehman, Joel, and Kenneth O. Stanley. "Abandoning objectives:
    // Evolution through the search for novelty alone."
    // Evolutionary computation 19.2 (2011): 189-223.
    //
    // [2] Mouret, Jean-Baptiste. "Novelty-based multiobjectivization."
    // New Horizons in Evolutionary Robotics. Springer Berlin Heidelberg,
    // 2011. 139-154.
    template<typename Phen, typename Params, typename Exact = stc::Itself>
    class Novelty {
     public:
      Novelty() : _rho_min(Params::novelty::rho_min_init), _not_added(0) {}

      template<typename Ea>
      void apply(Ea& ea) {
        SFERES_CONST size_t k = Params::novelty::k;
        // merge the population and the archive in a single archive
        pop_t archive = _archive;
        archive.insert(archive.end(), ea.pop().begin(), ea.pop().end());

        // we compute all the distances from pop(i) to archive(j) and store them
        Eigen::MatrixXf distances(ea.pop().size(), archive.size());
        novelty::_distance_f<Phen> f(ea.pop(), archive, distances);
        parallel::init();
        parallel::p_for(parallel::range_t(0, ea.pop().size()), f);

        // compute the sparseness of each individual of the population
        // and potentially add to the archive
        int added = 0;
        for (size_t i = 0; i < ea.pop().size(); ++i) {
          size_t nb_objs = ea.pop()[i]->fit().objs().size();
          Eigen::VectorXf vd = distances.row(i);

          double n = 0.0;
          std::partial_sort(vd.data(),
                            vd.data() + k,
                            vd.data() + vd.size());

          n = vd.head<k>().sum() / k;
          ea.pop()[i]->fit().set_obj(nb_objs - 1, n);
          // add to the archive
          if (n > _rho_min
              || misc::rand<float>() < Params::novelty::add_to_archive_prob) {
            _archive.push_back(ea.pop()[i]);
            _not_added = 0;
            ++added;
          } else {
            ++_not_added;
          }
        } // end for all individuals

        // update rho_min
        if (_not_added > Params::novelty::stalled_tresh) { //2500
          _rho_min *= 0.95;
          _not_added = 0;
        }
        if (_archive.size() > Params::novelty::k
            && added > Params::novelty::adding_tresh)//4
          _rho_min *= 1.05f;
      }
     protected:
      typedef boost::shared_ptr<Phen> phen_t;
      typedef std::vector<phen_t> pop_t;
      pop_t _archive;
      float _rho_min;
      size_t _not_added;
    };
  } // modif
} // sferes

#endif
