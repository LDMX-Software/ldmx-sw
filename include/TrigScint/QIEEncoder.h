#ifndef TRIGSCINT_QIEENCODER_H
#define TRIGSCINT_QIEENCODER_H

#include <iostream>
#include <fstream>

                                             
#include "Framework/Configure/Parameters.h"  // Needed to import parameters from configuration file
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"  //Needed to declare processor
#include "TrigScint/Event/TrigScintQIEDigis.h"
#include "TrigScint/Event/QIEStream.h"

namespace trigscint {


  class QIEEncoder : public framework::Producer{
 public:
  QIEEncoder(const std::string &name, framework::Process &process)
      : Producer(name, process) {}

  /**
   * Default destructor, closes up boost archive and input stream
   */
	~QIEEncoder() = default;

	/**
	 * Configure our converter based off the configuration parameters
	 * decoded from the passed python script
	 */
	virtual void configure(framework::config::Parameters &ps);

  virtual void produce(framework::Event &event);

  virtual void onFileOpen();

  virtual void onFileClose();

  virtual void onProcessStart();

  virtual void onProcessEnd();

	
	
  private:
	// electronics/detector ID channel map
	std::string channelMapFileName_;
	std::ifstream  channelMapFile_;
	std::map< int, int> channelMap_;

	//input collection name and pass name
	std::string inputCollection_;
	std::string inputPassName_;
	//and output
	std::string outputCollection_;

	// verbosity for very specific printouts that don't play well with logger format
	bool verbose_{false};

	// number of channels in the pad 
	int nChannels_{50};
	
  }; //encoder

}//namespace trigscint 
                                                                                                 

#endif // TRIGSCINT_QIEENCODER_H
