

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


#ifndef MOZAPOC_PREFSERVICE_H_INCLUDED
#define MOZAPOC_PREFSERVICE_H_INCLUDED

#include "mozApocPolicyBackend.h"

#include "nsIPrefService.h"
//Brian #include "nsIPrefBranch.h"
#include "pref/nsIPrefBranchInternal.h"
#include "nsIObserver.h"
#include "nsCOMPtr.h"
#include "nsWeakReference.h"

class mozApocPolicyData;
class mozApocBranchNotifier;

// { 833fbe65-5a14-4634-8387-fcf47e24c9ca }
#define MOZAPOC_PREFSERVICEWRAPPER_CID \
{ 0x833fbe65, 0x5a14, 0x4634, { 0x83, 0x87, 0xfc, 0xf4, 0x7e, 0x24, 0xc9, 0xca } }

class mozApocPrefServiceWrapper : public nsIPrefService
                                , public nsIObserver
//Brian                                , public nsIPrefBranch
                                , public nsIPrefBranchInternal
                                , public nsSupportsWeakReference
{
public:
    NS_DECL_ISUPPORTS

    NS_DECL_NSIPREFSERVICE
    NS_FORWARD_SAFE_NSIPREFBRANCH(mUserBranch)
#if 0 //Brian++
    NS_FORWARD_SAFE_NSIPREFBRANCHINTERNAL(
#else 
    NS_FORWARD_SAFE_NSIPREFBRANCH2(
#endif 
            nsCOMPtr<nsIPrefBranchInternal>(do_QueryInterface(mWrappedService))
                                        )   //  NS_DECL_NSIPREFBRANCHINTERNAL
    NS_FORWARD_SAFE_NSIOBSERVER(
            nsCOMPtr<nsIObserver>(do_QueryInterface(mWrappedService))
                                )               //  NS_DECL_NSIOBSERVER
        
    mozApocPrefServiceWrapper();
    virtual ~mozApocPrefServiceWrapper();

    nsresult Init();

private:
    nsresult CreateWrappedBranch(const char *aPrefRoot, PRBool bDefault, nsIPrefBranch **_retval);
    nsresult GetOriginalBranch(const char *aPrefRoot, PRBool bDefault, nsIPrefBranch **_retval);
    mozApocPolicyData GetPolicyData(const char * aPrefRoot);

    nsresult InitNotifier();
private:
    mozApocPolicyBackend     mBackend;
    nsCOMPtr<nsIPrefService> mWrappedService;
    nsCOMPtr<nsIPrefBranch>  mUserBranch;
    mozApocBranchNotifier *  mNotifier;
};

#endif

