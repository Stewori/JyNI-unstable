/*
 * Copyright of JyNI:
 * Copyright (c) 2013, 2014, 2015, 2016 Stefan Richthofer.
 * All rights reserved.
 *
 *
 * Copyright of Python and Jython:
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
 * 2010, 2011, 2012, 2013, 2014, 2015, 2016 Python Software Foundation.
 * All rights reserved.
 *
 *
 * This file is part of JyNI.
 *
 * JyNI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * JyNI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with JyNI.  If not, see <http://www.gnu.org/licenses/>.
 */


package JyNI;

import JyNI.gc.JyGCHead;
import JyNI.gc.JyWeakReferenceGC;
import JyNI.gc.JyNIFinalizeTriggerFactory;

import java.io.File;
import java.util.Properties;
import org.python.core.JythonInitializer;
import org.python.core.Py;
import org.python.core.PyModule;
import org.python.core.PyObject;
import org.python.core.PySystemState;
import org.python.core.imp;
import org.python.core.adapter.ExtensiblePyObjectAdapter;
import org.python.core.finalization.FinalizeTrigger;
import org.python.util.PythonInterpreter;
import org.python.modules.gc;
import org.python.modules._weakref.GlobalRef;

public class JyNIInitializer implements JythonInitializer {

	public static boolean initialized = false;
	static JyNIImporter importer;

	static class SentinelFinalizer implements JyGCHead {
		public SentinelFinalizer() {
			new JyWeakReferenceGC(this);
		}

		protected void finalize() throws Throwable {
			gc.notifyPreFinalization();
			new SentinelFinalizer();
			gc.notifyPostFinalization();
		}

		public long getHandle() {return 0;}
	}

	public void initialize(Properties preProperties, Properties postProperties, String[] argv,
			ClassLoader classLoader, ExtensiblePyObjectAdapter adapter)
	{
//		System.out.println("Init JyNI...");
//		System.out.println(System.getProperty("java.class.path"));
		//System.getProperties().list(System.out);
		PySystemState initState = PySystemState.doInitialize(preProperties,
				postProperties, argv, classLoader, adapter);
		importer = new JyNIImporter();
		initState.path_hooks.append(importer);

//	Currently this monkeypatching is done in JyNIImporter on first module import,
//	because doing it here during initialization - although the more complete, better
//	approach - currently breaks sysconfig.py.
//		initState.setPlatform(new PyJavaPlatformString(initState.getPlatform()));
//		PyModule osModule = (PyModule) imp.importName("os", true);
//		osModule.__setattr__("name".intern(), new PyOSNameString());

// Currently hosted in JyNIImporter until sysconfig.py is adjusted in Jython 2.7.2.:
//		initState.setPlatform(new PyShadowString(initState.getPlatform(),
//				JyNI.getNativePlatform()));
//		PyModule osModule = (PyModule) imp.importName("os", true);
//		String _name = osModule.__getattr__("_name".intern()).toString();
//		String nameval = "name".intern();
//		PyObject osname = osModule.__getattr__(nameval);
//		osModule.__setattr__(nameval, new PyShadowString(osname, _name));

		// We make sure that JyNI.jar is not only on classpath, but also on Jython-path:
		String[] cp = System.getProperty("java.class.path").split(File.pathSeparator);
		for (int i = 0; i < cp.length; ++i) {
			//System.out.println(cp[i]);
			if (cp[i].endsWith("JyNI.jar")) {
				initState.path.add(0, cp[i]);
			}
		}
//		initializeState(initState);
//	}
//
//	public void initializeState(PySystemState state) {
//		state.path_hooks.append(new JyNIImporter());
//		initializeBasic();
//	}
//
//	private void initializeBasic() {
		//add the JyNI-Importer to list of import hooks:
		//if (initialized) return;

		PythonInterpreter pint = new PythonInterpreter();
		//add support for sys.setdlopenflags and sys.getdlopenflags as available in common CPython:
		pint.exec("import sys");
		//pint.exec("import JyNI.JyNI");
		pint.exec("sys.dlopenflags = "+JyNI.RTLD_JyNI_DEFAULT);
		//pint.exec("sys.setdlopenflags = JyNI.JyNI.setDLOpenFlags");
		//pint.exec("sys.getdlopenflags = JyNI.JyNI.getDLOpenFlags");
		//pint.exec("sys.setdlopenflags = lambda n: (sys.dlopenflags = n)");
		pint.exec("def setdlopenflags(n): sys.dlopenflags = n");
		pint.exec("sys.setdlopenflags = setdlopenflags");
		pint.exec("sys.getdlopenflags = lambda: sys.dlopenflags");
		pint.cleanup();

		//Set up Jython hooks for JyNI:
		FinalizeTrigger.factory = new JyNIFinalizeTriggerFactory();
		GlobalRef.factory = new JyNIGlobalRefFactory();
		new SentinelFinalizer();
		gc.addJythonGCFlags(gc.FORCE_DELAYED_WEAKREF_CALLBACKS);
		gc.registerPreFinalizationProcess(new Runnable(){
				public void run() {JyNI.preProcessCStubGCCycle();}});
		//System.out.println("Init JyNI done");
		initialized = true;
	}
}
