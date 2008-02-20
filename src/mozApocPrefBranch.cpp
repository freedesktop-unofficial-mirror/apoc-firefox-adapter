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
 * $RCSfile: mozApocPrefBranch.cpp,v $
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
 * $Header: /export/src/cvs/firefox-apoc-adapter/mozApocPrefBranch.cpp,v 1.1.1.1 2006/10/30 03:23:53 sunop Exp $
 *******************************************************************************
 */

#include "mozApocPrefBranch.h"
#include "mozApocPrefService.h"
#include "mozApocPolicyData.h"
#include <stdio.h>

#ifndef MOZAPOC_ACCESS_TRACE
#ifdef DEBUG
#define MOZAPOC_ACCESS_TRACE 1
#else
#define MOZAPOC_ACCESS_TRACE 0
#endif
#endif

#define ACCTRACE(lvl) if ((lvl) >= (MOZAPOC_ACCESS_TRACE)); else ::fprintf
#define ACCESSTRACE0 ACCTRACE(0)
#define ACCESSTRACE1 ACCTRACE(1)
#define ACCESSTRACE2 ACCTRACE(2)

#if 0
NS_IMPL_THREADSAFE_ISUPPORTS5(mozApocPrefBranchWrapper,
                                nsIPrefBranch,nsIPrefBranchInternal,nsISecurityPref,
                                nsIObserver,nsISupportsWeakReference);
#else 
NS_IMPL_THREADSAFE_ADDREF(mozApocPrefBranchWrapper)
NS_IMPL_THREADSAFE_RELEASE(mozApocPrefBranchWrapper)

NS_INTERFACE_MAP_BEGIN(mozApocPrefBranchWrapper)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIPrefBranch)
  NS_INTERFACE_MAP_ENTRY(nsIPrefBranch)
  NS_INTERFACE_MAP_ENTRY_CONDITIONAL(nsIPrefBranch2, !mIsDefaultBranch)
  NS_INTERFACE_MAP_ENTRY_CONDITIONAL(nsIPrefBranchInternal, !mIsDefaultBranch)
  NS_INTERFACE_MAP_ENTRY(nsISecurityPref)
  NS_INTERFACE_MAP_ENTRY(nsIObserver)
  NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
NS_INTERFACE_MAP_END
#endif

#define MA_PBW_ENSURE_INITIALIZED() NS_ENSURE_TRUE(mWrappedBranch, NS_ERROR_NOT_INITIALIZED)
#define MA_PBW_ENSURE_BRANCH()      MA_PBW_ENSURE_INITIALIZED()

// class mozApocPrefBranchWrapper
//
mozApocPrefBranchWrapper::mozApocPrefBranchWrapper()
: mPolicyBranch()
, mWrappedBranch()
, mIsDefaultBranch()
{
}

mozApocPrefBranchWrapper::~mozApocPrefBranchWrapper()
{
}

nsresult mozApocPrefBranchWrapper::Init(const mozApocPolicyData& aPolicyBranch, 
                                        const nsCOMPtr<nsIPrefBranch> & aOriginalBranch, 
                                        PRBool bDefault)
{
    if (mWrappedBranch)
        return NS_ERROR_ALREADY_INITIALIZED;

    NS_ENSURE_ARG_POINTER(aOriginalBranch);

    NS_ENSURE_FALSE( aPolicyBranch.HasError(), aPolicyBranch.GetError() );

    mPolicyBranch = aPolicyBranch;
    mWrappedBranch = aOriginalBranch;
    mIsDefaultBranch = bDefault;
    
    return NS_OK;
}

// Policy Data Accessors
//


mozApocPolicyEntry mozApocPrefBranchWrapper::GetPolicyEntry(const char *aPrefName)
{
    return mPolicyBranch.GetEntry(aPrefName,PR_TRUE);
}

