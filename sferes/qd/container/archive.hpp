#ifndef QD_CONTAINER_ARCHIVE_HPP
#define QD_CONTAINER_ARCHIVE_HPP

#include "tools.hpp"
#include <tbb/parallel_for_each.h>
#include <Eigen/Core>

namespace sferes {
    namespace qd {
        namespace container {

	  template <typename Phen, typename Storage, typename Params> 
            class Archive {
            public:
	    typedef boost::shared_ptr<Phen> indiv_t;
	    typedef typename std::vector<indiv_t> pop_t;
	    typedef typename pop_t::iterator it_t;
	    typedef Eigen::Map<const Eigen::VectorXd> point_t;
	    typedef Storage storage_t;
	    
	    using knn_iterator_t = typename std::vector<typename Storage::data_t>::iterator;
	    using const_knn_iterator_t = typename std::vector<typename Storage::data_t>::const_iterator;
	    Archive() {}

	      
                void erase_content() { _archive = storage_t(); }

                void get_full_content(std::vector<indiv_t>& content)
                {
                    for (auto it = _archive.begin(); it != _archive.end(); it++)
                        content.push_back((*it).second);
                }

                bool add(indiv_t i1)
                {
		  // p_i1 is the behavioral coordinate of i1
		  point_t p_i1(i1->fit().desc().data(), i1->fit().desc().size());
		    
                    if (i1->fit().dead())
                        return false;
                    if (_archive.size() == 0
                        || (_archive.nearest(p_i1).first - p_i1).norm() > Params::nov::l) // ADD because new
                    {
		      // this is novel enough, we add
		      direct_add(i1);
		      return true;
                    }
                    else if (_archive.size() == 1) {
                        // there is only one indiv in the archive and the current one is too close
                        return false;
                    }
                    else // archive size >= 2
                    {
                        auto neigh_cur = _archive.knn(p_i1, 2);
                        if ((p_i1 - neigh_cur[1].first).norm() < (1 - Params::nov::eps) * Params::nov::l) // too close the second NN -- this works better
                        {
			  // too close the second nearest neighbor, we skip
			  return false;
                        }
                        auto nn = neigh_cur[0].second;
                        std::vector<double> score_cur(2, 0), score_nn(2, 0);
                        score_cur[0] = i1->fit().value();
                        score_nn[0] = nn->fit().value();

                        // Compute the Novelty
                        neigh_cur.clear();
			auto neigh_nn=neigh_cur;
			point_t p_nn(nn->fit().desc().data(), nn->fit().desc().size());

			// we look for the k+1 nearest neighbours as the first one is "nn" which might be or not replaced.
			neigh_cur = _archive.knn(p_i1, Params::nov::k + 1);
			neigh_nn = _archive.knn(p_nn, Params::nov::k + 1);
			  
                        score_cur[1] = get_novelty(i1, ++neigh_cur.begin(), neigh_cur.end());
                        score_nn[1] = get_novelty(nn, ++neigh_nn.begin(), neigh_nn.end());

                        // TEST
                        if ((score_cur[0] >= (1 - sign(score_nn[0]) * Params::nov::eps) * score_nn[0] &&
			     score_cur[1] >= (1 - sign(score_nn[1]) * Params::nov::eps) * score_nn[1]     ) &&
			   ((score_cur[0] - score_nn[0]) * std::abs(score_nn[1]) > -(score_cur[1] - score_nn[1]) * std::abs(score_nn[0])))
			  {
                            // add if significatively better on one objective
                            _replace(nn, i1);
                            return true;
			  }
                        else
			  {
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

	      //This assumes that the solutions between begin and end are sorted from the nearest to the furthest and that std::distance(begin, end) = Params::nov::k
	    static double get_novelty(const indiv_t& indiv, const_knn_iterator_t begin, const_knn_iterator_t end)
                {
		   point_t p_indiv(indiv->fit().desc().data(), indiv->fit().desc().size());
                    double sum = 0;
                    const_knn_iterator_t it = begin;
		    for (const_knn_iterator_t it = begin; it != end; it++) {
		      sum += ((*it).first - p_indiv).norm();
		    }
                    return sum / std::distance(begin, end);
                }

                static std::pair<double, double> get_nov_and_lq(const indiv_t& indiv, const storage_t& apop)
	      {
		    point_t p_indiv(indiv->fit().desc().data(), indiv->fit().desc().size());
		    // we look for the k+1 nearest neighbours as the first one is "indiv" which is already in the archive
		    auto neigh = apop.knn(p_indiv, Params::nov::k + 1);
		    
                    return std::pair<double, double>(
                        get_novelty(indiv, neigh.begin()++, neigh.end()),
                        get_lq(indiv, neigh.begin()++, neigh.end()));
                }

	      static double get_lq(const indiv_t& indiv, const_knn_iterator_t begin,
                    const_knn_iterator_t end)
                {
                    int count = 0;
                    for (const_knn_iterator_t it = begin; it != end; it++)
                        if ((*it).second->fit().value() < indiv->fit().value())
                            count++;
                    return count;
                }

	      const storage_t& archive() const { return _archive; }


                void direct_add(const indiv_t& tobeinserted)
                {
		  point_t p_tobeinserted(tobeinserted->fit().desc().data(), tobeinserted->fit().desc().size());
		  _archive.add(p_tobeinserted,tobeinserted);
                }

            protected:
                void _replace(const indiv_t& toberemoved, const indiv_t& tobeinserted)
                {
		  point_t p_toberemoved(toberemoved->fit().desc().data(), toberemoved->fit().desc().size());
		  _archive.remove(p_toberemoved);
		  direct_add(tobeinserted);
                }

                struct _p_novelty {
		  const storage_t& _apop;
                    _p_novelty(const storage_t& apop) : _apop(apop) {}
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

                storage_t _archive;
            };
        } // namespace container
    } // namespace qd
} // namespace sferes

#endif
