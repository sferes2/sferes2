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

#ifndef RAND_HPP_
#define RAND_HPP_

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <list>
#include <random>
#include <stdlib.h>
#include <type_traits>

// a few external tools for seeding (GPL-licensed)
#include "sferes/misc/rand_utils.hpp"

namespace sferes {
    namespace misc {     
        using generator_t = std::mt19937;
        inline generator_t make_rgen() { return generator_t(randutils::auto_seed_128{}.base()); }

        // rand for floating point types (see the dispatcher below)
        // this is supposed to generate a number in [min, max)
        // but this is not guaranteed in the current implementations
        // see notes here: http://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
        template <typename T>
        inline T rand(T min, T max, std::false_type)
        {
            assert(max > min);
            static thread_local generator_t rgen = make_rgen();
            std::uniform_real_distribution<T> dist(min, max);
            T v;
            do
                v = dist(rgen);
            while (v >= max);
            assert(v >= min);
            assert(v < max);
            return v;
        }

        // rand for integral types (see the dispatcher below)
        // be careful that uniform_int is in [a,b] but we do [a, b)
        // (so that we can use rand(0, list_size))
        template <typename T>
        inline T rand(T min, T max, std::true_type)
        {
            assert(max > min);
            static thread_local generator_t rgen = make_rgen();;
            // uniform_int is in [a,b], not [a,b)...
            std::uniform_int_distribution<size_t> dist(min, max - 1);
            size_t v = dist(rgen);
            assert(v >= min);
            assert(v < max);
            return v;
        }

        // the generic rand dispatches between uniform_real and uniform_int
        template <typename T>
        inline T rand(T min, T max)
        {
            return rand(min, max, std::is_integral<T>());
        }

        template <typename T>
        inline T rand(T max = 1.0)
        {
            return rand<T>((T)0.0, max);
        }

        template <typename T>
        inline T gaussian_rand(T m = 0.0, T v = 1.0)
        {
            static thread_local generator_t rgen = make_rgen();;
            std::normal_distribution<T> dist(m, v);
            return dist(rgen);
        }

        // randomize indices
        inline void rand_ind(std::vector<size_t>& a1, size_t size)
        {
            a1.resize(size);
            for (size_t i = 0; i < a1.size(); ++i)
                a1[i] = i;
            for (size_t i = 0; i < a1.size(); ++i) {
                size_t k = rand(i, a1.size());
                assert(k < a1.size());
                std::swap(a1[i], a1[k]);
            }
        }

        /// return a random it in the list
        template <typename T>
        inline typename std::list<T>::iterator rand_in_list(std::list<T>& l)
        {
            int n = rand((size_t)0, l.size());
            typename std::list<T>::iterator it = l.begin();
            for (int i = 0; i < n; ++i)
                ++it;
            return it;
        }

        inline bool flip_coin()
        {
            static thread_local generator_t rgen = make_rgen();;
            // uniform_int is in [a,b], not [a,b)...
            std::uniform_int_distribution<size_t> dist(0, 1);
            return (dist(rgen) == 0);
        }

        // todo : remove this
        template <typename L>
        inline typename L::iterator rand_l(L& l)
        {
            int n = rand((size_t)0, l.size());
            typename L::iterator it = l.begin();
            for (int i = 0; i < n; ++i)
                ++it;
            return it;
        }
    } // namespace misc
} // namespace sferes
#endif