PRBool mozApocPrefBranchWrapper::UsePolicyValue(const char * aPrefName, nsresult & rv)
{
    NS_PRECONDITION(mWrappedBranch, "apoc - ERROR: UsePolicyvalue called on non-initialized branch.");
    
    mozApocPolicyEntry aEntry = mPolicyBranch.GetEntry(aPrefName,PR_TRUE);
    
    if (aEntry.HasError())
    {
        // NS_WARNING("apoc - Error in PolicyEntry - ignoring for path:");
        ACCESSTRACE0(stderr,"apoc - Error in PolicyEntry - ignoring possible entry for path: '%s'\n",aPrefName);
        return PR_FALSE;
    }
    
    if (!aEntry.HasValue())
    {
        NS_WARN_IF_FALSE(!aEntry.IsProperty(),"apoc - Found property node that has no value");
        if (aEntry.IsInnerNode())
        {
            NS_WARNING("apoc - Attempting to access value of inner node ");
            rv = NS_ERROR_UNEXPECTED;
        }
        else if (aEntry.IsProtected() && !mIsDefaultBranch) 
        {
            // TODO: Review this implementation choice, that manipulates the native value
            ACCESSTRACE2(stderr,"apoc - Value for '%s' is locked to native default in policy\n", aPrefName);
            rv = mWrappedBranch->LockPref(aPrefName);
            NS_WARN_IF_FALSE(NS_SUCCEEDED(rv),"apoc - Cannot force native default: LockPref failed");
        }
        return PR_FALSE;
    }
    
    if (this->mIsDefaultBranch || aEntry.IsProtected())
    {
        ACCESSTRACE2(stderr,"apoc - Providing %s value for '%s' from policy\n",mIsDefaultBranch ? "default" : "protected", aPrefName);
        return PR_TRUE;
    }

    PRBool bHasUserValue = PR_TRUE; // assume true, if not accessible
    rv = mWrappedBranch->PrefHasUserValue(aPrefName, &bHasUserValue);
    NS_WARN_IF_FALSE(NS_SUCCEEDED(rv),"apoc - Cannot determine whether there is an overriding user value");
    if (bHasUserValue)    
        ACCESSTRACE2(stderr,"apoc - Policy value for '%s' is overridden\n",aPrefName);
    else
        ACCESSTRACE2(stderr,"apoc - Providing not-overridden value for '%s' from policy\n",aPrefName);
    return !bHasUserValue;
}

nsresult mozApocPrefBranchWrapper::ValidateWrite(const char *aPrefName)
{
    mozApocPolicyEntry aEntry = mPolicyBranch.GetEntry(aPrefName,PR_TRUE);
    NS_WARN_IF_FALSE(!aEntry.HasError(), "apoc - Error in PolicyEntry - ignoring");

    if ( aEntry.HasError() ) return NS_OK; // aEntry.GetError();

    if (aEntry.IsProtected())
    {
        ACCESSTRACE0(stderr,"apoc - Blocking write to protected entry '%s' !\n", aPrefName);
        return NS_ERROR_UNEXPECTED;
    }

    if (this->mIsDefaultBranch && aEntry.HasValue())
    {
        ACCESSTRACE0(stderr,"apoc - Blocking write to hidden default entry '%s' !\n", aPrefName);
        return NS_ERROR_UNEXPECTED;
    }

    return NS_OK;
}

/* readonly attribute string root; */
NS_IMETHODIMP mozApocPrefBranchWrapper::GetRoot(char * *aRoot)
{
    MA_PBW_ENSURE_BRANCH();
    return mWrappedBranch->GetRoot(aRoot);
}

/* long getPrefType (in string aPrefName); */
NS_IMETHODIMP mozApocPrefBranchWrapper::GetPrefType(const char *aPrefName, PRInt32 *_retval)
{
    MA_PBW_ENSURE_BRANCH();
    return mWrappedBranch->GetPrefType(aPrefName,_retval);
}

/* boolean getBoolPref (in string aPrefName); */
NS_IMETHODIMP mozApocPrefBranchWrapper::GetBoolPref(const char *aPrefName, PRBool *_retval)
{
    // TODO: Handle policy lock-to-default
    MA_PBW_ENSURE_BRANCH();
    NS_ENSURE_ARG_POINTER(aPrefName);
    
    nsresult rv = NS_OK;
    if (UsePolicyValue(aPrefName,rv))
    {
        ACCESSTRACE1(stderr,"apoc - Using Policy Value for Bool '%s'\n",aPrefName);
        
        NS_ENSURE_ARG_POINTER(_retval);
        
        mozApocPolicyEntry aEntry = GetPolicyEntry(aPrefName);
        rv = aEntry.GetBoolValue(_retval);
    }
    else
    {
        // NS_ENSURE_SUCCESS(rv,rv);
        rv = mWrappedBranch->GetBoolPref(aPrefName,_retval);
    }
    return rv;
}

