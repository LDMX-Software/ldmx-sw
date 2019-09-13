#include "Event/Event.h"
#include <sys/types.h>
#include <regex.h>

namespace ldmx {

  std::vector<ProductTag> Event::searchProducts(const std::string& namematch, const std::string& passmatch, const std::string& typematch) const {
    std::vector<ProductTag> retval;

    regex_t reg_name, reg_pass, reg_type;
    char errbuf[1000];
    int rv;

    if (!regcomp(&reg_name,(namematch.empty()?(".*"):(namematch.c_str())),REG_EXTENDED|REG_ICASE|REG_NOSUB)) {
      if (!regcomp(&reg_pass,(passmatch.empty()?(".*"):(passmatch.c_str())),REG_EXTENDED|REG_ICASE|REG_NOSUB)) {
	if (!regcomp(&reg_type,(typematch.empty()?(".*"):(typematch.c_str())),REG_EXTENDED|REG_ICASE|REG_NOSUB)) {

	  const std::vector<ProductTag>& products=getProducts();
	  for (std::vector<ProductTag>::const_iterator i=products.begin(); i!=products.end(); i++) {
	    if (!regexec(&reg_name,i->name().c_str(),0,0,0) &&
		!regexec(&reg_pass,i->passname().c_str(),0,0,0) &&
		!regexec(&reg_type,i->type().c_str(),0,0,0))
	      retval.push_back(*i);
		
	  } 
	  
	  regfree(&reg_type);
	} else { std::cout << "E3\n"; }
	regfree(&reg_pass);
      }   else { std::cout << "E2\n"; }  
      regfree(&reg_name);
    } else { std::cout << "E1\n"; }
    
    return retval;
  }

}
