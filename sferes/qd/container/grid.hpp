#ifndef QD_CONTAINER_GRID_HPP
#define QD_CONTAINER_GRID_HPP

namespace sferes {
    namespace qd {
        namespace container {
            template <typename Phen, typename Params> class Grid {
            public:
                typedef boost::shared_ptr<Phen> indiv_t;
                typedef typename std::vector<indiv_t> pop_t;
                typedef typename pop_t::iterator it_t;
                typedef typename std::vector<std::vector<indiv_t>> front_t;

                static const size_t dim = Params::qd::behav_dim;
                typedef boost::multi_array<indiv_t, dim> array_t;
                typedef typename array_t::multi_array_base::index_range index_range_t;
                typedef boost::detail::multi_array::index_gen<dim, dim> index_gen_t;
                typedef typename array_t::template const_array_view<dim>::type view_t;

                typedef boost::array<typename array_t::index, dim> behav_index_t;

                typedef boost::array<float, dim> point_t;

                behav_index_t grid_shape;

                Grid()
                {
                    assert(dim == Params::qd::grid_shape_size());

                    for (size_t i = 0; i < Params::qd::grid_shape_size(); ++i)
                        grid_shape[i] = Params::qd::grid_shape(i);

                    // allocate space for _array and _array_parents
                    _array.resize(grid_shape);
                }

	      
                template <typename I> behav_index_t get_index(const I& indiv) const
                {
                    point_t p = get_point(indiv);
                    behav_index_t behav_pos;
                    for (size_t i = 0; i < Params::qd::grid_shape_size(); ++i) {
                        behav_pos[i] = round(p[i] * (grid_shape[i] - 1));
                        // behav_pos[i] = std::min(behav_pos[i], grid_shape[i] - 1);
                        assert(behav_pos[i] < grid_shape[i]);
                    }
                    return behav_pos;
                }

                void get_full_content(std::vector<indiv_t>& content) const
                {
                    for (const indiv_t* i = _array.data();
                         i < (_array.data() + _array.num_elements()); ++i)
                        if (*i)
                            content.push_back(*i);
                }

                bool add(indiv_t i1)
                {
                    if (i1->fit().dead())
                        return false;

                    behav_index_t behav_pos = get_index(i1);

                    float epsilon = 0.00;
                    if (!_array(behav_pos)
                        || (i1->fit().value() - _array(behav_pos)->fit().value()) > epsilon
                        || (fabs(i1->fit().value() - _array(behav_pos)->fit().value()) <= epsilon
                               && _dist_center(i1) < _dist_center(_array(behav_pos)))) {
                        _array(behav_pos) = i1;
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
                    behav_index_t indiv_bd_index = get_index(indiv_to_insert);
                    _array(indiv_bd_index) = indiv_to_insert;
                }

                void update(pop_t& offspring, pop_t& parents)
                {
                    _update_novelty();
                    for (size_t i = 0; i < offspring.size(); i++)
                        _update_indiv(offspring[i]);
                    for (size_t i = 0; i < parents.size(); i++)
                        _update_indiv(parents[i]);
                }

                const array_t& archive() const { return _array; }

            protected:
                // Converts the descriptor into a Point_t
                template <typename I> point_t get_point(const I& indiv) const
                {
                    point_t p;
                    for (size_t i = 0; i < Params::qd::grid_shape_size(); ++i)
                        p[i] = std::min(1.0, indiv->fit().desc()[i]);

                    return p;
                }

                template <typename I> float _dist_center(const I& indiv)
                {
                    /* Returns distance to center of behavior descriptor cell */
                    float dist = 0.0;
                    point_t p = get_point(indiv);
                    for (size_t i = 0; i < Params::qd::grid_shape_size(); ++i)
                        dist += pow(p[i]
                                - (float)round(p[i] * (float)(grid_shape[i] - 1))
                                    / (float)(grid_shape[i] - 1),
                            2);

                    dist = sqrt(dist);
                    return dist;
                }

                void _update_novelty()
                {
                    tbb::parallel_for(tbb::blocked_range<indiv_t*>(
                                          _array.data(), _array.data() + _array.num_elements()),
                        [&](const tbb::blocked_range<indiv_t*>& r) {
                            for (indiv_t* indiv = r.begin(); indiv != r.end(); ++indiv)
                                if (*indiv) {
                                    _update_indiv(*indiv);
                                }
                        });
                }

                // Functor to iterate over a Boost MultiArray concept instance.
                template <typename T, typename V, size_t Dimensions = T::dimensionality>
                struct IterateHelper {
                    void operator()(T& array, V& vect) const
                    {
                        for (auto element : array)
                            IterateHelper<decltype(element), V>()(element, vect);
                    }
                };

                // Functor specialization for the final dimension.
                template <typename T, typename V> struct IterateHelper<T, V, 1> {
                    void operator()(T& array, V& vect) const
                    {
                        for (auto& element : array)
                            if (element)
                                vect.push_back(element);
                    }
                };

                // Utility function to apply a function to each element of a Boost
                // MultiArray concept instance (which includes views).
                template <typename T, typename V> static void iterate(T& array, V& vect)
                {
                    IterateHelper<T, V>()(array, vect);
                }

                // WARNING, individuals in population can be dead...
                void _update_indiv(indiv_t& indiv)
                {
                    if (indiv->fit().dead()) {
                        indiv->fit().set_novelty(-std::numeric_limits<double>::infinity());
                        indiv->fit().set_local_quality(-std::numeric_limits<double>::infinity());
                        return;
                    }

                    int count = 0;
                    view_t neighborhood = this->get_neighborhood(indiv);
                    std::vector<indiv_t> neigh;
                    iterate(neighborhood, neigh);

                    indiv->fit().set_novelty(-(double)neigh.size());
                    for (auto& n : neigh)
                        if (n->fit().value() < indiv->fit().value())
                            count++;
                    indiv->fit().set_local_quality(count);
                }

                inline view_t get_neighborhood(indiv_t indiv) const
                {
                    behav_index_t ind = get_index(indiv);
                    index_gen_t indix;
                    int i = 0;
                    for (auto it = indix.ranges_.begin(); it != indix.ranges_.end(); it++) {
                        *it = index_range_t(std::max((int)ind[i] - (int)Params::nov::deep, 0),
                            std::min(ind[i] + Params::nov::deep + 1,
                                (size_t)grid_shape[i])); // bound! so stop at id[i]+2-1

                        i++;
                    }

                    view_t ngbh = _array[indix];
                    return ngbh;
                }

                array_t _array;
            };
        } // namespace container
    } // namespace qd
} // namespace sferes
#endif
