#ifndef QD_SELECTOR_POPULATIONBASED_HPP
#define QD_SELECTOR_POPULATIONBASED_HPP

#include "score_proportionate.hpp"

namespace sferes {
    namespace qd {
        namespace selector {
            template <typename Phen, typename Selector, typename Params>
            struct PopulationBased {

                typedef boost::shared_ptr<Phen> indiv_t;

                template <typename EA>
                void operator()(std::vector<indiv_t>& pop, const EA& ea) const
                {
                    std::vector<indiv_t> temp(ea.parents().size() + ea.offspring().size());
                    assert(temp.size() >= pop.size());

                    size_t ind = 0;
                    for (auto& indiv : ea.parents()) {
                        temp[ind] = indiv;
                        ind++;
                    }
                    for (auto& indiv : ea.offspring()) {
                        temp[ind] = indiv;
                        ind++;
                    }
                    _selector(pop, temp);
                }
                Selector _selector;
            };
        } // namespace selector
    } // namespace qd
} // namespace sferes
#endif
