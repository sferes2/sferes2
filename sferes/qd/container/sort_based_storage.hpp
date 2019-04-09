#ifndef SFERES_SORT_BASED_STORAGE_HPP_
#define SFERES_SORT_BASED_STORAGE_HPP_

#include <Eigen/Core>
#include <algorithm>
#include <numeric>
#include <vector>
#include <cmath>

namespace sferes {
    namespace qd {
        namespace container {
            /// compute the k-nearest neighbors using std::partial_sort
            /// this is not effective in low-dimensional spaces (< 10)
            /// but it works better than a kd-tree for more than 10-dimensional spaces
            /// (also, this is straightforward to implement)
            template <typename Value> class SortBasedStorage {
            public:
                using data_t = std::pair<Eigen::VectorXd, Value>;
                using iterator_t = typename std::vector<data_t>::iterator;
                using const_iterator_t = typename std::vector<data_t>::const_iterator;

                /// number of elements in the archive
                size_t size() const { return _data.size(); }

                /// accesss to the elements
                iterator_t begin() { return _data.begin(); }
                iterator_t end() { return _data.end(); }
                const_iterator_t begin() const { return _data.begin(); }
                const_iterator_t end() const { return _data.end(); }
                
                /// Add a point to the storage
                void add(const Eigen::VectorXd& p, const Value& v)
                {
                    _data.push_back(std::make_pair(p, v));
                }

                /// Remove a point
                void remove(const Eigen::VectorXd& v)
                {
                    assert(!_data.empty());
                    auto pred = [&v](const data_t& d) { return d.first == v; };
                    auto it = std::remove_if(_data.begin(), _data.end(), pred);
                    _data.erase(it, _data.end());
                }

                // get the stored data as a list of pairs (point, data)
                const std::vector<data_t>& data() const { return _data; }

                // return the data that correspond to the k closest neighbors
                // (if you neeed indices, store them in Value)
                std::vector<data_t> knn(const Eigen::VectorXd& p, int k) const
                {
                    assert(!_data.empty());
                    k = std::min(k, (int)_data.size());

                    // we avoid to reallocate this for each query, but we still want to be thread-safe
                    static thread_local std::vector<size_t> indices;
                    static thread_local std::vector<double> distances;
                    indices.resize(_data.size());
                    distances.resize(_data.size());

                    // we first compute the distances, then do a partial_sort
                    std::iota(indices.begin(), indices.end(), 0);
                    for (size_t j = 0; j < _data.size(); ++j)
                        distances[j] = (p - _data[j].first).squaredNorm();
                    auto dist = [&](int x1, int x2) { return distances[x1] < distances[x2]; };
                    std::partial_sort(indices.begin(), indices.begin() + k, indices.end(), dist);
                    std::vector<data_t> result(k);
                    for (size_t i = 0; i < k; ++i)
                        result[i] = _data[indices[i]];
                    return result;
                }

                // return the data that correspond to the closest neighbor
                // (if you neeed indices, store them in Value)
                const data_t& nearest(const Eigen::VectorXd& p) const
                {
                    assert(!_data.empty());

                    static thread_local std::vector<int> indices;
                    static thread_local std::vector<double> distances;
                    indices.resize(_data.size());
                    distances.resize(_data.size());

                    std::iota(indices.begin(), indices.end(), 0);
                    for (size_t j = 0; j < _data.size(); ++j)
                        distances[j] = (p - _data[j].first).squaredNorm();
                    auto dist = [&](int x1, int x2) { return distances[x1] < distances[x2]; };
                    return _data[*std::min_element(indices.begin(), indices.end(), dist)];
                }

                /// Return true if the point p is already in the archive
                bool contains(const Eigen::VectorXd& p) const
                {
                    auto pred = [&p](const data_t& d) { return d.first == p; };
                    auto it = std::find_if(_data.begin(), _data.end(), pred);
                    return it != _data.end();
                }


	      void optimize(){}//does nothing. 
            protected:
                std::vector<data_t> _data;
            };
        } // namespace container
    } // namespace qd
} // namespace sferes
#endif
