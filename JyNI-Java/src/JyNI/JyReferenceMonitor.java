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

import org.python.core.*;

import JyNI.gc.*;
import java.util.*;
import java.lang.ref.*;

public class JyReferenceMonitor {
//	0000 00## - 01=increase, 10=decrease, 11=realloc, 00=other
//	0000 0#00 - memory action vs reference action/other
//	0000 #000 - native action
//	000# 0000 - GC action
//	00#0 0000 - pre/start
//	0#00 0000 - post/finnish
//	#000 0000 - finalize action
	public static final short INC_MASK      =   1;
	public static final short DEC_MASK      =   2;
	public static final short MEMORY_MASK   =   4;
	public static final short NATIVE_MASK   =   8;
	public static final short GC_MASK       =  16;
	public static final short PRE_MASK      =  32;
	public static final short POST_MASK     =  64;
	public static final short FINALIZE_MASK = 128;
	public static final short INLINE_MASK   = 256;
	public static final short IMMORTAL_MASK = 512;

	public static final short NAT_ALLOC = INC_MASK | MEMORY_MASK | NATIVE_MASK;
	public static final short NAT_FREE = DEC_MASK | MEMORY_MASK | NATIVE_MASK;
	public static final short NAT_REALLOC = NAT_ALLOC | NAT_FREE;
	public static final short NAT_ALLOC_GC = NAT_ALLOC | GC_MASK;
	public static final short NAT_FREE_GC = NAT_FREE | GC_MASK;
	public static final short NAT_REALLOC_GC = NAT_REALLOC | GC_MASK;
	public static final short NAT_FINALIZE = NATIVE_MASK | FINALIZE_MASK | PRE_MASK;
	//i.e. expected to be called at the beginning of finalizer-code
	public static final short OBJECT_FINALIZE = FINALIZE_MASK | PRE_MASK;
	public static final short OBJECT_RESURRECT = FINALIZE_MASK | INC_MASK;
	public static final short OBJECT_DELETE = DEC_MASK | MEMORY_MASK;
	public static final short GC_RUN = GC_MASK | PRE_MASK;
	public static final short GC_PRE_FINALIZE = GC_MASK | FINALIZE_MASK | PRE_MASK;
	public static final short GC_POST_FINALIZE = GC_MASK | FINALIZE_MASK | POST_MASK;
	public static final short GC_DONE = GC_MASK | POST_MASK;

	public static long startTime = System.currentTimeMillis();
	public static HashMap<Long, ObjectLog> nativeObjects = new HashMap<>();
	public static HashMap<Long, ObjectLog> permanentLeaks = new HashMap<>();
	public static long lastClearRefTime = 0;
	public static long lastGCRun = 0;
	public static boolean lastClearGraphValid = false;

	public static String actionToString(short action) {
		StringBuilder result = new StringBuilder();
		if ((action & NATIVE_MASK) != 0) result.append("_NATIVE");
		if ((action & PRE_MASK) != 0) result.append("_PRE");
		if ((action & POST_MASK) != 0) result.append("_POST");
		if ((action & MEMORY_MASK) != 0) {
			if ((action & GC_MASK) != 0) result.append("_GC");
			if ((action & INC_MASK) != 0 && (action &  DEC_MASK) != 0) result.append("_REALLOC");
			else if ((action & (INC_MASK)) != 0) result.append("_ALLOC");
			else if ((action & (DEC_MASK)) != 0) result.append("_FREE");
		} else if ((action & GC_MASK) == 0) {
			if ((action & (INC_MASK)) != 0 && (action & (DEC_MASK)) != 0) result.append("_REF-RESTORE");
			else if ((action & (INC_MASK)) != 0) result.append("_INCREF");
			else if ((action & (DEC_MASK)) != 0) result.append("_DECREF");
		} else {
			if ((action & (INC_MASK)) != 0 && (action & (DEC_MASK)) != 0) result.append("_GC-RETRACK(??)");
			else if ((action & (INC_MASK)) != 0) result.append("_GC-TRACK");
			else if ((action & (DEC_MASK)) != 0) result.append("_GC-UNTRACK");
			else result.append("_GC");
		}
		//if ((action & GC_MASK) != 0) result.append("_GC");
		if ((action & INLINE_MASK) != 0) result.append("_INLINE");
		if ((action & FINALIZE_MASK) != 0 && (action & INC_MASK) != 0)  result.append("_RESURRECT");
		else if ((action & FINALIZE_MASK) != 0) result.append("_FINALIZE");
		if ((action & IMMORTAL_MASK) != 0) result.append("_IMMORTAL");
		return result.substring(1);
	}

