

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


#ifndef MOZAPOC_PREFBRANCH_H_INCLUDED
#define MOZAPOC_PREFBRANCH_H_INCLUDED

//Brian #include "nsIPrefBranch.h"
#include "pref/nsIPrefBranchInternal.h"
#include "nsIObserver.h"
#include "pref/nsISecurityPref.h"
#include "nsCOMPtr.h"
#include "nsWeakReference.h"

#include "mozApocPolicyData.h"

//brian class mozApocPrefBranchWrapper  : public nsIPrefBranch
class mozApocPrefBranchWrapper  : public nsIPrefBranchInternal
                                , public nsISecurityPref
                                , public nsIObserver
                                , public nsSupportsWeakReference
{
public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIPREFBRANCH

#if 0 //Brian++
    NS_FORWARD_SAFE_NSIPREFBRANCHINTERNAL(
#else 
    NS_FORWARD_SAFE_NSIPREFBRANCH2(
#endif
            nsCOMPtr<nsIPrefBranchInternal>(do_QueryInterface(mWrappedBranch))
                                        )       // NS_DECL_NSIPREFBRANCHINTERNAL
    NS_FORWARD_SAFE_NSISECURITYPREF      (
            nsCOMPtr<nsISecurityPref>(do_QueryInterface(mWrappedBranch))
                                        )       // NS_DECL_NSISECURITYPREF
    NS_FORWARD_SAFE_NSIOBSERVER          (
            nsCOMPtr<nsIObserver>(do_QueryInterface(mWrappedBranch))
                                        )       // NS_DECL_NSIOBSERVER

    mozApocPrefBranchWrapper();
    virtual ~mozApocPrefBranchWrapper();

    nsresult Init(const mozApocPolicyData & aPolicyBranch, const nsCOMPtr<nsIPrefBranch> & aOriginalBranch, PRBool bDefault);
private:
    mozApocPolicyEntry  GetPolicyEntry(const char *aPrefName);
    PRBool              UsePolicyValue(const char *aPrefName, nsresult & rv);
    nsresult            ValidateWrite(const char *aPrefName);
private:
    mozApocPolicyData           mPolicyBranch;
    nsCOMPtr<nsIPrefBranch>     mWrappedBranch;
    PRBool                      mIsDefaultBranch;
};

#endif

