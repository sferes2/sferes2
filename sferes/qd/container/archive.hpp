#ifndef QD_CONTAINER_ARCHIVE_HPP
#define QD_CONTAINER_ARCHIVE_HPP

#ifdef USE_KDTREE

#define LIBSSRCKDTREE_HAVE_BOOST
#include <ssrc/spatial/kd_tree.h>

#include "tools.hpp"
#include <tbb/parallel_for_each.h>

namespace sferes {
    namespace qd {
        namespace container {

            template <typename Phen, typename Params> 
            class Archive {
            public:
                typedef boost::shared_ptr<Phen> indiv_t;
                typedef typename std::vector<indiv_t> pop_t;
                typedef typename pop_t::iterator it_t;
                typedef std::array<float, Params::qd::dim> point_t;
                typedef ssrc::spatial::kd_tree<point_t, indiv_t> Tree;

                Archive() {}

	      
                void erase_content() { _archive = Tree(); }

                void get_full_content(std::vector<indiv_t>& content)
                {
                    for (auto it = _archive.begin(); it != _archive.end(); it++)
                        content.push_back((*it).second);
                }

                bool add(indiv_t i1)
                {
                    // TODO
                    // update archive
                    if (i1->fit().dead())
                        return false;
                    if (_archive.size() == 0
                        || /*_archive.size()<Params::nov::k ||*/ _dist(
                               get_nearest(i1, _archive, false)->fit().desc(), i1->fit().desc())
                            > Params::nov::l) // ADD because new
                    {
                        direct_add(i1);
                        return true;
                    }
                    else if (_archive.size() == 1) {
                        // there is only one indiv in the archive and the current one is too close
                        return false;
                    }
                    else // archive size min = 2
                    {
                        pop_t neigh_current;

                        // Be careful, the first one referes to nn
                        get_knn(i1, _archive, 2, neigh_current, false);
                        if (_dist(i1->fit().desc(), neigh_current[1]->fit().desc())
                            < (1 - Params::nov::eps)
                                * Params::nov::l) // too close the second NN -- this works better
                        // if(_dist(i1->fit().desc(), neigh_current[1]->fit().desc())  <
                        // Params::nov::l)//too close the second NN
                        {
                            return false;
                        }
                        indiv_t nn = neigh_current[0];
                        std::vector<double> score_cur(2, 0), score_nn(2, 0);
                        score_cur[0] = i1->fit().value();
                        score_nn[0] = nn->fit().value();
                        // Compute the Novelty
                        neigh_current.clear();
                        if (_archive.size() < Params::nov::k + 1) {
                            // Be careful, the first one referes to nn
                            get_knn(i1, _archive, _archive.size(), neigh_current, false);
                        }
                        else {
                            // Be careful, the first one referes to nn
                            get_knn(i1, _archive, Params::nov::k + 1, neigh_current, false);
                        }
                        score_cur[1]
                            = get_novelty(i1, neigh_current.begin()++, neigh_current.end());
                        score_nn[1] = get_novelty(nn, _archive);
                        // TEST
                        int score = 0;
                        if ((score_cur[0]
                                    >= (1 - sign(score_nn[0]) * Params::nov::eps) * score_nn[0]
                                && score_cur[1]
                                    >= (1 - sign(score_nn[1]) * Params::nov::eps) * score_nn[1])
                            && ((score_cur[0] - score_nn[0]) * std::abs(score_nn[1])
                                   > -(score_cur[1] - score_nn[1]) * std::abs(score_nn[0]))) {
                            // add if significatively better on one objective
                            _replace(nn, i1);
                            return true;
                        }
                        else {
                            return false;
                        }
                    }
                }

                void update(pop_t& offspring, pop_t& parents)
                {
                    _archive.optimize();

                    _p_novelty nov(_archive);
                    tbb::parallel_for_each(_archive.begin(), _archive.end(), nov);
                    std::for_each(offspring.begin(), offspring.end(), nov);
                    std::for_each(parents.begin(), parents.end(), nov);
                }

                static indiv_t get_nearest(
                    const indiv_t& indiv, Tree& apop, const bool omit_query_point)
                {
                    typename Tree::key_type q;
                    Archive::_behavior_to_point(indiv->fit().desc(), &q);
                    typename Tree::iterator it = apop.find_nearest_neighbor(q, omit_query_point);
                    return it->second;
                }

                static void get_knn(const indiv_t& indiv, const Tree& apop, int k, pop_t& nearest,
                    const bool omit_query_point) 
                {
                    typename Tree::key_type q;
                    Archive::_behavior_to_point(indiv->fit().desc(), &q);
                    // k nearest points
                    typedef typename Tree::knn_iterator knn_iterator_t;
                    std::pair<knn_iterator_t, knn_iterator_t> range
                        = apop.find_nearest_neighbors(q, k, omit_query_point);

                    nearest.clear();
                    for (knn_iterator_t it = range.first, end = range.second;
                         it != end && nearest.size() < k; ++it)
                        // for (size_t z = 0; z < it->second && nearest.size() < Params::nov::k;
                        // ++z)
                        nearest.push_back(it->second);
                    assert(nearest.size() == k || nearest.size() == apop.size() || nearest.size() == apop.size());
                }

