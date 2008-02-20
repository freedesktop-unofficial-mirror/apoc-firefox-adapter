

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


#ifndef MOZAPOC_POLICYNODEDATA_H_INCLUDED
#define MOZAPOC_POLICYNODEDATA_H_INCLUDED

#include "mozApocPolicyNodeAttributes.h"
#include "nsAString.h"

class mozApocPolicyModelNode;
class mozApocPolicyComponentData;

class mozApocPolicyNodeData
{
    typedef const mozApocPolicyModelNode * rawData;
    rawData mRealData;
    mozApocEntryAttributes   mAttributes;
public:
    mozApocPolicyNodeData();
    explicit
    mozApocPolicyNodeData(rawData aRawData);
    explicit
    mozApocPolicyNodeData(const mozApocPolicyComponentData * aComponentData);
    mozApocPolicyNodeData(const mozApocPolicyNodeData & aOther);
    mozApocPolicyNodeData & operator=(const mozApocPolicyNodeData & aOther);
    ~mozApocPolicyNodeData();
    
    mozApocEntryAttributes GetAttributes() const { return mAttributes; }
    PRBool  IsProperty() const { return mozApocIsProperty(mAttributes); }
    PRBool  IsNode()     const { return mozApocIsNode(mAttributes); }
    PRBool  HasValue()   const { return mozApocHasValue(mAttributes); }
    
    // TODO: Add support for localized values
    nsresult GetValue(nsAString & _retval) const;
    
    PRBool IsReal() const { return mRealData != nsnull; }
    PRBool HasRealChild(const nsACString & aName) const;
    nsresult GotoChild(const nsACString & aName);
private:
    void Assign(rawData aNewData);
};

#endif

