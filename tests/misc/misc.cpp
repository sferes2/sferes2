
//| This file is a part of the sferes2 framework.
//| Copyright 2009-2018, Universite Pierre et Marie Curie (UPMC) & Inria
//| Main contributor(s): Jean-Baptiste Mouret, jean-baptiste.mouret@inria.fr
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
#define BOOST_TEST_MODULE misc
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/tmpdir.hpp>
#include <fstream>
#include <iostream>

#include <boost/test/unit_test.hpp>

#ifdef EIGEN3_ENABLED

#include <Eigen/Core>
#include <sferes/misc/eigen.hpp>

BOOST_AUTO_TEST_CASE(serialize_eigen)
{
    typedef boost::archive::binary_oarchive oa_bin_t;
    typedef boost::archive::binary_iarchive ia_bin_t;
    std::string filename = boost::archive::tmpdir();
    filename += "/serialize_eigen.sferes";

    Eigen::MatrixXd m1 = Eigen::MatrixXd::Random(100, 100);
    {

        std::ofstream ofs(filename.c_str());
        oa_bin_t oa(ofs);
        oa << m1;
    }
    {
        std::ifstream ifs(filename.c_str());
        ia_bin_t ia(ifs);
        Eigen::MatrixXd m2;
        ia >> m2;
        BOOST_CHECK_EQUAL(m1.rows(), m2.rows());
        BOOST_CHECK_EQUAL(m1.cols(), m2.cols());
        for (size_t i = 0; i < m1.rows(); ++i)
            for (size_t j = 0; j < m1.cols(); ++j)
                BOOST_CHECK_EQUAL(m1(i, j), m2(i, j));
    }
}
#else
// no test
BOOST_AUTO_TEST_CASE(serialize_eigen) {}
#endif