#ifndef QD_SELECTOR_SCOREPROPORTIONATE_HPP
#define QD_SELECTOR_SCOREPROPORTIONATE_HPP

#include <tbb/parallel_reduce.h>
#include <tbb/parallel_sort.h>

namespace sferes {
    namespace qd {
        namespace selector {

            template <typename Phen, typename ValueSelector, typename Params> 
	    class ScoreProportionate {
            public:
                typedef boost::shared_ptr<Phen> indiv_t;
                typedef typename std::vector<indiv_t>::iterator it_t;
                typedef typename std::vector<indiv_t>::const_iterator itc_t;

                template <typename EA>
                void operator()(std::vector<indiv_t>& pop, const EA& ea) const
                {
                    this->operator()(pop, ea.pop());
                }

                void operator()(std::vector<indiv_t>& pop, const std::vector<indiv_t>& ea_pop) const
                {
                    std::pair<double, double> res = getMinAndSum(ea_pop);
                    double min = res.first;
                    double sum = res.second - min * ea_pop.size();

                    for (auto& indiv : pop) {
                        double r = 0;
                        if (sum)
                            r = misc::rand((double)sum);
                        itc_t it = ea_pop.cbegin();
                        double p = (ValueSelector::getValue(*it) - min);

                        while (p < r && it != ea_pop.end()) {
                            it++;
                            p += (ValueSelector::getValue(*it) - min);
                        }
                        indiv = *it;
                    }
                }

            private:
                std::pair<double, double> getMinAndSum(const std::vector<indiv_t>& pop) const
                {
                    typedef tbb::blocked_range<itc_t> range_type;
                    return tbb::parallel_reduce(
                        range_type(pop.begin(), pop.end()), std::pair<double, double>(0, 0),
                        [](const range_type& r,
                            std::pair<double, double> value) -> std::pair<double, double> {
                            for (auto it = r.begin(); it != r.end(); ++it) {
                                value.first = std::min(value.first, ValueSelector::getValue(*it));
                                value.second += ValueSelector::getValue(*it);
                            }
                            return value;
                        },
                        [](std::pair<double, double> x,
                            std::pair<double, double> y) -> std::pair<double, double> {
                            return {std::min(x.first, y.first), x.second + y.second};
                        }

                    );
                }
            };

        } // namespace selector
    } // namespace qd
} // namespace sferes

#endif