	public static class NativeAction {
		public short action;
		public PyObject obj;
		public long nativeRef1;
		public long nativeRef2;

		public String cTypeName;
		public String cMethod;
		public int cLine;
		public String cFile;
	}

//	public static ObjectLogDebugInfo makeDebugInfo(String cTypeName, String cMethod,
//			int line, String cFile) {
//		ObjectLogDebugInfo result = new ObjectLogDebugInfo();
//		result.cTypeName = cTypeName;
//		result.cMethod = cMethod;
//		result.line = line;
//		result.cFile = cFile;
//		return result;
//	}

	public static class ObjectLogException extends Exception {
		public ObjectLog src;
		public NativeAction action;

		public ObjectLogException(String message, ObjectLog src, NativeAction action) {
			super(message);
			this.src = src;
			this.action = action;
		}
	}

	public static class ObjectLog {
		static String na = "n/a";
		long nativeRef = 0;
		long initialNativeRef = 0; //to track reallocs
		WeakReference<PyObject> object;
		String nativeType;

		/* These are timestamps or 0 if not yet happened */
		long nativeAlloc = 0;
		String nativeAllocFunc;
		/* We only track the latest realloc for now. */
		long nativeLatestRealloc = 0;
		String nativeReallocFunc;
		long nativeGCTrack = 0;
		String nativeGCTrackFunc;
		long nativeFree = 0;
		String nativeFreeFunc;
		boolean gc = false;
		long immortal = 0;
		String immortalFunc;
		boolean inline = false;
		long jyWeakRef = 0;
		long JyNIFree = 0;
		String repr = "n/a";
		/* to save the old log if a memory position is reused */
		ObjectLog previousLife = null;

		public ObjectLog(long initialRef) {
			this.initialNativeRef = initialRef;
			this.nativeRef = initialRef;
		}

		public PyObject getJythonObject() {
			return object != null ? object.get() : null;
		}

		public String repr() {
			if (nativeFree > 0) return na;
			if (object != null) {
				PyObject op = object.get();
				if (op != null) {
					String rp = op.toString();
					if (rp != null) {
						repr = "\""+rp+"\"_j";
						return repr;
					}
				}
			}
			if (nativeType != null && nativeType.equals("str")) {
				String rp = JyNI.PyObjectAsString(nativeRef,
						JyTState.prepareNativeThreadState(Py.getThreadState()));
				if (rp != null) {
					repr = "\""+rp+"\"_n";
				}
			}
			return repr;
		}

		public String getNativeTypeName() {
			if (nativeType == null) {
				String nt = JyNI.getNativeTypeName(nativeRef);
				if (nt != null) return nt;
				else return "type "+na;
			} else return nativeType;
		}

		public String toString() {
			String os = repr();
			if (os == null) os = na;
			/* We intentionally don't use "null" as descriptor for
			 * a missing reference. object being null just means
			 * that no information is available - the native object
			 * might or might not refer to null, so we describe it
			 * as "n/a".
			 */
			if (object != null) {
				PyObject op = object.get();
				os = op == null ? os+"-jfreed-" : os;
			}
//			else if (nativeType != null) {
//				if (nativeType.equals("str")) {
//					os = "\""+JyNI.PyObjectAsString(nativeRef,
//							JyTState.prepareNativeThreadState(Py.getThreadState()) )+"\"_n";
//				}
//			}
			if (nativeType == null) {
				String nt = JyNI.getNativeTypeName(nativeRef);
				System.out.println("JyNI-Warning: Null-type discovered: "+nt);
				System.out.println("object: "+(
						nativeFree == 0 ? JyNI.lookupFromHandle(nativeRef) : na));
				if (nt != null)
					nativeType = nt+"_n";
			}
			String inGC = gc ? "_GC" : "";
			if (jyWeakRef != 0) inGC += "_J";
			return nativeRef+inGC+" ("+nativeType+") #"+
					JyNI.currentNativeRefCount(nativeRef)+
					": "+os+" *"+(nativeAlloc-startTime);
		}

