#include "Framework/catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "Conditions/SimpleTableCondition.h"
#include "Conditions/SimpleTableStreamers.h"
#include "DetDescr/EcalID.h"

namespace ldmx {
  namespace test {

    using Catch::Matchers::Contains;
    
    /**
     * Test for various items related to conditions
     *
     * What does this test?
     *  - Correct creation, filling, and access to various implementations of conditions tables
     */

    TEST_CASE( "Conditions", "[Framework][Conditions]") {

      using namespace ldmx;

      // create a simple table
      
      std::vector<std::string> columns({"A","Q","V"});
      
      IntegerTableCondition itable("ITable",columns);
      
      for (int key=100; key>0; key-=10) {
	EcalID id(1,1,key);
	std::vector<int> vals;
	vals.push_back(key*2);
	vals.push_back(key/2);
	vals.push_back(key*key);
	itable.add(id.raw(),vals);
      }

      SECTION("Testing simple table construction") {

	
	REQUIRE(itable.getRowCount() == 10);

	REQUIRE(itable.getColumnCount() == 3);

	EcalID id(1,1,20);
	REQUIRE(itable.get(id.raw(),1) == 10);
	
	REQUIRE_THROWS_WITH( itable.add(2,std::vector<int>(2)), Contains( "columns into a table" ) );

	REQUIRE_THROWS_WITH( itable.add(id.raw(),std::vector<int>(3)), Contains( "existing id" ) );

	std::pair<unsigned int, std::vector<int> > row=itable.getRow(4);

	CHECK( row.first == 0x14021032 );

	CHECK( row.second.size()==3 );

	CHECK( row.second.at(2) == (50*50) );

	EcalID id2(1,1,60);
  
	CHECK( itable.getByName(id2.raw(),"Q") == 30 );	
       	
      }
      SECTION("Testing CSV IO") {
	ldmx::utility::SimpleTableStreamerCSV::store(itable, std::cout, true);
      }
    }
  }
}
