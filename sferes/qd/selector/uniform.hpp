#ifndef QD_SELECTOR_UNIFORM_HPP
#define QD_SELECTOR_UNIFORM_HPP

namespace sferes {
    namespace qd {
        namespace selector {
            // MAP-Elites style
            template <typename Phen, typename Params> struct Uniform {

                typedef boost::shared_ptr<Phen> indiv_t;

                template <typename EA>
                void operator()(std::vector<indiv_t>& pop, const EA& ea) const
                {
                    for (auto& indiv : pop) {
                        int x1 = misc::rand<int>(0, ea.pop().size());
                        indiv = ea.pop()[x1];
                    }
                }
            };
        } // namespace selector
    } // namespace qd
} // namespace sferes
#endif
