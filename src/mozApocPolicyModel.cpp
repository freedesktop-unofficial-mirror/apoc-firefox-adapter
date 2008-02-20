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


#include "mozApocPolicyModel.h"

#include "mozApocPolicyComponentData.h"
#include "mozApocNotificationDispatch.h"

#include "pratom.h"

#ifndef MOZAPOC_TRACE_MODEL
#define MOZAPOC_TRACE_MODEL 0 
#endif

#define MODEL_TRACE if (!(MOZAPOC_TRACE_MODEL )) ; else ::fprintf

struct mozApocPolicyModelNode::ValueData
{
    ValueData(nsAString const & aValue, mozApocNotificationDispatcher const & aNotifier)
    : PolicyValue(aValue)
    , HasValue(PR_TRUE)
    , NativeValue(aNotifier.RememberNativeValue())
    {
    }

   ValueData(PRBool /*bLockState*/, mozApocNotificationDispatcher const & aNotifier)
    : PolicyValue()
    , HasValue(PR_FALSE)
    , NativeValue(aNotifier.RememberNativeValue())
    {
    }

    nsString PolicyValue;
    PRBool   HasValue;
    const mozApocNativeSettingsMemento NativeValue;
};

mozApocPolicyModelNode::mozApocPolicyModelNode(const nsACString & aName)
: mRefCount(0)
, mNext()
, mChildren()
, mName(aName)
, mAttributes(0)
, mValue(nsnull)
{
    MODEL_TRACE(stderr, "apoc data model: Created Node '%s' at %p\n", 
                PromiseFlatCString(aName).get(), (void*)this);
}

mozApocPolicyModelNodeRef mozApocPolicyModelNode::New(const nsACString & aName, 
                                                        mozApocEntryAttributes aAttributes, 
                                                        mozApocNotificationDispatcher const & aNotifier)
{ 
    typedef mozApocPolicyModelNode node;
    node * Node = new node(aName);
    if (Node) Node->UpdateAttributes(aAttributes,aNotifier);
    return Node;
}


mozApocPolicyModelNode::~mozApocPolicyModelNode()
{
    MODEL_TRACE(stderr, "apoc data model: Destroyed Node '%s' at %p\n", PromiseFlatCString(mName).get(), (void*)this);
    if (mValue && mValue->NativeValue)
        mozApocDiscardNativeSettings(mValue->NativeValue);
    delete mValue;
}

void mozApocPolicyModelNode::AcquireReference() const
{
    PR_AtomicIncrement(&mRefCount);
}

void mozApocPolicyModelNode::ReleaseReference() const
{
    if (0 == PR_AtomicDecrement(&mRefCount))
        delete this;
}

mozApocPolicyModelNode * mozApocPolicyModelNode::ImplGetChild(const nsACString & aName) const
{
    typedef mozApocPolicyModelNode Node;
    for (Node * child = mChildren.Get(); child; child = child->mNext.Get())
    {
        if (child->IsNamed(aName))
            return child;
    }
    return nsnull;
}

const mozApocPolicyModelNode * mozApocPolicyModelNode::GetChild(const nsACString & aName, nsresult * _rv) const
{
    mozApocPolicyModelNode * result;
    nsresult rv;
    
    if (!mozApocIsProperty(mAttributes))  
    {
        result = ImplGetChild(aName);
        rv = NS_OK;
    }
    
    else
    {
        NS_ASSERTION(!mChildren,"apoc - Found children in property");
        result = nsnull;
        rv = NS_ERROR_UNEXPECTED;
    }
        
    
    if (_rv) *_rv = rv;
    return result;
}

nsresult mozApocPolicyModelNode::GetValue(nsAString & _retval) const
{
    nsresult rv;
    if (mValue && mValue->HasValue)
    {
        _retval = mValue->PolicyValue;
        rv = NS_OK;
    }
    else
        rv = NS_ERROR_UNEXPECTED;
    
    return rv;
}

nsresult mozApocPolicyModelNode::UpdateValue(const nsAString & aNewValue, mozApocNotificationDispatcher const & aNotifier)
{
    MODEL_TRACE(stderr, "apoc data model: Setting value of node '%s' at %p to '%s'\n",
                PromiseFlatCString(mName).get(), (void*)this, NS_ConvertUCS2toUTF8(aNewValue).get());

    NS_ENSURE_TRUE(mozApocHasValue(mAttributes), NS_ERROR_UNEXPECTED);
    if (!mValue) 
        mValue = new ValueData(aNewValue,aNotifier);
    else if (mValue->HasValue)
        mValue->PolicyValue = aNewValue;
    else
    {
        mValue->PolicyValue = aNewValue;
        mValue->HasValue = PR_TRUE;
        // remove the original lock-only notification
        aNotifier.NotifyLockChange(PR_FALSE);
    }

    if (!mValue) 
        return NS_ERROR_OUT_OF_MEMORY;

    aNotifier.NotifyValueChange(mValue->PolicyValue,this->IsProtected());

    return NS_OK;
}

