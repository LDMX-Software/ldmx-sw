/**
 * @file SimpleTableCondition
 * @brief A collection of simple homogenous tables indexed by detector id
 * @author Jeremy Mans, University of Minnesota
 */
#ifndef FRAMEWORK_SIMPLETABLECONDITION_H_
#define FRAMEWORK_SIMPLETABLECONDITION_H_

#include <vector>

#include "Framework/Exception/Exception.h"
#include "Framework/ConditionsObject.h"

namespace ldmx {

    /**
     * @class Base type for conditions objects which are tables indexed by raw detector id values
     */
    class BaseTableCondition : public ConditionsObject {
	public:
	
	
	/** Create table with given set of columns */
	BaseTableCondition(const std::string& name, const std::vector<std::string>& columns) : ConditionsObject{name}, columns_{columns} {
	    columnCount_=(unsigned int)(columns.size());
	}
	
	/**
	 * Get a the column number for the given column name
	 */
	unsigned int getColumnNumber(const std::string& colname) const {
	    for (unsigned int i=0; i<columnCount_; i++)
		if (colname==columns_[i]) return i;
	    return columnCount_;
	}
	
	/** 
	 * Get the number of columns
	 */
	unsigned int getColumnCount() const {
	    return columnCount_;
	}
	
	/**
	 * Get the name of the given column
	 */
	const std::string& getColumnName(unsigned int icol) const {
	    if (icol>=columnCount_) {
		EXCEPTION_RAISE("ConditionsException",std::string("Column index out of range in ")+getName());
	    }
	    return columns_[icol];
	}

	/** 
	 * Get the id for the given row
	 */
	unsigned int getRowId(unsigned int irow) const {
	    if (irow>=getRowCount()) {// raise exception
		EXCEPTION_RAISE("ConditionsException","Row out of range: "+std::to_string(irow));
	    }
	    return keys_[irow];
	}
	
	/**
	 * Get the names of the columns
	 */
	const std::vector<std::string>& getColumnNames() const {
	    return columns_;
	}

	/**
	 * Get the number of rows
	 */
	std::size_t getRowCount() const {
	    return keys_.size();
	}

	protected:
	
	std::size_t findKey(unsigned int id) const;
	
	std::size_t findKeyInsert(unsigned int id) const;
	
	std::vector<std::string> columns_;
	unsigned int columnCount_;
	std::vector<uint32_t> keys_;
    };
    
    template <class T>
	class HomogenousTableCondition : public BaseTableCondition {
	public:
	
	HomogenousTableCondition(const std::string& name, const std::vector<std::string>& columns) : BaseTableCondition(name, columns) {
	}

	virtual ~HomogenousTableCondition() {
	}

	/** 
	 * Clear the contents 
	 */
	void clear() {
	    keys_.clear();
	    values_.clear();
	}
      	
	/** Add an entry to the table */
	void add(unsigned int id, const std::vector<T>& values) {
	    if (values.size()!=columnCount_) {
		EXCEPTION_RAISE("ConditionsException",getName()+": Attempted to insert a row with "+std::to_string(values.size())+" columns into a table with "+std::to_string(columnCount_)+" columns");      
	    }
	    std::size_t loc=findKey(id);
	    if (loc!=getRowCount()) {
		EXCEPTION_RAISE("ConditionsException","Attempted to add condition in "+getName()+" for existing id "+std::to_string(id));
	    }
	    loc=findKeyInsert(id); // where to put it
	    
	    // insert into the keys
	    keys_.insert(keys_.begin()+loc,id);
	    // insert into the values
	    values_.insert(values_.begin()+loc*columnCount_,values.begin(),values.end());    
	}
	
	
	/**
	 * Get an entry by DetectorId and number.
	 * Throws an exception when id is unavailble
	 */
	T get(unsigned int id, unsigned int col) const {
	    std::size_t irow=findKey(id);
	    if (col>=columnCount_ || irow==getRowCount()) { // raise exception
		EXCEPTION_RAISE("ConditionsException","No such column "+std::to_string(col)+" or id "+std::to_string(id));
	    }
	    return values_[irow*columnCount_+col];
	}
	
	/**
	 * Get a row by number
	 * Used primarily for persisting the SimpleTableCondition
	 */
	std::pair<unsigned int,std::vector<T> > getRow(unsigned int irow) const {
	    if (irow>=getRowCount()) {// raise exception
		EXCEPTION_RAISE("ConditionsException","Row out of range: "+std::to_string(irow));
	    }
	    std::vector<T> rv(&(values_[irow*columnCount_]),&(values_[(irow+1)*columnCount_]));
	    return std::pair<unsigned int,std::vector<T> >(keys_[irow], rv);
	}
	
	/**
	 * Get a column by DetectorId and name
	 * Throws an exception when 
	 * @note This is an inefficient process.  Suggest caching the return of "getColumnNumber()" and using getColumn directly
	 */
	T getByName(unsigned int id, const std::string& colname) const {
	    return get(id, getColumnNumber(colname));
	}
	
	private:
	std::vector<T> values_; // unrolled array
    };

    
    /** 
     * @class Explicit specializeion of HomogenousTableCondition for double-precision contents
     */
    class DoubleTableCondition : public HomogenousTableCondition<double> {
	public:

	DoubleTableCondition(const std::string& name, const std::vector<std::string>& columns) : HomogenousTableCondition<double>(name,columns) {
	}

	virtual ~DoubleTableCondition() {
	}
	
    };
    
    /** 
     * @class Explicit specialization of HomogenousTableCondition for integer contents
     */
    class IntegerTableCondition : public HomogenousTableCondition<int> {
	public:

	IntegerTableCondition(const std::string& name, const std::vector<std::string>& columns) : HomogenousTableCondition<int>(name,columns) {
	}

	virtual ~IntegerTableCondition() {
	}
	
    };

}

#endif