/* void setBoolPref (in string aPrefName, in long aValue); */
NS_IMETHODIMP mozApocPrefBranchWrapper::SetBoolPref(const char *aPrefName, PRInt32 aValue)
{
    MA_PBW_ENSURE_BRANCH();
    NS_ENSURE_ARG_POINTER(aPrefName);
    
    nsresult rv = ValidateWrite(aPrefName);
    NS_ENSURE_SUCCESS(rv,rv);
    rv = mWrappedBranch->SetBoolPref(aPrefName,aValue);
    return rv;
}

/* string getCharPref (in string aPrefName); */
NS_IMETHODIMP mozApocPrefBranchWrapper::GetCharPref(const char *aPrefName, char **_retval)
{
    // TODO: Handle policy lock-to-default
    MA_PBW_ENSURE_BRANCH();
    NS_ENSURE_ARG_POINTER(aPrefName);
    
    nsresult rv = NS_ERROR_FAILURE;
    if (UsePolicyValue(aPrefName,rv))
    {
        ACCESSTRACE1(stderr,"apoc - Using Policy Value for String '%s'\n",aPrefName);
        
        NS_ENSURE_ARG_POINTER(_retval);
        
        mozApocPolicyEntry aEntry = GetPolicyEntry(aPrefName);
        rv = aEntry.GetCharValue(_retval);
    }
    else
    {
        rv = mWrappedBranch->GetCharPref(aPrefName,_retval);
    }
    return rv;
}

/* void setCharPref (in string aPrefName, in string aValue); */
NS_IMETHODIMP mozApocPrefBranchWrapper::SetCharPref(const char *aPrefName, const char *aValue)
{
    MA_PBW_ENSURE_BRANCH();
    NS_ENSURE_ARG_POINTER(aPrefName);
    
    nsresult rv = ValidateWrite(aPrefName);
    NS_ENSURE_SUCCESS(rv,rv);
    rv = mWrappedBranch->SetCharPref(aPrefName,aValue);
    return rv;
}

/* long getIntPref (in string aPrefName); */
NS_IMETHODIMP mozApocPrefBranchWrapper::GetIntPref(const char *aPrefName, PRInt32 *_retval)
{
    // TODO: Handle policy lock-to-default
    MA_PBW_ENSURE_BRANCH();
    NS_ENSURE_ARG_POINTER(aPrefName);
    
    nsresult rv = NS_ERROR_FAILURE;
    if (UsePolicyValue(aPrefName,rv))
    {
        ACCESSTRACE1(stderr,"apoc - Using Policy Value for Integer '%s'\n",aPrefName);
        
        NS_ENSURE_ARG_POINTER(_retval);
        
        mozApocPolicyEntry aEntry = GetPolicyEntry(aPrefName);
        rv = aEntry.GetIntValue(_retval);
    }
    else
    {
        rv = mWrappedBranch->GetIntPref(aPrefName,_retval);
    }
    return rv;
}

/* void setIntPref (in string aPrefName, in long aValue); */
NS_IMETHODIMP mozApocPrefBranchWrapper::SetIntPref(const char *aPrefName, PRInt32 aValue)
{
    MA_PBW_ENSURE_BRANCH();
    NS_ENSURE_ARG_POINTER(aPrefName);
    
    nsresult rv = ValidateWrite(aPrefName);
    NS_ENSURE_SUCCESS(rv,rv);
    rv = mWrappedBranch->SetIntPref(aPrefName,aValue);
    return rv;
}

/* void getComplexValue (in string aPrefName, in nsIIDRef aType, [iid_is (aType), retval] out nsQIResult aValue); */
NS_IMETHODIMP mozApocPrefBranchWrapper::GetComplexValue(const char *aPrefName, const nsIID & aType, void * *aValue)
{
    // TODO: Handle policy lock-to-default
    MA_PBW_ENSURE_BRANCH();
    NS_ENSURE_ARG_POINTER(aPrefName);
    
    nsresult rv = NS_ERROR_FAILURE;
    if (UsePolicyValue(aPrefName,rv))
    {
        ACCESSTRACE1(stderr,"apoc - Using Policy Value for Complex '%s'\n",aPrefName);
        
        NS_ENSURE_ARG_POINTER(aValue);
        
        mozApocPolicyEntry aEntry = GetPolicyEntry(aPrefName);
        rv = aEntry.GetComplexValue(aType,aValue);
    }
    else
    {
        rv = mWrappedBranch->GetComplexValue(aPrefName,aType,aValue);
    }
    return rv;
}

