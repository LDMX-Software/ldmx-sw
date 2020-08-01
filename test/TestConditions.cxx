#include "Framework/catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "Conditions/SimpleTableCondition.h"
#include "Conditions/SimpleTableStreamers.h"
#include "DetDescr/EcalID.h"
#include <sstream>

namespace ldmx {
  namespace test {


    void matchesAll(const ldmx::IntegerTableCondition&a, const ldmx::IntegerTableCondition& b) {
      REQUIRE( a.getColumnCount() == b.getColumnCount() );
      REQUIRE( a.getRowCount() == b.getRowCount() );
      REQUIRE( a.getColumnNames() == b.getColumnNames() );
      for (unsigned int i=0; i<a.getRowCount(); i++)
	REQUIRE( a.getRow(i) == b.getRow(i) );
    }
    
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
	std::stringstream ss;
	ldmx::utility::SimpleTableStreamerCSV::store(itable, ss, true);
	
	std::string image1=ss.str();
	const char* expected1="\"DetID\",\"subdetector\",\"layer\",\"module\",\"cell\",\"A\",\"Q\",\"V\"\n0x1402100a,5,1,1,10,20,5,100\n0x14021014,5,1,1,20,40,10,400\n0x1402101e,5,1,1,30,60,15,900\n0x14021028,5,1,1,40,80,20,1600\n0x14021032,5,1,1,50,100,25,2500\n0x1402103c,5,1,1,60,120,30,3600\n0x14021046,5,1,1,70,140,35,4900\n0x14021050,5,1,1,80,160,40,6400\n0x1402105a,5,1,1,90,180,45,8100\n0x14021064,5,1,1,100,200,50,10000\n";

	CHECK( image1 == expected1 );
	
	std::cout << image1;

	std::stringstream ss_read1(image1);
	IntegerTableCondition itable2("ITable",columns);
	ldmx::utility::SimpleTableStreamerCSV::load(itable2,ss_read1);

	matchesAll(itable,itable2);

	// missing id column example
	std::string image2("B,A,Q,V\n0,40,50,100\n0,60,70,90\n");
	std::stringstream ss_read2(image2);
	REQUIRE_THROWS_WITH( ldmx::utility::SimpleTableStreamerCSV::load(itable2, ss_read2) , Contains("Malformed CSV file with no DetId or subdetector column") );


        // missing column example
	std::string image3("DetID,A,Q\n0x1402100a,20,5\n0x14021014,40,10\n0x1402101e,60,15\n0x14021028,80,20\n0x14021032,100,25\n0x1402103c,120,30\n0x14021046,140,35\n0x14021050,160,40\n0x1402105a,180,45\n0x14021064,200,50\n");
	std::stringstream ss_read3(image3);
	REQUIRE_THROWS_WITH( ldmx::utility::SimpleTableStreamerCSV::load(itable2, ss_read3) , Contains("Missing column") );

	// varying line lengths example
	std::string image4("DetID,A,Q,V\n0x1402100a,20,5,100\n0x14021014,40,10\n0x1402101e,60,15,900\n");
	std::stringstream ss_read4(image4);
	REQUIRE_THROWS_WITH( ldmx::utility::SimpleTableStreamerCSV::load(itable2, ss_read4) , Contains("Mismatched number of columns (3!=4) on line 3") );
      }
    }
  }
}