		public boolean isLeak() {
			return nativeFree == 0 && immortal == 0;
		}

		public void updatePyObject() {
			if (object == null || object.get() == null)
				forceUpdatePyObject();
		}

		public void forceUpdatePyObject() {
//			System.out.println("forceUpdatePyObject "+nativeRef);//Free on non-allocated ref!
//			System.out.println("Src-func: "+nativeAllocFunc);
//			System.out.println("type: "+this.nativeType);
			PyObject op = nativeFree == 0 ? JyNI.lookupFromHandle(nativeRef) : null;
//			System.out.println("forceUpdatePyObject done");
			if (op != null) object = new WeakReference<>(op);
		}

		public void updateInfo(NativeAction nvac)//, String repr)
				throws ObjectLogException {
//			if (nativeType == null) {
//				System.out.println("JyNI-Warning: RefMonitor called with null-type.");
//				System.out.println("    "+cFile);
//				System.out.println("    "+cMethod);
//				System.out.println("    "+line);
//			}
			if (nvac.obj != null) {
				if (object != null && object.get() != null && object.get() != nvac.obj) {
					throw new ObjectLogException(
						"Log-Object error: Contradictory PyObject: "+nvac.obj+" vs "+object.get(),
						this, nvac);
				} else if (object == null) {
					object = new WeakReference<>(nvac.obj);
				}
			}
			if (nativeType != null) {
				if (this.nativeType != null && !this.nativeType.equals(nativeType)) {
					throw new ObjectLogException(
						"Log-Type error: Contradictory native type-strings!",
						this, nvac);
				} else {
					this.nativeType = nvac.cTypeName;
				}
			}
			if ((nvac.action & MEMORY_MASK) != 0) {
				if ((nvac.action & INC_MASK) != 0 && (nvac.action &  DEC_MASK) != 0) {
					if (nativeAlloc == 0)
						throw new ObjectLogException(
							"Log-Realloc error: Realloc on non-allocated ref!",
							this, nvac);
					if (nativeRef != nvac.nativeRef1)
						throw new ObjectLogException(
							"Log-Realloc error: Old ref doesn't match!",
							this, nvac);
					nativeRef = nvac.nativeRef2;
					nativeLatestRealloc = System.currentTimeMillis();
					nativeReallocFunc = nvac.cMethod+"/"+nvac.cLine;
				} else if ((nvac.action & INC_MASK) != 0) {
					if (nativeAlloc != 0)
						throw new ObjectLogException(
							"Log-Alloc error: Alloc on already allocated ref!",
							this, nvac);
					if (nativeFree != 0)
						throw new ObjectLogException(
							"Log-Alloc error: Alloc on already freed ref!",
							this, nvac);
					nativeAlloc = System.currentTimeMillis();
					nativeAllocFunc = nvac.cMethod+"/"+nvac.cLine;
				} else if ((nvac.action & DEC_MASK) != 0) {
					if (nativeAlloc == 0)
						throw new ObjectLogException(
							"Log-Free error: Free on non-allocated ref!",
							this, nvac);
					if (nativeFree != 0)
						throw new ObjectLogException(
							"Log-Free error: Free on already freed ref!",
							this, nvac);
					nativeFree = System.currentTimeMillis();
					nativeFreeFunc = nvac.cMethod+"/"+nvac.cLine;
				}
			}
			if ((nvac.action & IMMORTAL_MASK) != 0) {
				if (immortal != 0)
					throw new ObjectLogException(
						"Log-Immortal error: Object made immortal twice!",
						this, nvac);
				immortal = System.currentTimeMillis();
				immortalFunc = nvac.cMethod+"/"+nvac.cLine;
			}
			if ((nvac.action & GC_MASK) != 0) {
				gc = true;
			}
			if ((nvac.action & INLINE_MASK) != 0) {
				inline = true;
			}
			//if (repr != null) this.repr = repr;
		}
	}

//	public static class MemAction {
//		short action;
//		long timestamp;
//		long nativeRef = 0;
//
//		public MemAction(short action) {
//			this.action = action;
//			timestamp = System.currentTimeMillis();
//		}
//
//		public MemAction(short action, long timestamp) {
//			this.action = action;
//			this.timestamp = timestamp;
//		}
//	}
//
//	public static class PyObjectInfo {
//		WeakReference<PyObject> ref;
//		PyType type;
//		String repr;
//		int id;
//		boolean nativeFreed;
//		String nativeType;
//		String comment;
//	}
//
//	public static class ObjectAction extends MemAction {
//		PyObjectInfo info;
//
//		public ObjectAction(short action, PyObjectInfo info) {
//			super(action);
//			this.info = info;
//		}
//
//		public ObjectAction(short action, long timestamp, PyObjectInfo info) {
//			super(action, timestamp);
//			this.info = info;
//		}
//	}

