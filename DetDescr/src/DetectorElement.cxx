#include "DetDescr/DetectorElement.h"

#include <queue>

using namespace std;

namespace ldmx {

    void DetectorElementVisitor::walk(DetectorElement* de, DetectorElementVisitor* visitor) {
        queue<DetectorElement*> q;
        q.push(de);
        while (!q.empty()) {
            DetectorElement* cur = q.front();
            visitor->visit(cur);
            for (auto child : cur->getChildren()) {
                q.push(child);
            }
            q.pop();
        }
    }

}
