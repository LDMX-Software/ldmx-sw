#include "Event/EventImpl.h"
#include "Event/EventFile.h"
#include "Event/SimParticle.h"
#include "TFile.h"
#include "TLine.h"

#include <iostream>

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Missing command line argument." << std::endl;
        return 1;
    }

    if (!strcmp(argv[1], "-c")) {

        event::EventFile f("test.root", true);
        event::EventImpl evt("test1");
        f.setupEvent(&evt);

        TLine* tl = new TLine(10, 10, 20, 20);
        TClonesArray tca("event::SimParticle", 50);

        for (int i = 2; i < 10; i++) {
            tl->SetX1(i);
            evt.add("Straight", tl);

            tca.Clear();
            for (int j = 0; j < i; j++) {
                event::SimParticle* p = (event::SimParticle*) (tca.ConstructedAt(j));
                p->setEnergy(i + j);
                p->setTime(i);
            }

            evt.add("Web", &tca);

            f.nextEvent();
        }

        f.close();
    }

    if (!strcmp(argv[1], "-c2")) {
        event::EventFile f("test.root");
        event::EventFile f2("test2.root", &f);

        event::EventImpl evt("test2");
        f2.setupEvent(&evt);

        int j = 70;
        while (f2.nextEvent()) {
            TLine* tl = new TLine(j++, 10, 20, 20);
            evt.add("Straight", tl);
        }

        f2.close();
    }

    if (!strcmp(argv[1], "-r")) {
        event::EventFile f("test.root");
        event::EventImpl evt("test2");
        f.setupEvent(&evt);

        while (f.nextEvent()) {
            const TObject* to = evt.get("Straight", "test1");
            to->Print();
            const TObject* tbs = evt.get("Web", "test1");
            tbs->Print();
        }

        f.close();
    }

    if (!strcmp(argv[1], "-x")) {
        event::EventFile f("test.root");
        event::EventFile f2("test2.root", &f);

        event::EventImpl evt("test2");
        f2.setupEvent(&evt);

        TClonesArray tca("event::SimParticle", 50);

        while (f2.nextEvent()) {
            const TObject* to = evt.get("Straight", "test1");
            if (to == 0)
                printf("Null object!\n");
            else {
                to->Print();
                const TLine* tl = (const TLine*) to;

                TLine tlx(tl->GetY1(), tl->GetY2(), tl->GetX1(), tl->GetX2());
                evt.add("Swapped", &tlx);

                tca.Clear();
                for (int j = 0; j < 20; j++) {
                    event::SimParticle* p = (event::SimParticle*) (tca.ConstructedAt(j));
                    p->setEnergy(j);
                }

                evt.add("Targets", &tca);
            }
        }

        f2.close();
        f.close();
    }

    if (!strcmp(argv[1], "-d")) { // test dropping something while still using it
        event::EventFile f("test.root");
        event::EventFile f2("test_drop.root", &f);

        event::EventImpl evt("test2");
        f2.setupEvent(&evt);
        f2.addDrop("drop *_test1");

        while (f2.nextEvent()) {
            const TObject* to = evt.get("Straight", "test1");
            if (to == 0)
                printf("Null object!\n");
            else {
                to->Print();
                const TLine* tl = (const TLine*) to;

                TLine tlx(tl->GetY1(), tl->GetY2(), tl->GetX1(), tl->GetX2());
                evt.add("Swapped", &tlx);
            }
        }

        f2.close();
        f.close();
    }

    return 0;

}
