

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


#ifndef MOZAPOC_POLICYMODEL_H_INCLUDED
#define MOZAPOC_POLICYMODEL_H_INCLUDED

#include "mozApocPolicyNodeAttributes.h"
#include "nsString.h"

class mozApocPolicyModelNode;
class mozApocPolicyModelNodeRef
{
    struct NoAcquire {};

    mozApocPolicyModelNode * mPtr;
public:
    mozApocPolicyModelNodeRef()
    : mPtr(nsnull)
    {}

    mozApocPolicyModelNodeRef(mozApocPolicyModelNode * aPtr)
    : mPtr(aPtr) 
    { Acquire_(); }
    
    mozApocPolicyModelNodeRef(mozApocPolicyModelNode * aPtr, NoAcquire)
    : mPtr(aPtr) 
    {}
    
    mozApocPolicyModelNodeRef(const mozApocPolicyModelNodeRef& aOther)
    : mPtr(aOther.mPtr)
    { Acquire_(); }

    mozApocPolicyModelNodeRef & operator=(const mozApocPolicyModelNodeRef& aOther)
    {
        aOther.Acquire_();
        this->Release_();
        mPtr = aOther.mPtr;
        return *this;
    }

    ~mozApocPolicyModelNodeRef()
    { Release_(); }

    // support for NULL check and comparison
    struct OpaqueValue_;
    operator OpaqueValue_ const * () const 
    { return NS_REINTERPRET_CAST(OpaqueValue_ const *,mPtr); }
    
    mozApocPolicyModelNode * Get() const { return mPtr; }

    mozApocPolicyModelNode & operator * () const { return *mPtr; }
    mozApocPolicyModelNode * operator -> () const { return mPtr; }

    mozApocPolicyModelNodeRef Release()
    {
        mozApocPolicyModelNode * aPtr = mPtr;
        mPtr = 0;
        return mozApocPolicyModelNodeRef(aPtr, NoAcquire());
    }
private:
    void Acquire_() const;
    void Release_() const;
};

class mozApocNotificationDispatcher;
class mozApocPolicyModelNode
{
private:
    mozApocPolicyModelNode(const nsACString & aName);
    
public:
    void AcquireReference() const;
    void ReleaseReference() const;

public:
    typedef mozApocPolicyModelNodeRef Ref;
    static Ref New(const nsACString & aName, mozApocEntryAttributes aAttributes, 
                    mozApocNotificationDispatcher const & aNotifier);

    // explicit mozApocPolicyModelNode(rawData aRawData);
    ~mozApocPolicyModelNode();
    
    PRBool HasChildren() const { return mChildren != nsnull; }
    PRBool HasSiblings() const { return mNext != nsnull; }
    PRBool IsNamed(const nsACString & aName) const { return mName.Equals(aName); }
    
    nsAFlatCString const & GetName() const { return mName; }
    mozApocEntryAttributes GetAttributes() const { return mAttributes; }
    PRBool IsProtected() const { return mozApocAreAnyFlagsSet(mAttributes,MOZAPOC_ENTRY_PROTECTED_FLAGS); }

    // TODO: Add support for localized values
    nsresult GetValue(nsAString & _retval) const;
    PRBool   HasValue() const { return mValue != nsnull; }

    const mozApocPolicyModelNode * GetChild(const nsACString & aName, nsresult * _rv) const;

    Ref GetChild(const nsACString & aName) { return ImplGetChild(aName); }
    
    const mozApocPolicyModelNode * GetFirstChild() const { return mChildren.Get(); }
    const mozApocPolicyModelNode * GetNextSibling() const { return mNext.Get(); }

    nsresult UpdateValue(const nsAString & aNewValue, mozApocNotificationDispatcher const & aNotifier);
    void UpdateAttributes(mozApocEntryAttributes aNewAttributes, mozApocNotificationDispatcher const & aNotifier);
    
    nsresult AddChild(Ref const & aNode);
    Ref ReleaseChildren();

    static void NextInList(Ref & aListPosition) { aListPosition = aListPosition->mNext; }
    static void AppendToNodeList(Ref & aList, Ref const & aElement);
    static mozApocPolicyModelNodeRef ExtractFromNodeList(Ref & aList, const nsACString & aName);
    static void DisposeNodeList(mozApocPolicyModelNodeRef & aList, mozApocNotificationDispatcher const & aNotifier);
private:
    mozApocPolicyModelNode * ImplGetChild(const nsACString & aName) const;

    mozApocPolicyModelNodeRef ImplDisposeAndGetNext(mozApocNotificationDispatcher const & aNotifier);
private:
    mutable PRInt32 mRefCount;

    mozApocPolicyModelNodeRef mNext;
    mozApocPolicyModelNodeRef mChildren;
    
    nsCString                mName;
    mozApocEntryAttributes   mAttributes;

    struct ValueData;
    ValueData *              mValue;
private:
    // not implemented:
    mozApocPolicyModelNode(const mozApocPolicyModelNode & aOther);
    mozApocPolicyModelNode & operator=(const mozApocPolicyModelNode & aOther);
};

inline
void mozApocPolicyModelNodeRef::Acquire_() const 
{ if (mPtr) mPtr->AcquireReference(); }

inline
void mozApocPolicyModelNodeRef::Release_() const
{ if (mPtr) mPtr->ReleaseReference(); }

#endif

