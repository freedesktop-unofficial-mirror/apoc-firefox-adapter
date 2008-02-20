

/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 * 
 * Copyright 2007 Sun Microsystems, Inc. All rights reserved.
 * 
 * The contents of this file are subject to the terms of either
 * the GNU General Public License Version 2 only ("GPL") or
 * the Common Development and Distribution License("CDDL")
 * (collectively, the "License"). You may not use this file
 * except in compliance with the License. You can obtain a copy
 * of the License at www.sun.com/CDDL or at COPYRIGHT. See the
 * License for the specific language governing permissions and
 * limitations under the License. When distributing the software,
 * include this License Header Notice in each file and include
 * the License file at /legal/license.txt. If applicable, add the
 * following below the License Header, with the fields enclosed
 * by brackets [] replaced by your own identifying information:
 * "Portions Copyrighted [year] [name of copyright owner]"
 * 
 * Contributor(s):
 * 
 * If you wish your version of this file to be governed by
 * only the CDDL or only the GPL Version 2, indicate your
 * decision by adding "[Contributor] elects to include this
 * software in this distribution under the [CDDL or GPL
 * Version 2] license." If you don't indicate a single choice
 * of license, a recipient has the option to distribute your
 * version of this file under either the CDDL, the GPL Version
 * 2 or to extend the choice of license to its licensees as
 * provided above. However, if you add GPL Version 2 code and
 * therefore, elected the GPL Version 2 license, then the
 * option applies only if the new code is made subject to such
 * option by the copyright holder.
 */


#ifndef MOZAPOC_POLICYNODEATTRIBUTES_H_INCLUDED
#define MOZAPOC_POLICYNODEATTRIBUTES_H_INCLUDED

#include "prtypes.h"

enum mozApocEntryFlagValues
{
    MOZAPOC_ENTRY_TYPE_NONE         = 0,
    MOZAPOC_ENTRY_TYPE_STRING,
    MOZAPOC_ENTRY_TYPE_INT,
    MOZAPOC_ENTRY_TYPE_BOOL,
    MOZAPOC_ENTRY_TYPE_BINARY,
    MOZAPOC_ENTRY_TYPE_OTHER,
    
    MOZAPOC_ENTRY_TYPE_ERROR,
    MOZAPOC_ENTRY_TYPE_MASK         = 0x00FF,
    
    MOZAPOC_ENTRY_EXISTS_IN_POLICY  = 0x0100,
    MOZAPOC_ENTRY_FINALIZED         = 0x0200,
    MOZAPOC_ENTRY_MANDATORY         = 0x0400,
    MOZAPOC_ENTRY_PROTECTED         = 0x0800,
    MOZAPOC_ENTRY_PROPERTY          = 0x1000,
    MOZAPOC_ENTRY_HASVALUE          = 0x2000,
    
    MOZAPOC_ENTRY_IS_NOT_EMPTY =    MOZAPOC_ENTRY_EXISTS_IN_POLICY |
                                    MOZAPOC_ENTRY_PROPERTY |
                                    MOZAPOC_ENTRY_HASVALUE ,
    MOZAPOC_ENTRY_IS_PROPERTY =     MOZAPOC_ENTRY_EXISTS_IN_POLICY |
                                    MOZAPOC_ENTRY_PROPERTY ,
    MOZAPOC_ENTRY_PROTECTED_FLAGS = MOZAPOC_ENTRY_PROTECTED |
                                    MOZAPOC_ENTRY_FINALIZED,
    MOZAPOC_ENTRY_MANDATORY_FLAGS = MOZAPOC_ENTRY_PROTECTED |
                                    MOZAPOC_ENTRY_MANDATORY,
    MOZAPOC_LOCAL_PROTECTION_FLAGS= MOZAPOC_ENTRY_FINALIZED |
                                    MOZAPOC_ENTRY_MANDATORY,

    MOZAPOC_ENTRY_FLAGS_MASK   = 0xFF00
};
typedef PRUint16 mozApocEntryAttributes;

inline mozApocEntryFlagValues mozApocGetEntryType(mozApocEntryAttributes attr)
{ return mozApocEntryFlagValues(attr & MOZAPOC_ENTRY_TYPE_MASK); }

inline PRBool mozApocAreAnyFlagsSet(mozApocEntryAttributes attr, mozApocEntryFlagValues flags)
{ return (attr & flags) != 0; }

inline PRBool mozApocAreAllFlagsSet(mozApocEntryAttributes attr, mozApocEntryFlagValues flags)
{ return (attr & flags) == flags; }

inline void mozApocSetFlags(mozApocEntryAttributes & attr, mozApocEntryFlagValues flags)
{ attr |= flags; }

inline void mozApocResetFlags(mozApocEntryAttributes & attr, mozApocEntryFlagValues flags)
{ attr &= ~mozApocEntryAttributes(flags); }

inline PRBool  mozApocExistsInPolicy(mozApocEntryAttributes attr) 
{ return mozApocAreAllFlagsSet(attr,MOZAPOC_ENTRY_EXISTS_IN_POLICY); }

inline PRBool  mozApocIsNode(mozApocEntryAttributes attr) 
{ return (attr & MOZAPOC_ENTRY_IS_PROPERTY) == MOZAPOC_ENTRY_EXISTS_IN_POLICY; }

inline PRBool  mozApocIsProperty(mozApocEntryAttributes attr) 
{ return mozApocAreAllFlagsSet(attr,MOZAPOC_ENTRY_IS_PROPERTY); }

inline PRBool  mozApocHasValue(mozApocEntryAttributes attr) 
{ return mozApocAreAllFlagsSet(attr,MOZAPOC_ENTRY_HASVALUE); }


#endif

