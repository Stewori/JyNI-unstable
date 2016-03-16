/* This File is based on exceptions.c from CPython 2.7.5 release.
 * It has been modified to suit JyNI needs.
 *
 *
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


/*
 * New exceptions.c written in Iceland by Richard Jones and Georg Brandl.
 *
 * Thanks go to Tim Peters and Michael Hudson for debugging.
 */

#define PY_SSIZE_T_CLEAN
#include <JyNI.h>
#include "structmember_JyNI.h"
#include "osdefs_JyNI.h"

#define EXC_MODULE_NAME "exceptions."

/* NOTE: If the exception class hierarchy changes, don't forget to update
 * Lib/test/exception_hierarchy.txt
 */

PyDoc_STRVAR(exceptions_doc, "Python's standard exception class hierarchy.\n\
\n\
Exceptions found here are defined both in the exceptions module and the\n\
built-in namespace.  It is recommended that user-defined exceptions\n\
inherit from Exception.  See the documentation for the exception\n\
inheritance hierarchy.\n\
");

/*
 *	BaseException
 */
static PyObject *
BaseException_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	PyBaseExceptionObject *self;
	//JyNI-note: This should call PyType_GenericAlloc actually:
	//(and that's fine, since we adjusted that method to allocate space for JyObject)
	self = (PyBaseExceptionObject *)type->tp_alloc(type, 0);
	if (!self)
		return NULL;
	/* the dict is created on the fly in PyObject_GenericSetAttr */
	self->message = self->dict = NULL;

	self->args = PyTuple_New(0);
	if (!self->args) {
		Py_DECREF(self);
		return NULL;
	}

	self->message = PyString_FromString("");
	if (!self->message) {
		Py_DECREF(self);
		return NULL;
	}

	return (PyObject *)self;
}

static int
BaseException_init(PyBaseExceptionObject *self, PyObject *args, PyObject *kwds)
{
	if (!_PyArg_NoKeywords(Py_TYPE(self)->tp_name, kwds))
		return -1;

	env(-1);
	jobject jargs = (*env)->NewObjectArray(env, PyTuple_GET_SIZE(args), pyObjectClass, NULL);
	Py_ssize_t i;
	for (i = 0; i < PyTuple_GET_SIZE(args); ++i)
		(*env)->SetObjectArrayElement(env, jargs, i,
			JyNI_JythonPyObject_FromPyObject(PyTuple_GET_ITEM(args, i)));
	(*env)->CallVoidMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) self),
			pyBaseException__init__, jargs, length0StringArray);
//	Py_DECREF(self->args);
//	self->args = args;
//	Py_INCREF(self->args);
//
//	if (PyTuple_GET_SIZE(self->args) == 1) {
//		Py_CLEAR(self->message);
//		self->message = PyTuple_GET_ITEM(self->args, 0);
//		Py_INCREF(self->message);
//	}
	return 0;
}

static int
BaseException_clear(PyBaseExceptionObject *self)
{
// JyNI-note: For now, we don't clear Jython-side.
//	Py_CLEAR(self->dict);
//	Py_CLEAR(self->args);
//	Py_CLEAR(self->message);
	return 0;
}

static void
BaseException_dealloc(PyBaseExceptionObject *self)
{
	//_JyNI_GC_UNTRACK(self);
	BaseException_clear(self);
	Py_TYPE(self)->tp_free((PyObject *)self);
	//JyNI-note: tp_free should be okay here since we adjusted all
	//tp_free methods of potential supertypes.
}

//static int
//BaseException_traverse(PyBaseExceptionObject *self, visitproc visit, void *arg)
//{
//	Py_VISIT(self->dict);
//	Py_VISIT(self->args);
//	Py_VISIT(self->message);
//	return 0;
//}

static PyObject *
BaseException_str(PyBaseExceptionObject *self)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env,
		JyNI_JythonPyObject_FromPyObject((PyObject*) self), pyObject__str__));
//	PyObject *out;
//
//	switch (PyTuple_GET_SIZE(self->args)) {
//	case 0:
//		out = PyString_FromString("");
//		break;
//	case 1:
//		out = PyObject_Str(PyTuple_GET_ITEM(self->args, 0));
//		break;
//	default:
//		out = PyObject_Str(self->args);
//		break;
//	}
//
//	return out;
}

#ifdef Py_USING_UNICODE
static PyObject *
BaseException_unicode(PyBaseExceptionObject *self)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env,
		JyNI_JythonPyObject_FromPyObject((PyObject*) self), pyBaseException__unicode__));
//	PyObject *out;
//
//	/* issue6108: if __str__ has been overridden in the subclass, unicode()
//	   should return the message returned by __str__ as used to happen
//	   before this method was implemented. */
//	if (Py_TYPE(self)->tp_str != (reprfunc)BaseException_str) {
//		PyObject *str;
//		/* Unlike PyObject_Str, tp_str can return unicode (i.e. return the
//		   equivalent of unicode(e.__str__()) instead of unicode(str(e))). */
//		str = Py_TYPE(self)->tp_str((PyObject*)self);
//		if (str == NULL)
//			return NULL;
//		out = PyObject_Unicode(str);
//		Py_DECREF(str);
//		return out;
//	}
//
//	switch (PyTuple_GET_SIZE(self->args)) {
//	case 0:
//		out = PyUnicode_FromString("");
//		break;
//	case 1:
//		out = PyObject_Unicode(PyTuple_GET_ITEM(self->args, 0));
//		break;
//	default:
//		out = PyObject_Unicode(self->args);
//		break;
//	}
//
//	return out;
}
#endif

static PyObject *
BaseException_repr(PyBaseExceptionObject *self)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) self), pyObject__repr__));
//	PyObject *repr_suffix;
//	PyObject *repr;
//	char *name;
//	char *dot;
//
//	repr_suffix = PyObject_Repr(self->args);
//	if (!repr_suffix)
//		return NULL;
//
//	name = (char *)Py_TYPE(self)->tp_name;
//	dot = strrchr(name, '.');
//	if (dot != NULL) name = dot+1;
//
//	repr = PyString_FromString(name);
//	if (!repr) {
//		Py_DECREF(repr_suffix);
//		return NULL;
//	}
//
//	PyString_ConcatAndDel(&repr, repr_suffix);
//	return repr;
}

/* Pickling support */
static PyObject *
BaseException_reduce(PyBaseExceptionObject *self)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) self), pyObject__reduce__));
//	if (self->args && self->dict)
//		return PyTuple_Pack(3, Py_TYPE(self), self->args, self->dict);
//	else
//		return PyTuple_Pack(2, Py_TYPE(self), self->args);
}

/*
 * Needed for backward compatibility, since exceptions used to store
 * all their attributes in the __dict__. Code is taken from cPickle's
 * load_build function.
 */
static PyObject *
BaseException_setstate(PyObject *self, PyObject *state)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(self), pyBaseException__setstate__,
		JyNI_JythonPyObject_FromPyObject(state)));
//	PyObject *d_key, *d_value;
//	Py_ssize_t i = 0;
//
//	if (state != Py_None) {
//		if (!PyDict_Check(state)) {
//			PyErr_SetString(PyExc_TypeError, "state is not a dictionary");
//			return NULL;
//		}
//		while (PyDict_Next(state, &i, &d_key, &d_value)) {
//			if (PyObject_SetAttr(self, d_key, d_value) < 0)
//				return NULL;
//		}
//	}
//	Py_RETURN_NONE;
}


static PyMethodDef BaseException_methods[] = {
   {"__reduce__", (PyCFunction)BaseException_reduce, METH_NOARGS },
   {"__setstate__", (PyCFunction)BaseException_setstate, METH_O },
#ifdef Py_USING_UNICODE
   {"__unicode__", (PyCFunction)BaseException_unicode, METH_NOARGS },
#endif
   {NULL, NULL, 0, NULL},
};



static PyObject *
BaseException_getitem(PyBaseExceptionObject *self, Py_ssize_t index)
{
	env(NULL);
	jobject jself = JyNI_JythonPyObject_FromPyObject((PyObject*) self);
	ENTER_SubtypeLoop_Safe_ModePy(jself, self, __finditem__)
	jobject result = (*env)->CallObjectMethod(env, jself, JMID(__finditem__),
			JyNI_JythonPyObject_FromPyObject(PyInt_FromSsize_t(index)));
	LEAVE_SubtypeLoop_Safe_ModePy(jself, __finditem__)
	return JyNI_PyObject_FromJythonPyObject(result);
//	if (PyErr_WarnPy3k("__getitem__ not supported for exception "
//					   "classes in 3.x; use args attribute", 1) < 0)
//		return NULL;
//	return PySequence_GetItem(self->args, index);
}

static PyObject *
BaseException_getslice(PyBaseExceptionObject *self,
						Py_ssize_t start, Py_ssize_t stop)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env,
		JyNI_JythonPyObject_FromPyObject((PyObject*) self),
		pyObject__getslice__,
		JyNI_JythonPyObject_FromPyObject(PyInt_FromSsize_t(start)),
		JyNI_JythonPyObject_FromPyObject(PyInt_FromSsize_t(stop))));
//	if (PyErr_WarnPy3k("__getslice__ not supported for exception "
//					   "classes in 3.x; use args attribute", 1) < 0)
//		return NULL;
//	return PySequence_GetSlice(self->args, start, stop);
}

static PySequenceMethods BaseException_as_sequence = {
	0,					  /* sq_length; */
	0,					  /* sq_concat; */
	0,					  /* sq_repeat; */
	(ssizeargfunc)BaseException_getitem,  /* sq_item; */
	(ssizessizeargfunc)BaseException_getslice,  /* sq_slice; */
	0,					  /* sq_ass_item; */
	0,					  /* sq_ass_slice; */
	0,					  /* sq_contains; */
	0,					  /* sq_inplace_concat; */
	0					   /* sq_inplace_repeat; */
};

static PyObject *
BaseException_get_dict(PyBaseExceptionObject *self)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env,
		JyNI_JythonPyObject_FromPyObject((PyObject*) self),
		pyObjectGetDict));
//	if (self->dict == NULL) {
//		self->dict = PyDict_New();
//		if (!self->dict)
//			return NULL;
//	}
//	Py_INCREF(self->dict);
//	return self->dict;
}

static int
BaseException_set_dict(PyBaseExceptionObject *self, PyObject *val)
{
	if (val == NULL) {
		PyErr_SetString(PyExc_TypeError, "__dict__ may not be deleted");
		return -1;
	}
	if (!PyDict_Check(val)) {
		PyErr_SetString(PyExc_TypeError, "__dict__ must be a dictionary");
		return -1;
	}
	env(NULL);
	(*env)->CallVoidMethod(env,
		JyNI_JythonPyObject_FromPyObject((PyObject*) self),
		pyObjectSetDict, JyNI_JythonPyObject_FromPyObject(val));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
//	Py_CLEAR(self->dict);
//	Py_INCREF(val);
//	self->dict = val;
	return 0;
}

static PyObject *
BaseException_get_args(PyBaseExceptionObject *self)
{
	env(NULL);
	jobject jargs = (*env)->GetObjectField(env, JyNI_JythonPyObject_FromPyObject((PyObject*) self),
			pyBaseExceptionArgs);
	if (jargs == NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	PyObject* args = JyNI_PyObject_FromJythonPyObject(jargs);
	Py_INCREF(args); //JyNI todo: Check, whether incref is needed here
	return args;
//	if (self->args == NULL) {
//		Py_INCREF(Py_None);
//		return Py_None;
//	}
//	Py_INCREF(self->args);
//	return self->args;
}

static int
BaseException_set_args(PyBaseExceptionObject *self, PyObject *val)
{
	env(-1);
	if (val == NULL) {
		PyErr_SetString(PyExc_TypeError, "args may not be deleted");
		return -1;
	}
	jobject jself = JyNI_JythonPyObject_FromPyObject((PyObject*) self);
	//JyNI_Py_CLEAR((*env)->GetObjectField(env, jself, pyBaseExceptionArgs)); //JyNI todo: Check whether Py_Clear would be needed here
	(*env)->CallVoidMethod(env, jself,
		pyBaseExceptionSetArgs, JyNI_JythonPyObject_FromPyObject(val));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}

//	PyObject *seq;
//	seq = PySequence_Tuple(val);
//	if (!seq)
//		return -1;
//	Py_CLEAR(self->args);
//	self->args = seq;
	return 0;
}

