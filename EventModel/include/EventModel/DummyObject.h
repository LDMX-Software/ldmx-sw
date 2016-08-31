#ifndef EVENTMODEL_DUMMYOBJECT_H_
#define EVENTMODEL_DUMMYOBJECT_H_ 1

// dummy object class for testing ROOT IO 
class DummyObject {

    public:
        DummyObject() {}

        DummyObject(int field1, double field2) {
            _field1 = field1;
            _field2 = field2;
        }            
    
    private:
        int _field1;
        double _field2;
};

#endif