	public static void addJyWeakRef(long handle) {
		ObjectLog log = nativeObjects.get(handle);
		if (log != null) {
			if (log.jyWeakRef != 0) {
				System.out.println("addJyWeakRef: JyWeakRef already present!");
			}
			log.jyWeakRef = System.currentTimeMillis();
		}
	}

	public static void clearJyWeakRef(long handle) {
		ObjectLog log = nativeObjects.get(handle);
		if (log != null) {
			if (log.jyWeakRef == 0) {
				System.out.println("clearJyWeakRef: No JyWeakRef present!");
			}
			log.jyWeakRef = 0;
		}
	}

	public static void notifyResurrect(long handle, PyObject obj) {
		ObjectLog log = nativeObjects.get(handle);
		if (log != null) {
			// We keep this warning silent for now, since it appears to
			// happen due to natural Java-GC-behavior.
//			if (log.JyNIFree == 0) {
//				System.out.println("Resurrection on non-deleted object!");
//			}
			log.JyNIFree = 0;
			log.object = new WeakReference<>(obj);
		}
	}

	public static void notifyJyNIFree(long handle) {
		ObjectLog log = nativeObjects.get(handle);
		if (log != null) {
			if (log.JyNIFree != 0) {
				System.out.println("JyNIFree already present!");
			}
			log.JyNIFree = System.currentTimeMillis();
			//System.out.println("jnfr: "+log.JyNIFree);
		}
	}

	public static void notifyGCRun() {
		lastGCRun = System.currentTimeMillis();
		//System.out.println("lgcr: "+lastGCRun);
	}

	public static void notifyClearReferences(long[] refs, boolean validGraph) {
		lastClearRefTime = System.currentTimeMillis();
		//System.out.println("notifyClearReferences: validGraph? "+validGraph);
		lastClearGraphValid = validGraph;
//		for (long l: refs) {
//			ObjectLog log = nativeObjects.get(l);
//			if (log != null) {
//				if (log.JyNIFree != 0) {
//				}
//				log.JyNIFree = System.currentTimeMillis();
//			}
//		}
	}

//	public static void addAction(short action, PyObject obj) {
//		
//	}

