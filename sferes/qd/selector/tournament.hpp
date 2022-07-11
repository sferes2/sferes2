#ifndef QD_SELECTOR_TOURNAMENT_HPP
#define QD_SELECTOR_TOURNAMENT_HPP

#include <sferes/qd/selector/value_selector.hpp>

namespace sferes {
    namespace qd {
        namespace selector {
            template <typename Phen, typename ValueSelector, typename Params> 
            class Tournament {
            public:
                typedef boost::shared_ptr<Phen> indiv_t;

                template <typename EA> void operator()(std::vector<indiv_t>& pop, const EA& ea)
                {
                    this->operator()(pop, ea.pop());
                }

                void operator()(std::vector<indiv_t>& pop, const std::vector<indiv_t>& ea_pop)
                {
                    for (auto& indiv : pop) {
                        int x1 = misc::rand<int>(0, ea_pop.size());
                        int x2 = misc::rand<int>(0, ea_pop.size());
                        if (ValueSelector::getValue(ea_pop[x1])
                            > ValueSelector::getValue(ea_pop[x2]))
                            indiv = ea_pop[x1];
                        else
                            indiv = ea_pop[x2];
                    }
                }
            };
        } // namespace selector
    } // namespace qd
} // namespace sferes
#endif
