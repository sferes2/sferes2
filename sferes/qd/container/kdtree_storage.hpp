#ifndef SFERES_KDTREE_STORAGE_HPP_
#define SFERES_KDTREE_STORAGE_HPP_

#include <Eigen/Core>
#include <algorithm>
#include <numeric>
#include <vector>

#ifdef USE_KDTREE
#define LIBSSRCKDTREE_HAVE_BOOST

#include <ssrc/spatial/kd_tree.h>

namespace sferes {
    namespace qd {
        namespace container {
            /// compute the k-nearest neighbors using a kd-tree (optional external library)
            /// this is effective in low-dimensional spaces (< 10)
            /// but not in high-dimensional spaces (> 10)
            template <typename Value, int Dim> class KdtreeStorage {
            public:
                using point_t = std::array<float, Dim>;
                using tree_t = ssrc::spatial::kd_tree<point_t, Value>;
                using data_t = std::pair<Eigen::VectorXd, Value>;
                using iterator_t = typename tree_t::iterator;
                using const_iterator_t = typename tree_t::const_iterator;

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
                    point_t pp = convert_point(p);
                    _data.insert(pp, v);
                }

                /// Remove a point (return false if not found)
                void remove(const Eigen::VectorXd& v)
                {
                    point_t p = convert_point(v);
                    _data.remove(p);
                }


                // return the data that correspond to the k closest neighbors
                // (if you neeed indices, store them in Value)
                std::vector<data_t> knn(const Eigen::VectorXd& pp, int k) const
                {
                    assert(!_data.empty());
                    k = std::min(k, (int)_data.size());
                    point_t p = convert_point(pp);
                    auto range = _data.find_nearest_neighbors(p, k, true);
                    std::vector<data_t> res;
                    for (auto it = range.first, end = range.second;
                         it != end && res.size() < k; ++it)
                        res.push_back(std::make_pair(convert_point(it->first), it->second));
                    assert(res.size() <= k);
                    assert(res.size() != 0);
                    return res;
                }

                // return the data that correspond to the closest neighbor
                // (if you neeed indices, store them in Value)
                data_t nearest(const Eigen::VectorXd& p) const
                {
                    assert(!_data.empty());
                    point_t pp = convert_point(p);
                    auto it = _data.find_nearest_neighbor(pp, true);
                    return std::make_pair(convert_point(it->first), it->second);
                }

                /// Return true if the point p is already in the archive
                bool contains(const Eigen::VectorXd& p) const
                {
                   assert(0); // not implemented yet
                }


                point_t convert_point(const Eigen::VectorXd& v) const {
                    assert(v.size() == Dim);
                    point_t p;
                    for (size_t i = 0; i < v.size(); ++i)
                        p[i] = v[i];
                    return p;
                }
                Eigen::VectorXd convert_point(const point_t& v) const {
                    Eigen::VectorXd p(v.size());
                    for (size_t i = 0; i < v.size(); ++i)
                        p[i] = v[i];
                    return p;
                }
            protected:
                tree_t _data;
            };
        } // namespace container
    } // namespace qd
} // namespace sferes
#endif
#endif