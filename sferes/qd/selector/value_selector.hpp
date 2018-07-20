#ifndef QD_SELECTOR_VALUESELECTOR_HPP
#define QD_SELECTOR_VALUESELECTOR_HPP

namespace sferes {
    namespace qd {
        namespace selector {

            struct getFitness {
                template <typename Phen> inline double getValue(const Phen& p) const
                {
                    return p->fit().value();
                }
            };

            struct getNovelty {
                template <typename Phen> inline double getValue(const Phen& p) const
                {
                    return p->fit().novelty();
                }
            };

            struct getCuriosity {
                template <typename Phen> inline double getValue(const Phen& p) const
                {
                    return p->fit().curiosity();
                }
            };

            struct getLocalQuality {
                template <typename Phen> inline double getValue(const Phen& p) const
                {
                    return p->fit().local_quality();
                }
            };
        } // namespace selector
    } // namespace qd
} // namespace sferes
#endif
