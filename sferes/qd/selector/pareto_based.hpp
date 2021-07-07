#ifndef QD_SELECTOR_PARETOBASED_HPP
#define QD_SELECTOR_PARETOBASED_HPP

#include <sferes/ea/crowd.hpp>
#include <sferes/ea/dom_sort.hpp>

namespace sferes {
    namespace qd {
        namespace selector {
            template <typename Phen, typename ObjSelector, typename Params> 
            struct ParetoBased {
                typedef boost::shared_ptr<ea::crowd::Indiv<Phen>> indiv_t;
                typedef typename std::vector<indiv_t> pop_t;
                typedef typename pop_t::iterator it_t;
                typedef typename std::vector<std::vector<indiv_t>> front_t;

                template <typename EA>
                void operator()(std::vector<boost::shared_ptr<Phen>>& final_pop, const EA& ea) const
                {
                    assert(ea.parents().size());
                    assert(ea.offspring().size());

                    // void _fill_nondominated_sort(pop_t& mixed_pop, pop_t& new_pop) {
                    pop_t parents, offspring, mixed_pop;
                    _pop2crow(ea.parents(), parents);
                    _pop2crow(ea.offspring(), offspring);
                    _merge(parents, offspring, mixed_pop);

                    _update_obj(mixed_pop);

                    assert(mixed_pop.size());
                    front_t fronts;

                    _rank_crowd(mixed_pop, fronts);

                    pop_t new_pop;

                    // fill the i first layers
                    size_t i;
                    for (i = 0; i < fronts.size(); ++i)
                        if (fronts[i].size() + new_pop.size() < Params::pop::size)
                            new_pop.insert(new_pop.end(), fronts[i].begin(), fronts[i].end());
                        else
                            break;

                    size_t size = Params::pop::size - new_pop.size();
                    // sort the last layer
                    if (new_pop.size() < Params::pop::size) {
                        std::sort(fronts[i].begin(), fronts[i].end(), ea::crowd::compare_crowd());
                        for (size_t k = 0; k < size; ++k) {
                            assert(i < fronts.size());
                            new_pop.push_back(fronts[i][k]);
                        }
                    }

                    // for(auto& indiv:new_pop)
                    //  std::cout<<ea.gen()<<"  "<<indiv->fit().novelty()<<"
                    //  "<<indiv->fit().local_quality()<< "   "<<indiv->fit().objs()[0]<< "
                    //  "<<indiv->fit().objs()[1]<<std::endl;

                    _selection(new_pop, final_pop);

                    assert(final_pop.size() == Params::pop::size);
                }

                // --- tournament selection ---
                void _selection(pop_t& old_pop, std::vector<boost::shared_ptr<Phen>>& new_pop) const
                {
                    new_pop.resize(old_pop.size());
                    std::vector<size_t> a1, a2;
                    misc::rand_ind(a1, old_pop.size());
                    misc::rand_ind(a2, old_pop.size());

                    // todo : this loop could be parallelized
                    for (size_t i = 0; i < old_pop.size(); i += 4) {
                        const indiv_t& p1 = _tournament(old_pop[a1[i]], old_pop[a1[i + 1]]);
                        const indiv_t& p2 = _tournament(old_pop[a1[i + 2]], old_pop[a1[i + 3]]);
                        const indiv_t& p3 = _tournament(old_pop[a2[i]], old_pop[a2[i + 1]]);
                        const indiv_t& p4 = _tournament(old_pop[a2[i + 2]], old_pop[a2[i + 3]]);
                        new_pop[i] = p1;
                        new_pop[i + 1] = p2;
                        new_pop[i + 2] = p3;
                        new_pop[i + 3] = p4;

                        // assert(i + 3 < new_pop.size());
                        // p1->cross(p2, new_pop[i], new_pop[i + 1]);
                        // p3->cross(p4, new_pop[i + 2], new_pop[i + 3]);
                    }
                }

                const indiv_t& _tournament(const indiv_t& i1, const indiv_t& i2) const
                {
                    // if (i1->rank() < i2->rank())
                    //   return i1;
                    // else if (i2->rank() > i1->rank())
                    //   return i2;
                    // else if (misc::flip_coin())
                    //   return i1;
                    // else
                    //   return i2;

                    int flag = fit::dominate_flag(i1, i2);
                    if (flag == 1)
                        return i1;
                    if (flag == -1)
                        return i2;
                    if (i1->crowd() > i2->crowd())
                        return i1;
                    if (i1->crowd() < i2->crowd())
                        return i2;
                    if (misc::flip_coin())
                        return i1;
                    else
                        return i2;
                }

                void _rank_crowd(pop_t& pop, front_t& fronts) const
                {
                    std::vector<size_t> ranks;

                    ea::dom_sort(pop, fronts, ranks);
                    //_update_pareto_front(fronts);
                    parallel::p_for(parallel::range_t(0, fronts.size()),
                        ea::crowd::assign_crowd<indiv_t>(fronts, Params::pareto::genoDiv));

                    for (size_t i = 0; i < ranks.size(); ++i)
                        pop[i]->set_rank(ranks[i]);
                    parallel::sort(pop.begin(), pop.end(), ea::crowd::compare_ranks());
                    ;
                }

                void _pop2crow(const std::vector<boost::shared_ptr<Phen>>& pop, pop_t& pop2) const
                {
                    assert(!pop.empty());
                    pop2.resize(pop.size());
                    for (size_t i = 0; i < pop.size(); ++i)
                        pop2[i] = boost::shared_ptr<ea::crowd::Indiv<Phen>>(
                            new ea::crowd::Indiv<Phen>(*pop[i]));
                }

                void _crow2pop(const pop_t& pop1, std::vector<boost::shared_ptr<Phen>>& pop2) const
                {
                    pop2.resize(pop1.size());
                    for (size_t i = 0; i < pop1.size(); ++i)
                        pop2[i] = pop1[i];
                }

                //
                void _merge(const pop_t& pop1, const pop_t& pop2, pop_t& pop3) const
                {
                    assert(pop1.size());
                    assert(pop2.size());
                    pop3.clear();
                    pop3.insert(pop3.end(), pop1.begin(), pop1.end());
                    pop3.insert(pop3.end(), pop2.begin(), pop2.end());
                    assert(pop3.size() == pop1.size() + pop2.size());
                }

                struct ObjUpdator {
                    ObjUpdator(indiv_t& indiv) : _indiv(indiv) {}
                    indiv_t& _indiv;
                    template <typename T> size_t operator()(size_t index, T& x) const
                    {

                        _indiv->fit().set_obj(index, x.getValue(_indiv));
                        return ++index;
                    }
                };
                void _update_obj(pop_t& pop) const
                {
                    for (auto& indiv : pop) {
                        while (indiv->fit().objs().size() != boost::fusion::size(_obj_selector))
                            indiv->fit().add_obj();
                        boost::fusion::accumulate(_obj_selector, 0, ObjUpdator(indiv));
                        // for(auto& obj: indiv->fit().objs())
                        //  std::cout<<obj<<"  ";
                        // std::cout<<std::endl;
                    }
                }

                ObjSelector _obj_selector;
            };
        } // namespace selector
    } // namespace qd
} // namespace sferes
#endif
