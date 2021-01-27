#ifndef QD_CONTAINER_CVT_HPP
#define QD_CONTAINER_CVT_HPP
#include <Eigen/Core>
#include <sferes/misc/eigen.hpp>

#include "compute_cvt.hpp"
#include "sort_based_storage.hpp"

namespace sferes {
    namespace qd {
        namespace container {
            // parameters:
            // qd::n_niches (e.g, 10000): number of niches
            // qd::behav_dim (e.g, 2): number of feature dimensions
            // The following parameters are ignored if you use a cached CVT (but they are required for compilation):
            // qd::cvt::qd::n_samples (e.g., 100000): number of samples for CVT (more than n_niches)
            // qd::cvt::max_iterations (e.g. 100): number of iterations of the CVT algorithm
            // qd::cvt::n_restarts (e.g. 1): number of restarts of the CVT algorithm
            // qd::cvt::tolerance (e.g. 1e-8): when to stop the CVT algorithm
            template <typename Phen, typename Storage, typename Params> class CVT {
            public:
                typedef boost::shared_ptr<Phen> indiv_t;
                typedef typename std::vector<indiv_t> pop_t;
                typedef Eigen::VectorXd point_t;
                typedef Storage storage_t;

                CVT()
                {
		  SFERES_CONST int dim = Params::qd::behav_dim;
                    SFERES_CONST int n_niches = Params::qd::n_niches;
                    _archive.resize(n_niches);
                    _archive_parents.resize(n_niches);

                    // we cache the centroids because it can be slow to compute
                    std::string cache_filename = std::string("cache_centroids_")
                        + boost::lexical_cast<std::string>(dim) + "_"
                        + boost::lexical_cast<std::string>(n_niches)
                        + ".bin";
                    if(!boost::filesystem::exists(cache_filename)) {
                        std::cerr << "CVT: no centroid cache found ("
                                  << cache_filename << ") "
                                  << "=> computing a new CVT (this might take a dozen of minutes!)"
                                  << std::endl;
                        _centroids = cvt::cvt(Params::qd::behav_dim, Params::qd::n_niches,
                            Params::qd::cvt::n_samples, Params::qd::cvt::max_iterations,
                            Params::qd::cvt::n_restarts, Params::qd::cvt::tolerance);
                        std::ofstream ofs(cache_filename.c_str());
                        boost::archive::binary_oarchive oa_a(ofs);
                        oa_a << _centroids;
                        std::cerr << "CVT saved as: " << cache_filename << std::endl;
                    }
                    else {
                        std::cerr << "CVT: using centroid cache: " << cache_filename << std::endl;
                        std::ifstream ifs(cache_filename.c_str());
                        boost::archive::binary_iarchive ia_a(ifs);
                        ia_a >> _centroids;
                        std::cerr << _centroids.rows() << " centroids loaded from cache" << std::endl;
                        assert(_centroids.rows() == Params::qd::n_niches);
                        assert(_centroids.cols() == dim);
                    }
                    // put the centroids in storage for fast KNN computation
                    for (size_t i = 0; i < _centroids.rows(); ++i)
                        _centroids_storage.add(_centroids.row(i), i);
                }

                void get_full_content(std::vector<indiv_t>& content) const
                {
                    for (size_t i = 0; i < _archive.size(); ++i)
                        if (_archive[i])
                            content.push_back(_archive[i]);
                }

                bool add(indiv_t i1)
                {
                    if (i1->fit().dead())
                        return false;

                    point_t p = _get_point(i1);

                    // Find the closest cluster
                    size_t archive_index = _centroids_storage.nearest(p).second;

                    // If the archive is empty or the stored individual is less fit
                    if (!_archive[archive_index]
                        || i1->fit().value() > _archive[archive_index]->fit().value()) {
                        _archive[archive_index] = i1;
                        return true;
                    }

                    return false;
                }

                /**
                 * adds directly an individual to the container regardless of its fitness
                 * This method is mostly used for resuming a qd algorithm
                 *
                 * @param indiv_to_insert Individual to add to the grid
                 */
                void direct_add(const indiv_t& indiv_to_insert)
                {
                  point_t point_indiv_to_insert = _get_point(indiv_to_insert);
                  size_t indiv_bd_index = _centroids_storage.nearest(point_indiv_to_insert).second;

                  _archive[indiv_bd_index] = indiv_to_insert;
                }

                void update(pop_t& offspring, pop_t& parents)
                {
                   // _update_novelty();
                    // for (size_t i = 0; i < offspring.size(); i++)
                    //     _update_indiv(offspring[i]);
                    // for (size_t i = 0; i < parents.size(); i++)
                    //     _update_indiv(parents[i]);


                    // Do nothing for now
                    // _update_novelty();
                    // for (size_t i = 0; i < offspring.size(); i++)
                    //     _update_indiv(offspring[i], *this);
                    // for (size_t i = 0; i < parents.size(); i++)
                    //     _update_indiv(parents[i], *this);
                }

                const pop_t& archive() const { return _archive; }
                const pop_t& parents() const { return _archive_parents; }

            protected:
                pop_t _archive;
                pop_t _archive_parents;
                Eigen::MatrixXd _centroids;
                storage_t _centroids_storage;

                // Convert the descriptor into a Point_t
                template <typename I> point_t _get_point(const I& indiv) const
                {
		  size_t dim = Params::qd::behav_dim;// needed for linking??
                    point_t p(dim);
                    for (size_t i = 0; i < Params::qd::behav_dim; ++i)
                        p(i) = indiv->fit().desc()[i];
                    return p;
                }

                // void _update_novelty()
                // {
                //     tbb::parallel_for(tbb::blocked_range<indiv_t *>(_array.data(), _array.data()
                //     + _array.num_elements()),
                //                       Par_novelty<Grid<Phen, Params>>(*this));
                // }
            };
        } // namespace container
    } // namespace qd
} // namespace sferes
#endif
