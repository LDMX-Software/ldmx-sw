
# Generating the Documentation

Because of how different python and C++ are as languages, they are generally documented in very different ways.
Because of this, we use two different tools to generate the documentation for ldmx-sw: doxygen and sphinx.
You need to install both of them for your local documentation generation to work.

## [doxygen](http://doxygen.nl/)
This is the tool we use to generate the C++ documentation.
After it is installed, all you have to do is run the following command inside the `docs` directory (this directory).
```bash
doxygen doxygen.conf
```
This will generate the C++ API documentation and put it in the `docs/html/_doxygen` directory.

## [sphinx](https://www.sphinx-doc.org/en/stable/)
This is the tool to generate the python documentation.
It is more complicated than the doxygen documentation because the structure of our source code is different from the vast majority of python projects.

Install sphinx and the theme we use:
```bash
pip install -U Sphinx groundwork-sphinx-theme
```

We need our python code to be packaged together like a more normal python package, 
the simplest way to do that is to build and install ldmx-sw as normal.
This runs through the cmake-magic that puts all the python modules together into one python package.

Inside of the `docs` directory (this directory), we can generate the python API documentation by 
referencing the installed python package.
```
sphinx-apidoc -f -T -o sphinx.conf <path-to-ldmx-sw-install>/lib/python/LDMX
```

Then we can build the python html docs.
```
sphinx-build sphinx.conf html/_sphinx
```
which will produce the documentation in `docs/html/_sphinx`.
