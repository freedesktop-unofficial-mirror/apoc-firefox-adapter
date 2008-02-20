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
 * $RCSfile: mozApocPrefService.cpp,v $
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
 * $Header: /export/src/cvs/firefox-apoc-adapter/mozApocPrefService.cpp,v 1.1.1.1 2006/10/30 03:23:53 sunop Exp $
 *******************************************************************************
 */

#include "mozApocPrefService.h"
#include "mozApocPrefBranch.h"
#include "mozApocNotificationImpl.h"

//Brian++ #include "nsIServiceManagerUtils.h"
#include "nsServiceManagerUtils.h"
#include "nsIProxyObjectManager.h"
#include "nsDebug.h"

#include <stdio.h>

#ifndef MOZAPOC_INIT_TRACE
#ifdef DEBUG
#define MOZAPOC_INIT_TRACE 1
#else
#define MOZAPOC_INIT_TRACE 0
#endif
#endif
#ifndef MOZAPOC_BRANCH_TRACE
#ifdef DEBUG
#define MOZAPOC_BRANCH_TRACE 1
#else
#define MOZAPOC_BRANCH_TRACE 0
#endif
#endif

#define INITTRACE   if (!(MOZAPOC_INIT_TRACE))   ; else ::fprintf
#define BRANCHTRACE if (!(MOZAPOC_BRANCH_TRACE)) ; else ::fprintf
static NS_DEFINE_CID(kProxyObjectManagerCID, NS_PROXYEVENT_MANAGER_CID);

#if 0
NS_IMPL_THREADSAFE_ISUPPORTS5(mozApocPrefServiceWrapper,
                                nsIPrefService,nsIPrefBranch,nsIPrefBranchInternal,
                                nsIObserver,nsISupportsWeakReference);
#else 
NS_IMPL_THREADSAFE_ADDREF(mozApocPrefServiceWrapper)
NS_IMPL_THREADSAFE_RELEASE(mozApocPrefServiceWrapper)

NS_INTERFACE_MAP_BEGIN(mozApocPrefServiceWrapper)
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIPrefService)
    NS_INTERFACE_MAP_ENTRY(nsIPrefService)     
    NS_INTERFACE_MAP_ENTRY(nsIObserver)     
    NS_INTERFACE_MAP_ENTRY(nsIPrefBranch)
    NS_INTERFACE_MAP_ENTRY(nsIPrefBranch2)
    NS_INTERFACE_MAP_ENTRY(nsIPrefBranchInternal)
    NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
NS_INTERFACE_MAP_END

#endif


#define MA_PSW_ENSURE_INITIALIZED() NS_ENSURE_TRUE(mWrappedService, NS_ERROR_NOT_INITIALIZED)

mozApocPrefServiceWrapper::mozApocPrefServiceWrapper()
: mWrappedService() , mUserBranch()
, mNotifier(nsnull)
{
    NS_INIT_ISUPPORTS();
}

mozApocPrefServiceWrapper::~mozApocPrefServiceWrapper()
{
    delete mNotifier;
}

nsresult mozApocPrefServiceWrapper::Init()
{
    static NS_DEFINE_CID( kWrappedClsid, NS_PREFSERVICE_CID );
    
    INITTRACE(stderr,"apoc: Initializing PrefServiceWrapper.\n");
    
    if (mUserBranch)
        return NS_ERROR_ALREADY_INITIALIZED;

    nsresult rv = NS_OK;
    mWrappedService = do_GetService(kWrappedClsid,&rv);
    
    if (NS_SUCCEEDED(rv))
    {
        InitNotifier(); // failure considered non-fatal
        rv = CreateWrappedBranch(NULL,false,getter_AddRefs(mUserBranch));
    }

    if (NS_FAILED(rv))
        INITTRACE(stderr,"apoc: Initializing PrefServiceWrapper failed [rv = 0x%#.8x]\n", unsigned(rv));
    
    return rv;
}

nsresult mozApocPrefServiceWrapper::InitNotifier()
{
    NS_ENSURE_TRUE(!mNotifier, NS_ERROR_ALREADY_INITIALIZED);
    NS_ENSURE_TRUE(mWrappedService, NS_ERROR_NOT_INITIALIZED);

    nsresult rv = NS_OK;
    nsCOMPtr<nsIPrefBranch> aDefaultBranch;
    nsCOMPtr<nsIPrefBranch> aDefaultBranchProxy;
    nsCOMPtr<nsIProxyObjectManager> proxyObjectManager(do_GetService(kProxyObjectManagerCID, &rv));
    if (NS_FAILED(rv))
      return rv;
    rv = mWrappedService->GetDefaultBranch(nsnull,getter_AddRefs(aDefaultBranch));
    if (NS_FAILED(rv))
      return rv;
    rv = proxyObjectManager->GetProxyForObject(NS_UI_THREAD_EVENTQ, NS_GET_IID(nsIPrefBranch), 
                                              aDefaultBranch, PROXY_SYNC | PROXY_ALWAYS,
                                              getter_AddRefs(aDefaultBranchProxy));
    if (NS_SUCCEEDED(rv))
    {
        mNotifier = new mozApocBranchNotifier(aDefaultBranchProxy);

        if (!mNotifier) rv = NS_ERROR_OUT_OF_MEMORY;
    }

    if (NS_FAILED(rv))
        INITTRACE(stderr,"apoc: Initializing Notifier failed - notification will not work"
                    " [rv = 0x%#.8x]\n", unsigned(rv));

    mBackend.SetNotifier(mNotifier);
    
    return rv;
}

