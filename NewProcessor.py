#!/usr/bin/python

## 
# @file NewProcessor.py
# @brief Python script to generate template new processor header and source files.
# @author Tom Eichlersmith, University of Minnesota
#
# A Module is a directory that has several processors grouped together (e.g. Event or Framework).
# A Processor is one analyzer or producer.
# This script generates the template for one processor in the input module name. If the input module already
# exists, then the processor is inserted into that module's directory structure.
# This script will exit if processor with the input name already exists in the input module. 

import os #checking for and making directories
import argparse #cli inputs

ldmxpath_d = os.getcwd()
#get cli inputs
parser = argparse.ArgumentParser("usage: %prog [options]")
parser.add_argument( "--ldmxPath" , dest="ldmxPath" , help = "Full path to ldmx directory up to and including ldmx-sw (default = \"$PWD\")" , default = ldmxpath_d , type = str )
parser.add_argument( "--moduleName" , dest="moduleName" , help = "Name of new or existing module (default = \"NewModule\")" , default = "NewModule" , type = str )
parser.add_argument( "--processorName" , dest="processorName" , help = "Name of new processor to be added to module (default = \"NewProcessor\")" , default = "NewProcessor" , type = str )
parser.add_argument( "--isProducer" , dest="isProducer" , help = "Declares new processor as producer, without this, it is an analyzer" , default = False , action = "store_true" )
parser.add_argument( "--moduleDeps" , dest="moduleDeps" , help = "Space-separated list of module dependencies withing ldmx framework (default = \"Framework Event\")" , default = "Framework Event" , type = str )
parser.add_argument( "--externalDeps" , dest="externalDeps" , help = "Space-separated list of dependencies outside of ldmx framewokr (default = \"ROOT\")" , default = "ROOT" , type = str )
parser.add_argument( "--makeConfig" , dest="makeConfig" , help = "Tells script to make template config.py file in python subdirectory" , default = False , action = "store_true" )
arg = parser.parse_args()

moduledir = arg.ldmxPath+"/"+arg.moduleName
incdir = moduledir+"/include/"+arg.moduleName
srcdir = moduledir+"/src"
pythondir = moduledir+"/python"

#check if module exists
# if does -> do nothing
# if doesn't -> create module tree, make cmakelists.txt, and add module name to module list
if os.path.exists( moduledir ):
    print "Inserting new processor "+arg.processorName+" into existing module "+arg.moduleName+"."
else:
    print "Making new module "+arg.moduleName+"."
    #make module directory tree
    os.makedirs( moduledir )
    os.makedirs( incdir )
    os.makedirs( srcdir )
    os.makedirs( pythondir )
    if not os.path.exists( moduledir ):
        print "Unable to create module directory "+moduledir+".\nExiting."
        quit()
    if not os.path.exists( incdir ):
        print "Unable to create include directory "+incdir+".\nExiting."
        quit()
    if not os.path.exists( srcdir ):
        print "Unable to create source directory "+srcdir+".\nExiting."
        quit()
    if not os.path.exists( pythondir ) :
        print "Unable to create python directory "+pythondir+".\nExiting."
        quit()
    #write CMakeLists.txt file template
    cmakelistsfile = open( "%s/CMakeLists.txt"%(moduledir) , "w" )
    cmakelistsfile.write( "# declare "+arg.moduleName+" module\n" )
    cmakelistsfile.write( "module(\n" )
    cmakelistsfile.write( "  NAME "+arg.moduleName+"\n" )
    cmakelistsfile.write( "  DEPENDENCIES "+arg.moduleDeps+"\n" )
    cmakelistsfile.write( "  EXTERNAL_DEPENDENCIES "+arg.externalDeps+"\n" )
    cmakelistsfile.write( ")" )
    cmakelistsfile.close()
#endif - making new module or inserting into existing

#set processor type
processortype = "Analyzer"
processoraction = "analyze"
if arg.isProducer :
    processortype = "Producer"
    processoraction = "produce"

srcfilepath = "%s/%s.cxx"%(srcdir,arg.processorName)
headerfilepath = "%s/%s.h"%(incdir,arg.processorName)
if( os.path.isfile(headerfilepath) or os.path.isfile(srcfilepath) ) :
    #header or src file already exists
    print "Processor with name "+arg.processorName+" already exists in "+arg.moduleName+".\nExiting."
    quit()

#write header file template
ifndeftxt = arg.moduleName.upper()+"_"+arg.processorName.upper()+"_H"

headerfile = open( headerfilepath , "w" )

