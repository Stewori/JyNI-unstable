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

/*
 * JyAttributes.c
 *
 *  Created on: 23.04.2013, 03:29:44
 *      Author: Stefan Richthofer
 */
#include <JyNI.h>

const char* JyAttributePyChecksum     = "pyCs";
const char* JyAttributeJyChecksum     = "jyCs";
const char* JyAttributeSyncFunctions  = "sync";
const char* JyAttributeModuleFile     = "modf";
const char* JyAttributeModuleName     = "modn";
const char* JyAttributeTypeName       = "typn";
const char* JyAttributeStringInterned = "strI";
const char* JyAttributeSetEntry       = "setE";
const char* JyAttributeJyGCHead       = "jyGC";
const char* JyAttributeJyGCRefTmp     = "gcrf";
const char* JyAttributeWeakRefCount   = "wrct";
//const char* JyAttributeSubDelegFlags  = "sdlr";

//defaults to 0; note that on alloc this value is added to the anyway allocated size sizeof(PyObjectHead)
//const char* JyAttributeTruncateSize = "trSi";

inline void JyNI_ClearJyAttributes(JyObject* obj)
{
	JyAttribute* nxt = obj->attr;
	while (nxt != NULL)
	{
		JyAttribute* nxt2 = nxt->next;
		//if (nxt->value != NULL)
//		if ((nxt->flags & JY_ATTR_OWNS_VALUE_FLAG_MASK) && (nxt->value))
//			free(nxt->value);
		JyNI_ClearJyAttributeValue(nxt);
		free(nxt);
		nxt = nxt2;
	}
	obj->attr = NULL;
}

inline void JyNI_ClearJyAttributeValue(JyAttribute* att)
{
	if ((att->flags & JY_ATTR_OWNS_VALUE_FLAG_MASK) && (att->value))
	{
		if (att->flags & JY_ATTR_VAR_SIZE_FLAG_MASK)
		{
			JyAttributeElement* elem = (JyAttributeElement*) att->value;
			while (elem != NULL)
			{
				JyAttributeElement* tmp = elem;
				elem = elem->next;
				if (att->flags & JY_ATTR_JWEAK_VALUE_FLAG_MASK) {
					env();
					(*env)->DeleteWeakGlobalRef(env, (jweak) tmp->value);
				} else
					free(tmp->value);
				free(tmp);
			}
		} else {
			if (att->flags & JY_ATTR_JWEAK_VALUE_FLAG_MASK) {
				env();
				(*env)->DeleteWeakGlobalRef(env, (jweak) att->value);
			} else
				free(att->value);
		}
	}
}

inline void JyNI_ClearJyAttribute(JyObject* obj, const char* name)
{
	JyAttribute* nxt = obj->attr;
	while (nxt != NULL)
	{
		JyAttribute* nxt2 = nxt->next;
		if (nxt2 != NULL && nxt2->name == name)
		{
			nxt->next = nxt2->next;
			//if ((nxt2->flags & JY_ATTR_OWNS_VALUE_FLAG_MASK) && (nxt2->value))
			//	free(nxt2->value);
			JyNI_ClearJyAttributeValue(nxt);
			free(nxt2);
		} else
			nxt = nxt2;
	}
}

//No hashing is done, since use of JyAttributes is expected to be rare.
inline void* JyNI_GetJyAttribute(JyObject* obj, const char* name)
{
	JyAttribute* nxt = obj->attr;
	while (nxt != NULL)
	{
//		jputsLong(__LINE__);
//		jputs(nxt->name);
		if (nxt->name == name)
			return nxt->value;
		else
			nxt = nxt->next;
	}
	return NULL;
}

//No hashing is done, since use of JyAttributes is expected to be rare.
inline jboolean JyNI_HasJyAttribute(JyObject* obj, const char* name)
{
	JyAttribute* nxt = obj->attr;
	while (nxt != NULL)
	{
		if (nxt->name == name)
			return JNI_TRUE;
		else
			nxt = nxt->next;
	}
	return JNI_FALSE;
}

//We don't check, whether the attribute already exists, because
//usually the caller tries to retrieve the attribute before adding
//it. If it was already present, the caller would not add it anyway.
inline void JyNI_AddJyAttribute(JyObject* obj, const char* name, void* value)
{
	JyAttribute* nat = malloc(sizeof(JyAttribute));
	nat->name = name;
	nat->value = value;
	nat->flags = 0;
	nat->next = obj->attr;
	obj->attr = nat;
}

inline void JyNI_AddJyAttributeWithFlags(JyObject* obj, const char* name, void* value, char flags)
{
	JyAttribute* nat = malloc(sizeof(JyAttribute));
	nat->name = name;
	nat->value = value;
	nat->flags = flags;
	nat->next = obj->attr;
	obj->attr = nat;
}

inline void JyNI_AddOrSetJyAttribute(JyObject* obj, const char* name, void* value)
{
	JyAttribute* nxt = obj->attr;
	while (nxt != NULL)
	{
		if (nxt->name == name)
		{
			JyNI_ClearJyAttributeValue(nxt);
//			if ((nxt->flags & JY_ATTR_OWNS_VALUE_FLAG_MASK) && (nxt->value))
//				free(nxt->value);
			nxt->value = value;
			nxt->flags = 0;
			return;
		}
		else
			nxt = nxt->next;
	}
	JyNI_AddJyAttribute(obj, name, value);
}

inline void JyNI_AddOrSetJyAttributeWithFlags(JyObject* obj, const char* name, void* value, char flags)
{
	JyAttribute* nxt = obj->attr;
	while (nxt != NULL)
	{
		if (nxt->name == name)
		{
			JyNI_ClearJyAttributeValue(nxt);
//			if ((nxt->flags & JY_ATTR_OWNS_VALUE_FLAG_MASK) && (nxt->value))
//				free(nxt->value);
			nxt->value = value;
			nxt->flags = flags;
			return;
		}
		else
			nxt = nxt->next;
	}
	JyNI_AddJyAttributeWithFlags(obj, name, value, flags);
}
