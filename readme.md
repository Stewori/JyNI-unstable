If not yet done, make sure to visit our project homepage at www.jyni.org

![JyNI Logo](http://www.jyni.org/_static/JyNILogo2s.png)


Table of contents
---------------------------------------------

1. What is JyNI?
2. Current state
3. Building and testing
4. Roadmap
5. Binary compatibility
6. Summary of changes to Python-code
7. Copyright notice
8. License
9. Contact

---------------------------------------------




1. What is JyNI?
----------------

One well known current drawback of Jython is that it does not support native
extensions written for CPython like NumPy and SciPy. Since most scientific
Python-code fundamentally depends on exactly such native extensions, it
usually cannot be run with Jython. JyNI aims to close this gap. It is a
layer that enables Jython-users to load native CPython-extensions and access
them from Jython the same way as they would do in CPython. In order to
leverage the JyNI functionality, you just have to put it on the
Java-classpath when Jython is launched. It neither requires you to recompile
the extension-code, nor to build a customized Jython-fork. That means, it is
binary compatible with existing extension-builds.

At the time when this is written, JyNI does not fully implement
the Python C-API and we are in fact just capable to load and run simple examples
that only involve most basic builtin-types.

The concept is rather complete though and our goal is to
provide the C-API needed to load NumPy as soon as possible.
After that we will focus on SciPy and others.




2. Current state
----------------

We are currently able to load a C-extension into Jython, call methods and access
attributes, provided that the extension uses only CPython-API we have already
implemented.

The parse- and build-functions PyArg_ParseTuple(), PyArg_ParseTupleAndKeywords(),
PyArg_Parse(), Py_BuildValue and related only work with format-strings as long
as the corresponding builtin-types are already supported. Alternatively,
supported builtin types can be accessed via their C-API-functions and even
returned to Jython. Keywords work as well.


The following built-in types are already supported:

* Number types PyInt, PyLong, PyFloat, PyComplex
* Sequence types PyTuple, PyList, PySlice, PyString, PyUnicode
* Data structure types PyDict, PySet, PyFrozenSet
* Operational types PyModule, PyFunction, PyCode, PyProperty, PyDictProxy, PyCell
* OOP types PyClass, PyInstance, PyMethod, PyClassMethod, PyStaticMethod
* Singleton types PyBool, PyNone, PyNotImplemented, PyEllipsis
* Native types PyCFunction, PyCapsule, PyCObject
* Natively defined custom-types
* Exception types
* PyType as static type or heap type
* weak reference types

Passing instances of new-style classes to an extension does not work yet.


JyNI has been tested on

* Linux Mint Debian edition (LMDE) (32 bit and 64 bit)
* Linux Mint 17 (64 bit)
* Mac OS-X (10.10 and 10.11)

It would presumably work also on other posix systems.
If you try it on further distributions, please consider to report your results
(see contact section).




3. Building and testing
-----------------------

* Download the sources from the latest release or clone the github repository
(https://github.com/Stewori/JyNI).

* Provide Java: Make sure that Java 7 JDK or newer is installed. Some systems
(e.g. Linux MINT) provide a symlink called "/usr/lib/jvm/default-java" that
points to the specific Java folder named by Java version and architecture. If
your system does not provide this generic link (e.g. Ubuntu does not), you
must specify your Java folder in the JyNI make file. To do so open "makefile"
from the JyNI base folder with a text editor of your choice and edit the line
starting with "JAVA_HOME" such that it is appropriate for your system.

* Provide Jython: You have two ways to provide Jython. Either you simply copy
jython.jar into the JyNI base directory or you edit "makefile" and adjust the
JYTHON-variable such that it points to jython.jar. If you want to use
JyNIDemo.sh, you need to adjust the Jython-path in that file too. Same for
the other demonstration shell-scripts.
Note: Current JyNI repository version requires Jython 2.7.1 (beta 3) or newer.
Jython 2.7.1 (beta 3) is available here: http://fwierzbicki.blogspot.de/2016/02/jython-271-beta3-released.html

* If not yet done, install the dev-package of Python 2.7. JyNI only needs
pyconfig.h from that package. Alternatively (if you know what you're doing)
you can provide your own pyconfig.h. In that case you'll have to adjust the
corresponding include-path in the makefile.

* Build JyNI: Open a terminal and enter the JyNI base directory. Type "make".
Optionally run "make clean" afterwards. The resulting binaries are placed in
the subfolder named "build".
On OS-X append "-f makefile.osx".
To build on Linux with clang (rather than gcc) append "-f makefile.clang".
(For OS-X only a clang-based makefile is available.)

To run Jython with JyNI support, call (from JyNI base dir)

java -cp jython.jar:build/JyNI.jar org.python.util.jython someFile.py

Alternatively one can use Jython's start-script
(assuming you copied it to the JyNI base directory):

jython -J-cp build/JyNI.jar

To run it from a different place, you just need to adjust the paths for
JyNI and Jython in the above commands.


Note that

java -cp build/JyNI.jar -jar jython.jar

is NOT suitable to run Jython with JyNI support. It does not pass the
provided classpath to Jython.


###Test-Example

"DemoExtension" is a CPython-extension that demonstrates some
basic features of JyNI. It can be built like an
ordinary Python-extension as described in the tutorial
http://docs.python.org/2/extending/extending.html

Enter the folder DemoExtension and type "python setup.py build".

JyNI-Demo/src/test_JyNI.py runs unittests for some basic JyNI features.
You can run it via
java -cp jython.jar:build/JyNI.jar org.python.util.jython JyNI-Demo/src/test_JyNI.py
or
./JyNI_unittest.sh
provided that jython.jar is placed in JyNI's directory. Otherwise you need to
provide the correct path for your jython.jar in the command.

JyNI-Demo/src/JyNIDemo.py demonstrates the use of DemoExtension from
Python-side. It should run perfectly with either CPython 2.7.x or
Jython+JyNI. To run it, type
java -cp jython.jar:build/JyNI.jar org.python.util.jython JyNI-Demo/src/JyNIDemo.py
Like with the unittest you can alternatively run
./JyNIDemo.sh

To see a basic demonstration that JyNI is capable of using the original
datetime module, run
java -cp jython.jar:build/JyNI.jar org.python.util.jython JyNI-Demo/src/JyNIDatetimeTest.py

To see a demonstration of exception support (on native level), run
java -cp jython.jar:build/JyNI.jar org.python.util.jython JyNI-Demo/src/JyNIExceptionTest.py

To see a demonstration of Tkinter support, run
java -cp jython.jar:build/JyNI.jar org.python.util.jython JyNI-Demo/src/JyNITkinterTest.py
or
./JyNITkinterDemo.sh

For a demonstration of ctypes support, run
java -cp jython.jar:build/JyNI.jar org.python.util.jython JyNI-Demo/src/JyNIctypesTest.py
or
./JyNIctypesDemo.sh




4. Roadmap
----------

Further steps:
- improve ctypes support
- support newstyle classes
- support numpy
- support buffer protocol
- test and support on other platforms
- provide autotools-support




5. Binary compatibility
-----------------------

CPython-extensions must be compiled with the flag WITH_PYMALLOC activated
(which is actually the default anyway). Otherwise they must not use
the macros PyObject_MALLOC, PyObject_REALLOC and PyObject_FREE directly.
If they use these macros with WITH_PYMALLOC not activated, JyNI will most
likely produce segmentation faults. This is because without the flag, these
macros directly map to the system's malloc-function family. In that case
JyNI would not be able to prepare the JyObject-header in every case.
However, it would be easy to recompile such extensions for JyNI. Simply add
the WITH_PYMALLOC-definition at compile-time or replace the macros by their
corresponding function-calls.

In general, the less an extension hacks with CPython-specific internals,
the greater the chance it will run with JyNI. Especially
allocation/deallocation should not be done by hand, since JyNI must be able
to setup the JyObject-headers.

We hope that most extensions are not affected by this issue.




6. Summary of changes to Python-code
------------------------------------

Briefly speaking, we took the python files crucial for loading native
C-extensions and modified them to perform the explained purpose of JyNI.
Mainly we changed alloc and dealloc behavior of PyObjects consequently, also
when inlined like in stringobject.c or intobject.c. The new allocation
behavior adds a header called JyObject before the PyObject header or even in
front of PyGC_Head if present. This header contains information that allows to
wrap or mirror a corresponding Jython jobject in a seamless way.

Mirror mode is used for builtin types that allow access to their data-structure
via macros. To keep these accesses valid, the data structure mirrors the actual
Jython jobject. Syncing is not an issue for immutable objects and can be done
initially for each object. One mutable builtin object needing mirror-mode is
PyList. We perform sync from java by inserting a special java.util.List
implementation as a backend into the corresponding Jython PyList.
A bit more tricky is PyByteArray, which also features a direct-access-macro,
namely PyByteArray_AS_STRING. However a fix for this has low priority, as this
macro is not used by NumPy. Nevertheless, we can say a bit about this issue. As
PyByteArray in Jython uses a primitive byte[]-array as backend, we can't replace
the backend by a subclass. Our most promising idea to solve the issue anyway, is
to insert a custom implementation of SequenceIndexDelegate as delegator
(PyByteArray is a subclass of PySequence and thus offers a delegator-field
that allows to customize the indexed access-behavior).

Builtins that don't provide access via macros can be wrapped. That means, the
original CPython data structure is not used and not even allocated. All
functions of such objects are rewritten to delegate their calls to Jython.
However, also in these cases, we usually don't rewrite everything. Functions
that access the data only via other functions are mostly kept unchanged.




7. Copyright notice
-------------------

Copyright of Python and Jython:
Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010,
2011, 2012, 2013, 2014, 2015, 2016 Python Software Foundation.
All rights reserved.

Copyright of JyNI:
Copyright (c) 2013, 2014, 2015, 2016 Stefan Richthofer.
All rights reserved.




8. License
----------

The software in this package is distributed under the
GNU Lesser General Public License.

A copy of GNU Lesser General Public License (LGPL) is included in this
distribution, in the files "COPYING" and "COPYING.LESSER".
If you do not have the source code, it is available at:

    https://github.com/Stewori/JyNI


JyNI is partly based on source-files from CPython 2.7.3, 2.7.4, 2.7.5, 2.7.6,
2.7.7, 2.7.8 and 2.7.9. As such, it includes the common license of CPython
2.7.3, 2.7.4, 2.7.5, 2.7.6, 2.7.7, 2.7.8, 2.7.9, 2.7.10, 2.7.11 and Jython in
the file "PSF-LICENSE-2". Whether a source-file is directly based on CPython
source-code is documented at the beginning of each file.

For compliance with PSF LICENSE AGREEMENT FOR PYTHON 2, section 3,
the required overview of changes done to CPython-code is given
in section 6 of this file.

For convenience, a copy of the current section is provided in the file
"LICENSE".




9. Contact
----------

Please use the contact information provided on www.jyni.org/#contact.

