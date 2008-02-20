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


#include "mozApocPolicyModelBuilder.h"
#include "mozApocPolicyModel.h"
#include "mozApocNotificationDispatch.h"

#ifndef MOZAPOC_TRACE_MODEL
#define MOZAPOC_TRACE_MODEL 0
#endif
#define BUILDER_TRACE if (!(MOZAPOC_TRACE_MODEL)) ; else ::fprintf

struct mozApocPolicyModelBuilderData
{
    typedef mozApocPolicyModelBuilderData   Data;
    typedef mozApocPolicyModelNode          Node;
    typedef mozApocPolicyModelNodeRef       NodeRef;
    
    explicit
    mozApocPolicyModelBuilderData(NodeRef const & aRootNode, mozApocNotifier * pNotifier)
    : mParent(nsnull)
    , mNotifier(NS_LITERAL_CSTRING(""),pNotifier)
    , mNode()
    , mUnmatchedChildren(aRootNode)
    {
        NS_ASSERTION(!aRootNode || !aRootNode->HasSiblings(),
                     "apoc - Component root actually is a list");
    }
    
    mozApocPolicyModelBuilderData(Data & aParent, const nsACString & aName, 
                                  mozApocEntryAttributes aAttributes)
    : mParent(&aParent)
    , mNotifier(aParent.mNotifier,aName)
    , mNode( Node::ExtractFromNodeList(aParent.mUnmatchedChildren,aName) )
    , mUnmatchedChildren()
    {   
        if (mNode)
        {
            mUnmatchedChildren = mNode->ReleaseChildren();
            mNode->UpdateAttributes(aAttributes, mNotifier);
        }
        else
            mNode = mozApocPolicyModelNode::New(aName,aAttributes,mNotifier);
    }

    ~mozApocPolicyModelBuilderData()
    {
        Node::DisposeNodeList(mUnmatchedChildren,mNotifier);
    }
    
    Data * mParent;

    mozApocNotificationDispatcher mNotifier;

    NodeRef mNode;
    NodeRef mUnmatchedChildren;

};

mozApocPolicyModelBuilder::mozApocPolicyModelBuilder()
: mData(nsnull)
{
}

mozApocPolicyModelBuilder::~mozApocPolicyModelBuilder()
{
    NS_PRECONDITION(mData == nsnull, "apoc - Destroying policy model builder that is still in use");
}

nsresult mozApocPolicyModelBuilder::UpdateModel(const nsACString & aComponent, mozApocNotifier * pNotifier, mozApocPolicyModelNodeRef & aModel)
{
    NS_PRECONDITION(mData == nsnull, "apoc - Reusing policy model builder that is still in use");
    NS_PRECONDITION(!aModel || aModel->IsNamed(aComponent),
                     "apoc - Existing model does not match component being requested");

    mData = new Data(aModel,pNotifier); 
    if (!mData) return NS_ERROR_OUT_OF_MEMORY;

    nsresult rv = this->DoBuildModel(aComponent);
    if (NS_FAILED(rv)) 
    {
        // unwind Data stack
        while (Data * pParent = mData->mParent)
        {
            // this data was abandoned due to error processing
            delete mData;
            mData = pParent;
        }
    }
    
    NS_ASSERTION(!mData->mParent, "apoc - Policy model builder: Unmatched Enter()");
    NS_ASSERTION(!(mData->mNode && mData->mUnmatchedChildren), "apoc - Policy model builder: Orphaned old node found");
    
    aModel = mData->mNode;
    delete mData,mData = nsnull;

    NS_POSTCONDITION(!aModel || aModel->IsNamed(aComponent),
                     "apoc - Newly built model does not match component being requested");

    return rv;
}

nsresult mozApocPolicyModelBuilder::Enter(const nsACString & aName, mozApocEntryAttributes aNewAttributes)
{
    NS_PRECONDITION(mData, "apoc - Illegal use of policy model builder - not in update");

    Data * newData = new Data(*mData, aName, aNewAttributes);
    NS_ENSURE_TRUE(newData && newData->mNode, NS_ERROR_OUT_OF_MEMORY);
    mData = newData;

    NS_POSTCONDITION(!mData->mNode->HasSiblings(), "apoc - Node being processed has siblings.");
    NS_POSTCONDITION( mData->mNode->IsNamed(aName), "apoc - Not using correct node");
    NS_POSTCONDITION( mData->mNode->GetAttributes() == aNewAttributes, "apoc - Not using correct node");
    BUILDER_TRACE(stderr, "apoc model builder - Entered Node %s, attributes = %#.8x \n", 
                    PromiseFlatCString(aName).get(), unsigned(aNewAttributes));
    return NS_OK;
}

nsresult mozApocPolicyModelBuilder::AddPropertyValue(const nsAString & aValue)
{
    NS_PRECONDITION(mData, "apoc - Illegal use of policy model builder - not in update");
    NS_PRECONDITION(mData->mNode, "apoc - Policy model builder: AddPropertyValue() called without a node");

    mozApocEntryAttributes attributes = mData->mNode->GetAttributes();

    NS_WARN_IF_FALSE(!mozApocHasValue(attributes), "apoc - Policy model builder: multiple values for a node; ignoring all but last");

    mozApocSetFlags( attributes, MOZAPOC_ENTRY_HASVALUE );
    mData->mNode->UpdateAttributes(attributes, mData->mNotifier);
    
    return this->SetPropertyValue(aValue);
}

nsresult mozApocPolicyModelBuilder::SetPropertyValue(const nsAString & aValue)
{
    NS_PRECONDITION(mData, "apoc - Illegal use of policy model builder - not in update");
    NS_PRECONDITION(mData->mNode, "apoc - Policy model builder: SetPropertyValue() called without a node");

    BUILDER_TRACE(stderr, "apoc model builder -     Setting property value %s \n", 
                    NS_ConvertUCS2toUTF8(aValue).get());

    nsresult rv = mData->mNode->UpdateValue(aValue, mData->mNotifier);
    return rv;
}

nsresult mozApocPolicyModelBuilder::Leave()
{
    NS_PRECONDITION(mData, "apoc - Illegal use of policy model builder - not in update");
    NS_PRECONDITION(mData->mParent, "apoc - Policy model builder: Unmatched Leave()");
    NS_PRECONDITION(mData->mNode, "apoc - Policy model builder: Leave() called without a node");

    nsresult rv = NS_OK;
    
    Data * aCompletedData = mData;
    mData = mData->mParent;

    BUILDER_TRACE(stderr, "apoc model builder - Left current node \n"); 
    if (mData->mNode)
    {
        NS_ASSERTION(mData->mParent,"apoc - Found node within the root data element");
        rv = mData->mNode->AddChild(aCompletedData->mNode);
    }
    else // set result to the root non-node
    {
        NS_ASSERTION(!mData->mParent,"apoc - Found non-node that is not root");
        mData->mNode = aCompletedData->mNode;
        BUILDER_TRACE(stderr, "apoc model builder - That was the root node: Building the model completed \n"); 
    }
    delete aCompletedData;
    return rv;
}

