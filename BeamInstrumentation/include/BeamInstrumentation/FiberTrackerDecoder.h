#ifndef FIBER_TRACKER_DECODER_H
#define FIBER_TRACKER_DECODER_H

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "BeamInstrumentation/Event/FiberTracker.h"

namespace beaminstrumentation {

	class FiberTrackerDecoder : public framework::Producer{
	public:
		FiberTrackerDecoder(const std::string &name, framework::Process &process) : Producer(name, process) {};

		~FiberTrackerDecoder() = default;

		virtual void configure(framework::config::Parameters &ps);

		virtual void produce(framework::Event &event);

		virtual void onFileOpen();

		virtual void onFileClose();

		virtual void onProcessStart();

		virtual void onProcessEnd();
	
	private:
		std::string inputCollectionDownstreamHorizontal_; //FT50
		std::string inputCollectionDownstreamVertical_; //FT51
		std::string inputCollectionUpstreamHorizontal_; //FT41
		std::string inputCollectionUpstreamVertical_; //FT42
		std::string outputCollection_;
		std::string inputPassName_;
		
	};

} // namespace beaminstrumentation

#endif // FIBER_TRACKER_DECODER_H
