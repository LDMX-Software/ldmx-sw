
#######################################################
# @class UserAction
# @brief Class to pass parameters into a UserAction
# @author Omar Moreno, SLAC
class UserAction: 

    def __init__(self, instance_name, class_name):

        self.class_name    = class_name
        self.instance_name = instance_name
        self.parameters    = dict() 

    def __str__(self): 

        string = "UserAction (" + self.__repr__() + ")\n"
        string += " Parameters: \n"
        for k, v in self.parameters.items(): 
            string += "  %s : %s \n" % (k, v)

        return string

    def __repr__(self):
        return '%s of class %s' % (self.instance_name, self.class_name)

#######################################################
# @class PrimaryGenerator
# @brief Class to pass parameters into a PrimaryGenerator
# @author Tom Eichlersmith, University of Minnesota
class PrimaryGenerator: 

    def __init__(self, instance_name, class_name):

        self.class_name    = class_name
        self.instance_name = instance_name
        self.parameters    = dict() 

    def __str__(self): 

        string = "PrimaryGenerator (" + self.__repr__() + ")\n" 
        string += " Parameters: \n"
        for k, v in self.parameters.items(): 
            string += "  %s : %s \n" % (k, v)

        return string

    def __repr__(self):
        return '%s of class %s' % (self.instance_name, self.class_name)
