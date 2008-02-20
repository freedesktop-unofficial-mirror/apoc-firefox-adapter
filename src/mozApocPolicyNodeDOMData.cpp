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

/* vim:expandtab:shiftwidth=4:tabstop=4:
 */
/*
 *******************************************************************************
 * $RCSfile: mozApocPolicyNodeDOMData.cpp,v $
 *
 * Description: 
 *
 * Last change: $Date: 2006/10/30 03:23:53 $ $Revision: 1.1.1.1 $
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved. Use of this 
 * product is subject to license terms. 
 *
 *******************************************************************************
 * Source Code Control System - Header
 *
 * $Header: /export/src/cvs/firefox-apoc-adapter/mozApocPolicyNodeDOMData.cpp,v 1.1.1.1 2006/10/30 03:23:53 sunop Exp $
 *******************************************************************************
 */

#include "mozApocPolicyNodeDOMData.h"
#include "mozApocPolicyComponentDOMData.h"
#include "mozApocStringList.h"
//brian  #include "nsString2.h"
#include "nsLiteralString.h"

#ifndef MOZAPOC_NAVIGATION_TRACE
#ifdef DEBUG
#define MOZAPOC_NAVIGATION_TRACE 0
#else
#define MOZAPOC_NAVIGATION_TRACE 0
#endif
#endif

#define NAVTRACE if (!(MOZAPOC_NAVIGATION_TRACE)); else ::fprintf

mozApocPolicyNodeLayerDOMData::mozApocPolicyNodeLayerDOMData()
: mPolicyData()
, mAttributes(0)
{
}

mozApocPolicyNodeLayerDOMData::mozApocPolicyNodeLayerDOMData(const mozApocPolicyModelNodeRef & aDOMRoot, const nsACString & aComponent)
: mPolicyData(aDOMRoot)
, mAttributes(0)
{
    if (mPolicyData)
    {
        mozApocSetFlags(mAttributes,MOZAPOC_ENTRY_EXISTS_IN_POLICY);
        NS_ASSERTION(mPolicyData->IsNamed(aComponent),
                     "apoc - Name of root element does not match component name");
        AdjustAttributes();
        NS_WARN_IF_FALSE(ExistsInPolicy(), "apoc - Could Not Initialize PolicyNodeLayerDOMData from existing DOM root");
    }
}

nsresult mozApocPolicyNodeLayerDOMData::GetValue(nsAString & _retval) const
{
    if (!mPolicyData) return NS_ERROR_UNEXPECTED;
    if (!IsProperty()) return NS_ERROR_UNEXPECTED;

    // we must be within a <prop> node here
    return mPolicyData->GetValue(_retval);
}

nsresult mozApocPolicyNodeLayerDOMData::GotoChild(const nsACString & aName)
{
    if (!this->ExistsInPolicy()) return NS_OK; // leave unchanged
    
    NS_ENSURE_TRUE(mPolicyData, NS_ERROR_UNEXPECTED);
    
    if (IsProperty())
    {
        NS_WARNING("apoc - unexpected: Attempting to GotoChild of a property node");
        NAVTRACE(stderr,"apoc - No child %s in property !\n", PromiseFlatCString(aName).get());
        mPolicyData.Release();
        AdjustAttributes();
        return NS_ERROR_UNEXPECTED;
    }
    
    nsresult rv = GetChildElement(aName, mPolicyData);

    // In case of failure: clear this node (but ignore the error)
    if (NS_FAILED(rv)) mPolicyData.Release();

    rv = AdjustAttributes();
    NS_WARN_IF_FALSE(NS_SUCCEEDED(rv),"apoc - unexpected: Adjusting attributes for child node failed");
    return rv;
}

nsresult mozApocPolicyNodeLayerDOMData::GetChildElement(const nsACString & aName, mozApocPolicyModelNodeRef & outElement)
{
    NS_PRECONDITION(mPolicyData,"apoc - trying to get child of non-existing element");

    mozApocPolicyModelNodeRef aFoundElement = mPolicyData->GetChild(aName);
    
    if (aFoundElement)
    {
        outElement = aFoundElement;
        return NS_OK;
    }
    else
    {
        return NS_ERROR_FAILURE;
    }
}