static PyObject *
BaseException_get_message(PyBaseExceptionObject *self)
{
	env(NULL);
	PyObject* msg = JyNI_PyObject_FromJythonPyObject((*env)->CallObjectMethod(env,
		JyNI_JythonPyObject_FromPyObject((PyObject*) self), pyBaseExceptionGetMessage));
	Py_INCREF(msg);
	return msg;

//	PyObject *msg;
//
//	/* if "message" is in self->dict, accessing a user-set message attribute */
//	if (self->dict &&
//		(msg = PyDict_GetItemString(self->dict, "message"))) {
//		Py_INCREF(msg);
//		return msg;
//	}
//
//	if (self->message == NULL) {
//		PyErr_SetString(PyExc_AttributeError, "message attribute was deleted");
//		return NULL;
//	}
//
//	/* accessing the deprecated "builtin" message attribute of Exception */
//	if (PyErr_WarnEx(PyExc_DeprecationWarning,
//					 "BaseException.message has been deprecated as "
//					 "of Python 2.6", 1) < 0)
//		return NULL;
//
//	Py_INCREF(self->message);
//	return self->message;
}

static int
BaseException_set_message(PyBaseExceptionObject *self, PyObject *val)
{
	env(-1);
	jobject jself = JyNI_JythonPyObject_FromPyObject((PyObject*) self);
	//jobject msg = (*env)->CallObjectMethod(env, jself, pyBaseExceptionGetMessage);
	//if (msg) JyNI_Py_CLEAR(msg); //JyNI todo: Check whether Py_Clear would be needed here
	(*env)->CallVoidMethod(env, jself,
			pyBaseExceptionSetMessage, JyNI_JythonPyObject_FromPyObject(val));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
//	/* if val is NULL, delete the message attribute */
//	if (val == NULL) {
//		if (self->dict && PyDict_GetItemString(self->dict, "message")) {
//			if (PyDict_DelItemString(self->dict, "message") < 0)
//				return -1;
//		}
//		Py_CLEAR(self->message);
//		return 0;
//	}
//
//	/* else set it in __dict__, but may need to create the dict first */
//	if (self->dict == NULL) {
//		self->dict = PyDict_New();
//		if (!self->dict)
//			return -1;
//	}
//	return PyDict_SetItemString(self->dict, "message", val);
}

static PyGetSetDef BaseException_getset[] = {
	{"__dict__", (getter)BaseException_get_dict, (setter)BaseException_set_dict},
	{"args", (getter)BaseException_get_args, (setter)BaseException_set_args},
	{"message", (getter)BaseException_get_message,
			(setter)BaseException_set_message},
	{NULL},
};


static PyTypeObject _PyExc_BaseException = {
	PyObject_HEAD_INIT(NULL)
	0,                                     /*ob_size*/
	EXC_MODULE_NAME "BaseException",       /*tp_name*/
	sizeof(PyBaseExceptionObject),         /*tp_basicsize*/
	0,                                     /*tp_itemsize*/
	(destructor)BaseException_dealloc,     /*tp_dealloc*/
	0,                                     /*tp_print*/
	0,                                     /*tp_getattr*/
	0,                                     /*tp_setattr*/
	0,                                     /*tp_compare*/
	(reprfunc)BaseException_repr,          /*tp_repr*/
	0,                                     /*tp_as_number*/
	&BaseException_as_sequence,            /*tp_as_sequence*/
	0,                                     /*tp_as_mapping*/
	0,                                     /*tp_hash */
	0,                                     /*tp_call*/
	(reprfunc)BaseException_str,           /*tp_str*/
	PyObject_GenericGetAttr,               /*tp_getattro*/
	PyObject_GenericSetAttr,               /*tp_setattro*/
	0,                                     /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | //Py_TPFLAGS_HAVE_GC |
		Py_TPFLAGS_BASE_EXC_SUBCLASS,      /*tp_flags*/
	PyDoc_STR("Common base class for all exceptions"), /* tp_doc */
	0,//(traverseproc)BaseException_traverse, /* tp_traverse */
	(inquiry)BaseException_clear,          /* tp_clear */
	0,                                     /* tp_richcompare */
	0,                                     /* tp_weaklistoffset */
	0,                                     /* tp_iter */
	0,                                     /* tp_iternext */
	BaseException_methods,                 /* tp_methods */
	0,                                     /* tp_members */
	BaseException_getset,                  /* tp_getset */
	0,                                     /* tp_base */
	0,                                     /* tp_dict */
	0,                                     /* tp_descr_get */
	0,                                     /* tp_descr_set */
	offsetof(PyBaseExceptionObject, dict), /* tp_dictoffset */
	(initproc)BaseException_init,          /* tp_init */
	0,                                     /* tp_alloc */
	BaseException_new,                     /* tp_new */
};
/* the CPython API expects exceptions to be (PyObject *) - both a hold-over
from the previous implementation and also allowing Python objects to be used
in the API */
PyObject *PyExc_BaseException = (PyObject *)&_PyExc_BaseException;

/* note these macros omit the last semicolon so the macro invocation may
 * include it and not look strange.
 */
#define SimpleExtendsException(EXCBASE, EXCNAME, EXCDOC) \
static PyTypeObject _PyExc_ ## EXCNAME = { \
	PyObject_HEAD_INIT(NULL) \
	0, \
	EXC_MODULE_NAME # EXCNAME, \
	sizeof(PyBaseExceptionObject), \
	0, (destructor)BaseException_dealloc, 0, 0, 0, 0, 0, 0, 0, \
	0, 0, 0, 0, 0, 0, 0, \
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE /*| Py_TPFLAGS_HAVE_GC*/, \
	PyDoc_STR(EXCDOC), 0 /*(traverseproc)BaseException_traverse*/, \
	(inquiry)BaseException_clear, 0, 0, 0, 0, 0, 0, 0, &_ ## EXCBASE, \
	0, 0, 0, offsetof(PyBaseExceptionObject, dict), \
	(initproc)BaseException_init, 0, BaseException_new,\
}; \
PyObject *PyExc_ ## EXCNAME = (PyObject *)&_PyExc_ ## EXCNAME

#define MiddlingExtendsException(EXCBASE, EXCNAME, EXCSTORE, EXCDOC) \
static PyTypeObject _PyExc_ ## EXCNAME = { \
	PyObject_HEAD_INIT(NULL) \
	0, \
	EXC_MODULE_NAME # EXCNAME, \
	sizeof(Py ## EXCSTORE ## Object), \
	0, (destructor)EXCSTORE ## _dealloc, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
	0, 0, 0, 0, 0, \
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE /*| Py_TPFLAGS_HAVE_GC*/, \
	PyDoc_STR(EXCDOC), 0 /*(traverseproc)EXCSTORE ## _traverse*/, \
	(inquiry)EXCSTORE ## _clear, 0, 0, 0, 0, 0, 0, 0, &_ ## EXCBASE, \
	0, 0, 0, offsetof(Py ## EXCSTORE ## Object, dict), \
	(initproc)EXCSTORE ## _init, 0, BaseException_new,\
}; \
PyObject *PyExc_ ## EXCNAME = (PyObject *)&_PyExc_ ## EXCNAME

#define ComplexExtendsException(EXCBASE, EXCNAME, EXCSTORE, EXCDEALLOC, EXCMETHODS, EXCMEMBERS, EXCSTR, EXCDOC) \
static PyTypeObject _PyExc_ ## EXCNAME = { \
	PyObject_HEAD_INIT(NULL) \
	0, \
	EXC_MODULE_NAME # EXCNAME, \
	sizeof(Py ## EXCSTORE ## Object), 0, \
	(destructor)EXCSTORE ## _dealloc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
	(reprfunc)EXCSTR, 0, 0, 0, \
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE /*| Py_TPFLAGS_HAVE_GC*/, \
	PyDoc_STR(EXCDOC), 0 /*(traverseproc)EXCSTORE ## _traverse*/, \
	(inquiry)EXCSTORE ## _clear, 0, 0, 0, 0, EXCMETHODS, \
	EXCMEMBERS, 0, &_ ## EXCBASE, \
	0, 0, 0, offsetof(Py ## EXCSTORE ## Object, dict), \
	(initproc)EXCSTORE ## _init, 0, BaseException_new,\
}; \
PyObject *PyExc_ ## EXCNAME = (PyObject *)&_PyExc_ ## EXCNAME

//Added for JyNI (uses EXCGETSET instead of EXCMEMBERS):
#define ComplexExtendsException2(EXCBASE, EXCNAME, EXCSTORE, EXCDEALLOC, EXCMETHODS, EXCGETSETS, EXCSTR, EXCDOC) \
static PyTypeObject _PyExc_ ## EXCNAME = { \
	PyObject_HEAD_INIT(NULL) \
	0, \
	EXC_MODULE_NAME # EXCNAME, \
	sizeof(Py ## EXCSTORE ## Object), 0, \
	(destructor)EXCSTORE ## _dealloc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
	(reprfunc)EXCSTR, 0, 0, 0, \
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE /*| Py_TPFLAGS_HAVE_GC*/, \
	PyDoc_STR(EXCDOC), 0 /*(traverseproc)EXCSTORE ## _traverse*/, \
	(inquiry)EXCSTORE ## _clear, 0, 0, 0, 0, EXCMETHODS, \
	/*EXCMEMBERS*/ 0, EXCGETSETS, &_ ## EXCBASE, \
	0, 0, 0, offsetof(Py ## EXCSTORE ## Object, dict), \
	(initproc)EXCSTORE ## _init, 0, BaseException_new,\
}; \
PyObject *PyExc_ ## EXCNAME = (PyObject *)&_PyExc_ ## EXCNAME

/*
 *	Exception extends BaseException
 */
SimpleExtendsException(PyExc_BaseException, Exception,
					   "Common base class for all non-exit exceptions.");


/*
 *	StandardError extends Exception
 */
SimpleExtendsException(PyExc_Exception, StandardError,
	"Base class for all standard Python exceptions that do not represent\n"
	"interpreter exiting.");


/*
 *	TypeError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, TypeError,
					   "Inappropriate argument type.");


/*
 *	StopIteration extends Exception
 */
SimpleExtendsException(PyExc_Exception, StopIteration,
					   "Signal the end from iterator.next().");


/*
 *	GeneratorExit extends BaseException
 */
SimpleExtendsException(PyExc_BaseException, GeneratorExit,
					   "Request that a generator exit.");


/*
 *	SystemExit extends BaseException
 */
static int
SystemExit_init(PySystemExitObject *self, PyObject *args, PyObject *kwds)
{
	env(-1);
	jobject jdict = JyNI_JythonPyObject_FromPyObject(kwds);
	ENTER_SubtypeLoop_Safe_ModePy(jdict, kwds, __len__)
	jint dictSize = (*env)->CallIntMethod(env, jdict, JMID(__len__));
	LEAVE_SubtypeLoop_Safe_ModePy(jdict, __len__)
	jobject jargs = (*env)->NewObjectArray(env,
		PyTuple_GET_SIZE(args)+dictSize,
		stringClass, NULL);
	int i;
	for (i = 0; i < PyTuple_GET_SIZE(args); ++i)
		(*env)->SetObjectArrayElement(env, jargs, i,
			JyNI_JythonPyObject_FromPyObject(PyTuple_GET_ITEM(args, i)));
	jobject jkw;
	if (dictSize > 0) jkw = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_prepareKeywordArgs, jargs, jdict);
	else jkw = length0StringArray;
	(*env)->CallStaticVoidMethod(env, exceptionsClass,
			exceptionsSystemExit__init__, JyNI_JythonPyObject_FromPyObject((PyObject*) self), jargs, jkw);
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}

//	Py_ssize_t size = PyTuple_GET_SIZE(args);
//
//	if (BaseException_init((PyBaseExceptionObject *)self, args, kwds) == -1)
//		return -1;
//
//	if (size == 0)
//		return 0;
//	Py_CLEAR(self->code);
//	if (size == 1)
//		self->code = PyTuple_GET_ITEM(args, 0);
//	else if (size > 1)
//		self->code = args;
//	Py_INCREF(self->code);
	return 0;
}

static int
SystemExit_clear(PySystemExitObject *self)
{
	//Py_CLEAR(self->code);
	return BaseException_clear((PyBaseExceptionObject *)self);
}

static void
SystemExit_dealloc(PySystemExitObject *self)
{
	//_JyNI_GC_UNTRACK(self);
	SystemExit_clear(self);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

//static int
//SystemExit_traverse(PySystemExitObject *self, visitproc visit, void *arg)
//{
//	Py_VISIT(self->code);
//	return BaseException_traverse((PyBaseExceptionObject *)self, visit, arg);
//}

//static PyMemberDef SystemExit_members[] = {
//	{"code", T_OBJECT, offsetof(PySystemExitObject, code), 0, PyDoc_STR("exception code")},
//	{NULL}  /* Sentinel */
//};

//Written for JyNI to provide the "code"-member as getset (since members are only possible in mirror mode)
static PyObject *
SystemExit_get_code(PySystemExitObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "code")));
}

