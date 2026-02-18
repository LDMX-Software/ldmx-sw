// DataAggregator.hh
#ifndef DATAAGGREGATOR_H
#define DATAAGGREGATOR_H
#include "EventMerger.hh"
#include "PhysicsEventData.hh"

namespace eventbuilder {

class DataAggregator {
public:
    DataAggregator(EventMerger& merger_ref) : m_merger(merger_ref) {}

    // Accept an rvalue reference to allow std::move to bind
    void aggregate(PhysicsEventData&& event) {
        m_merger.merge_event(std::move(event));
    }
private:
    EventMerger& m_merger;
};

} // namespace eventbuilder

#endif
