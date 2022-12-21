#ifndef WHITE_RABBIT_DECODER_H
#define WHITE_RABBIT_DECODER_H

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "BeamInstrumentation/Event/WhiteRabbitResult.h"

namespace beaminstrumentation {

  
	class WhiteRabbitDecoder : public framework::Producer{
	public:
		WhiteRabbitDecoder(const std::string &name, framework::Process &process) : Producer(name, process) {};

		~WhiteRabbitDecoder() = default;

		virtual void configure(framework::config::Parameters &ps);

		virtual void produce(framework::Event &event);

		virtual void onFileOpen();

		virtual void onFileClose();

		virtual void onProcessStart();

		virtual void onProcessEnd();
	
	private:
		std::string inputCollection_;
		std::string outputCollection_;
		std::string inputPassName_;
		
	};
  

} // namespace beaminstrumentation

#endif // WHITE_RABBIT_DECODER_H