static int
SystemExit_set_code(PySystemExitObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "code"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyGetSetDef SystemExit_getsets [] = {
	{"code", (getter)SystemExit_get_code, (setter)SystemExit_set_code, PyDoc_STR("exception code")},
	{NULL} /* Sentinel */
};

//ComplexExtendsException(PyExc_BaseException, SystemExit, SystemExit,
//						SystemExit_dealloc, 0, /*SystemExit_members*/ 0, 0,
//						"Request to exit from the interpreter.");
ComplexExtendsException2(PyExc_BaseException, SystemExit, SystemExit,
						SystemExit_dealloc, 0, SystemExit_getsets, 0,
						"Request to exit from the interpreter.");

/*
 *	KeyboardInterrupt extends BaseException
 */
SimpleExtendsException(PyExc_BaseException, KeyboardInterrupt,
					   "Program interrupted by user.");


/*
 *	ImportError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, ImportError,
		  "Import can't find module, or can't find name in module.");


/*
 *	EnvironmentError extends StandardError
 */

/* Where a function has a single filename, such as open() or some
 * of the os module functions, PyErr_SetFromErrnoWithFilename() is
 * called, giving a third argument which is the filename.  But, so
 * that old code using in-place unpacking doesn't break, e.g.:
 *
 * except IOError, (errno, strerror):
 *
 * we hack args so that it only contains two items.  This also
 * means we need our own __str__() which prints out the filename
 * when it was supplied.
 */
static int
EnvironmentError_init(PyEnvironmentErrorObject *self, PyObject *args, PyObject *kwds)
{
	env(-1);
	jobject jdict = JyNI_JythonPyObject_FromPyObject(kwds);
	ENTER_SubtypeLoop_Safe_ModePy(jdict, kwds, __len__)
	jint dictSize = (*env)->CallIntMethod(env, jdict, JMID(__len__));
	LEAVE_SubtypeLoop_Safe_ModePy(jdict, __len__)
	jobject jargs = (*env)->NewObjectArray(env,
		PyTuple_GET_SIZE(args)
		+dictSize,
		stringClass, NULL);
	int i;
	for (i = 0; i < PyTuple_GET_SIZE(args); ++i)
		(*env)->SetObjectArrayElement(env, jargs, i,
			JyNI_JythonPyObject_FromPyObject(PyTuple_GET_ITEM(args, i)));
	jobject jkw;
	if (dictSize > 0) jkw = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_prepareKeywordArgs, jargs, jdict);
	else jkw = length0StringArray;
	(*env)->CallStaticVoidMethod(env, exceptionsClass,
			exceptionsEnvironmentError__init__, JyNI_JythonPyObject_FromPyObject((PyObject*) self), jargs, jkw);
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
//	PyObject *myerrno = NULL, *strerror = NULL, *filename = NULL;
//	PyObject *subslice = NULL;
//
//	if (BaseException_init((PyBaseExceptionObject *)self, args, kwds) == -1)
//		return -1;
//
//	if (PyTuple_GET_SIZE(args) <= 1 || PyTuple_GET_SIZE(args) > 3) {
//		return 0;
//	}
//
//	if (!PyArg_UnpackTuple(args, "EnvironmentError", 2, 3,
//						   &myerrno, &strerror, &filename)) {
//		return -1;
//	}
//	Py_CLEAR(self->myerrno);	   /* replacing */
//	self->myerrno = myerrno;
//	Py_INCREF(self->myerrno);
//
//	Py_CLEAR(self->strerror);	  /* replacing */
//	self->strerror = strerror;
//	Py_INCREF(self->strerror);
//
//	/* self->filename will remain Py_None otherwise */
//	if (filename != NULL) {
//		Py_CLEAR(self->filename);	  /* replacing */
//		self->filename = filename;
//		Py_INCREF(self->filename);
//
//		subslice = PyTuple_GetSlice(args, 0, 2);
//		if (!subslice)
//			return -1;
//
//		Py_DECREF(self->args);  /* replacing args */
//		self->args = subslice;
//	}
	return 0;
}

static int
EnvironmentError_clear(PyEnvironmentErrorObject *self)
{
//	Py_CLEAR(self->myerrno);
//	Py_CLEAR(self->strerror);
//	Py_CLEAR(self->filename);
	return BaseException_clear((PyBaseExceptionObject *)self);
}

static void
EnvironmentError_dealloc(PyEnvironmentErrorObject *self)
{
	//_JyNI_GC_UNTRACK(self);
	EnvironmentError_clear(self);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

//static int
//EnvironmentError_traverse(PyEnvironmentErrorObject *self, visitproc visit,
//		void *arg)
//{
//	Py_VISIT(self->myerrno);
//	Py_VISIT(self->strerror);
//	Py_VISIT(self->filename);
//	return BaseException_traverse((PyBaseExceptionObject *)self, visit, arg);
//}

static PyObject *
EnvironmentError_str(PyEnvironmentErrorObject *self)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsEnvironmentError__str__,
				JyNI_JythonPyObject_FromPyObject((PyObject*) self), length0PyObjectArray, length0StringArray));
//	PyObject *rtnval = NULL;
//
//	if (self->filename) {
//		PyObject *fmt;
//		PyObject *repr;
//		PyObject *tuple;
//
//		fmt = PyString_FromString("[Errno %s] %s: %s");
//		if (!fmt)
//			return NULL;
//
//		repr = PyObject_Repr(self->filename);
//		if (!repr) {
//			Py_DECREF(fmt);
//			return NULL;
//		}
//		tuple = PyTuple_New(3);
//		if (!tuple) {
//			Py_DECREF(repr);
//			Py_DECREF(fmt);
//			return NULL;
//		}
//
//		if (self->myerrno) {
//			Py_INCREF(self->myerrno);
//			PyTuple_SET_ITEM(tuple, 0, self->myerrno);
//		}
//		else {
//			Py_INCREF(Py_None);
//			PyTuple_SET_ITEM(tuple, 0, Py_None);
//		}
//		if (self->strerror) {
//			Py_INCREF(self->strerror);
//			PyTuple_SET_ITEM(tuple, 1, self->strerror);
//		}
//		else {
//			Py_INCREF(Py_None);
//			PyTuple_SET_ITEM(tuple, 1, Py_None);
//		}
//
//		PyTuple_SET_ITEM(tuple, 2, repr);
//
//		rtnval = PyString_Format(fmt, tuple);
//
//		Py_DECREF(fmt);
//		Py_DECREF(tuple);
//	}
//	else if (self->myerrno && self->strerror) {
//		PyObject *fmt;
//		PyObject *tuple;
//
//		fmt = PyString_FromString("[Errno %s] %s");
//		if (!fmt)
//			return NULL;
//
//		tuple = PyTuple_New(2);
//		if (!tuple) {
//			Py_DECREF(fmt);
//			return NULL;
//		}
//
//		if (self->myerrno) {
//			Py_INCREF(self->myerrno);
//			PyTuple_SET_ITEM(tuple, 0, self->myerrno);
//		}
//		else {
//			Py_INCREF(Py_None);
//			PyTuple_SET_ITEM(tuple, 0, Py_None);
//		}
//		if (self->strerror) {
//			Py_INCREF(self->strerror);
//			PyTuple_SET_ITEM(tuple, 1, self->strerror);
//		}
//		else {
//			Py_INCREF(Py_None);
//			PyTuple_SET_ITEM(tuple, 1, Py_None);
//		}
//
//		rtnval = PyString_Format(fmt, tuple);
//
//		Py_DECREF(fmt);
//		Py_DECREF(tuple);
//	}
//	else
//		rtnval = BaseException_str((PyBaseExceptionObject *)self);
//
//	return rtnval;
}

//static PyMemberDef EnvironmentError_members[] = {
//	{"errno", T_OBJECT, offsetof(PyEnvironmentErrorObject, myerrno), 0,
//		PyDoc_STR("exception errno")},
//	{"strerror", T_OBJECT, offsetof(PyEnvironmentErrorObject, strerror), 0,
//		PyDoc_STR("exception strerror")},
//	{"filename", T_OBJECT, offsetof(PyEnvironmentErrorObject, filename), 0,
//		PyDoc_STR("exception filename")},
//	{NULL}  /* Sentinel */
//};

//Written for JyNI to provide the "code"-member as getset (since members are only possible in mirror mode)
static PyObject *
EnvironmentError_get_errno(PyEnvironmentErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "errno")));
}

static int
EnvironmentError_set_errno(PyEnvironmentErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "errno"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyObject *
EnvironmentError_get_strerror(PyEnvironmentErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "strerror")));
}

static int
EnvironmentError_set_strerror(PyEnvironmentErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "strerror"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyObject *
EnvironmentError_get_filename(PyEnvironmentErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "filename")));
}

static int
EnvironmentError_set_filename(PyEnvironmentErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "filename"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyGetSetDef EnvironmentError_getsets [] = {
	{"errno", (getter)EnvironmentError_get_errno, (setter)EnvironmentError_set_errno, PyDoc_STR("exception errno")},
	{"strerror", (getter)EnvironmentError_get_strerror, (setter)EnvironmentError_set_strerror, PyDoc_STR("exception strerror")},
	{"filename", (getter)EnvironmentError_get_filename, (setter)EnvironmentError_set_filename, PyDoc_STR("exception filename")},
	{NULL} /* Sentinel */
};

static PyObject *
EnvironmentError_reduce(PyEnvironmentErrorObject *self)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsEnvironmentError__reduce__,
		JyNI_JythonPyObject_FromPyObject((PyObject*) self), length0PyObjectArray, length0StringArray));
//	PyObject *args = self->args;
//	PyObject *res = NULL, *tmp;
//
//	/* self->args is only the first two real arguments if there was a
//	 * file name given to EnvironmentError. */
//	if (PyTuple_GET_SIZE(args) == 2 && self->filename) {
//		args = PyTuple_New(3);
//		if (!args)
//			return NULL;
//
//		tmp = PyTuple_GET_ITEM(self->args, 0);
//		Py_INCREF(tmp);
//		PyTuple_SET_ITEM(args, 0, tmp);
//
//		tmp = PyTuple_GET_ITEM(self->args, 1);
//		Py_INCREF(tmp);
//		PyTuple_SET_ITEM(args, 1, tmp);
//
//		Py_INCREF(self->filename);
//		PyTuple_SET_ITEM(args, 2, self->filename);
//	} else
//		Py_INCREF(args);
//
//	if (self->dict)
//		res = PyTuple_Pack(3, Py_TYPE(self), args, self->dict);
//	else
//		res = PyTuple_Pack(2, Py_TYPE(self), args);
//	Py_DECREF(args);
//	return res;
}

static PyMethodDef EnvironmentError_methods[] = {
	{"__reduce__", (PyCFunction)EnvironmentError_reduce, METH_NOARGS},
	{NULL}
};

//ComplexExtendsException(PyExc_StandardError, EnvironmentError,
//						EnvironmentError, EnvironmentError_dealloc,
//						EnvironmentError_methods, 0 /*EnvironmentError_members*/,
//						EnvironmentError_str,
//						"Base class for I/O related errors.");
ComplexExtendsException2(PyExc_StandardError, EnvironmentError,
						EnvironmentError, EnvironmentError_dealloc,
						EnvironmentError_methods, EnvironmentError_getsets,
						EnvironmentError_str,
						"Base class for I/O related errors.");

