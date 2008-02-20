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


#ifndef MOZAPOC_POLICYDATA_H_INCLUDED
#define MOZAPOC_POLICYDATA_H_INCLUDED

#include "mozApocPolicyNodeData.h"
#include "nsID.h"

class mozApocPolicyBackend;
class mozApocPolicyComponentData;
class mozApocPathParser;

class mozApocPolicyEntry
{
    mozApocPolicyNodeData mData;
    nsresult mErrCode;
public:
    mozApocPolicyEntry();
    mozApocPolicyEntry(mozApocPolicyBackend & aBackend, char const * aPath, PRBool bPreferProp=PR_FALSE);
    mozApocPolicyEntry(mozApocPolicyComponentData const * aComponent, char const * aPath, PRBool bPreferProp);
    mozApocPolicyEntry(mozApocPolicyEntry const & aEntry, char const * aRelativePath, PRBool bPreferProp);
        
    PRBool   HasError()  const { return NS_FAILED(mErrCode); }
    nsresult GetError()  const { return mErrCode; }

    PRBool IsNothing()   const { return mData.GetAttributes() == 0; }
    PRBool IsReal()      const { return mozApocExistsInPolicy(mData.GetAttributes()); }
    PRBool IsProtected() const { return mozApocAreAnyFlagsSet(mData.GetAttributes(),MOZAPOC_ENTRY_PROTECTED_FLAGS); }
    PRBool IsMandatory() const { return mozApocAreAnyFlagsSet(mData.GetAttributes(),MOZAPOC_ENTRY_MANDATORY_FLAGS); }
    PRBool HasValue()    const { return mozApocHasValue(mData.GetAttributes()); }
    PRBool IsProperty()  const { return mozApocIsProperty(mData.GetAttributes()); }
    PRBool IsInnerNode() const { return mozApocIsNode(mData.GetAttributes()); }
    
    mozApocEntryFlagValues GetValueType() const { return mozApocGetEntryType(mData.GetAttributes()); }
    
    nsresult GetBoolValue(PRBool * _retval) const;
    nsresult GetIntValue(PRInt32 * _retval) const;
    nsresult GetCharValue(char **_retval) const;
    nsresult GetUnicodeValue(PRUnichar **_retval) const;
    nsresult GetComplexValue(const nsIID & aType, void * *aValue) const;
private:
    void SetError(nsresult aErrCode);
    void FollowPath(mozApocPathParser & aParser, PRBool bPreferProp);
};

class mozApocPolicyData
{
public:
    mozApocPolicyData();
    explicit
    mozApocPolicyData(mozApocPolicyBackend & aBackend);
    explicit
    mozApocPolicyData(mozApocPolicyEntry const & aComponentRootEntry);
    ~mozApocPolicyData();

    mozApocPolicyEntry GetEntry(char const * path, PRBool bPreferProp) const;
    
    PRBool   HasError() const;
    nsresult GetError() const;
private:
    mozApocPolicyBackend * mBackend;
    mutable mozApocPolicyEntry mRootEntry;
};


#endif

