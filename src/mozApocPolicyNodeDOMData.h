

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


#ifndef MOZAPOC_POLICYNODEDOMDATA_H_INCLUDED
#define MOZAPOC_POLICYNODEDOMDATA_H_INCLUDED

#include "mozApocPolicyNodeAttributes.h"

#include "mozApocPolicyModel.h"

class mozApocPolicyComponentDOMData;
class mozApocUniqueCStringList;

class mozApocPolicyNodeLayerDOMData
{
    mozApocPolicyModelNodeRef   mPolicyData;
    mozApocEntryAttributes      mAttributes;
public:
    mozApocPolicyNodeLayerDOMData();
    explicit
    mozApocPolicyNodeLayerDOMData(const mozApocPolicyModelNodeRef & aDOMRoot, const nsACString & aComponent);
    
    mozApocEntryAttributes GetAttributes() const { return mAttributes; }
    PRBool  ExistsInPolicy() const { return mPolicyData != nsnull; }
    // PRBool  ExistsInPolicy() const { return mozApocExistsInPolicy(mAttributes); }
    PRBool  IsNode()     const { return mozApocIsNode(mAttributes); }
    PRBool  IsProperty() const { return mozApocIsProperty(mAttributes); }
    PRBool  HasValue()   const { return mozApocHasValue(mAttributes); }
    
    nsresult GetValue(nsAString & _retval) const;

    nsresult ListChildren(mozApocUniqueCStringList & aList) const;

    nsresult GotoChild(const nsACString & aName);
private:
    nsresult GetChildElement(const nsACString & aName, mozApocPolicyModelNodeRef & aElement);
    nsresult AdjustAttributes();
};

class mozApocPolicyNodeDOMData
{
    mozApocPolicyNodeLayerDOMData * mLayers;
    PRInt32 mLayerCount;
    mozApocEntryAttributes  mEffectiveAttributes;
public:
    mozApocPolicyNodeDOMData();
    explicit
    mozApocPolicyNodeDOMData(const mozApocPolicyComponentDOMData * aRawLayerData);
    mozApocPolicyNodeDOMData(const mozApocPolicyNodeDOMData & aOther);
    mozApocPolicyNodeDOMData & operator=(const mozApocPolicyNodeDOMData & aOther);
    ~mozApocPolicyNodeDOMData();
    
    mozApocEntryAttributes GetAttributes() const { return mEffectiveAttributes; }
    PRBool  IsReal()     const { return mozApocExistsInPolicy(mEffectiveAttributes); }
    PRBool  IsNode()     const { return mozApocIsNode(mEffectiveAttributes); }
    PRBool  IsProperty() const { return mozApocIsProperty(mEffectiveAttributes); }
    PRBool  HasValue()   const { return mozApocHasValue(mEffectiveAttributes); }
    
    nsresult GetValue(nsAString & _retval) const;
    // TODO: Add support for localized values
    
    nsresult ListChildren(mozApocUniqueCStringList & aList) const;

    nsresult GotoChild(const nsACString & aName);
private:
    PRBool CopyLayers(const mozApocPolicyNodeLayerDOMData * pData, PRInt32 nCount);
    void AccumulateAttributes();
    void SuppressDataFrom(PRInt32 nIndex);
    void Clear();
};

#endif

