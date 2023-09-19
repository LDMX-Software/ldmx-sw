#pragma once

#include <Acts/Geometry/TrackingGeometry.hpp>

#include "Framework/ConditionsObject.h"

namespace tracking::geo {

/// class name of provider
class GeometryContextProvider;

/**
 * The context for a specific geometry
 *
 * This connects the Acts::GeometryContext object to
 * our conditions system so that it can be loaded from
 * it. The GeometryContext is what holds things like
 * alignment constants within ACTS. Currently, we don't
 * have any alignment constants so this just holds a
 * default-constructed object for the entire run
 * without doing anything else.
 *
 * The context is both the object used as a condition
 * and the provider of that condition. We could separate
 * the two if a new context needs to be constructed when
 * the conditions are updated but for now keeping them
 * together reduces the amount of code needed.
 */
class GeometryContext : public framework::ConditionsObject {
 public:
  /// Conditions object name
  static const std::string NAME;
  /**
   * get a reference to the actual geometry context ACTS object
   *
   * @note For future developers, the conditions object should handle
   * the validity of the geometry context. In other words, if someone
   * has a valid handle to a GeometryContext object, they should be able to
   * call this function without worry.
   */
  const Acts::GeometryContext& get() const;
 private:
  /// the provider is a friend and so it can make one
  friend class GeometryContextProvider;
  /**
   * This constructor is where parameters would be passed into the
   * Geometry context from the provider. Right now, there aren't
   * any parameters to pass and so it is just default constructed.
   * We still have it be private so that only the provider can
   * make a new one.
   */
  GeometryContext();

  /**
   * the actual geometry context we are wrapping
   *
   * For now, this is just a regular object but if we want to actually
   * modify the geometry context by re-constructing it in the future,
   * it may need to evolve into some smart pointer so that it can be
   * reconstructed at need OR we separate the conditions object which
   * would be recreated whenever this object needs to be recreated.
   */
  Acts::GeometryContext geometry_context_;
};

}