/*
 *	IOError extends EnvironmentError
 */
MiddlingExtendsException(PyExc_EnvironmentError, IOError,
						 EnvironmentError, "I/O operation failed.");


/*
 *	OSError extends EnvironmentError
 */
MiddlingExtendsException(PyExc_EnvironmentError, OSError,
						 EnvironmentError, "OS system call failed.");


/*
 *	WindowsError extends OSError
 */
//#ifdef MS_WINDOWS
//#include "errmap.h"
//
//static int
//WindowsError_clear(PyWindowsErrorObject *self)
//{
////	Py_CLEAR(self->myerrno);
////	Py_CLEAR(self->strerror);
////	Py_CLEAR(self->filename);
////	Py_CLEAR(self->winerror);
//	return BaseException_clear((PyBaseExceptionObject *)self);
//}
//
//static void
//WindowsError_dealloc(PyWindowsErrorObject *self)
//{
//	_JyNI_GC_UNTRACK(self);
//	WindowsError_clear(self);
//	Py_TYPE(self)->tp_free((PyObject *)self);
//}

//static int
//WindowsError_traverse(PyWindowsErrorObject *self, visitproc visit, void *arg)
//{
//	Py_VISIT(self->myerrno);
//	Py_VISIT(self->strerror);
//	Py_VISIT(self->filename);
//	Py_VISIT(self->winerror);
//	return BaseException_traverse((PyBaseExceptionObject *)self, visit, arg);
//}
//
//static int
//WindowsError_init(PyWindowsErrorObject *self, PyObject *args, PyObject *kwds)
//{
//	PyObject *o_errcode = NULL;
//	long errcode;
//	long posix_errno;
//
//	if (EnvironmentError_init((PyEnvironmentErrorObject *)self, args, kwds)
//			== -1)
//		return -1;
//
//	if (self->myerrno == NULL)
//		return 0;
//
//	/* Set errno to the POSIX errno, and winerror to the Win32
//	   error code. */
//	errcode = PyInt_AsLong(self->myerrno);
//	if (errcode == -1 && PyErr_Occurred())
//		return -1;
//	posix_errno = winerror_to_errno(errcode);
//
//	Py_CLEAR(self->winerror);
//	self->winerror = self->myerrno;
//
//	o_errcode = PyInt_FromLong(posix_errno);
//	if (!o_errcode)
//		return -1;
//
//	self->myerrno = o_errcode;
//
//	return 0;
//}
//
//
//static PyObject *
//WindowsError_str(PyWindowsErrorObject *self)
//{
//	PyObject *rtnval = NULL;
//
//	if (self->filename) {
//		PyObject *fmt;
//		PyObject *repr;
//		PyObject *tuple;
//
//		fmt = PyString_FromString("[Error %s] %s: %s");
//		if (!fmt)
//			return NULL;
//
//		repr = PyObject_Repr(self->filename);
//		if (!repr) {
//			Py_DECREF(fmt);
//			return NULL;
//		}
//		tuple = PyTuple_New(3);
//		if (!tuple) {
//			Py_DECREF(repr);
//			Py_DECREF(fmt);
//			return NULL;
//		}
//
//		if (self->winerror) {
//			Py_INCREF(self->winerror);
//			PyTuple_SET_ITEM(tuple, 0, self->winerror);
//		}
//		else {
//			Py_INCREF(Py_None);
//			PyTuple_SET_ITEM(tuple, 0, Py_None);
//		}
//		if (self->strerror) {
//			Py_INCREF(self->strerror);
//			PyTuple_SET_ITEM(tuple, 1, self->strerror);
//		}
//		else {
//			Py_INCREF(Py_None);
//			PyTuple_SET_ITEM(tuple, 1, Py_None);
//		}
//
//		PyTuple_SET_ITEM(tuple, 2, repr);
//
//		rtnval = PyString_Format(fmt, tuple);
//
//		Py_DECREF(fmt);
//		Py_DECREF(tuple);
//	}
//	else if (self->winerror && self->strerror) {
//		PyObject *fmt;
//		PyObject *tuple;
//
//		fmt = PyString_FromString("[Error %s] %s");
//		if (!fmt)
//			return NULL;
//
//		tuple = PyTuple_New(2);
//		if (!tuple) {
//			Py_DECREF(fmt);
//			return NULL;
//		}
//
//		if (self->winerror) {
//			Py_INCREF(self->winerror);
//			PyTuple_SET_ITEM(tuple, 0, self->winerror);
//		}
//		else {
//			Py_INCREF(Py_None);
//			PyTuple_SET_ITEM(tuple, 0, Py_None);
//		}
//		if (self->strerror) {
//			Py_INCREF(self->strerror);
//			PyTuple_SET_ITEM(tuple, 1, self->strerror);
//		}
//		else {
//			Py_INCREF(Py_None);
//			PyTuple_SET_ITEM(tuple, 1, Py_None);
//		}
//
//		rtnval = PyString_Format(fmt, tuple);
//
//		Py_DECREF(fmt);
//		Py_DECREF(tuple);
//	}
//	else
//		rtnval = EnvironmentError_str((PyEnvironmentErrorObject *)self);
//
//	return rtnval;
//}
//
//static PyMemberDef WindowsError_members[] = {
//	{"errno", T_OBJECT, offsetof(PyWindowsErrorObject, myerrno), 0,
//		PyDoc_STR("POSIX exception code")},
//	{"strerror", T_OBJECT, offsetof(PyWindowsErrorObject, strerror), 0,
//		PyDoc_STR("exception strerror")},
//	{"filename", T_OBJECT, offsetof(PyWindowsErrorObject, filename), 0,
//		PyDoc_STR("exception filename")},
//	{"winerror", T_OBJECT, offsetof(PyWindowsErrorObject, winerror), 0,
//		PyDoc_STR("Win32 exception code")},
//	{NULL}  /* Sentinel */
//};
//
//ComplexExtendsException(PyExc_OSError, WindowsError, WindowsError,
//						WindowsError_dealloc, 0, WindowsError_members,
//						WindowsError_str, "MS-Windows OS system call failed.");
//
//#endif /* MS_WINDOWS */


/*
 *	VMSError extends OSError (I think)
 */
#ifdef __VMS
MiddlingExtendsException(PyExc_OSError, VMSError, EnvironmentError,
						 "OpenVMS OS system call failed.");
#endif


/*
 *	EOFError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, EOFError,
					   "Read beyond end of file.");


/*
 *	RuntimeError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, RuntimeError,
					   "Unspecified run-time error.");


/*
 *	NotImplementedError extends RuntimeError
 */
SimpleExtendsException(PyExc_RuntimeError, NotImplementedError,
					   "Method or function hasn't been implemented yet.");

/*
 *	NameError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, NameError,
					   "Name not found globally.");

/*
 *	UnboundLocalError extends NameError
 */
SimpleExtendsException(PyExc_NameError, UnboundLocalError,
					   "Local name referenced but not bound to a value.");

/*
 *	AttributeError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, AttributeError,
					   "Attribute not found.");


/*
 *	SyntaxError extends StandardError
 */

static int
SyntaxError_init(PySyntaxErrorObject *self, PyObject *args, PyObject *kwds)
{
	env(-1);
	jobject jdict = JyNI_JythonPyObject_FromPyObject(kwds);
	ENTER_SubtypeLoop_Safe_ModePy(jdict, kwds, __len__)
	jint dictSize = (*env)->CallIntMethod(env, jdict, JMID(__len__));
	LEAVE_SubtypeLoop_Safe_ModePy(jdict, __len__)
	jobject jargs = (*env)->NewObjectArray(env,
		PyTuple_GET_SIZE(args)
		+dictSize,
		stringClass, NULL);
	int i;
	for (i = 0; i < PyTuple_GET_SIZE(args); ++i)
		(*env)->SetObjectArrayElement(env, jargs, i,
			JyNI_JythonPyObject_FromPyObject(PyTuple_GET_ITEM(args, i)));
	jobject jkw;
	if (dictSize > 0) jkw = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_prepareKeywordArgs, jargs, jdict);
	else jkw = length0StringArray;
	(*env)->CallStaticVoidMethod(env, exceptionsClass,
			exceptionsSyntaxError__init__, JyNI_JythonPyObject_FromPyObject((PyObject*) self), jargs, jkw);
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
//	PyObject *info = NULL;
//	Py_ssize_t lenargs = PyTuple_GET_SIZE(args);
//
//	if (BaseException_init((PyBaseExceptionObject *)self, args, kwds) == -1)
//		return -1;
//
//	if (lenargs >= 1) {
//		Py_CLEAR(self->msg);
//		self->msg = PyTuple_GET_ITEM(args, 0);
//		Py_INCREF(self->msg);
//	}
//	if (lenargs == 2) {
//		info = PyTuple_GET_ITEM(args, 1);
//		info = PySequence_Tuple(info);
//		if (!info)
//			return -1;
//
//		if (PyTuple_GET_SIZE(info) != 4) {
//			/* not a very good error message, but it's what Python 2.4 gives */
//			PyErr_SetString(PyExc_IndexError, "tuple index out of range");
//			Py_DECREF(info);
//			return -1;
//		}
//
//		Py_CLEAR(self->filename);
//		self->filename = PyTuple_GET_ITEM(info, 0);
//		Py_INCREF(self->filename);
//
//		Py_CLEAR(self->lineno);
//		self->lineno = PyTuple_GET_ITEM(info, 1);
//		Py_INCREF(self->lineno);
//
//		Py_CLEAR(self->offset);
//		self->offset = PyTuple_GET_ITEM(info, 2);
//		Py_INCREF(self->offset);
//
//		Py_CLEAR(self->text);
//		self->text = PyTuple_GET_ITEM(info, 3);
//		Py_INCREF(self->text);
//
//		Py_DECREF(info);
//	}
	return 0;
}

static int
SyntaxError_clear(PySyntaxErrorObject *self)
{
//	Py_CLEAR(self->msg);
//	Py_CLEAR(self->filename);
//	Py_CLEAR(self->lineno);
//	Py_CLEAR(self->offset);
//	Py_CLEAR(self->text);
//	Py_CLEAR(self->print_file_and_line);
	return BaseException_clear((PyBaseExceptionObject *)self);
}

static void
SyntaxError_dealloc(PySyntaxErrorObject *self)
{
	//_JyNI_GC_UNTRACK(self);
	SyntaxError_clear(self);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

//static int
//SyntaxError_traverse(PySyntaxErrorObject *self, visitproc visit, void *arg)
//{
//	Py_VISIT(self->msg);
//	Py_VISIT(self->filename);
//	Py_VISIT(self->lineno);
//	Py_VISIT(self->offset);
//	Py_VISIT(self->text);
//	Py_VISIT(self->print_file_and_line);
//	return BaseException_traverse((PyBaseExceptionObject *)self, visit, arg);
//}

//JyNI note: No reason to rewrite this one - would only cause additional work and less performance
/* This is called "my_basename" instead of just "basename" to avoid name
   conflicts with glibc; basename is already prototyped if _GNU_SOURCE is
   defined, and Python does define that. */
static char *
my_basename(char *name)
{
	char *cp = name;
	char *result = name;

	if (name == NULL)
		return "???";
	while (*cp != '\0') {
		if (*cp == SEP)
			result = cp + 1;
		++cp;
	}
	return result;
}


static PyObject *
SyntaxError_str(PySyntaxErrorObject *self)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsSyntaxError__str__,
		JyNI_JythonPyObject_FromPyObject((PyObject*) self), length0PyObjectArray, length0StringArray));
