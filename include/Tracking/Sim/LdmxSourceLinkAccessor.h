#ifndef LDMXSOURCELINKACCESSOR_H_
#define LDMXSOURCELINKACCESSOR_H_

#include "Acts/Geometry/GeometryIdentifier.hpp"
#include <unordered_map>
#include <vector>

namespace tracking {
namespace sim {
  
//T can be whatever source link implementation
    
template <typename T>
using GeometrySourceLinkMap = std::unordered_map<Acts::GeometryIdentifier, std::vector<T> >;
    
template <typename T>
struct LdmxSourceLinkAccessor {
          
  using Container = GeometrySourceLinkMap<T>;
  using Key       = Acts::GeometryIdentifier;
  //using Value     = typename GeometrySourceLinkMap<T>::value_type;
  using Value     = T;
  //using Iterator  = typename GeometrySourceLinkMap<T>::const_iterator;
  using Iterator  = typename std::vector<T>::const_iterator;
          
  const Container* container = nullptr;
          
  size_t count(const Acts::GeometryIdentifier& geoId) const {
    assert(container != nullptr);
    //return container->count(geoId);
    return container->at(geoId).size();
  }
  
  std::pair<Iterator, Iterator> range (
      const Acts::GeometryIdentifier& geoId) const {
    assert(container != nullptr);
    return std::pair<Iterator,Iterator>(container->at(geoId).begin(), container->at(geoId).end());
  }

  //const Value& at(const Iterator& it) const {return *it;}
  const T& at(const Iterator& it) const {return *it;}
          
};


/// The map(-like) container accessor
//See: https://github.com/acts-project/acts/blob/v15.0.0/Tests/UnitTests/Core/TrackFinding/CombinatorialKalmanFilterTests.cpp#L109-L114
template <typename container_t>
struct GeneralContainerAccessor {
  using Container = container_t;
  using Key = typename container_t::key_type;
  using Value = typename container_t::mapped_type;
  using Iterator = typename container_t::const_iterator;
  
  // pointer to the container
  const Container* container = nullptr;
  
  // count the number of elements with requested key
  size_t count(const Key& key) const {
    assert(container != nullptr);
    return container->count(key);
  }
  
  // get the range of elements with requested key
  std::pair<Iterator, Iterator> range(const Key& key) const {
    assert(container != nullptr);
    return container->equal_range(key);
  }
  
  // get the element using the iterator
  const Value& at(const Iterator& it) const { return (*it).second; }
};

}
}

#endif 