void mozApocPolicyModelNode::UpdateAttributes(mozApocEntryAttributes aNewAttributes, mozApocNotificationDispatcher const & aNotifier)
{
    MODEL_TRACE(stderr, "apoc data model: Updating attributes of Node '%s' at %p to %#.8x\n", 
                PromiseFlatCString(mName).get(), (void*)this, unsigned(aNewAttributes));

    // get protected state before attribute change
    PRBool bWasProtected = this->IsProtected();

    mAttributes = aNewAttributes;

    PRBool bProtectionChanged = (this->IsProtected() != bWasProtected);

    if (!mozApocHasValue(aNewAttributes))
    {
        // check if this is a property that is being discarded -> restore native data
        if (mValue)
        {
            if (mValue->HasValue || bProtectionChanged)
            {
                aNotifier.RestoreAndNotifyNativeValue(mValue->NativeValue);
                delete mValue, mValue = nsnull;
            }
        }
        // check, if this is a lock-only property
        if (bProtectionChanged && mozApocIsProperty(aNewAttributes))
        {
            PRBool bLocked = this->IsProtected();
            if (bLocked)
            {
                NS_ASSERTION(!mValue, "Logic error: Leaking ValueData object");
           
                mValue = new ValueData(bLocked,aNotifier);
                aNotifier.NotifyLockChange(bLocked);
            }
        }
    } 
}

nsresult mozApocPolicyModelNode::AddChild(Ref const & aNode)
{
    NS_PRECONDITION(aNode, "apoc - Trying to add a NULL node as child");
    NS_PRECONDITION(!aNode->mNext, "apoc - Trying to add an entire list as child");
    NS_PRECONDITION(! ImplGetChild(aNode->mName), "apoc - Child being added already exists");
    
    MODEL_TRACE(stderr, "apoc data model: Adding Node '%s' at %p as new child to Node '%s' at %p\n", 
                PromiseFlatCString(aNode->mName).get(), (void*)aNode.Get(), 
                PromiseFlatCString(mName).get(), (void*)this ); 

    aNode->mNext = mChildren;
    mChildren = aNode;

    return NS_OK;
}

mozApocPolicyModelNodeRef mozApocPolicyModelNode::ReleaseChildren()
{
    MODEL_TRACE(stderr, "apoc data model: Releasing children of Node '%s' at %p (first child was at %p)\n", 
                PromiseFlatCString(mName).get(), (void*)this, (void*)mChildren.Get() ); 

    return mChildren.Release();
}

void mozApocPolicyModelNode::AppendToNodeList(Ref & aList, Ref const & aElement)
{
    Ref * pNode = &aList; 
    while (*pNode)
        pNode = &(*pNode)->mNext;
    *pNode = aElement;
}

mozApocPolicyModelNodeRef mozApocPolicyModelNode::ExtractFromNodeList(mozApocPolicyModelNodeRef & aList, const nsACString & aName)
{
    mozApocPolicyModelNodeRef * ppNode = &aList; 
    while (*ppNode)
    {
        mozApocPolicyModelNodeRef pNode = *ppNode;
        if (pNode->IsNamed(aName))
        {
            // unlink, but don't release pNode
            *ppNode = pNode->mNext.Release();
            
            return pNode;
        }
            
        ppNode = &pNode->mNext;
    }
    return Ref();
}

mozApocPolicyModelNodeRef mozApocPolicyModelNode::ImplDisposeAndGetNext(mozApocNotificationDispatcher const & aNotifier)
{
    MODEL_TRACE(stderr, "apoc data model:   Disposing Node '%s' at %p \n", 
                PromiseFlatCString(mName).get(), (void*)this ); 

    UpdateAttributes(0, aNotifier);
    NS_ASSERTION(!mValue, "apoc - Value not disposed properly");

    DisposeNodeList(mChildren, aNotifier);

    MODEL_TRACE(stderr, "apoc data model:   Done disposing Node '%s' at %p \n", 
                PromiseFlatCString(mName).get(), (void*)this ); 

    return mNext.Release();
}

void mozApocPolicyModelNode::DisposeNodeList(mozApocPolicyModelNodeRef & aList, mozApocNotificationDispatcher const & aParentNotifier)
{
    MODEL_TRACE(stderr, "apoc data model: Disposing list of nodes starting at %p\n", (void*)aList.Get());

    Ref aNode = aList.Release(); 
    while (aNode)
    {
        // first dispose, then release
        aNode = aNode->ImplDisposeAndGetNext(aParentNotifier.GetChildNotifier(aNode->mName));
    }
}

