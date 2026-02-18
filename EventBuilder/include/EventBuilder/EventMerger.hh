// EventMerger.hh
#ifndef EVENTMERGER_H
#define EVENTMERGER_H
#include <map>
#include <mutex>
#include <iostream>
#include <algorithm>
#include "PhysicsEventData.hh"

namespace eventbuilder {

class EventMerger {
public:
    // Function to receive and merge partial events
    void merge_event(PhysicsEventData&& partial_event) {
        std::lock_guard<std::mutex> lock(m_mutex);
        unsigned int id = partial_event.event_id;

        if (m_incomplete_events.find(id) == m_incomplete_events.end()) {
            m_incomplete_events[id] = std::move(partial_event);
            std::cout << "[Merger] Stored first part of Event ID " << id << std::endl;
        } else {
            PhysicsEventData& existing_event = m_incomplete_events[id];

            existing_event.systems_readout.insert(
                existing_event.systems_readout.end(),
                partial_event.systems_readout.begin(),
                partial_event.systems_readout.end()
            );

            std::cout << "[Merger] Merged new data into Event ID " << id << ". Total subsystems read: " << existing_event.systems_readout.size() << std::endl;
        }
    }

    // A simple function to report final merged event sizes for demonstration
    void print_merged_status() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& pair : m_incomplete_events) {
            std::cout << "[Merger Status] Event ID " << pair.first << " is finalized with data from "
                      << pair.second.systems_readout.size() << " subsystem fragments." << std::endl;
        }
    }

private:
    std::map<unsigned int, PhysicsEventData> m_incomplete_events;
    std::mutex m_mutex;
};

} // namespace eventbuilder

#endif
