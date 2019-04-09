#ifndef STAT_QD_SELECTION_HPP_
#define STAT_QD_SELECTION_HPP_

#include <boost/multi_array.hpp>
#include <numeric>
#include <sferes/stat/stat.hpp>


namespace sferes {
    namespace stat {
        SFERES_STAT(QdSelection, Stat) {
        public:
            typedef boost::shared_ptr<Phen> phen_t;
            typedef boost::multi_array<phen_t, Params::qd::behav_dim> array_t;

            template <typename E> void refresh(const E& ea)
            {
                if (!ofs.is_open()) {
                    std::string prefix("selection");

                    std::string fname = ea.res_dir() + "/" + prefix + std::string(".dat");
                    ofs.open(fname.c_str());
                }

                assert(ea.offspring().size() == ea.parents().size());
                assert(ea.offspring().size() == ea.added().size());

                for (size_t i = 0; i < ea.offspring().size(); i++) {
                    ofs << ea.gen() << "  ";
                    for (size_t dim = 0; dim < ea.parents()[i]->fit().desc().size(); ++dim)
                        ofs << ea.parents()[i]->fit().desc()[dim] << " ";
                    ofs << "   " << ea.parents()[i]->fit().value() << "   ";
                    ofs << "   " << ea.parents()[i]->fit().novelty() << "   ";
                    ofs << "   " << ea.parents()[i]->fit().local_quality() << "   ";
                    ofs << "   " << ea.parents()[i]->fit().curiosity() << "   ";

                    for (size_t dim = 0; dim < ea.offspring()[i]->fit().desc().size(); ++dim)
                        ofs << ea.offspring()[i]->fit().desc()[dim] << " ";
                    ofs << "   " << ea.offspring()[i]->fit().value() << "   ";
                    ofs << "   " << ea.offspring()[i]->fit().novelty() << "   ";
                    ofs << "   " << ea.offspring()[i]->fit().local_quality() << "   ";
                    ofs << "   " << ea.offspring()[i]->fit().curiosity() << "   ";

                    ofs << "   " << ea.added()[i] << std::endl;
                }
            }

        protected:
            std::ofstream ofs;
        };

    } // namespace stat
} // namespace sferes

#endif
