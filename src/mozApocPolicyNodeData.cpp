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
 * $RCSfile: mozApocPolicyNodeData.cpp,v $
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
 * $Header: /export/src/cvs/firefox-apoc-adapter/mozApocPolicyNodeData.cpp,v 1.1.1.1 2006/10/30 03:23:53 sunop Exp $
 *******************************************************************************
 */

#include "mozApocPolicyNodeData.h"
#include "mozApocPolicyComponentData.h"
#include "mozApocPolicyModel.h"

#ifndef MOZAPOC_VALTRACE
#define MOZAPOC_VALTRACE 0
#endif 

#define VALTRACE if (!(MOZAPOC_VALTRACE)) ; else ::fprintf

mozApocPolicyNodeData::mozApocPolicyNodeData()
: mRealData(nsnull)
, mAttributes(0)
{
    
}

mozApocPolicyNodeData::mozApocPolicyNodeData(const mozApocPolicyComponentData * aComponentData)
: mRealData(nsnull)
, mAttributes(0)
{
    if (aComponentData)
        Assign(aComponentData->getRootNode());
}

mozApocPolicyNodeData::mozApocPolicyNodeData(const mozApocPolicyModelNode * aRawData)
: mRealData(nsnull)
, mAttributes(0)
{
    Assign(aRawData);
}

mozApocPolicyNodeData::mozApocPolicyNodeData(const mozApocPolicyNodeData & aOther)
: mRealData(nsnull)
, mAttributes(0)
{
    Assign(aOther.mRealData);
}

mozApocPolicyNodeData & mozApocPolicyNodeData::operator=(const mozApocPolicyNodeData & aOther)
{
    Assign(aOther.mRealData);
    return *this;
}

mozApocPolicyNodeData::~mozApocPolicyNodeData()
{
    Assign(nsnull);
}

void mozApocPolicyNodeData::Assign(const mozApocPolicyModelNode * aNewData)
{
    if (aNewData) aNewData->AcquireReference();
    if (mRealData) mRealData->ReleaseReference();
    mRealData = aNewData;
    mAttributes = mRealData ? mRealData->GetAttributes() : 0;
}

PRBool mozApocPolicyNodeData::HasRealChild(const nsACString & aName) const
{
    nsresult dummyrv = NS_OK;
    return mRealData && mRealData->GetChild(aName,&dummyrv) != nsnull;
}

nsresult mozApocPolicyNodeData::GotoChild(const nsACString & aName)
{
    nsresult rv = NS_OK;
    if (mRealData)
        Assign( mRealData->GetChild(aName,&rv) );
    return rv;
}

nsresult mozApocPolicyNodeData::GetValue(nsAString & _retval) const
{
    nsresult rv = NS_ERROR_UNEXPECTED;
    if (mRealData)
        rv = mRealData->GetValue(_retval);

    if (NS_SUCCEEDED(rv))
        VALTRACE(stderr,"apoc - Providing node value: '%s'\n", NS_ConvertUCS2toUTF8(_retval).get());
    else
        NS_WARNING("apoc - GetNodeValue failed");
    
    return rv;
}

