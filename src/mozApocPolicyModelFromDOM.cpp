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
 * $RCSfile: mozApocPolicyModelFromDOM.cpp,v $
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
 * $Header: /export/src/cvs/firefox-apoc-adapter/mozApocPolicyModelFromDOM.cpp,v 1.1.1.1 2006/10/30 03:23:53 sunop Exp $
 *******************************************************************************
 */

#include "mozApocPolicyModelFromDOM.h"
#include "mozApocPolicyComponentDOMData.h"
#include "mozApocPolicyNodeDOMData.h"
#include "mozApocStringList.h"

nsresult mozApocBuildPolicyModelFromDOM::BuildModelForNode(const nsACString & aNodeName, mozApocPolicyNodeDOMData const & aDOMNode)
{
    NS_PRECONDITION(aDOMNode.IsReal(),"apoc - Unexpected attempt to build model for non-existing node");

    nsresult rv = this->Enter(aNodeName, aDOMNode.GetAttributes());
    NS_ENSURE_SUCCESS(rv,rv);

    if (aDOMNode.HasValue())
    {
        nsAutoString aValue;
        rv = aDOMNode.GetValue(aValue);
        NS_ENSURE_SUCCESS(rv,rv);

        rv = this->SetPropertyValue(aValue);
        NS_ENSURE_SUCCESS(rv,rv);
    }
    if (aDOMNode.IsNode())
    {
        mozApocUniqueCStringList aChildList;
        rv = aDOMNode.ListChildren(aChildList);

        for (mozApocUniqueCStringList::iterator it = aChildList.Iterate(); it; it = it->Next())
        {
            mozApocPolicyNodeDOMData aChildNode(aDOMNode);
            nsresult rv2 = aChildNode.GotoChild(it->Value());
            if (NS_SUCCEEDED(rv2))
                rv2 = BuildModelForNode(it->Value(),aChildNode);
            
            if (NS_SUCCEEDED(rv)) rv = rv2;
        }
        NS_ENSURE_SUCCESS(rv,rv);
    }
    rv = this->Leave();

    return rv;
}

nsresult mozApocBuildPolicyModelFromDOM::DoBuildModel(const nsACString & aComponent)
{
    if (! mDOMData)
        return NS_OK;

    NS_ASSERTION(mDOMData->GetComponentName().Equals(aComponent), "apoc - Component To Build does not match DOM data");

    mozApocPolicyNodeDOMData aRootNode(mDOMData);
    return BuildModelForNode(aComponent, aRootNode);
}