mozApocPolicyData mozApocPrefServiceWrapper::GetPolicyData(const char * aPrefRoot)
{
    if (aPrefRoot && *aPrefRoot)
    {
        return mozApocPolicyData(mozApocPolicyEntry(mBackend,aPrefRoot));
    }
    else
    {
        return mozApocPolicyData(mBackend);
    }
}

nsresult mozApocPrefServiceWrapper::CreateWrappedBranch(const char *aPrefRoot, PRBool bDefault, nsIPrefBranch **_retval)
{
    NS_PRECONDITION(mWrappedService, "Wrapped service must be initialized before calling CreateWrappedBranch");

    BRANCHTRACE(stderr,"apoc: Creating wrapped %s PrefBranch at %s.\n", 
                bDefault ? "default" : "user",
                aPrefRoot ? *aPrefRoot ? aPrefRoot : "<ROOT> [empty]" : "<ROOT> [NULL]" );
    
    NS_ENSURE_ARG_POINTER(_retval);
    *_retval = nsnull;
    
    nsCOMPtr<nsIPrefBranch> aOriginalBranch;
    nsresult rv = NS_OK;
    if (aPrefRoot || bDefault)
        rv = GetOriginalBranch(aPrefRoot,bDefault,getter_AddRefs(aOriginalBranch));
    else
        rv = mWrappedService->GetBranch(aPrefRoot,getter_AddRefs(aOriginalBranch));
    NS_ENSURE_SUCCESS(rv,rv);

    mozApocPrefBranchWrapper * wrapper; 
    NS_NEWXPCOM( wrapper, mozApocPrefBranchWrapper );
    if (!wrapper)
        return NS_ERROR_OUT_OF_MEMORY;
    
    nsCOMPtr< nsIPrefBranch > aResultBranch(wrapper);
    rv = wrapper->Init(GetPolicyData(aPrefRoot),aOriginalBranch,bDefault);
    if (NS_SUCCEEDED(rv))
    {
        *_retval = aResultBranch;
        NS_ADDREF(*_retval);
    }
    else if (rv == NS_ERROR_NOT_AVAILABLE) 
    {
       //handle connection failures gracefully
       *_retval = aOriginalBranch;
       NS_ADDREF(*_retval);
       rv = NS_OK;
    }
                                                                                
    return rv;
}

nsresult mozApocPrefServiceWrapper::GetOriginalBranch(const char *aPrefRoot, PRBool bDefault, nsIPrefBranch **_retval)
{
    NS_PRECONDITION(mWrappedService, "Wrapped service must be initialized before calling GetOriginalBranch");

    return bDefault ? mWrappedService->GetDefaultBranch(aPrefRoot,_retval) 
                    : mWrappedService->GetBranch       (aPrefRoot,_retval);
}

// nsIPrefService
/* void readUserPrefs (in nsIFile aFile); */
NS_IMETHODIMP mozApocPrefServiceWrapper::ReadUserPrefs(nsIFile *aFile)
{
    MA_PSW_ENSURE_INITIALIZED();
    return mWrappedService->ReadUserPrefs(aFile);
}

/* void resetPrefs (); */
NS_IMETHODIMP mozApocPrefServiceWrapper::ResetPrefs()
{
    MA_PSW_ENSURE_INITIALIZED();
    return mWrappedService->ResetPrefs();
}

/* void resetUserPrefs (); */
NS_IMETHODIMP mozApocPrefServiceWrapper::ResetUserPrefs()
{
    MA_PSW_ENSURE_INITIALIZED();
    return mWrappedService->ResetUserPrefs();
}

/* void savePrefFile (in nsIFile aFile); */
NS_IMETHODIMP mozApocPrefServiceWrapper::SavePrefFile(nsIFile *aFile)
{
    MA_PSW_ENSURE_INITIALIZED();
    return mWrappedService->SavePrefFile(aFile);
}

/* nsIPrefBranch getBranch (in string aPrefRoot); */
NS_IMETHODIMP mozApocPrefServiceWrapper::GetBranch(const char *aPrefRoot, nsIPrefBranch **_retval)
{
    MA_PSW_ENSURE_INITIALIZED();
    
    if (!aPrefRoot && mUserBranch)
    {
        NS_ENSURE_ARG_POINTER(_retval);
        *_retval = mUserBranch.get();
        NS_ADDREF(*_retval);
        return NS_OK;
    }
    else
        return CreateWrappedBranch(aPrefRoot,PR_FALSE,_retval);
}

/* nsIPrefBranch getDefaultBranch (in string aPrefRoot); */
NS_IMETHODIMP mozApocPrefServiceWrapper::GetDefaultBranch(const char *aPrefRoot, nsIPrefBranch **_retval)
{
    MA_PSW_ENSURE_INITIALIZED();
    
    return CreateWrappedBranch(aPrefRoot,PR_TRUE,_retval);
}