headerfile.write( "/**\n" )
headerfile.write( " * @file "+arg.processorName+".h\n" )
headerfile.write( " * @brief\n" )
headerfile.write( " * @author\n" )
headerfile.write( " */\n" )
headerfile.write( "\n" )
headerfile.write( "#ifndef "+ifndeftxt+"\n" )
headerfile.write( "#define "+ifndeftxt+"\n" )
headerfile.write( "\n" )
headerfile.write( "//LDMX Framework\n" )
headerfile.write( "#include \"Event/Event.h\"\n" )
headerfile.write( "#include \"Framework/EventProcessor.h\" //Needed to declare processor\n" ) 
headerfile.write( "#include \"Framework/ParameterSet.h\" // Needed to import parameters from configuration file\n" )
headerfile.write( "\n" )
headerfile.write( "namespace ldmx {\n" )
headerfile.write( "    \n" )
headerfile.write( "    /**\n" )
headerfile.write( "     * @class "+arg.processorName+"\n" )
headerfile.write( "     * @brief \n" )
headerfile.write( "     */\n" )
headerfile.write( "    class "+arg.processorName+" : public ldmx::"+processortype+" {\n" )
headerfile.write( "        public:\n" )
headerfile.write( "\n" )
headerfile.write( "            "+arg.processorName+"(const std::string& name, ldmx::Process& process) : ldmx::"+processortype+"(name, process) {}\n" )
headerfile.write( "\n" )
headerfile.write( "            virtual void configure(const ldmx::ParameterSet& ps);\n" )
headerfile.write( "\n" )
headerfile.write( "            virtual void "+processoraction+"(const ldmx::Event& event);\n" )
headerfile.write( "\n" )
headerfile.write( "            virtual void onFileOpen();\n" )
headerfile.write( "\n" )
headerfile.write( "            virtual void onFileClose();\n" )
headerfile.write( "\n" )
headerfile.write( "            virtual void onProcessStart(); \n" )
headerfile.write( "\n" )
headerfile.write( "            virtual void onProcessEnd();\n" )
headerfile.write( "\n" )
headerfile.write( "        private:\n" )
headerfile.write( "\n" )
headerfile.write( "    };\n" )
headerfile.write( "}\n" )
headerfile.write( "\n" )
headerfile.write( "#endif /* "+ifndeftxt+" */\n" )

headerfile.close()

#write src file template
srcfile = open( srcfilepath , "w" )

srcfile.write( "/**\n" )
srcfile.write( " * @file "+arg.processorName+".cxx\n" )
srcfile.write( " * @brief \n" )
srcfile.write( " * @author \n" )
srcfile.write( " */\n" )
srcfile.write( "\n" )
srcfile.write( "#include \""+arg.moduleName+"/"+arg.processorName+".h\"\n" )
srcfile.write( "\n" )
srcfile.write( "namespace ldmx {\n" )
srcfile.write( "\n" )
srcfile.write( "    void "+arg.processorName+"::configure(const ldmx::ParameterSet& ps) {\n" )
srcfile.write( "\n" )
srcfile.write( "        return;\n" )
srcfile.write( "    }\n" )
srcfile.write( "\n" )
srcfile.write( "    void "+arg.processorName+"::"+processoraction+"(const ldmx::Event& event) {\n" )
srcfile.write( "\n" )
srcfile.write( "        return;\n" )
srcfile.write( "    }\n" )
srcfile.write( "    \n" )
srcfile.write( "    void "+arg.processorName+"::onFileOpen() {\n" )
srcfile.write( "\n" )
srcfile.write( "        return;\n" )
srcfile.write( "    }\n" )
srcfile.write( "\n" )
srcfile.write( "    void "+arg.processorName+"::onFileClose() {\n" )
srcfile.write( "\n" )
srcfile.write( "        return;\n" )
srcfile.write( "    }\n" )
srcfile.write( "\n" )
srcfile.write( "    void "+arg.processorName+"::onProcessStart() {\n" )
srcfile.write( "\n" )
srcfile.write( "        return;\n" )
srcfile.write( "    }\n" )
srcfile.write( "\n" )
srcfile.write( "    void "+arg.processorName+"::onProcessEnd() {\n" )
srcfile.write( "\n" )
srcfile.write( "        return;\n" )
srcfile.write( "    }\n" )
srcfile.write( "\n" )
srcfile.write( "}\n" )
srcfile.write( "\n" )
srcfile.write( "DECLARE_"+processortype.upper()+"_NS(ldmx, "+arg.processorName+");\n" )

srcfile.close()

#write config script template
if arg.makeConfig :
    pythonfilepath = "%s/%sDEFAULT.py"%(pythondir,arg.processorName)
    if( os.path.isfile(pythonfilepath) ) :
        #python file already exits
        print "Config file "+pythonfilepath+" already exists.\nExiting."
        quit()
    
    pyfile = open( pythonfilepath , "w" )

    pyfile.write( "#!/usr/bin/python\n" )
    pyfile.write( "\n" )
    pyfile.write( "# we need the ldmx configuration package to construct the object\n" )
    pyfile.write( "from LDMX.Framework import ldmxcfg\n" )
    pyfile.write( "\n" )
    pyfile.write( arg.processorName+"DEFAULT = ldmxcfg."+processortype+"(\""+arg.processorName+"DEFAULT\", \"ldmx::"+arg.processorName+"\")\n" )
    pyfile.write( "\n" )
    pyfile.write( "#Put parameters for "+arg.processorName+" below\n" )
    pyfile.write( "\n" )


    pyfile.close()
    