/* void setComplexValue (in string aPrefName, in nsIIDRef aType, in nsISupports aValue); */
NS_IMETHODIMP mozApocPrefBranchWrapper::SetComplexValue(const char *aPrefName, const nsIID & aType, nsISupports *aValue)
{
    MA_PBW_ENSURE_BRANCH();
    NS_ENSURE_ARG_POINTER(aPrefName);
    
    nsresult rv = ValidateWrite(aPrefName);
    NS_ENSURE_SUCCESS(rv,rv);
    rv = mWrappedBranch->SetComplexValue(aPrefName,aType,aValue);
    return rv;
}

/* void clearUserPref (in string aPrefName); */
NS_IMETHODIMP mozApocPrefBranchWrapper::ClearUserPref(const char *aPrefName)
{
    MA_PBW_ENSURE_BRANCH();
    return mWrappedBranch->ClearUserPref(aPrefName);
}

/* void lockPref (in string aPrefName); */
NS_IMETHODIMP mozApocPrefBranchWrapper::LockPref(const char *aPrefName)
{
    MA_PBW_ENSURE_BRANCH();
    NS_ENSURE_ARG_POINTER(aPrefName);

    if (GetPolicyEntry(aPrefName).IsProtected())
        return NS_ERROR_UNEXPECTED;
    
    return mWrappedBranch->LockPref(aPrefName);
}

/* boolean prefHasUserValue (in string aPrefName); */
NS_IMETHODIMP mozApocPrefBranchWrapper::PrefHasUserValue(const char *aPrefName, PRBool *_retval)
{
    MA_PBW_ENSURE_BRANCH();
    return mWrappedBranch->PrefHasUserValue(aPrefName,_retval);
}

/* boolean prefIsLocked (in string aPrefName); */
NS_IMETHODIMP mozApocPrefBranchWrapper::PrefIsLocked(const char *aPrefName, PRBool *_retval)
{
    MA_PBW_ENSURE_BRANCH();
    NS_ENSURE_ARG_POINTER(aPrefName);

    if (GetPolicyEntry(aPrefName).IsProtected())
    {
        NS_ENSURE_ARG_POINTER(_retval);

        *_retval = PR_TRUE;
        return NS_OK;
    }
    return mWrappedBranch->PrefIsLocked(aPrefName,_retval);
}

/* void unlockPref (in string aPrefName); */
NS_IMETHODIMP mozApocPrefBranchWrapper::UnlockPref(const char *aPrefName)
{
    MA_PBW_ENSURE_BRANCH();
    NS_ENSURE_ARG_POINTER(aPrefName);

    if (GetPolicyEntry(aPrefName).IsProtected())
        return NS_ERROR_UNEXPECTED;
    
    return mWrappedBranch->UnlockPref(aPrefName);
}

/* void deleteBranch (in string aStartingAt); */
NS_IMETHODIMP mozApocPrefBranchWrapper::DeleteBranch(const char *aStartingAt)
{
    MA_PBW_ENSURE_BRANCH();
    NS_ENSURE_ARG_POINTER(aStartingAt);
    
    mozApocPolicyEntry aEntry = mPolicyBranch.GetEntry(aStartingAt,PR_FALSE);
    if (aEntry.IsMandatory())
        return NS_ERROR_UNEXPECTED;
    
    return mWrappedBranch->DeleteBranch(aStartingAt);
}


/* void getChildList (in string aStartingAt, out unsigned long aCount, [array, size_is (aCount), retval] out string aChildArray); */
NS_IMETHODIMP mozApocPrefBranchWrapper::GetChildList(const char *aStartingAt, PRUint32 *aCount, char ***aChildArray)
{
    MA_PBW_ENSURE_BRANCH();
    return mWrappedBranch->GetChildList(aStartingAt,aCount,aChildArray);
}

/* void resetBranch (in string aStartingAt); */
NS_IMETHODIMP mozApocPrefBranchWrapper::ResetBranch(const char *aStartingAt)
{
    MA_PBW_ENSURE_BRANCH();
    return mWrappedBranch->ResetBranch(aStartingAt);
}

