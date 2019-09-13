/**
 * @file ProductTag.h
 * @brief Class defining the identity of a data product in the event
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef EVENT_PRODUCTTAG_H_
#define EVENT_PRODUCTTAG_H_

// C++
#include <ostream>

// STL
#include <string>

namespace ldmx {
  
    /**
     * @class ProductTag
     * @brief Defines the identity of a product and can be used for searches
     *
     */
    class ProductTag {
	public:
	/**
	 * Class constructor.
	 * 
	 * Blank/empty parameters are generally considered as wildcards
	 * when searches are performed.
	 *
	 * @param name Product name
	 * @param pass Pass name
	 * @param type Type name
	 */
	ProductTag(const std::string& name, const std::string& pass, const std::string& type) : name_{name}, passname_{pass}, typename_{type} { }
	
	/** 
	 * Get the product name
	 */
	const std::string& name() const { return name_; }
	
	/** 
	 * Get the product pass name
	 */
	const std::string& passname() const { return passname_; }
	
	/** 
	 * Get the product type name
	 */
	const std::string& type() const { return typename_; }
	
	private:
	
	/**
	 * Name given to the product
	 */
	std::string name_;
	
	/**
	 * Passname given when product was written
	 */
	std::string passname_;
	
	/**
	 * Typename of the product
	 */
	std::string typename_;   
    };
    
}

/**
 * Print/stream method for ldmx::ProductTag
 */
std::ostream& operator<<(std::ostream&,const ldmx::ProductTag&);

#endif