//	PyObject *str;
//	PyObject *result;
//	int have_filename = 0;
//	int have_lineno = 0;
//	char *buffer = NULL;
//	Py_ssize_t bufsize;
//
//	if (self->msg)
//		str = PyObject_Str(self->msg);
//	else
//		str = PyObject_Str(Py_None);
//	if (!str)
//		return NULL;
//	/* Don't fiddle with non-string return (shouldn't happen anyway) */
//	if (!PyString_Check(str))
//		return str;
//
//	/* XXX -- do all the additional formatting with filename and
//	   lineno here */
//
//	have_filename = (self->filename != NULL) &&
//		PyString_Check(self->filename);
//	have_lineno = (self->lineno != NULL) && PyInt_Check(self->lineno);
//
//	if (!have_filename && !have_lineno)
//		return str;
//
//	bufsize = PyString_GET_SIZE(str) + 64;
//	if (have_filename)
//		bufsize += PyString_GET_SIZE(self->filename);
//
//	buffer = PyMem_MALLOC(bufsize);
//	if (buffer == NULL)
//		return str;
//
//	if (have_filename && have_lineno)
//		PyOS_snprintf(buffer, bufsize, "%s (%s, line %ld)",
//			PyString_AS_STRING(str),
//			my_basename(PyString_AS_STRING(self->filename)),
//			PyInt_AsLong(self->lineno));
//	else if (have_filename)
//		PyOS_snprintf(buffer, bufsize, "%s (%s)",
//			PyString_AS_STRING(str),
//			my_basename(PyString_AS_STRING(self->filename)));
//	else /* only have_lineno */
//		PyOS_snprintf(buffer, bufsize, "%s (line %ld)",
//			PyString_AS_STRING(str),
//			PyInt_AsLong(self->lineno));
//
//	result = PyString_FromString(buffer);
//	PyMem_FREE(buffer);
//
//	if (result == NULL)
//		result = str;
//	else
//		Py_DECREF(str);
//	return result;
}

//static PyMemberDef SyntaxError_members[] = {
//	{"msg", T_OBJECT, offsetof(PySyntaxErrorObject, msg), 0,
//		PyDoc_STR("exception msg")},
//	{"filename", T_OBJECT, offsetof(PySyntaxErrorObject, filename), 0,
//		PyDoc_STR("exception filename")},
//	{"lineno", T_OBJECT, offsetof(PySyntaxErrorObject, lineno), 0,
//		PyDoc_STR("exception lineno")},
//	{"offset", T_OBJECT, offsetof(PySyntaxErrorObject, offset), 0,
//		PyDoc_STR("exception offset")},
//	{"text", T_OBJECT, offsetof(PySyntaxErrorObject, text), 0,
//		PyDoc_STR("exception text")},
//	{"print_file_and_line", T_OBJECT,
//		offsetof(PySyntaxErrorObject, print_file_and_line), 0,
//		PyDoc_STR("exception print_file_and_line")},
//	{NULL}  /* Sentinel */
//};

static PyObject *
SyntaxError_get_msg(PySyntaxErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "msg")));
}

static int
SyntaxError_set_msg(PySyntaxErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "msg"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyObject *
SyntaxError_get_filename(PySyntaxErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "filename")));
}

static int
SyntaxError_set_filename(PySyntaxErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "filename"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyObject *
SyntaxError_get_lineno(PySyntaxErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "lineno")));
}

static int
SyntaxError_set_lineno(PySyntaxErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "lineno"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyObject *
SyntaxError_get_offset(PySyntaxErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "offset")));
}

static int
SyntaxError_set_offset(PySyntaxErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "offset"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyObject *
SyntaxError_get_text(PySyntaxErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "text")));
}

static int
SyntaxError_set_text(PySyntaxErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "text"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyObject *
SyntaxError_get_print_file_and_line(PySyntaxErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "print_file_and_line")));
}

static int
SyntaxError_set_print_file_and_line(PySyntaxErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "print_file_and_line"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyGetSetDef SyntaxError_getsets [] = {
	{"msg", (getter)SyntaxError_get_msg, (setter)SyntaxError_set_msg, PyDoc_STR("exception msg")},
	{"filename", (getter)SyntaxError_get_filename, (setter)SyntaxError_set_filename, PyDoc_STR("exception filename")},
	{"lineno", (getter)SyntaxError_get_lineno, (setter)SyntaxError_set_lineno, PyDoc_STR("exception lineno")},
	{"offset", (getter)SyntaxError_get_offset, (setter)SyntaxError_set_offset, PyDoc_STR("exception offset")},
	{"text", (getter)SyntaxError_get_text, (setter)SyntaxError_set_text, PyDoc_STR("exception text")},
	{"print_file_and_line", (getter)SyntaxError_get_print_file_and_line, (setter)SyntaxError_set_print_file_and_line, PyDoc_STR("exception print_file_and_line")},
	{NULL} /* Sentinel */
};

//ComplexExtendsException(PyExc_StandardError, SyntaxError, SyntaxError,
//						SyntaxError_dealloc, 0, 0 /*SyntaxError_members*/,
//						SyntaxError_str, "Invalid syntax.");
ComplexExtendsException2(PyExc_StandardError, SyntaxError, SyntaxError,
						SyntaxError_dealloc, 0, SyntaxError_getsets,
						SyntaxError_str, "Invalid syntax.");

/*
 *	IndentationError extends SyntaxError
 */
MiddlingExtendsException(PyExc_SyntaxError, IndentationError, SyntaxError,
						 "Improper indentation.");


/*
 *	TabError extends IndentationError
 */
MiddlingExtendsException(PyExc_IndentationError, TabError, SyntaxError,
						 "Improper mixture of spaces and tabs.");


/*
 *	LookupError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, LookupError,
					   "Base class for lookup errors.");


/*
 *	IndexError extends LookupError
 */
SimpleExtendsException(PyExc_LookupError, IndexError,
					   "Sequence index out of range.");


/*
 *	KeyError extends LookupError
 */
static PyObject *
KeyError_str(PyBaseExceptionObject *self)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsKeyError__str__,
		JyNI_JythonPyObject_FromPyObject((PyObject*) self), length0PyObjectArray, length0StringArray));
//	/* If args is a tuple of exactly one item, apply repr to args[0].
//	   This is done so that e.g. the exception raised by {}[''] prints
//		 KeyError: ''
//	   rather than the confusing
//		 KeyError
//	   alone.  The downside is that if KeyError is raised with an explanatory
//	   string, that string will be displayed in quotes.  Too bad.
//	   If args is anything else, use the default BaseException__str__().
//	*/
//	if (PyTuple_GET_SIZE(self->args) == 1) {
//		return PyObject_Repr(PyTuple_GET_ITEM(self->args, 0));
//	}
//	return BaseException_str(self);
}

ComplexExtendsException(PyExc_LookupError, KeyError, BaseException,
						0, 0, 0, KeyError_str, "Mapping key not found.");


/*
 *	ValueError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, ValueError,
					   "Inappropriate argument value (of correct type).");

/*
 *	UnicodeError extends ValueError
 */

SimpleExtendsException(PyExc_ValueError, UnicodeError,
					   "Unicode related error.");

#ifdef Py_USING_UNICODE
//JyNI-note: Delegating this to Jython is not necessary.
static PyObject *
get_string(PyObject *attr, const char *name)
{
//	env(NULL);
//	return JyNI_PyObject_FromJythonPyObject(
//		(*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsGetString,
//		JyNI_JythonPyObject_FromPyObject(attr), (*env)->NewStringUTF(env, name)));
	if (!attr) {
		PyErr_Format(PyExc_TypeError, "%.200s attribute not set", name);
		return NULL;
	}

	if (!PyString_Check(attr)) {
		PyErr_Format(PyExc_TypeError, "%.200s attribute must be str", name);
		return NULL;
	}
	Py_INCREF(attr);
	return attr;
}


static int
set_string(PyObject **attr, const char *value)
{
	PyObject *obj = PyString_FromString(value);
	if (!obj)
		return -1;
	Py_CLEAR(*attr);
	*attr = obj;
	return 0;
}


static PyObject *
get_unicode(PyObject *attr, const char *name)
{
//	env(NULL);
//	return JyNI_PyObject_FromJythonPyObject(
//		(*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsGetUnicode,
//		JyNI_JythonPyObject_FromPyObject(attr), (*env)->NewStringUTF(env, name)));
	if (!attr) {
		PyErr_Format(PyExc_TypeError, "%.200s attribute not set", name);
		return NULL;
	}

	if (!PyUnicode_Check(attr)) {
		PyErr_Format(PyExc_TypeError,
					 "%.200s attribute must be unicode", name);
		return NULL;
	}
	Py_INCREF(attr);
	return attr;
}

PyObject *
PyUnicodeEncodeError_GetEncoding(PyObject *exc)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) exc), pyObject__getattr__,
		(*env)->NewStringUTF(env, "encoding")));
//	return get_string(((PyUnicodeErrorObject *)exc)->encoding, "encoding");
}

PyObject *
PyUnicodeDecodeError_GetEncoding(PyObject *exc)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) exc), pyObject__getattr__,
		(*env)->NewStringUTF(env, "encoding")));
//	return get_string(((PyUnicodeErrorObject *)exc)->encoding, "encoding");
}

PyObject *
PyUnicodeEncodeError_GetObject(PyObject *exc)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) exc), pyObject__getattr__,
		(*env)->NewStringUTF(env, "object")));
//	return get_unicode(((PyUnicodeErrorObject *)exc)->object, "object");
}

PyObject *
PyUnicodeDecodeError_GetObject(PyObject *exc)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) exc), pyObject__getattr__,
		(*env)->NewStringUTF(env, "object")));
//	return get_string(((PyUnicodeErrorObject *)exc)->object, "object");
}

PyObject *
PyUnicodeTranslateError_GetObject(PyObject *exc)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) exc), pyObject__getattr__,
		(*env)->NewStringUTF(env, "object")));
//	return get_unicode(((PyUnicodeErrorObject *)exc)->object, "object");
}

int
PyUnicodeEncodeError_GetStart(PyObject *exc, Py_ssize_t *start)
{
	env(NULL);
	jint tmp = (*env)->CallStaticIntMethod(env, exceptionsClass, exceptionsGetStart,
			JyNI_JythonPyObject_FromPyObject(exc), JNI_TRUE);
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	*start = tmp;
	return 0;
//	Py_ssize_t size;
//	PyObject *obj = get_unicode(((PyUnicodeErrorObject *)exc)->object,
//								"object");
//	if (!obj)
//		return -1;
//	*start = ((PyUnicodeErrorObject *)exc)->start;
//	size = PyUnicode_GET_SIZE(obj);
//	if (*start<0)
//		*start = 0; /*XXX check for values <0*/
//	if (*start>=size)
//		*start = size-1;
//	Py_DECREF(obj);
//	return 0;
}


int
PyUnicodeDecodeError_GetStart(PyObject *exc, Py_ssize_t *start)
{
	env(NULL);
	jint tmp = (*env)->CallStaticIntMethod(env, exceptionsClass, exceptionsGetStart,
		JyNI_JythonPyObject_FromPyObject(exc), JNI_FALSE);
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	*start = tmp;
	return 0;
//	Py_ssize_t size;
//	PyObject *obj = get_string(((PyUnicodeErrorObject *)exc)->object,
//							   "object");
//	if (!obj)
//		return -1;
//	size = PyString_GET_SIZE(obj);
//	*start = ((PyUnicodeErrorObject *)exc)->start;
//	if (*start<0)
//		*start = 0;
//	if (*start>=size)
//		*start = size-1;
//	Py_DECREF(obj);
//	return 0;
}


int
PyUnicodeTranslateError_GetStart(PyObject *exc, Py_ssize_t *start)
{
	return PyUnicodeEncodeError_GetStart(exc, start);
}


int
PyUnicodeEncodeError_SetStart(PyObject *exc, Py_ssize_t start)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(exc), pyObject__setattr__,
		(*env)->NewStringUTF(env, "start"), JyNI_JythonPyObject_FromPyObject(PyInt_FromSsize_t(start)));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
//	((PyUnicodeErrorObject *)exc)->start = start;
//	return 0;
}


int
PyUnicodeDecodeError_SetStart(PyObject *exc, Py_ssize_t start)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(exc), pyObject__setattr__,
		(*env)->NewStringUTF(env, "start"), JyNI_JythonPyObject_FromPyObject(PyInt_FromSsize_t(start)));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
//	((PyUnicodeErrorObject *)exc)->start = start;
//	return 0;
}


