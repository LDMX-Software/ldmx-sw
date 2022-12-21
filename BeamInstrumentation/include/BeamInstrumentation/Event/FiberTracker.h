#ifndef FIBER_TRACKER_H
#define FIBER_TRACKER_H

#include <vector>
#include <stdint.h>
#include <string>

#include "TObject.h"

namespace beaminstrumentation {
	class FiberTracker {
	public:
		FiberTracker() = default;
    FiberTracker(std::vector<uint> hitsDownstreamHorizontal, std::vector<uint> hitsDownstreamVertical, std::vector<uint> hitsUpstreamHorizontal, std::vector<uint> hitsUpstreamVertical): 
      hitsDownstreamHorizontal(hitsDownstreamHorizontal), hitsDownstreamVertical(hitsDownstreamVertical), hitsUpstreamHorizontal(hitsUpstreamHorizontal), hitsUpstreamVertical(hitsUpstreamVertical) {} 
		~FiberTracker() = default;

		void Print(Option_t* option = "") const {};
		void Clear(Option_t* option = "") {};
		bool operator <(const FiberTracker &rhs) const {
			return true;
		}

	private:
    std::vector<uint> hitsDownstreamHorizontal; //FT50
    std::vector<uint> hitsDownstreamVertical; //FT51 
    std::vector<uint> hitsUpstreamHorizontal; //FT41
    std::vector<uint> hitsUpstreamVertical; //FT42

		ClassDef(FiberTracker, 1);
	};
} // namespace beaminstrumentation

#endif //FIBER_TRACKER_H
