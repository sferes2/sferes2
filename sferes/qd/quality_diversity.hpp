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

#ifndef QD_HPP_
#define QD_HPP_

#include <algorithm>
#include <limits>

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/multi_array.hpp>
#include <boost/timer/timer.hpp>

#include <sferes/ea/ea.hpp>
#include <sferes/fit/fitness.hpp>
#include <sferes/stc.hpp>

#include <sferes/qd/selector/uniform.hpp>
#include <sferes/qd/container/grid.hpp>
#include <sferes/qd/container/cvt.hpp>
#include <sferes/qd/container/sort_based_storage.hpp>

namespace sferes {
    namespace qd {

        // Main class
        template <typename Phen, typename Eval, typename Stat, typename FitModifier,
            typename Selector, typename Container, typename Params, typename Exact = stc::Itself>
        class QualityDiversity
            : public ea::Ea<Phen, Eval, Stat, FitModifier, Params,
                  typename stc::FindExact<QualityDiversity<Phen, Eval, Stat, FitModifier, Selector,
                                              Container, Params, Exact>,
                      Exact>::ret> {
        public:
            typedef Phen phen_t;
            typedef boost::shared_ptr<Phen> indiv_t;
            typedef typename std::vector<indiv_t> pop_t;
            typedef typename pop_t::iterator it_t;

            QualityDiversity() {}

            // Random initialization of _parents and _offspring
            void random_pop()
            {
                parallel::init();

		this->_pop.clear();
		
		_offspring.resize(Params::pop::size);
                BOOST_FOREACH (indiv_t& indiv, this->_offspring) {
                    indiv = indiv_t(new Phen());
                    indiv->random();
                }
                this->_eval_pop(this->_offspring, 0, this->_offspring.size());
                this->apply_modifier();

                _add(_offspring, _added);
		
                this->_parents = this->_offspring;
                _offspring.resize(Params::pop::size);

                BOOST_FOREACH (indiv_t& indiv, this->_offspring) {
                    indiv = indiv_t(new Phen());
                    indiv->random();
                }

                this->_eval_pop(this->_offspring, 0, this->_offspring.size());
                this->apply_modifier();
                _add(_offspring, _added);


                _container.get_full_content(this->_pop);
            }

            // Main Iteration of the QD algorithm
            void epoch()
            {
	        _parents.resize(Params::pop::size);

                // Selection of the parents (will fill the _parents vector)
                _selector(_parents, *this); // not a nice API

                // CLEAR _offspring ONLY after selection, as it can be
                // used by the selector (via this->_offspring)
                _offspring.clear();
		_offspring.resize(Params::pop::size);
		
                // Generation of the offspring
                std::vector<size_t> a;
                misc::rand_ind(a, _parents.size());
                assert(_parents.size() == Params::pop::size);
                for (size_t i = 0; i < Params::pop::size; i += 2) {
                    boost::shared_ptr<Phen> i1, i2;
                    _parents[a[i]]->cross(_parents[a[i + 1]], i1, i2);
                    i1->mutate();
                    i2->mutate();
                    i1->develop();
                    i2->develop();
                    _offspring[a[i]]=i1;
                    _offspring[a[i+1]]=i2;
                }

                // Evaluation of the offspring
                this->_eval_pop(_offspring, 0, _offspring.size());
                this->apply_modifier();

                // Addition of the offspring to the container
                _add(_offspring, _added, _parents);

                assert(_offspring.size() == _parents.size());

                this->_pop.clear();

                // Copy of the containt of the container into the _pop object.
                _container.get_full_content(this->_pop);
            }

            const Container& container() const { return _container; }
	  
	    const pop_t& pop() const { return this->_pop; }
	    pop_t& pop() { return this->_pop; }

	    const pop_t& offspring() const { return _offspring; }
	    pop_t& offspring() { return _offspring; }

	    const pop_t& parents() const { return _parents; }
	    pop_t& parents() { return _parents; }

	    const std::vector<bool>& added() const { return _added; }
	    std::vector<bool>& added() { return _added; }

        protected:
            // Add the offspring into the container and update the score of the individuals from the
            // container and both of the sub population (offspring and parents)
	    void _add(pop_t& pop_off, std::vector<bool>& added, pop_t& pop_parents)
            {
	        added.resize(pop_off.size());
                for (size_t i = 0; i < pop_off.size(); ++i)
                    added[i] = _add_to_container(pop_off[i], pop_parents[i]);
                _container.update(pop_off, pop_parents);
            }

            // Same function, but without the need of parent.
	    void _add(pop_t& pop_off, std::vector<bool>& added)
            {
	        added.resize(pop_off.size());
                for (size_t i = 0; i < pop_off.size(); ++i)
                    added[i] = _container.add(pop_off[i]);
                pop_t empty;
                _container.update(pop_off, empty);
            }

            // add to the container procedure.
            // TODO JBM: curiosity is hardcoded here...
            bool _add_to_container(indiv_t i1, indiv_t parent)
            {
                if (_container.add(i1)) {
                    parent->fit().set_curiosity(parent->fit().curiosity() + 1);
                    return true;
                }
                else {
                    parent->fit().set_curiosity(parent->fit().curiosity() - 0.5);
                    return false;
                }
            }

            // ---- attributes ----

            Selector _selector;
            Container _container;

            pop_t _offspring, _parents;
            std::vector<bool> _added;
        };

        template <typename Phen, typename Eval, typename Stat, typename Modifier, typename Params>
        using MapElites = qd::QualityDiversity<Phen, Eval, Stat, Modifier,
            selector::Uniform<Phen, Params>, container::Grid<Phen, Params>, Params>;

        template <typename Phen, typename Eval, typename Stat, typename Modifier, typename Params>
        using CvtMapElites = qd::QualityDiversity<Phen, Eval, Stat, Modifier,
            selector::Uniform<Phen, Params>, container::CVT<Phen, container::SortBasedStorage<int>, Params>, Params>;

    } // namespace qd
} // namespace sferes
#endif