	public static void addNativeAction(NativeAction nvac) {
//	public static void addNativeAction(short action, PyObject obj, long nativeRef1, long nativeRef2,
//			ObjectLogDebugInfo debugInfo) {
			//String nativeType, String cMethod, String cFile, int line, String nativeRepr) {
		//System.out.println(actionToString(action)+" - "+action+" ("+cMethod+", "+nativeRef1+")");
		ObjectLog log = nativeObjects.get(nvac.nativeRef1);
		if (log == null) {
			log = new ObjectLog(nvac.nativeRef1);
			nativeObjects.put(nvac.nativeRef1, log);
		} else if (log.nativeFree != 0) {
			ObjectLog log2 = new ObjectLog(nvac.nativeRef1);
			log2.previousLife = log;
			nativeObjects.put(nvac.nativeRef1, log2);
			log = log2;
			//System.out.println("Replace log for reference "+nativeRef1+" ("+nativeType+") freed? "+log.nativeFree);
		}
		try {
			log.updateInfo(nvac);
		} catch (ObjectLogException ole) {
			System.err.println(ole.getMessage());
		}

		//System.out.println(line);
		//System.out.println("addAction: "+action+" ("+cMethod+")");
		/*if ((action & NATIVE_MASK) == 0) {
			System.out.println("addAction error: Calling addNativeAction with non-native flags!");
			return;
		}
		if ((action & MEMORY_MASK) != 0) {
			if ((action & INC_MASK) != 0 && (action &  DEC_MASK) != 0) return; //realloc currently not observed
			if ((action & DEC_MASK) != 0) {
				MemAction ma = nativeAllocatedObjects.get(nativeRef1);
				if (ma == null) {
					System.out.println("addAction error: Freeing non-allocated object!");
					return;
				} else {
				}
			} else if ((action & INC_MASK) != 0) {
			}
		}*/
	}

	public static List<ObjectLog> getCurrentNativeLeaks() {
		ArrayList<ObjectLog> tmp = new ArrayList<>(nativeObjects.values());
		Map<Long, Object> lsrc = new HashMap<>(tmp.size());
		for (ObjectLog obl: tmp) {
			lsrc.put(obl.nativeRef, obl);
		}
		Map<Long, Object> ldest = new HashMap<>();
		moveStaticallyReachable(lsrc, ldest);
		ArrayList<ObjectLog> result = new ArrayList<>();
		for (ObjectLog log: tmp) {
			if (lsrc.containsKey(log.nativeRef) && log.isLeak()) {
				result.add(log);
			}
		}
		return result;
	}

	public static void declareLeaksPermanent(Collection<ObjectLog> leaks) {
		for (ObjectLog l: leaks) {
			nativeObjects.remove(l.initialNativeRef);
			permanentLeaks.put(l.initialNativeRef, l);
		}
	}

	public static void listLeaks() {
		List<ObjectLog> tmp = getCurrentNativeLeaks();
		boolean leaksFound = false;
		for (ObjectLog log: tmp) {
			//if (lsrc.containsKey(log.nativeRef) && log.isLeak()) {
			if (!leaksFound) {
				leaksFound = true;
				System.out.println("Current native leaks:");
			}
			log.updatePyObject();
			System.out.println(log);
			//} //else if (ldest.containsKey(log.nativeRef) && log.isLeak()) {
//				if (!leaksFound) {
//					leaksFound = true;
//					System.out.println("Current native leaks:");
//				}
//				log.updatePyObject();
//				System.out.println("  ["+log+"]");
//			}
		}
		if (!leaksFound) System.out.println("no leaks recorded");
	}

	public static void listWouldDeleteNative() {
		ArrayList<ObjectLog> tmp = new ArrayList<>(nativeObjects.values());
		boolean found = false;
		for (ObjectLog log: tmp) {
			if (log.JyNIFree != 0 && log.JyNIFree >= lastGCRun) {
				if (!found) {
					found = true;
					System.out.println("Native delete-attempts:");
				}
				log.updatePyObject();
				System.out.println(log);
			}
		}
		if (!found) System.out.println("no native delete-attempts recorded");
	}

	public static void listFreed() {
		ArrayList<ObjectLog> tmp = new ArrayList<>(nativeObjects.values());
		for (ObjectLog log: tmp) {
			if (log.nativeFree != 0) {
				log.updatePyObject();
				System.out.println(log);
			}
		}
	}

	public static void listFreeStatus(String typeName) {
		ArrayList<ObjectLog> tmp = new ArrayList<>(nativeObjects.values());
		for (ObjectLog log: tmp) {
			if (log.getNativeTypeName().equals(typeName)) {
				log.updatePyObject();
				System.out.println(log.nativeRef+": "+log.nativeFree+" "+log.getJythonObject());
			}
		}
	}

	public static ObjectLog getLog(long nativeReference) {
		return nativeObjects.get(nativeReference);
	}

