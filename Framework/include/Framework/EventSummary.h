/**
 * @file EventSummary.h
 * @brief Class that provides a summary of event assembly with metadata and error flags
 */

#ifndef EVENT_EVENTSUMMARY_H_
#define EVENT_EVENTSUMMARY_H_

// ROOT
#include "TObject.h"  // For ClassDef
#include <vector>
#include <cstdint>

namespace ldmx {

/**
 * @class EventSummary
 * @brief Provides summary information about an assembled event, including
 * error flags for data quality assessment
 *
 * This object tracks event assembly metadata such as which subsystems
 * participated, total payload size, and any errors encountered during
 * the assembly process.
 */
class EventSummary : public TObject {
 public:
  /**
   * Error flag bit definitions
   */
  enum ErrorFlags : uint32_t {
    ERROR_CRC_MISMATCH = (1 << 0),        ///< CRC/checksum validation failed
    ERROR_PARSE_FAILURE = (1 << 1),       ///< Frame parsing error
    ERROR_INCOHERENT_TIMING = (1 << 2),   ///< Fragment timestamps outside coherence window
    ERROR_MISSING_SUBSYSTEM = (1 << 3),   ///< Expected subsystem data missing
    ERROR_INCOMPLETE_EVENT = (1 << 4),    ///< Event lacks expected number of fragments
    ERROR_TRUNCATED_EVENT = (1 << 5),     ///< Event was truncated (EOF reached)
    ERROR_DUPLICATE_SUBSYSTEM = (1 << 6)  ///< Duplicate subsystem in single event
  };

  /**
   * Name of EventSummary branch
   */
  static const std::string BRANCH;

  /**
   * Class constructor.
   */
  EventSummary() = default;

  /**
   * Class destructor.
   */
  virtual ~EventSummary() = default;

  /**
   * Clear information from this object.
   *
   * @param[in] o ROOT-style Option (ignored)
   */
  void clear(Option_t* o = "");

  /**
   * Print this object.
   * @param[in] o ROOT-style Option (ignored)
   */
  void print(Option_t* o = "") const;

  /**
   * Get the event ID.
   * @return The event ID
   */
  uint64_t getEventId() const { return event_id_; }

  /**
   * Set the event ID.
   * @param[in] id The event ID
   */
  void setEventId(uint64_t id) { event_id_ = id; }

  /**
   * Get the timestamp in nanoseconds.
   * @return The timestamp
   */
  uint64_t getTimestampNs() const { return timestamp_ns_; }

  /**
   * Set the timestamp in nanoseconds.
   * @param[in] ts The timestamp
   */
  void setTimestampNs(uint64_t ts) { timestamp_ns_ = ts; }

  /**
   * Get the number of systems in this event.
   * @return Number of subsystems
   */
  uint32_t getNSystems() const { return nsystems_; }

  /**
   * Set the number of systems in this event.
   * @param[in] ns Number of subsystems
   */
  void setNSystems(uint32_t ns) { nsystems_ = ns; }

  /**
   * Get the system IDs that participated in this event.
   * @return Vector of system IDs
   */
  const std::vector<uint64_t>& getSystemIds() const { return system_ids_; }

  /**
   * Set the system IDs.
   * @param[in] ids Vector of system IDs
   */
  void setSystemIds(const std::vector<uint64_t>& ids) { system_ids_ = ids; }

  /**
   * Get the total payload size in bytes.
   * @return Payload size
   */
  uint64_t getPayloadSize() const { return payload_size_; }

  /**
   * Set the total payload size.
   * @param[in] size Payload size in bytes
   */
  void setPayloadSize(uint64_t size) { payload_size_ = size; }

  /**
   * Get the error flags for this event.
   * @return Error flags bitmask
   */
  uint32_t getErrorFlags() const { return error_flags_; }

  /**
   * Set the error flags.
   * @param[in] flags Error flags bitmask
   */
  void setErrorFlags(uint32_t flags) { error_flags_ = flags; }

  /**
   * Check if a specific error flag is set.
   * @param[in] flag The error flag to check
   * @return true if the flag is set, false otherwise
   */
  bool hasError(ErrorFlags flag) const { return (error_flags_ & flag) != 0; }

  /**
   * Set a specific error flag.
   * @param[in] flag The error flag to set
   */
  void setError(ErrorFlags flag) { error_flags_ |= flag; }

 private:
  /**
   * The event ID
   */
  uint64_t event_id_ = 0;

  /**
   * The event timestamp in nanoseconds
   */
  uint64_t timestamp_ns_ = 0;

  /**
   * Number of subsystems that contributed to this event
   */
  uint32_t nsystems_ = 0;

  /**
   * List of subsystem IDs that participated in this event
   */
  std::vector<uint64_t> system_ids_;

  /**
   * Total payload size in bytes
   */
  uint64_t payload_size_ = 0;

  /**
   * Error flags bitmask
   */
  uint32_t error_flags_ = 0;

  /**
   * ROOT class definition.
   */
  ClassDef(EventSummary, 1);
};

}  // namespace ldmx

#endif /* EVENT_EVENTSUMMARY_H_ */