                static double get_novelty(const indiv_t& indiv, Tree& apop)
                {
                    pop_t nearest;
                    if (apop.size() < (Params::nov::k + 1))
                        // here we omit because indivs are in the archive
                        get_knn(indiv, apop, apop.size() - 1, nearest, true);
                    else
                        // here we omit because indivs are in the archive
                        get_knn(indiv, apop, Params::nov::k, nearest, true);

                    // compute the mean distance
                    // double sum = 0.0f;
                    // BOOST_FOREACH(indiv_t& x, nearest)
                    //  sum += _dist(x->fit().desc(), indiv->fit().desc());
                    // return sum/Params::nov::k;;

                    return get_novelty(indiv, nearest.begin(), nearest.end());
                }

                static double get_novelty(const indiv_t& indiv, typename pop_t::iterator begin,
                    typename pop_t::iterator end)
                {
                    double sum = 0;
                    typename pop_t::iterator it = begin;
                    // for(int i =0;i<Params::nov::k; i++){
                    for (typename pop_t::iterator it = begin; it != end; it++) {
                        sum += _dist((*it)->fit().desc(), indiv->fit().desc());
                        // it++;
                    }
                    return sum / std::distance(begin, end); // Params::nov::k;
                }

                static std::pair<double, double> get_nov_and_lq(
                    const indiv_t& indiv, Tree& apop)
                {
                    pop_t nearest;
                    if (apop.size() < (Params::nov::k + 1))
                        get_knn(indiv, apop, apop.size() - 1, nearest,
                            true); // here we omit because indivs are in the archive
                    else
                        get_knn(indiv, apop, Params::nov::k, nearest,
                            true); // here we omit because indivs are in the archive

                    return std::pair<double, double>(
                        get_novelty(indiv, nearest.begin(), nearest.end()),
                        get_lq(indiv, nearest.begin(), nearest.end()));
                }
                static double get_lq(const indiv_t& indiv, typename pop_t::iterator begin,
                    typename pop_t::iterator end)
                {
                    if (std::distance(begin, end) > Params::nov::k) {
                        // NEED to sort
                        //  std::sort(begin,end,_compare_dist_f(indiv));
                        assert(false);
                    }
                    int count = 0;
                    // TODO remove this
                    /* typename pop_t::iterator it= begin;
                       for(int i =0;i<Params::nov::k; i++){
                       if((*it)->fit().value() < indiv->fit().value())
                       count++;
                       it++;
                       }*/
                    for (typename pop_t::iterator it = begin; it != end; it++) {
                        if ((*it)->fit().value() < indiv->fit().value())
                            count++;
                    }

                    return count;
                }

                const Tree& archive() const { return _archive; }


                void direct_add(const indiv_t& tobeinserted)
                {
                    point_t p;
                    this->_behavior_to_point(tobeinserted->fit().desc(), &p);
                    typename Tree::iterator it = _archive.find(p);
                    if (it == _archive.end()) {
                        _archive.insert(p, tobeinserted);
                    }
                }

            protected:
                void _replace(const indiv_t& toberemoved, const indiv_t& tobeinserted)
                {
                    point_t remove;
                    this->_behavior_to_point(toberemoved->fit().desc(), &remove);
                    _archive.remove(remove);
                    // std::cout<<"problem remove"<<std::endl;
                    direct_add(tobeinserted);
                }

                template <typename Behavior, typename Point>
                static void _behavior_to_point(const Behavior& b, Point* p)
                {
                    assert(p->size() == b.size());
                    for (size_t i = 0; i < p->size(); ++i)
                        (*p)[i] = b[i];
                }

                struct _p_novelty {
                    Tree& _apop;
                    _p_novelty( Tree& apop) : _apop(apop) {}
                    _p_novelty(const _p_novelty& ev) : _apop(ev._apop) {}

                    // use the Euclidean distance !
                    void operator()(indiv_t& indiv) const
                    {

                        if (indiv->fit().dead()) {
                            indiv->fit().set_novelty(-std::numeric_limits<double>::infinity());
                            indiv->fit().set_local_quality(
                                -std::numeric_limits<double>::infinity());
                            return;
                        }

                        auto res = Archive::get_nov_and_lq(indiv, _apop);
                        indiv->fit().set_novelty(res.first);
                        indiv->fit().set_local_quality(res.second);
                    }
                    template <typename value_type> void operator()(value_type& v) const
                    {
                        this->operator()(v.second);
                    }
                };

                Tree _archive;
            };
        } // namespace container
    } // namespace qd
} // namespace sferes
#else
#warning "No KD_TREE library found: no qd/container/archive.hpp"
#endif
#endif
