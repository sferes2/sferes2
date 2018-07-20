#ifndef QD_SELECTOR_NOSELECTION_HPP
#define QD_SELECTOR_NOSELECTION_HPP

namespace sferes {
    namespace qd {
        namespace selector {
            // this corresponds to random sampling (no QD)
            template <typename Phen, typename Params> struct NoSelection {
                typedef boost::shared_ptr<Phen> indiv_t;

                template <typename EA>
                void operator()(std::vector<indiv_t>& pop, const EA& ea) const
                {
                    for (auto& indiv : pop) {
                        indiv = boost::shared_ptr<Phen>(new Phen());
                        indiv->random();
                    }
                }
            };
        } // namespace selector
    } // namespace qd
} // namespace sferes
#endif