int
PyUnicodeTranslateError_SetStart(PyObject *exc, Py_ssize_t start)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(exc), pyObject__setattr__,
		(*env)->NewStringUTF(env, "start"), JyNI_JythonPyObject_FromPyObject(PyInt_FromSsize_t(start)));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
//	((PyUnicodeErrorObject *)exc)->start = start;
//	return 0;
}


int
PyUnicodeEncodeError_GetEnd(PyObject *exc, Py_ssize_t *end)
{
	env(NULL);
	jint tmp = (*env)->CallStaticIntMethod(env, exceptionsClass, exceptionsGetEnd,
		JyNI_JythonPyObject_FromPyObject(exc), JNI_TRUE);
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	*end = tmp;
	return 0;
//	Py_ssize_t size;
//	PyObject *obj = get_unicode(((PyUnicodeErrorObject *)exc)->object,
//								"object");
//	if (!obj)
//		return -1;
//	*end = ((PyUnicodeErrorObject *)exc)->end;
//	size = PyUnicode_GET_SIZE(obj);
//	if (*end<1)
//		*end = 1;
//	if (*end>size)
//		*end = size;
//	Py_DECREF(obj);
//	return 0;
}


int
PyUnicodeDecodeError_GetEnd(PyObject *exc, Py_ssize_t *end)
{
	env(NULL);
	jint tmp = (*env)->CallStaticIntMethod(env, exceptionsClass, exceptionsGetEnd,
		JyNI_JythonPyObject_FromPyObject(exc), JNI_FALSE);
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	*end = tmp;
	return 0;
//	Py_ssize_t size;
//	PyObject *obj = get_string(((PyUnicodeErrorObject *)exc)->object,
//							   "object");
//	if (!obj)
//		return -1;
//	*end = ((PyUnicodeErrorObject *)exc)->end;
//	size = PyString_GET_SIZE(obj);
//	if (*end<1)
//		*end = 1;
//	if (*end>size)
//		*end = size;
//	Py_DECREF(obj);
//	return 0;
}


int
PyUnicodeTranslateError_GetEnd(PyObject *exc, Py_ssize_t *start)
{
	return PyUnicodeEncodeError_GetEnd(exc, start);
}


int
PyUnicodeEncodeError_SetEnd(PyObject *exc, Py_ssize_t end)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(exc), pyObject__setattr__,
		(*env)->NewStringUTF(env, "end"), JyNI_JythonPyObject_FromPyObject(PyInt_FromSsize_t(end)));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
//	((PyUnicodeErrorObject *)exc)->end = end;
//	return 0;
}


int
PyUnicodeDecodeError_SetEnd(PyObject *exc, Py_ssize_t end)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(exc), pyObject__setattr__,
		(*env)->NewStringUTF(env, "end"), JyNI_JythonPyObject_FromPyObject(PyInt_FromSsize_t(end)));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
//	((PyUnicodeErrorObject *)exc)->end = end;
//	return 0;
}


int
PyUnicodeTranslateError_SetEnd(PyObject *exc, Py_ssize_t end)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(exc), pyObject__setattr__,
		(*env)->NewStringUTF(env, "end"), JyNI_JythonPyObject_FromPyObject(PyInt_FromSsize_t(end)));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
//	((PyUnicodeErrorObject *)exc)->end = end;
//	return 0;
}

PyObject *
PyUnicodeEncodeError_GetReason(PyObject *exc)
{
	env(NULL);
	return get_string(JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(exc), pyObject__getattr__,
		(*env)->NewStringUTF(env, "reason"))), "reason");
	//return get_string(((PyUnicodeErrorObject *)exc)->reason, "reason");
}


PyObject *
PyUnicodeDecodeError_GetReason(PyObject *exc)
{
	env(NULL);
	return get_string(JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(exc), pyObject__getattr__,
		(*env)->NewStringUTF(env, "reason"))), "reason");
//	return get_string(((PyUnicodeErrorObject *)exc)->reason, "reason");
}


PyObject *
PyUnicodeTranslateError_GetReason(PyObject *exc)
{
	env(NULL);
	return get_string(JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(exc), pyObject__getattr__,
		(*env)->NewStringUTF(env, "reason"))), "reason");
//	return get_string(((PyUnicodeErrorObject *)exc)->reason, "reason");
}


int
PyUnicodeEncodeError_SetReason(PyObject *exc, const char *reason)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(exc), pyObject__setattr__,
		(*env)->NewStringUTF(env, "reason"), (*env)->NewStringUTF(env, reason));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
//	return set_string(&((PyUnicodeErrorObject *)exc)->reason, reason);
}


int
PyUnicodeDecodeError_SetReason(PyObject *exc, const char *reason)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(exc), pyObject__setattr__,
		(*env)->NewStringUTF(env, "reason"), (*env)->NewStringUTF(env, reason));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
//	return set_string(&((PyUnicodeErrorObject *)exc)->reason, reason);
}


int
PyUnicodeTranslateError_SetReason(PyObject *exc, const char *reason)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject(exc), pyObject__setattr__,
		(*env)->NewStringUTF(env, "reason"), (*env)->NewStringUTF(env, reason));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
//	return set_string(&((PyUnicodeErrorObject *)exc)->reason, reason);
}


//static int
//UnicodeError_init(PyUnicodeErrorObject *self, PyObject *args, PyObject *kwds,
//				  PyTypeObject *objecttype)
//{
//	env(-1);
//	jobject jdict = JyNI_JythonPyObject_FromPyObject(kwds);
//	jint dictSize = (*env)->CallIntMethod(env, jdict, pyDictSize);
//	jobject jargs = (*env)->NewObjectArray(env,
//		PyTuple_GET_SIZE(args)
//		+dictSize,
//		stringClass, NULL);
//	int i;
//	for (i = 0; i < PyTuple_GET_SIZE(args); ++i)
//		(*env)->SetObjectArrayElement(env, jargs, i,
//			JyNI_JythonPyObject_FromPyObject(PyTuple_GET_ITEM(args, i)));
//	jobject jkw;
//	if (dictSize > 0) jkw = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_prepareKeywordArgs, jargs, jdict);
//	else jkw = length0StringArray;
//	(*env)->CallStaticVoidMethod(env, exceptionsClass,
//			exceptionsUnicodeError__init__, JyNI_JythonPyObject_FromPyObject(self), jargs, jkw);
//	if ((*env)->ExceptionCheck(env))
//	{
//		(*env)->ExceptionClear(env);
//		return -1;
//	}
////	Py_CLEAR(self->encoding);
////	Py_CLEAR(self->object);
////	Py_CLEAR(self->reason);
////
////	if (!PyArg_ParseTuple(args, "O!O!nnO!",
////		&PyString_Type, &self->encoding,
////		objecttype, &self->object,
////		&self->start,
////		&self->end,
////		&PyString_Type, &self->reason)) {
////		self->encoding = self->object = self->reason = NULL;
////		return -1;
////	}
////
////	Py_INCREF(self->encoding);
////	Py_INCREF(self->object);
////	Py_INCREF(self->reason);
//	return 0;
//}

static int
UnicodeError_clear(PyUnicodeErrorObject *self)
{
//	Py_CLEAR(self->encoding);
//	Py_CLEAR(self->object);
//	Py_CLEAR(self->reason);
	return BaseException_clear((PyBaseExceptionObject *)self);
}

static void
UnicodeError_dealloc(PyUnicodeErrorObject *self)
{
	//_JyNI_GC_UNTRACK(self);
	UnicodeError_clear(self);
	Py_TYPE(self)->tp_free((PyObject *)self);
}

//static int
//UnicodeError_traverse(PyUnicodeErrorObject *self, visitproc visit, void *arg)
//{
//	Py_VISIT(self->encoding);
//	Py_VISIT(self->object);
//	Py_VISIT(self->reason);
//	return BaseException_traverse((PyBaseExceptionObject *)self, visit, arg);
//}

//static PyMemberDef UnicodeError_members[] = {
//	{"encoding", T_OBJECT, offsetof(PyUnicodeErrorObject, encoding), 0,
//		PyDoc_STR("exception encoding")},
//	{"object", T_OBJECT, offsetof(PyUnicodeErrorObject, object), 0,
//		PyDoc_STR("exception object")},
//	{"start", T_PYSSIZET, offsetof(PyUnicodeErrorObject, start), 0,
//		PyDoc_STR("exception start")},
//	{"end", T_PYSSIZET, offsetof(PyUnicodeErrorObject, end), 0,
//		PyDoc_STR("exception end")},
//	{"reason", T_OBJECT, offsetof(PyUnicodeErrorObject, reason), 0,
//		PyDoc_STR("exception reason")},
//	{NULL}  /* Sentinel */
//};


static PyObject *
PyUnicodeError_get_encoding(PyUnicodeErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "encoding")));
}

static int
PyUnicodeError_set_encoding(PyUnicodeErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "encoding"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyObject *
PyUnicodeError_get_object(PyUnicodeErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "object")));
}

static int
PyUnicodeError_set_object(PyUnicodeErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "object"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyObject *
PyUnicodeError_get_start(PyUnicodeErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "start")));
}

static int
PyUnicodeError_set_start(PyUnicodeErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "start"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyObject *
PyUnicodeError_get_end(PyUnicodeErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "end")));
}

static int
PyUnicodeError_set_end(PyUnicodeErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "end"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyObject *
PyUnicodeError_get_reason(PyUnicodeErrorObject *obj)//, void *closure)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__getattr__,
		(*env)->NewStringUTF(env, "reason")));
}

static int
PyUnicodeError_set_reason(PyUnicodeErrorObject *obj, PyObject* value)//, void *closure)
{
	env(-1);
	(*env)->CallObjectMethod(env, JyNI_JythonPyObject_FromPyObject((PyObject*) obj), pyObject__setattr__,
		(*env)->NewStringUTF(env, "reason"), JyNI_JythonPyObject_FromPyObject(value));
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
}

static PyGetSetDef UnicodeError_getsets [] = {
	{"encoding", (getter)PyUnicodeError_get_encoding, (setter)PyUnicodeError_set_encoding, PyDoc_STR("exception encoding")},
	{"object", (getter)PyUnicodeError_get_object, (setter)PyUnicodeError_set_object, PyDoc_STR("exception object")},
	{"start", (getter)PyUnicodeError_get_start, (setter)PyUnicodeError_set_start, PyDoc_STR("exception start")},
	{"end", (getter)PyUnicodeError_get_end, (setter)PyUnicodeError_set_end, PyDoc_STR("exception end")},
	{"reason", (getter)PyUnicodeError_get_reason, (setter)PyUnicodeError_set_reason, PyDoc_STR("exception reason")},
	{NULL} /* Sentinel */
};

/*
 *	UnicodeEncodeError extends UnicodeError
 */
//JyNI-note: It probably would have been sufficient to keep the original
//implementation.
static int
UnicodeEncodeError_init(PyObject *self, PyObject *args, PyObject *kwds)
{
	env(-1);
	jobject jdict = JyNI_JythonPyObject_FromPyObject(kwds);
	ENTER_SubtypeLoop_Safe_ModePy(jdict, kwds, __len__)
	jint dictSize = (*env)->CallIntMethod(env, jdict, JMID(__len__));
	LEAVE_SubtypeLoop_Safe_ModePy(jdict, __len__)
	jobject jargs = (*env)->NewObjectArray(env,
		PyTuple_GET_SIZE(args)
		+dictSize,
		stringClass, NULL);
	int i;
	for (i = 0; i < PyTuple_GET_SIZE(args); ++i)
		(*env)->SetObjectArrayElement(env, jargs, i,
			JyNI_JythonPyObject_FromPyObject(PyTuple_GET_ITEM(args, i)));
	jobject jkw;
	if (dictSize > 0) jkw = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_prepareKeywordArgs, jargs, jdict);
	else jkw = length0StringArray;
	(*env)->CallStaticVoidMethod(env, exceptionsClass,
			exceptionsUnicodeEncodeError__init__, JyNI_JythonPyObject_FromPyObject(self), jargs, jkw);
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
//	if (BaseException_init((PyBaseExceptionObject *)self, args, kwds) == -1)
//		return -1;
//	return UnicodeError_init((PyUnicodeErrorObject *)self, args,
//							 kwds, &PyUnicode_Type);
}

