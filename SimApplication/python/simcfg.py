
class UserAction: 

    def __init__(self, instance_name, class_name):

        self.class_name    = class_name
        self.instance_name = instance_name
        self.parameters    = dict() 

    def __str__(self): 

        string = "UserAction (%s of class %s)\n" % (self.instance_name, self.class_name)
        string += "\t Parameters: \n"
        for k, v in self.parameters.items(): 
            string += "\t\t %s : %s \n" % (k, v)

        return string
