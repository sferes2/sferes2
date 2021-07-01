#ifndef SFERES2_RESUME_STATE_QD_HPP
#define SFERES2_RESUME_STATE_QD_HPP

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/nvp.hpp>
#include <sferes/stat/stat.hpp>

namespace sferes {
    namespace stat {
        // a statistics class that saves the parents and the offspring of QD containers

        SFERES_STAT(StateQD, Stat) {
        public:
          template<typename EA>
          void refresh(const EA& ea) {
            m_parents = ea.parents();
            m_offspring = ea.offspring();
          }

          const std::vector<boost::shared_ptr<Phen> >& parents() const {
            return m_parents;
          }

          const std::vector<boost::shared_ptr<Phen> >& offspring() const {
            return m_offspring;
          }

          template<class Archive>
          void serialize(Archive & ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(m_parents);
            ar & BOOST_SERIALIZATION_NVP(m_offspring);
          }

        protected:
          std::vector<boost::shared_ptr<Phen> > m_parents, m_offspring;
    };
}
}

#endif //SFERES2_RESUME_STATE_QD_HPP