	public static void listImmortal() {
		ArrayList<ObjectLog> tmp = new ArrayList<>(nativeObjects.values());
		for (ObjectLog log: tmp) {
			if (log.immortal != 0) {
				log.updatePyObject();
				System.out.println(log);
			}
		}
	}

	public static void listAll() {
		ArrayList<ObjectLog> tmp = new ArrayList<>(nativeObjects.values());
		for (ObjectLog log: tmp) {
			log.updatePyObject();
			System.out.println(log);
		}
	}

	static class ReachableJyGCHeadFinder implements Visitproc, JyVisitproc {
		Stack<Object> searchList = new Stack<>();
		IdentityHashMap<Object, Object> alreadySearched = new IdentityHashMap<>();
		Map<Long, Object> src, dest;

		public ReachableJyGCHeadFinder(Map<Long, Object> src, Map<Long, Object> dest) {
			this.src = src;
			this.dest = dest;
		}

		public void addToSearch(Object obj) {
			if ((obj instanceof Traverseproc || obj instanceof TraversableGCHead
					|| obj instanceof PyObjectGCHead)
					&& !alreadySearched.containsKey(obj))
				searchList.push(obj);
			else
				move(obj);
		}

		void move(Object obj) {
			//System.out.println("Move obj "+obj.getClass());
			if (obj instanceof JyGCHead) {
				long handle = ((JyGCHead) obj).getHandle();
				Object op = src.remove(handle);
				if (op != null) dest.put(handle, op);
			} else if (obj instanceof PyObject) {
				long handle = JyNI.lookupNativeHandle((PyObject) obj);
				if (handle != 0) {
					Object op = src.remove(handle);
					if (op != null) dest.put(handle, op);
				}
			}
		}

		public int search() {
			int counter = 0;
			Object obj;
			while (!searchList.isEmpty()) {
				obj = searchList.pop();
				alreadySearched.put(obj, obj);
				move(obj);
//				if (obj instanceof CStubSimpleGCHead) {
//					System.out.println("expl CStubSimpleGCHead");
//				}
				if (obj instanceof Traverseproc)
					((Traverseproc) obj).traverse(this, null);
				if (obj instanceof TraversableGCHead)
					((TraversableGCHead) obj).jyTraverse(this, null);
				if (obj instanceof PyObjectGCHead) {
					//System.out.println("expl PyObjectGCHead");
					PyObject pop = ((PyObjectGCHead) obj).getPyObject();
					if (pop != null) {
						//System.out.println("Explore single object: "+pop.getType().getName());
						addToSearch(pop);
					} //else {
//						System.out.println("pop null "+obj.getClass());
//					}
				}
			}
			return counter;
		}

		public int jyVisit(JyGCHead object, Object arg) {
//			long handle = object.getHandle();
//			Object op = src.remove(handle);
//			if (op != null) dest.put(handle, op);
			addToSearch(object);
			return 0;
		}

		public int visit(PyObject object, Object arg) {
//			if (object instanceof JyGCHead) {
//				long handle = ((JyGCHead) object).getHandle();
//				Object op = src.remove(handle);
//				if (op != null) dest.put(handle, op);
//			} else {
//				long handle = JyNI.lookupNativeHandle(object);
//				if (handle != 0) {
//					Object op = src.remove(handle);
//					if (op != null) dest.put(handle, op);
//				}
//			}
			addToSearch(object);
			return 0;
		}
	}

	public static void moveStaticallyReachable(Map<Long, Object> src, Map<Long, Object> dest) {
		ReachableJyGCHeadFinder searcher = new ReachableJyGCHeadFinder(src, dest);
//		for (PyDictionary obj: JyNI.nativeStaticTypeDicts.values())
//			searcher.addToSearch(obj);
		for (JyGCHead jgh: JyNI.nativeStaticPyObjectHeads.values())
			searcher.addToSearch(jgh);
		for (JyNIModuleInfo obj: JyNIImporter.dynModules.values())
			searcher.addToSearch(obj.module);
		if (JyNI.nativeInternedStrings != null)
			searcher.addToSearch(JyNI.nativeInternedStrings);
		searcher.search();
	}
}