nsresult mozApocPolicyNodeLayerDOMData::ListChildren(mozApocUniqueCStringList & aList) const
{
    if (IsNode()) // else we have nothing to add
    {
        NS_ASSERTION(mPolicyData,"apoc - trying to list children of non-existing element");

        for (const mozApocPolicyModelNode * Node = mPolicyData->GetFirstChild(); Node; Node = Node->GetNextSibling())
            aList.Add( Node->GetName() );
    }
    return NS_OK;
}

nsresult mozApocPolicyNodeLayerDOMData::AdjustAttributes()
{
    // propagate access control
    if (mozApocAreAnyFlagsSet(mAttributes,MOZAPOC_ENTRY_PROTECTED_FLAGS))
        mozApocSetFlags(mAttributes,MOZAPOC_ENTRY_PROTECTED);
    mozApocResetFlags(mAttributes, MOZAPOC_LOCAL_PROTECTION_FLAGS);
    
    mozApocResetFlags(mAttributes,MOZAPOC_ENTRY_TYPE_MASK);
    mozApocResetFlags(mAttributes,MOZAPOC_ENTRY_IS_NOT_EMPTY);
    
    if (mPolicyData)
    {
        mozApocEntryFlagValues aDOMFlags = NS_STATIC_CAST(mozApocEntryFlagValues , mPolicyData->GetAttributes());
        mozApocSetFlags( mAttributes, aDOMFlags );
    }

    return NS_OK;
}

mozApocPolicyNodeDOMData::mozApocPolicyNodeDOMData()
: mLayers(nsnull)
, mLayerCount(0)
, mEffectiveAttributes(0)
{
    
}

mozApocPolicyNodeDOMData::mozApocPolicyNodeDOMData(const mozApocPolicyComponentDOMData * aRawLayerData)
: mLayers(nsnull)
, mLayerCount(0)
, mEffectiveAttributes(0)
{
    if (!aRawLayerData) return;
    
    PRInt32 nCount = aRawLayerData->GetLayerCount();
    if (nCount == 0) return;
    
    mLayers = new mozApocPolicyNodeLayerDOMData[nCount];
    // TODO: add error handling
    if (!mLayers) return;
    
    mLayerCount = nCount;
    
    mozApocParsedLayerDataList aRawLayer = aRawLayerData->GetLayers();
    const nsACString & aComponent = aRawLayerData->GetComponentName();
    
    for (PRInt32 i=0; i<mLayerCount; ++i)
    {
        mLayers[i] = mozApocPolicyNodeLayerDOMData(aRawLayer,aComponent);
        mozApocPolicyModelNode::NextInList(aRawLayer);
    }
    AccumulateAttributes();
    NS_WARN_IF_FALSE(mozApocAreAnyFlagsSet(mEffectiveAttributes,MOZAPOC_ENTRY_EXISTS_IN_POLICY), 
                     "apoc - Could Not Initialize PolicyNodeData from existing layer data");
}

mozApocPolicyNodeDOMData::mozApocPolicyNodeDOMData(const mozApocPolicyNodeDOMData & aOther)
{
    if (CopyLayers(aOther.mLayers,aOther.mLayerCount))
        mEffectiveAttributes = aOther.mEffectiveAttributes;
    else
        mEffectiveAttributes = 0;
}

mozApocPolicyNodeDOMData & mozApocPolicyNodeDOMData::operator=(const mozApocPolicyNodeDOMData & aOther)
{
    if (this != &aOther)
    {
        delete [] mLayers;
        if (CopyLayers(aOther.mLayers,aOther.mLayerCount))
            mEffectiveAttributes = aOther.mEffectiveAttributes;
        else
            mEffectiveAttributes = 0;
    }
    return *this;
}

mozApocPolicyNodeDOMData::~mozApocPolicyNodeDOMData()
{
    delete [] mLayers;
}