static PyObject *
UnicodeEncodeError_str(PyObject *self)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsUnicodeEncodeError__str__,
		JyNI_JythonPyObject_FromPyObject(self), length0PyObjectArray, length0StringArray));
//	PyUnicodeErrorObject *uself = (PyUnicodeErrorObject *)self;
//	PyObject *result = NULL;
//	PyObject *reason_str = NULL;
//	PyObject *encoding_str = NULL;
//
//	/* Get reason and encoding as strings, which they might not be if
//	   they've been modified after we were contructed. */
//	reason_str = PyObject_Str(uself->reason);
//	if (reason_str == NULL)
//		goto done;
//	encoding_str = PyObject_Str(uself->encoding);
//	if (encoding_str == NULL)
//		goto done;
//
//	if (uself->start < PyUnicode_GET_SIZE(uself->object) && uself->end == uself->start+1) {
//		int badchar = (int)PyUnicode_AS_UNICODE(uself->object)[uself->start];
//		char badchar_str[20];
//		if (badchar <= 0xff)
//			PyOS_snprintf(badchar_str, sizeof(badchar_str), "x%02x", badchar);
//		else if (badchar <= 0xffff)
//			PyOS_snprintf(badchar_str, sizeof(badchar_str), "u%04x", badchar);
//		else
//			PyOS_snprintf(badchar_str, sizeof(badchar_str), "U%08x", badchar);
//		result = PyString_FromFormat(
//			"'%.400s' codec can't encode character u'\\%s' in position %zd: %.400s",
//			PyString_AS_STRING(encoding_str),
//			badchar_str,
//			uself->start,
//			PyString_AS_STRING(reason_str));
//	}
//	else {
//		result = PyString_FromFormat(
//			"'%.400s' codec can't encode characters in position %zd-%zd: %.400s",
//			PyString_AS_STRING(encoding_str),
//			uself->start,
//			uself->end-1,
//			PyString_AS_STRING(reason_str));
//	}
//done:
//	Py_XDECREF(reason_str);
//	Py_XDECREF(encoding_str);
//	return result;
}

static PyTypeObject _PyExc_UnicodeEncodeError = {
	PyObject_HEAD_INIT(NULL)
	0,
	EXC_MODULE_NAME "UnicodeEncodeError",
	sizeof(PyUnicodeErrorObject), 0,
	(destructor)UnicodeError_dealloc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	(reprfunc)UnicodeEncodeError_str, 0, 0, 0,
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE /*| Py_TPFLAGS_HAVE_GC*/,
	PyDoc_STR("Unicode encoding error."), 0 /*(traverseproc)UnicodeError_traverse*/,
	(inquiry)UnicodeError_clear, 0, 0, 0, 0, 0, 0 /*UnicodeError_members*/,
	UnicodeError_getsets, &_PyExc_UnicodeError, 0, 0, 0, offsetof(PyUnicodeErrorObject, dict),
	(initproc)UnicodeEncodeError_init, 0, BaseException_new,
};
PyObject *PyExc_UnicodeEncodeError = (PyObject *)&_PyExc_UnicodeEncodeError;

PyObject *
PyUnicodeEncodeError_Create(
	const char *encoding, const Py_UNICODE *object, Py_ssize_t length,
	Py_ssize_t start, Py_ssize_t end, const char *reason)
{
	return PyObject_CallFunction(PyExc_UnicodeEncodeError, "su#nns",
								 encoding, object, length, start, end, reason);
}


/*
 *	UnicodeDecodeError extends UnicodeError
 */

static int
UnicodeDecodeError_init(PyObject *self, PyObject *args, PyObject *kwds)
{
	env(-1);
	jobject jdict = JyNI_JythonPyObject_FromPyObject(kwds);
	ENTER_SubtypeLoop_Safe_ModePy(jdict, kwds, __len__)
	jint dictSize = (*env)->CallIntMethod(env, jdict, JMID(__len__));
	LEAVE_SubtypeLoop_Safe_ModePy(jdict, __len__)
	jobject jargs = (*env)->NewObjectArray(env,
		PyTuple_GET_SIZE(args)
		+dictSize,
		stringClass, NULL);
	int i;
	for (i = 0; i < PyTuple_GET_SIZE(args); ++i)
		(*env)->SetObjectArrayElement(env, jargs, i,
			JyNI_JythonPyObject_FromPyObject(PyTuple_GET_ITEM(args, i)));
	jobject jkw;
	if (dictSize > 0) jkw = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_prepareKeywordArgs, jargs, jdict);
	else jkw = length0StringArray;
	(*env)->CallStaticVoidMethod(env, exceptionsClass,
			exceptionsUnicodeDecodeError__init__, JyNI_JythonPyObject_FromPyObject(self), jargs, jkw);
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
	return 0;
//	if (BaseException_init((PyBaseExceptionObject *)self, args, kwds) == -1)
//		return -1;
//	return UnicodeError_init((PyUnicodeErrorObject *)self, args,
//							 kwds, &PyString_Type);
}

static PyObject *
UnicodeDecodeError_str(PyObject *self)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsUnicodeDecodeError__str__,
		JyNI_JythonPyObject_FromPyObject(self), length0PyObjectArray, length0StringArray));
//	PyUnicodeErrorObject *uself = (PyUnicodeErrorObject *)self;
//	PyObject *result = NULL;
//	PyObject *reason_str = NULL;
//	PyObject *encoding_str = NULL;
//
//	/* Get reason and encoding as strings, which they might not be if
//	   they've been modified after we were contructed. */
//	reason_str = PyObject_Str(uself->reason);
//	if (reason_str == NULL)
//		goto done;
//	encoding_str = PyObject_Str(uself->encoding);
//	if (encoding_str == NULL)
//		goto done;
//
//	if (uself->start < PyUnicode_GET_SIZE(uself->object) && uself->end == uself->start+1) {
//		/* FromFormat does not support %02x, so format that separately */
//		char byte[4];
//		PyOS_snprintf(byte, sizeof(byte), "%02x",
//					  ((int)PyString_AS_STRING(uself->object)[uself->start])&0xff);
//		result = PyString_FromFormat(
//			"'%.400s' codec can't decode byte 0x%s in position %zd: %.400s",
//			PyString_AS_STRING(encoding_str),
//			byte,
//			uself->start,
//			PyString_AS_STRING(reason_str));
//	}
//	else {
//		result = PyString_FromFormat(
//			"'%.400s' codec can't decode bytes in position %zd-%zd: %.400s",
//			PyString_AS_STRING(encoding_str),
//			uself->start,
//			uself->end-1,
//			PyString_AS_STRING(reason_str));
//	}
//done:
//	Py_XDECREF(reason_str);
//	Py_XDECREF(encoding_str);
//	return result;
}

static PyTypeObject _PyExc_UnicodeDecodeError = {
	PyObject_HEAD_INIT(NULL)
	0,
	EXC_MODULE_NAME "UnicodeDecodeError",
	sizeof(PyUnicodeErrorObject), 0,
	(destructor)UnicodeError_dealloc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	(reprfunc)UnicodeDecodeError_str, 0, 0, 0,
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE /*| Py_TPFLAGS_HAVE_GC*/,
	PyDoc_STR("Unicode decoding error."), 0 /*(traverseproc)UnicodeError_traverse*/,
	(inquiry)UnicodeError_clear, 0, 0, 0, 0, 0, 0 /*UnicodeError_members*/,
	UnicodeError_getsets, &_PyExc_UnicodeError, 0, 0, 0, offsetof(PyUnicodeErrorObject, dict),
	(initproc)UnicodeDecodeError_init, 0, BaseException_new,
};
PyObject *PyExc_UnicodeDecodeError = (PyObject *)&_PyExc_UnicodeDecodeError;

PyObject *
PyUnicodeDecodeError_Create(
	const char *encoding, const char *object, Py_ssize_t length,
	Py_ssize_t start, Py_ssize_t end, const char *reason)
{
	return PyObject_CallFunction(PyExc_UnicodeDecodeError, "ss#nns",
								 encoding, object, length, start, end, reason);
}


/*
 *	UnicodeTranslateError extends UnicodeError
 */

static int
UnicodeTranslateError_init(PyUnicodeErrorObject *self, PyObject *args, PyObject *kwds)
{
	env(-1);
	jobject jdict = JyNI_JythonPyObject_FromPyObject(kwds);
	ENTER_SubtypeLoop_Safe_ModePy(jdict, kwds, __len__)
	jint dictSize = (*env)->CallIntMethod(env, jdict, JMID(__len__));
	LEAVE_SubtypeLoop_Safe_ModePy(jdict, __len__)
	jobject jargs = (*env)->NewObjectArray(env,
		PyTuple_GET_SIZE(args)
		+dictSize,
		stringClass, NULL);
	int i;
	for (i = 0; i < PyTuple_GET_SIZE(args); ++i)
		(*env)->SetObjectArrayElement(env, jargs, i,
			JyNI_JythonPyObject_FromPyObject(PyTuple_GET_ITEM(args, i)));
	jobject jkw;
	if (dictSize > 0) jkw = (*env)->CallStaticObjectMethod(env, JyNIClass, JyNI_prepareKeywordArgs, jargs, jdict);
	else jkw = length0StringArray;
	(*env)->CallStaticVoidMethod(env, exceptionsClass,
			exceptionsUnicodeTranslateError__init__, JyNI_JythonPyObject_FromPyObject((PyObject*) self), jargs, jkw);
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		return -1;
	}
//	if (BaseException_init((PyBaseExceptionObject *)self, args, kwds) == -1)
//		return -1;
//
//	Py_CLEAR(self->object);
//	Py_CLEAR(self->reason);
//
//	if (!PyArg_ParseTuple(args, "O!nnO!",
//		&PyUnicode_Type, &self->object,
//		&self->start,
//		&self->end,
//		&PyString_Type, &self->reason)) {
//		self->object = self->reason = NULL;
//		return -1;
//	}
//
//	Py_INCREF(self->object);
//	Py_INCREF(self->reason);
	return 0;
}


static PyObject *
UnicodeTranslateError_str(PyObject *self)
{
	env(NULL);
	return JyNI_PyObject_FromJythonPyObject(
		(*env)->CallStaticObjectMethod(env, exceptionsClass, exceptionsUnicodeTranslateError__str__,
		JyNI_JythonPyObject_FromPyObject(self), length0PyObjectArray, length0StringArray));
//	PyUnicodeErrorObject *uself = (PyUnicodeErrorObject *)self;
//	PyObject *result = NULL;
//	PyObject *reason_str = NULL;
//
//	/* Get reason as a string, which it might not be if it's been
//	   modified after we were contructed. */
//	reason_str = PyObject_Str(uself->reason);
//	if (reason_str == NULL)
//		goto done;
//
//	if (uself->start < PyUnicode_GET_SIZE(uself->object) && uself->end == uself->start+1) {
//		int badchar = (int)PyUnicode_AS_UNICODE(uself->object)[uself->start];
//		char badchar_str[20];
//		if (badchar <= 0xff)
//			PyOS_snprintf(badchar_str, sizeof(badchar_str), "x%02x", badchar);
//		else if (badchar <= 0xffff)
//			PyOS_snprintf(badchar_str, sizeof(badchar_str), "u%04x", badchar);
//		else
//			PyOS_snprintf(badchar_str, sizeof(badchar_str), "U%08x", badchar);
//		result = PyString_FromFormat(
//			"can't translate character u'\\%s' in position %zd: %.400s",
//			badchar_str,
//			uself->start,
//			PyString_AS_STRING(reason_str));
//	} else {
//		result = PyString_FromFormat(
//			"can't translate characters in position %zd-%zd: %.400s",
//			uself->start,
//			uself->end-1,
//			PyString_AS_STRING(reason_str));
//	}
//done:
//	Py_XDECREF(reason_str);
//	return result;
}

