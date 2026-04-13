// EventSummary.hh - DEPRECATED
// EventSummary has been moved to Framework/include/Framework/EventSummary.h
// in the ldmx namespace for proper ROOT serialization
// This file is kept for backwards compatibility during transition

#ifndef EVENT_SUMMARY_H
#define EVENT_SUMMARY_H

#include "Framework/EventSummary.h"

// For backwards compatibility, create aliases in eventbuilder namespace
namespace eventbuilder {
  using EventSummary = ldmx::EventSummary;
  using ERROR_FLAGS = ldmx::EventSummary::ErrorFlags;
}

#endif // EVENT_SUMMARY_H
