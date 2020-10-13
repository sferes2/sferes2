//| This file is a part of the sferes2 framework.
//| Copyright 2009, ISIR / Universite Pierre et Marie Curie (UPMC) /
//| Main contributor(s): Antoine Cully a.cully@imperial.ac.uk, Jean-Baptiste Mouret, mouret@isir.fr
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

#ifndef QD_FIT_QD_HPP__
#define QD_FIT_QD_HPP__

#include <sferes/fit/fitness.hpp>

#define FIT_QD(Name) SFERES_FITNESS(Name, sferes::fit::FitQD)

namespace sferes {
    namespace fit {
        SFERES_FITNESS(FitQD, sferes::fit::Fitness)
        {
        public:
            FitQD()
                : _dead(false),
                  _desc(Params::qd::behav_dim),
                  _novelty(-std::numeric_limits<double>::infinity()),
                  _curiosity(0),
                  _lq(0)
            {
            }

            const std::vector<double>& desc() const { return _desc; }
            double novelty() const { return _novelty; }
            void set_novelty(double nov) { _novelty = nov; }
            double curiosity() const { return _curiosity; }
            void set_curiosity(double cur) { _curiosity = cur; }

            double local_quality() const { return _lq; }
            void set_local_quality(double lq) { _lq = lq; }

            void set_desc(std::vector<double> & x)
            {
	      assert(x.size() == Params::qd::behav_dim);
                _desc = x;
            }
            bool dead() const { return _dead; }

            void set_value(float val) { this->_value = val; }

  	    // Serialization
	    template<class Archive> void serialize(Archive & ar, const unsigned int version) 
	    {
	      ar & BOOST_SERIALIZATION_NVP(this->_objs);
	      ar & BOOST_SERIALIZATION_NVP(this->_value);
	      ar & BOOST_SERIALIZATION_NVP(_dead);
	      ar & BOOST_SERIALIZATION_NVP(_desc);
	      ar & BOOST_SERIALIZATION_NVP(_novelty);
	      ar & BOOST_SERIALIZATION_NVP(_curiosity);
	      ar & BOOST_SERIALIZATION_NVP(_lq);
	    }
	    
	    
        protected:
            bool _dead;
            std::vector<double> _desc;
            double _novelty;
            double _curiosity;
            double _lq;
        };
    } // namespace fit
} // namespace sferes

#endif
