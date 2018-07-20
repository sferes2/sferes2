#ifndef STAT_QD_PROGRESS_HPP_
#define STAT_QD_PROGRESS_HPP_

#include <numeric>
#include <sferes/stat/stat.hpp>

namespace sferes {
    namespace stat {
        SFERES_STAT(QdProgress, Stat){
        public:
            template <typename E> void refresh(const E& ea)
            {
                if (ea.gen() % Params::pop::dump_period == 0) {
                    _write_progress(std::string("progress"), ea);
                }
            }
            template <typename EA>
            void _write_progress(const std::string& prefix, const EA& ea) const
            {
                std::cout << "writing..." << prefix << std::endl;
                std::string fname = ea.res_dir() + "/" + prefix + std::string(".dat");

                std::ofstream ofs(fname.c_str(), std::ofstream::out | std::ofstream::app);

                size_t archive_size = ea.pop().size();
                double archive_max = ea.pop()[0]->fit().value();
                double sum_novelty = 0.0f;
                double sum_quality = 0.0f;
                double var_novelty = 0.0f;

                for (auto it = ea.pop().begin(); it != ea.pop().end(); ++it) {
                    sum_quality += (*it)->fit().value();
                    sum_novelty += (*it)->fit().novelty();
                    if (archive_max < (*it)->fit().value())
                        archive_max = (*it)->fit().value();
                }

                for (auto it = ea.pop().begin(); it != ea.pop().end(); ++it) {
                    var_novelty += std::pow((*it)->fit().novelty() - sum_novelty / archive_size, 2);
                }
                var_novelty /= archive_size;

                ofs << ea.gen() << " " << archive_size << " "
                    << " " << archive_max << " " << sum_quality << "   " << sum_novelty << " "
                    << var_novelty << std::endl;
            }
        };

    } // namespace stat
} // namespace sferes

#endif
