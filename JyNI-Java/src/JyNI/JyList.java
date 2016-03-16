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

import java.util.AbstractList;
import java.util.Collection;
import org.python.core.PyObject;

import JyNI.gc.*;

import java.lang.ref.WeakReference;

/**
 * JyList is an implementation of java.util.List, that
 * redirects all list-calls to a native backend, namely
 * a Python PyListObject as defined in listobject.h.
 * backendHandle must be a valid pointer to the backing
 * c-struct. The purpose of this implementation is to
 * allow perfect sync between a Jython org.python.core.PyList
 * object and a native PyListObject as defined in listobject.h.
 * Since the native object allows modification via macros, there
 * is no way to detect that modification and mirror it on java-side.
 * So the only way is, not to mirror anything, but to actually map
 * all list accesses to the original native data.
 * 
 * If JyNI syncs a native PyListObject to a jython PyList,
 * there are two cases. If the original object is native, a
 * Jython PyList is created with a JyList as backend.
 * If the original object is a Jython PyList, we must replace
 * its backend with a JyList. First we setup an appropriate
 * native PyListObject. Then we convert all objects from
 * the non-native PyList to native PyObjects and insert them
 * into the native list object. Having that done, we setup
 * a JyList with our new native PyListObject as backend and
 * finally replace the original backend of the PyList with
 * our newly created JyList. However since PyList's backend
 * is stored in a private final field, replacing it is a rather
 * sneaky operation, which indeed might corrupt Extensions written
 * in Java, that also customize backends of PyLists.
 * Future JyNI versions will provide configuration-options
 * to tune this behavior - for instance to sync the original
 * backend as far as possible, maybe even by polling the native
 * list for changes.
 * 
 * See http://docs.oracle.com/javase/specs/jls/se5.0/html/memory.html#17.5.3
 * for information about subsequently modifying final fields.
 * See also these discussions about setting private final fields:
 * stackoverflow.com/questions/3301635/change-private-static-final-field-using-java-reflection
 * stackoverflow.com/questions/4516381/changing-private-final-fields-via-reflection
 * Note that PyList sets its backend in the constructor, so
 * replacing it should work like mentioned in the second discussion.
 * (By the way, I am not involved in any of them, I just found them
 * and thought it would be useful here.)
 */
public class JyList extends AbstractList<PyObject> implements TraversableGCHead, PyObjectGCHead {
	long backendHandle;
	Object headLinks;

	/* This is actually supposed to be a SoftReference, but we use a
	 * WeakReference for easier debugging for now.
	 */
	WeakReference<PyObject> frontend;

	public JyList(long backendHandle) {
		super();
		this.backendHandle = backendHandle;
		new JyWeakReferenceGC(this);
	}

	public JyList(Collection<PyObject> c, long backendHandle) {
		super();
		this.backendHandle = backendHandle;
		addAll(0, c);
		new JyWeakReferenceGC(this);
	}

	public PyObject getPyObject() {
		return frontend != null ? frontend.get() : null;
	}

	public void setPyObject(PyObject object) {
		frontend = new WeakReference<>(object);
	}

	public void setLinks(Object links){
		headLinks = links;
//		System.out.println("JyList setLinks "+this);
//		for (PyObjectGCHead op: ((java.util.List<PyObjectGCHead>) links)) {
//			System.out.println("   "+op+" - "+op.getPyObject());
//		}
	}

	@Override
	public int setLink(int index, JyGCHead link) {
		return DefaultTraversableGCHead.setLink(headLinks, index, link);
	}

	@Override
	public int insertLink(int index, JyGCHead link) {
		return DefaultTraversableGCHead.insertLink(headLinks, index, link);
	}

	@Override
	public int clearLink(int index) {
		return DefaultTraversableGCHead.clearLink(headLinks, index);
	}

	@Override
	public int clearLinksFromIndex(int startIndex) {
		return DefaultTraversableGCHead.clearLinksFromIndex(headLinks, startIndex);
	}

	@Override
	public int jyTraverse(JyVisitproc visit, Object arg) {
		return DefaultTraversableGCHead.jyTraverse(headLinks, visit, arg);
	}

	@Override
	public long[] toHandleArray() {
		return DefaultTraversableGCHead.toHandleArray(headLinks);
	}

	@Override
	public long getHandle() {
		return backendHandle;
	}

	/*public void installToPyList(PyList list)
	{
		try
		{
			Field backend = PyList.class.getDeclaredField("list");
			backend.setAccessible(true);
			backend.set(list, this);
		} catch (Exception e)
		{
			System.err.println("Problem modifying PyList backend: "+e);
		}
	}

	public List<PyObject> installToPyListAndGetOldBackend(PyList list)
	{
		try
		{
			Field backend = list.getClass().getDeclaredField("list");
			backend.setAccessible(true);
			List<PyObject> er = (List<PyObject>) backend.get(list);
			backend.set(list, this);
			return er;
		} catch (Exception e)
		{
			System.err.println("Problem in installToPyListAndGetOldBackend: "+e);
			return null;
		}
	}*/

	public PyObject get(int index) {
		//System.out.println(backendHandle+" JyList-get "+index);
		return JyNI.JyList_get(backendHandle, index);
	}

	public int size() {
		//System.out.println(backendHandle+" JyList-size");
		return JyNI.JyList_size(backendHandle);
	}

	public PyObject set(int index, PyObject o) {
		return JyNI.JyList_set(backendHandle, index, o, JyNI.lookupNativeHandle(o));
	}

	public void add(int index, PyObject o) {
		JyNI.JyList_add(backendHandle, index, o, JyNI.lookupNativeHandle(o));
	}

	public PyObject remove(int index) {
		return JyNI.JyList_remove(backendHandle, index);
	}
}