PRBool mozApocPolicyNodeDOMData::CopyLayers(const mozApocPolicyNodeLayerDOMData * pData, PRInt32 nCount)
{
    mLayers = new mozApocPolicyNodeLayerDOMData[mLayerCount = nCount];
    if (mLayers)
    {
        for (PRInt32 i=0; i<mLayerCount; ++i)
        {
            mLayers[i] = pData[i];
        }
        AccumulateAttributes();
    }
    else
        mLayerCount = 0;

    return mLayers != nsnull;
}

void mozApocPolicyNodeDOMData::AccumulateAttributes()
{
    if (!mLayers) return;
    
    mozApocEntryAttributes attributes = 0;
    mozApocEntryFlagValues valueType = MOZAPOC_ENTRY_TYPE_NONE;
    
    for (PRInt32 i = 0; i<mLayerCount; ++i)
    {
        mozApocEntryAttributes layerAttributes = mLayers[i].GetAttributes();
        // TODO: do consistency checks here
        attributes |= layerAttributes & MOZAPOC_ENTRY_FLAGS_MASK;
        
        // if this layer is protected, further layers can be ignored
        if (mozApocAreAnyFlagsSet(attributes,MOZAPOC_ENTRY_PROTECTED_FLAGS))
            SuppressDataFrom(i+1); // loop will terminate automatically

        mozApocEntryFlagValues layerType = mozApocGetEntryType(layerAttributes);
        if (layerType != MOZAPOC_ENTRY_TYPE_NONE)
        {
            if (valueType ==  MOZAPOC_ENTRY_TYPE_NONE)
                valueType = layerType;
            else if (valueType != layerType)
                valueType = MOZAPOC_ENTRY_TYPE_ERROR;
        }
    }
    // TODO: check for consistency with previous attributes
    mEffectiveAttributes = attributes | mozApocEntryAttributes(valueType);
}

void mozApocPolicyNodeDOMData::SuppressDataFrom(PRInt32 nIndex)
{
    for (PRInt32 i=nIndex; i<mLayerCount; ++i)
        mLayers[i] = mozApocPolicyNodeLayerDOMData();
    mLayerCount = nIndex;
}

void mozApocPolicyNodeDOMData::Clear()
{
    delete [] mLayers, mLayers = 0;
    mLayerCount = 0;
}

nsresult mozApocPolicyNodeDOMData::GotoChild(const nsACString & aName)
{
    nsresult rv = NS_OK;
    for (PRInt32 i=0; i<mLayerCount; ++i)
    {
        rv = mLayers[i].GotoChild(aName);
        if (NS_FAILED(rv)) 
            break;
    }
    if (NS_SUCCEEDED(rv))
        AccumulateAttributes();
    else
        mEffectiveAttributes = 0; // triggers subsequent Clear
    
    if (!mozApocAreAnyFlagsSet(mEffectiveAttributes,MOZAPOC_ENTRY_EXISTS_IN_POLICY))
        Clear();

    else
        NAVTRACE(stderr,"apoc - Got child data for %s [attributes=%#.4x]\n",
                    PromiseFlatCString(aName).get(), unsigned(mEffectiveAttributes));
    
    return rv;
}

nsresult mozApocPolicyNodeDOMData::ListChildren(mozApocUniqueCStringList & aList) const
{
    nsresult rv = NS_OK;
    for (PRInt32 i=0; i<mLayerCount; ++i)
    {
        nsresult rv2 = mLayers[i].ListChildren(aList);
        if (NS_SUCCEEDED(rv)) rv = rv2; 
    }
    return rv;
}

nsresult mozApocPolicyNodeDOMData::GetValue(nsAString & _retval) const
{
    if (!IsProperty()) return NS_ERROR_UNEXPECTED;
    
    PRInt32 nIndexOfValue = mLayerCount-1;
    if (HasValue())
    {
        // we have a value somewhere - lets go find it
        for ( ; nIndexOfValue >= 0; --nIndexOfValue)
            if (mLayers[nIndexOfValue].HasValue())
                break;
    }
    // else simply let the last layer provide an error.
    NS_ASSERTION( nIndexOfValue >= 0, "apoc - Error: PolicyNodeData did not find a value although it used to be there");

    return mLayers[nIndexOfValue].GetValue(_retval);
}

