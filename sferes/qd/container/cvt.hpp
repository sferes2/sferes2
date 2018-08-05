#ifndef QD_CONTAINER_CVT_HPP
#define QD_CONTAINER_CVT_HPP

namespace sferes
{
namespace container
{

template <typename Phen, typename Params>
class CVT
{
  public:
    static constexpr size_t feature_dimensionality =
        Params::ea::behav_dim;

    typedef boost::shared_ptr<Phen> indiv_t;
    typedef typename std::vector<indiv_t> pop_t;
    typedef boost::array<double, feature_dimensionality> point_t;

    static constexpr size_t number_of_clusters = Params::ea::number_of_clusters;
    std::vector<point_t> centroids;

    CVT()
    {
        _archive.resize(number_of_clusters);
        _archive_parents.resize(number_of_clusters);
        centroids = Params::ea::centroids;
    }

    void get_full_content(std::vector<indiv_t> &content) const
    {
        for (size_t i = 0; i < _archive.size(); ++i)
            if (_archive[i])
                content.push_back(_archive[i]);
    }

    bool add(indiv_t i1, indiv_t parent)
    {
        if (i1->fit().dead())
            return false;

        point_t p = _get_point(i1);

        // Find the closest cluster
        size_t archive_index = -1;
        double min_dist = std::numeric_limits<double>::max();

        for (size_t i = 0; i < centroids.size(); ++i)
        {
            double dist = _calc_dist(centroids[i], p);

            if (dist < min_dist)
            {
                min_dist = dist;
                archive_index = i;
            }

            // Since the minimum distance cannot be less than 0
            // we could accelerate computation by breaking
            if (min_dist == 0.0)
                break;
        }

        // If the archive is empty or the stored individual is less fit
        if (!_archive[archive_index] ||
            i1->fit().value() > _archive[archive_index]->fit().value())
        {
            _archive[archive_index] = i1;
            _archive_parents[archive_index] = parent;
            return true;
        }

        return false;
    }

    void update(pop_t &offspring, pop_t &parents)
    {
        // Do nothing for now
        // _update_novelty();
        // for (size_t i = 0; i < offspring.size(); i++)
        //     _update_indiv(offspring[i], *this);
        // for (size_t i = 0; i < parents.size(); i++)
        //     _update_indiv(parents[i], *this);
    }

    const pop_t &archive() const { return _archive; }
    const pop_t &parents() const { return _archive_parents; }

  protected:
    pop_t _archive;
    pop_t _archive_parents;

    // Euclidean distance
    double _calc_dist(const point_t &p1, const point_t &p2)
    {
        double dist = 0.0;

        for (size_t i = 0; i < feature_dimensionality; ++i)
            dist += pow(p1[i] - p2[i], 2);

        return sqrt(dist);
    }

    // Converts the descriptor into a Point_t
    template <typename I>
    point_t _get_point(const I &indiv) const
    {
        point_t p;
        for (size_t i = 0; i < feature_dimensionality; ++i)
            p[i] = indiv->fit().desc()[i];

        return p;
    }

    // void _update_novelty()
    // {
    //     tbb::parallel_for(tbb::blocked_range<indiv_t *>(_array.data(), _array.data() + _array.num_elements()),
    //                       Par_novelty<Grid<Phen, Params>>(*this));
    // }
};
} // namespace container
} // namespace sferes

#endif
