
# Generating the Documentation

Because of how different python and C++ are as languages, they are generally documented in very different ways.
Because of this, we use two different tools to generate the documentation for ldmx-sw: doxygen and sphinx.
You need to install both of them for your local documentation generation to work.

## doxygen
This is the tool we use to generate the C++ documentation.
After it is installed, all you have to do is run the following command 
```bash
doxygen docs/doxygen.conf/doxyfile
```
This will generate the C++ API documentation and put it in the `docs/html/_doxygen` directory.

**Note**: The current doxyfile contains an "alias" allowing you to link to the python config module documentation
from your doxygen comment. Use `\pythonpackage` to do this (if you are on the doxygen website, this will appear
as the HTML code to link to the python manual.).

## sphinx
This is the tool to generate the python documentation.
It is more complicated than the doxygen documentation because the structure of our source code is different from the vast majority of python projects.

Install sphinx and the theme we use:
```bash
pip install -U Sphinx Pillow
```

We need our python code to be packaged together like a more normal python package, 
the simplest way to do that is to build and install ldmx-sw as normal.
This runs through the cmake-magic that puts all the python modules together into one python package.

We can generate the python API documentation by referencing the installed python package
after creating ("touching") a few extra files that `sphinx-apidoc` needs to recognized directories
as modules.
```
find install/python/LDMX -type d -exec touch {}/__init__.py ';'
sphinx-apidoc -f -T -o docs/sphinx.conf install/python/LDMX
```

Then we can build the python html docs.
```
sphinx-build docs/sphinx.conf docs/html/python
```
which will produce the documentation in `docs/html/python`.