static PyTypeObject _PyExc_UnicodeTranslateError = {
	PyObject_HEAD_INIT(NULL)
	0,
	EXC_MODULE_NAME "UnicodeTranslateError",
	sizeof(PyUnicodeErrorObject), 0,
	(destructor)UnicodeError_dealloc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	(reprfunc)UnicodeTranslateError_str, 0, 0, 0,
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE /*| Py_TPFLAGS_HAVE_GC*/,
	PyDoc_STR("Unicode translation error."), 0 /*(traverseproc)UnicodeError_traverse*/,
	(inquiry)UnicodeError_clear, 0, 0, 0, 0, 0, 0 /*UnicodeError_members*/,
	UnicodeError_getsets, &_PyExc_UnicodeError, 0, 0, 0, offsetof(PyUnicodeErrorObject, dict),
	(initproc)UnicodeTranslateError_init, 0, BaseException_new,
};
PyObject *PyExc_UnicodeTranslateError = (PyObject *)&_PyExc_UnicodeTranslateError;

PyObject *
PyUnicodeTranslateError_Create(
	const Py_UNICODE *object, Py_ssize_t length,
	Py_ssize_t start, Py_ssize_t end, const char *reason)
{
	return PyObject_CallFunction(PyExc_UnicodeTranslateError, "u#nns",
								 object, length, start, end, reason);
}
#endif


/*
 *	AssertionError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, AssertionError,
					   "Assertion failed.");


/*
 *	ArithmeticError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, ArithmeticError,
					   "Base class for arithmetic errors.");


/*
 *	FloatingPointError extends ArithmeticError
 */
SimpleExtendsException(PyExc_ArithmeticError, FloatingPointError,
					   "Floating point operation failed.");


/*
 *	OverflowError extends ArithmeticError
 */
SimpleExtendsException(PyExc_ArithmeticError, OverflowError,
					   "Result too large to be represented.");


/*
 *	ZeroDivisionError extends ArithmeticError
 */
SimpleExtendsException(PyExc_ArithmeticError, ZeroDivisionError,
		  "Second argument to a division or modulo operation was zero.");


/*
 *	SystemError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, SystemError,
	"Internal error in the Python interpreter.\n"
	"\n"
	"Please report this to the Python maintainer, along with the traceback,\n"
	"the Python version, and the hardware/OS platform and version.");


/*
 *	ReferenceError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, ReferenceError,
					   "Weak ref proxy used after referent went away.");


/*
 *	MemoryError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, MemoryError, "Out of memory.");

/*
 *	BufferError extends StandardError
 */
SimpleExtendsException(PyExc_StandardError, BufferError, "Buffer error.");


/* Warning category docstrings */

/*
 *	Warning extends Exception
 */
SimpleExtendsException(PyExc_Exception, Warning,
					   "Base class for warning categories.");


/*
 *	UserWarning extends Warning
 */
SimpleExtendsException(PyExc_Warning, UserWarning,
					   "Base class for warnings generated by user code.");


/*
 *	DeprecationWarning extends Warning
 */
SimpleExtendsException(PyExc_Warning, DeprecationWarning,
					   "Base class for warnings about deprecated features.");


/*
 *	PendingDeprecationWarning extends Warning
 */
SimpleExtendsException(PyExc_Warning, PendingDeprecationWarning,
	"Base class for warnings about features which will be deprecated\n"
	"in the future.");


/*
 *	SyntaxWarning extends Warning
 */
SimpleExtendsException(PyExc_Warning, SyntaxWarning,
					   "Base class for warnings about dubious syntax.");


/*
 *	RuntimeWarning extends Warning
 */
SimpleExtendsException(PyExc_Warning, RuntimeWarning,
				 "Base class for warnings about dubious runtime behavior.");


/*
 *	FutureWarning extends Warning
 */
SimpleExtendsException(PyExc_Warning, FutureWarning,
	"Base class for warnings about constructs that will change semantically\n"
	"in the future.");


/*
 *	ImportWarning extends Warning
 */
SimpleExtendsException(PyExc_Warning, ImportWarning,
		  "Base class for warnings about probable mistakes in module imports");


/*
 *	UnicodeWarning extends Warning
 */
SimpleExtendsException(PyExc_Warning, UnicodeWarning,
	"Base class for warnings about Unicode related problems, mostly\n"
	"related to conversion problems.");

/*
 *	BytesWarning extends Warning
 */
SimpleExtendsException(PyExc_Warning, BytesWarning,
	"Base class for warnings about bytes and buffer related problems, mostly\n"
	"related to conversion from str or comparing to str.");

/*JyNI-note: The rest is stuff to register the methods in this file as a module.
 * This is not wise for Jython, since it provides the module on java-side.
 * So we exclude everything following this comment from JyNI.
 */

/* Pre-computed MemoryError instance.  Best to create this as early as
 * possible and not wait until a MemoryError is actually raised!
 */
//PyObject *PyExc_MemoryErrorInst=NULL;
//
///* Pre-computed RuntimeError instance for when recursion depth is reached.
//   Meant to be used when normalizing the exception for exceeding the recursion
//   depth will cause its own infinite recursion.
//*/
//PyObject *PyExc_RecursionErrorInst = NULL;
//
///* module global functions */
//static PyMethodDef functions[] = {
//	/* Sentinel */
//	{NULL, NULL}
//};
//
//#define PRE_INIT(TYPE) if (PyType_Ready(&_PyExc_ ## TYPE) < 0) \
//	Py_FatalError("exceptions bootstrapping error.");
//
//#define POST_INIT(TYPE) Py_INCREF(PyExc_ ## TYPE); \
//	PyModule_AddObject(m, # TYPE, PyExc_ ## TYPE); \
//	if (PyDict_SetItemString(bdict, # TYPE, PyExc_ ## TYPE)) \
//		Py_FatalError("Module dictionary insertion problem.");
//
//
//PyMODINIT_FUNC
//_PyExc_Init(void)
//{
//	PyObject *m, *bltinmod, *bdict;
//
//	PRE_INIT(BaseException)
//	PRE_INIT(Exception)
//	PRE_INIT(StandardError)
//	PRE_INIT(TypeError)
//	PRE_INIT(StopIteration)
//	PRE_INIT(GeneratorExit)
//	PRE_INIT(SystemExit)
//	PRE_INIT(KeyboardInterrupt)
//	PRE_INIT(ImportError)
//	PRE_INIT(EnvironmentError)
//	PRE_INIT(IOError)
//	PRE_INIT(OSError)
//#ifdef MS_WINDOWS
//	PRE_INIT(WindowsError)
//#endif
//#ifdef __VMS
//	PRE_INIT(VMSError)
//#endif
//	PRE_INIT(EOFError)
//	PRE_INIT(RuntimeError)
//	PRE_INIT(NotImplementedError)
//	PRE_INIT(NameError)
//	PRE_INIT(UnboundLocalError)
//	PRE_INIT(AttributeError)
//	PRE_INIT(SyntaxError)
//	PRE_INIT(IndentationError)
//	PRE_INIT(TabError)
//	PRE_INIT(LookupError)
//	PRE_INIT(IndexError)
//	PRE_INIT(KeyError)
//	PRE_INIT(ValueError)
//	PRE_INIT(UnicodeError)
//#ifdef Py_USING_UNICODE
//	PRE_INIT(UnicodeEncodeError)
//	PRE_INIT(UnicodeDecodeError)
//	PRE_INIT(UnicodeTranslateError)
//#endif
//	PRE_INIT(AssertionError)
//	PRE_INIT(ArithmeticError)
//	PRE_INIT(FloatingPointError)
//	PRE_INIT(OverflowError)
//	PRE_INIT(ZeroDivisionError)
//	PRE_INIT(SystemError)
//	PRE_INIT(ReferenceError)
//	PRE_INIT(MemoryError)
//	PRE_INIT(BufferError)
//	PRE_INIT(Warning)
//	PRE_INIT(UserWarning)
//	PRE_INIT(DeprecationWarning)
//	PRE_INIT(PendingDeprecationWarning)
//	PRE_INIT(SyntaxWarning)
//	PRE_INIT(RuntimeWarning)
//	PRE_INIT(FutureWarning)
//	PRE_INIT(ImportWarning)
//	PRE_INIT(UnicodeWarning)
//	PRE_INIT(BytesWarning)
//
//	m = Py_InitModule4("exceptions", functions, exceptions_doc,
//		(PyObject *)NULL, PYTHON_API_VERSION);
//	if (m == NULL)
//		return;
//
//	bltinmod = PyImport_ImportModule("__builtin__");
//	if (bltinmod == NULL)
//		Py_FatalError("exceptions bootstrapping error.");
//	bdict = PyModule_GetDict(bltinmod);
//	if (bdict == NULL)
//		Py_FatalError("exceptions bootstrapping error.");
//
//	POST_INIT(BaseException)
//	POST_INIT(Exception)
//	POST_INIT(StandardError)
//	POST_INIT(TypeError)
//	POST_INIT(StopIteration)
//	POST_INIT(GeneratorExit)
//	POST_INIT(SystemExit)
//	POST_INIT(KeyboardInterrupt)
//	POST_INIT(ImportError)
//	POST_INIT(EnvironmentError)
//	POST_INIT(IOError)
//	POST_INIT(OSError)
//#ifdef MS_WINDOWS
//	POST_INIT(WindowsError)
//#endif
//#ifdef __VMS
//	POST_INIT(VMSError)
//#endif
//	POST_INIT(EOFError)
//	POST_INIT(RuntimeError)
//	POST_INIT(NotImplementedError)
//	POST_INIT(NameError)
//	POST_INIT(UnboundLocalError)
//	POST_INIT(AttributeError)
//	POST_INIT(SyntaxError)
//	POST_INIT(IndentationError)
//	POST_INIT(TabError)
//	POST_INIT(LookupError)
//	POST_INIT(IndexError)
//	POST_INIT(KeyError)
//	POST_INIT(ValueError)
//	POST_INIT(UnicodeError)
//#ifdef Py_USING_UNICODE
//	POST_INIT(UnicodeEncodeError)
//	POST_INIT(UnicodeDecodeError)
//	POST_INIT(UnicodeTranslateError)
//#endif
//	POST_INIT(AssertionError)
//	POST_INIT(ArithmeticError)
//	POST_INIT(FloatingPointError)
//	POST_INIT(OverflowError)
//	POST_INIT(ZeroDivisionError)
//	POST_INIT(SystemError)
//	POST_INIT(ReferenceError)
//	POST_INIT(MemoryError)
//	POST_INIT(BufferError)
//	POST_INIT(Warning)
//	POST_INIT(UserWarning)
//	POST_INIT(DeprecationWarning)
//	POST_INIT(PendingDeprecationWarning)
//	POST_INIT(SyntaxWarning)
//	POST_INIT(RuntimeWarning)
//	POST_INIT(FutureWarning)
//	POST_INIT(ImportWarning)
//	POST_INIT(UnicodeWarning)
//	POST_INIT(BytesWarning)
//
//	PyExc_MemoryErrorInst = BaseException_new(&_PyExc_MemoryError, NULL, NULL);
//	if (!PyExc_MemoryErrorInst)
//		Py_FatalError("Cannot pre-allocate MemoryError instance");
//
//	PyExc_RecursionErrorInst = BaseException_new(&_PyExc_RuntimeError, NULL, NULL);
//	if (!PyExc_RecursionErrorInst)
//		Py_FatalError("Cannot pre-allocate RuntimeError instance for "
//						"recursion errors");
//	else {
//		PyBaseExceptionObject *err_inst =
//			(PyBaseExceptionObject *)PyExc_RecursionErrorInst;
//		PyObject *args_tuple;
//		PyObject *exc_message;
//		exc_message = PyString_FromString("maximum recursion depth exceeded");
//		if (!exc_message)
//			Py_FatalError("cannot allocate argument for RuntimeError "
//							"pre-allocation");
//		args_tuple = PyTuple_Pack(1, exc_message);
//		if (!args_tuple)
//			Py_FatalError("cannot allocate tuple for RuntimeError "
//							"pre-allocation");
//		Py_DECREF(exc_message);
//		if (BaseException_init(err_inst, args_tuple, NULL))
//			Py_FatalError("init of pre-allocated RuntimeError failed");
//		Py_DECREF(args_tuple);
//	}
//	Py_DECREF(bltinmod);
//}

void
_PyExc_Fini(void)
{
//	Py_CLEAR(PyExc_MemoryErrorInst);
//	Py_CLEAR(PyExc_RecursionErrorInst);
}
