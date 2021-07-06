#ifndef FRAMEWORK_EVENT_EVENTFILE_H_
#define FRAMEWORK_EVENT_EVENTFILE_H_

//---< C++ >---//
#include <map>
#include <string>
#include <vector>

//---< Framework >---//
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"

//---< ROOT >---//
#include "TFile.h"
#include "TTree.h"

namespace ldmx {
class RunHeader;
}

namespace framework {

/**
 * This class manages all ROOT file input/output operations.
 */
class EventFile {
 public:
  /**
   * Constructor to make a general file.
   *
   * This is not used directly, but it is called in the more
   * specialised constructors. This method is mainly focused on
   * reducing code copying.
   *
   * @param[in] params The parameters used to configure this EventFile.
   * @param[in] filename the name of the file to read/write
   * @param[in] parent a pointer to the parent file to copy
   * @param[in] isOutputFile true if this file is written out
   * @param[in] isSingleOutput true if only one output file is being written to
   * @param[in] isLoopable true for an input file where events can be reused
   */
  EventFile(const framework::config::Parameters &params,
            const std::string &filename, EventFile *parent, bool isOutputFile,
            bool isSingleOutput, bool isLoopable);

  /**
   * Constructor to make a pileup overlay file.
   *
   * This is an additional input file from which collections are pulled
   * to be overlaid with simulated hit collections.
   *
   * @param[in] params The parameters used to configure this EventFile.
   * @param fileName The file name.
   * @param isLoopable true if we want to reset the event counter and keep
   * processing from the start when we hit the end of the event tree in
   * this input file (set in the call in the producer)
   */
  EventFile(const framework::config::Parameters &param,
            const std::string &fileName, bool isLoopable);

  /**
   * Class constructor for cloning data from a "parent" file.
   *
   * This is used for output files when there is an input file.
   * (OR for files with a parent EventFile to clone)
   *
   * @param[in] params The parameters used to configure this EventFile.
   * @param[in] fileName The file name.
   * @param[in] parent Parent file for cloning data tree.
   * @param[in] isSingleOutput boolean check if only one output file is being
   * written to
   */
  EventFile(const framework::config::Parameters &param,
            const std::string &fileName, EventFile *parent,
            bool isSingleOutput = false);

  /**
   * Class constructor to make a file to read in an event root file.
   *
   * This constructor just wraps the constructor above and sets the parent to
   * null and the parameters isOutputFile/isSingleOutput to false.
   *
   * This is used for all input files.
   *
   * @param[in] params The parameters used to configure this EventFile.
   * @param[in] fileName The file name.
   */
  EventFile(const framework::config::Parameters &params,
            const std::string &fileName);

  /// Defult destructor
  ~EventFile() = default;

  /**
   * Add a rule for dropping collections from the output.
   *
   * This needs to be called *after* setupEvent.
   * This method uses the event to help drop collections.
   *
   * The rules should be of the following form:
   *      <drop,keep, or ignore> exp
   *      where exp is an expression that matches the collectionName.
   *      ignore ==> any branch with name matching exp is not even read in from
   * the input file (if it exists) keep   ==> any branch with name matching exp
   * is read in from the input (if exists) and written to output (if exists)
   *      drop   ==> any branch with name matching exp is read in from the input
   * (if exists) and NOT written to output
   *
   * The default behavior for all branches is keep.
   *
   * ROOT uses the internal TRegexp to match branch names to the passed
   * expression and set the status of the branch (whether it will be read or
   * not). This internal object has different rules than real regular
   * expressions, so it is best to keep it safe and only use asterisks or full
   * names. Additionally, the rules you pass are analyzed in succession, so you
   * can go from something more general to something more specific.
   *
   * For example to drop all EcalSimHits.*
   *      drop EcalSimHits.*
   *
   * or drop scoring plane collections
   *      drop .*ScoringPlane.*
   *
   * or drop scoring plane collections but keep EcalScoringPlane collection
   *      drop .*ScoringPlane.*
   *      keep EcalScoringPlane.*
   *
   * @note The Event::getImpl overrides any ignore rules for the input file in
   * order to avoid any seg faults. The items accessed will still be dropped.
   *
   * @param rule The rule for dropping collections.
   */
  void addDrop(const std::string &rule);

  /**
   * Set an Event object containing the event data to work with this file.
   * @param evt The Event object with event data.
   */
  void setupEvent(Event *evt);

  /**
   * Change pointer to different parent file.
   *
   * We assume that the parent file is an EventFile
   * configured to be an input file. This allows us
   * to assume that the tree_ member variable of parent
   * is valid.
   *
   * @param parent pointer to new parent file
   */
  void updateParent(EventFile *parent);

