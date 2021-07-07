// C++ code for creating a CVT
// Vassilis Vassiliades - Inria, Nancy - April 2018
// This file can be used outside of sferes if needed
#ifndef COMPUTE_CVT_HPP_
#define COMPUTE_CVT_HPP_

#include <algorithm> //for random_shuffle
#include <fstream>
#include <iostream>
#include <numeric> //for iota
#include <vector>

#include <Eigen/Core>
#include <boost/program_options.hpp>

#ifndef NO_PARALLEL
#include <tbb/tbb.h>
#endif

namespace cvt {
    namespace details {

        // Random numbers in [0,1] (Eigen creates in [-1,1])
        Eigen::MatrixXd init_uniform(size_t number_of_sample_points, size_t dimensionality)
        {
            Eigen::MatrixXd data = Eigen::MatrixXd::Random(number_of_sample_points, dimensionality);
            data = (data + Eigen::MatrixXd::Constant(number_of_sample_points, dimensionality, 1.0))
                / 2.0;
            return data;
        }

        // we shuffle the data at each restart
        Eigen::MatrixXd init_restart(const Eigen::MatrixXd& data, const size_t num_clusters)
        {
            assert((int)num_clusters <= data.rows());

            std::vector<size_t> indices(data.rows());
            std::iota(indices.begin(), indices.end(), 0);
            std::random_shuffle(indices.begin(), indices.end());

            size_t dim = data.cols();
            Eigen::MatrixXd centroids = Eigen::MatrixXd::Zero(num_clusters, dim);

            for (size_t i = 0; i < num_clusters; ++i)
                centroids.row(i) = data.row(indices[i]);

            return centroids;
        }

        template<typename D>
        double calc_distances(const Eigen::MatrixXd& data, const Eigen::MatrixXd& centroids,
            Eigen::MatrixXd& new_centroids, std::vector<size_t>& counts, const D& dist_fun)
        {
            size_t nb_points = data.rows();
            double sum = 0.0;
            static tbb::mutex sm;
#ifndef NO_PARALLEL
            tbb::parallel_for(size_t(0), nb_points, size_t(1),
                [&](size_t i) {
#else
            for (size_t i = 0; i < nb_points; ++i) {
#endif
                    // Find the closest centroid to this point.
                    double min_distance = std::numeric_limits<double>::infinity();
                    size_t closest_cluster = centroids.rows(); // Invalid value.

                    for (int j = 0; j < centroids.rows(); j++) {
                        double distance = dist_fun(data.row(i), centroids.row(j));

                        if (distance < min_distance) {
                            min_distance = distance;
                            closest_cluster = j;
                        }

                        // Since the minimum distance cannot be less than 0
                        // we could accelerate computation by breaking
                        if (min_distance == 0.0)
                            break;
                    }
#ifndef NO_PARALLEL // for the lock
                    tbb::mutex::scoped_lock lock; // create a lock
                    lock.acquire(sm);
#endif
                    sum += min_distance;
                    // We now have the minimum distance centroid index.
                    new_centroids.row(closest_cluster) += data.row(i);
                    counts[closest_cluster]++;
#ifndef NO_PARALLEL
                    lock.release();
#endif // for the lock
                }
#ifndef NO_PARALLEL // for the parallel loop
            );
#endif

            // The loss is the mean
            return sum / (double)nb_points;
        } // namespace details

        void update_centroids(Eigen::MatrixXd& new_centroids, const std::vector<size_t>& counts)
        {
            // TODO: vectorize
            for (int i = 0; i < new_centroids.rows(); ++i) {
                new_centroids.row(i) = new_centroids.row(i) / (double)counts[i];
            }
        }

        double euclidean_distance(const Eigen::VectorXd& p1, const Eigen::VectorXd& p2)
        {
            return (p1 - p2).norm();
        }
    } // namespace details

    template <typename Distance = decltype(details::euclidean_distance), typename Init = decltype(details::init_uniform)>
    Eigen::MatrixXd cvt(size_t dimensionality, size_t num_clusters, size_t num_points = 10000,
        size_t max_iterations = 100, size_t restarts = 1, double tolerance = 10e-8,
        const Distance& dist_fun = details::euclidean_distance, const Init& init_fun = details::init_uniform)
    {
        Eigen::MatrixXd data = init_fun(num_points, dimensionality);
        std::vector<Eigen::MatrixXd> 
            all_centroids(restarts, Eigen::MatrixXd::Zero(num_clusters, data.cols()));
        std::vector<double> all_losses(restarts, 0.0);

        for (size_t r = 0; r < restarts; ++r) {
            // Initialize for this restart
            all_centroids[r] = details::init_restart(data, num_clusters);

            // Iterate (EM)
            double loss, prev_loss;
            loss = prev_loss = 0.0;
            double delta = tolerance;

            for (size_t i = 0; i < max_iterations; ++i) {
                Eigen::MatrixXd new_centroids = Eigen::MatrixXd::Zero(num_clusters, data.cols());

                // Calculate the distances
                std::vector<size_t> counts(num_clusters, 0);
                loss = details::calc_distances(data, all_centroids[r], new_centroids, counts, dist_fun);

                // delta = fabs(prev_loss - loss) / loss;
                delta = fabs(prev_loss - loss);

                if (delta < tolerance)
                    break;

                prev_loss = loss;

                // Update the centroids
                details::update_centroids(new_centroids, counts);

                all_centroids[r] = new_centroids;
            }

            // Store this centroid and the loss
            all_losses[r] = loss;
        }

        // Return the set of centroids with the lowest loss
        auto min_e = std::min_element(all_losses.begin(), all_losses.end());
        size_t argmin_index = std::distance(all_losses.begin(), min_e);

        assert(all_centroids[argmin_index].cols() == dimensionality);
        assert(all_centroids[argmin_index].rows() == num_clusters);

        return all_centroids[argmin_index];
    }
} // namespace cvt

#endif