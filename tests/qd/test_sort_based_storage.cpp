//| This file is a part of the sferes2 framework.
//| Copyright 2016, ISIR / Universite Pierre et Marie Curie (UPMC)
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
#define BOOST_TEST_MODULE cvt

#include <boost/test/unit_test.hpp>

#include <cmath>
#include <iostream>
#include <sferes/qd/container/sort_based_storage.hpp>

BOOST_AUTO_TEST_CASE(test_sort_based_storage)
{
    Eigen::VectorXd v1(3), v2(3), v3(3), v4(3), v5(3);
    v1 << 0, 30, 0;
    v2 << 1, 2, 1;
    v3 << 100, 2, 3;
    v4 << 5, 0, 0;
    v5 << 2, 1, 1.5;

    sferes::qd::container::SortBasedStorage<int> storage;
    storage.add(v1, 0);
    storage.add(v2, 1);
    storage.add(v3, 2);
    storage.add(v4, 3);
    storage.add(v5, 4);

    Eigen::VectorXd p(3);
    p << 1, 1, 1;

    std::cout << " 0:" << (p - v1).norm() << std::endl
              << " 1:" << (p - v2).norm() << std::endl
              << " 2:" << (p - v3).norm() << std::endl
              << " 3:" << (p - v4).norm() << std::endl
              << " 4:" << (p - v5).norm() << std::endl;

    auto n = storage.nearest(p);
    BOOST_CHECK_EQUAL(n.second, 1);
    BOOST_CHECK_EQUAL(n.first, v2);

    auto nv = storage.knn(p, 2);
    BOOST_CHECK_EQUAL(nv[0].second, 1);
    BOOST_CHECK_EQUAL(nv[0].first, v2);
    BOOST_CHECK_EQUAL(nv[1].second, 4);
    BOOST_CHECK_EQUAL(nv[1].first, v5);
    

}