  /**
   * Get the Event object containing the event data.
   * @return The Event object containing event data.
   */
  Event *getEvent() { return event_; };

  /**
   * Prepare the next event.
   *
   * We do the necessary set-up if ientry_ < 0.
   *  This means if we are an output file with a parent file,
   *  we clone our parent tree to our tree so we have the 
   *  same branches.
   *
   * After the first entry (when ientry_ >= 0), we "close up"
   * the last event, filling our tree if we storeCurrentEvent
   * is true and telling the event bus to clear.
   *
   * Going to the next event depends on the configuration of the
   * event file. THere are three cases.
   *
   * 1. We have a parent file - Just follow the parent's lead
   *    by calling the parent's nextEvent.
   * 2. We are an output file (and we don't have a parent file).
   *    Just increment the number of entries and the entry index.
   * 3. We are an input file. Now we need to load the next entry
   *    of the our tree into the event bus objects.
   *    If we are configured to be "loopable", then we will 
   *    reset ientry_ to -1 when we reach the last event.
   *    Otherwise, we will return false as a stopping condition.
   *
   * @param[in] storeCurrentEvent Should we save the current event 
   *  to the output (if we are an output file)?
   * @return If event was prepared/read successfully.
   */
  bool nextEvent(bool storeCurrentEvent = true);

  /**
   * Skip events using an offset. Used in pileup overlay.
   * @return New event number if read successfully, else -1.
   */
  int skipToEvent(int offset);

  /**
   * Close the file, writing the tree to disk if creating an output file.
   *
   * Deletes any RunHeaders that this instance of EventFile owns.
   *
   * @throw Exception if run tree already exists in output file.
   */
  void close();

  /**
   * Write the run header into the run map
   *
   * Any RunHeader passed here is not owned or cleaned up
   * by this EventFile instance.
   *
   * @param runHeader The run header to write into the map
   * @throw Exception if run number is already in run map
   */
  void writeRunHeader(ldmx::RunHeader &runHeader);

  /**
   * Get the RunHeader for a given run, if it exists in the input file.
   * @param runNumber The run number.
   * @return The RunHeader from the input file.
   * @throw Exception if there is no RunHeader in the map with the given run
   * number.
   */
  ldmx::RunHeader &getRunHeader(int runNumber);

  /// @return the name of the ROOT file being managed.
  const std::string &getFileName() { return fileName_; }

private:
  /**
   * Fill the internal map of run numbers to RunHeader objects from the input
   * file.
   *
   * If this file is an output file and parent_ and parent_->file_ are valid
   * pointers, then the run headers are imported from parent_->file_.
   *
   * Otherwise, we try to import run headers from file_.
   *
   * Does not check if any run headers are getting overwritten!
   *
   * Any RunHeaders read in from this function are owned by this instance
   * of EventFile and are deleted in close().
   *
   * @note This function does nothing if parent_->file_ and file_ are nullptrs.
   */
  void importRunHeaders();

private:
  /// The number of entries in the tree.
  Long64_t entries_{-1};

  /// The current entry in the tree.
  Long64_t ientry_{-1};

  /// The file name.
  std::string fileName_;

  /// True if file is an output file being written to disk.
  bool isOutputFile_;

  /// True if there is only one output file
  bool isSingleOutput_;

  /// True if this is an input file with pileup overlay events */
  bool isLoopable_{false};

  /// The backing TFile for this EventFile.
  TFile *file_{nullptr};

  /// The tree with event data.
  TTree *tree_{nullptr};

  /// A parent file containing event data.
  EventFile *parent_{nullptr};

  /// The object containing the actual event data (trees and branches).
  Event *event_{nullptr};

  /**
   * Pre-clone rules.
   *
   * The series of rules to call before cloning/copying the
   * parent tree.
   */
  std::vector<std::pair<std::string, bool>> preCloneRules_;

  /**
   * Vector of drop rules that have been parsed and
   * need to be used to reactivate these branches on the input tree
   *
   * The branches were initial deactivated so they don't get cloned
   * to output tree.
   */
  std::vector<std::string> reactivateRules_;

  /**
   * Map of run numbers to RunHeader objects
   *
   * The value object is a pair that should remain internal.
   *  1. True if EventFile owns the RunHeader (and needs to clean it up)
   *     - This happens when RunHeaders are imported from an input file
   *  2. False if EventFile does not own the RunHeader
   *     - This happens when the RunHeader is created by Process::run during
   * production
   */
  std::map<int, std::pair<bool, ldmx::RunHeader *>> runMap_;
};
} // namespace framework

#endif // FRAMEWORK_EVENT_EVENTFILE_H
