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


#ifndef MOZAPOC_POLICYBACKEND_H_INCLUDED
#define MOZAPOC_POLICYBACKEND_H_INCLUDED

#include "mozApocPolicyComponentData.h"
#include "nsClassHashtable.h"
#include "nsHashKeys.h"
#include "mozApocPAPI.h"
#include "prmon.h"

class mozApocPolicyComponentDOMData;
class mozApocNotifier;

class mozApocPolicyBackend : private mozApocPAPIListener
{
    struct ComponentInfo : mozApocPolicyComponentData
    {
        ComponentInfo(const nsACString & aComponent) 
        : mozApocPolicyComponentData(aComponent) 
        , ListenerId()
        {}

        mozApocPAPIListenerId ListenerId;
    };
    typedef nsClassHashtable< nsCStringHashKey, ComponentInfo > ComponentCache;
private:
    mozApocPAPI mPolicyStore;
    mozApocPAPIListenerId mGlobalListenerId ;
    ComponentCache mCache;
    PRBool mEnableCache;
    PRMonitor * mUpdateLock;
    mozApocNotifier * mNotifier;
public:
    mozApocPolicyBackend();
    virtual ~mozApocPolicyBackend();

    mozApocPolicyComponentData const * GetComponentData(const nsACString & aComponentName, nsresult * _rv = nsnull);

    void Reset();

    void SetNotifier(mozApocNotifier * pNotifier) { mNotifier = pNotifier; }
private:
    nsresult RegisterForNotfication(const nsAFlatCString & aFullComponentName, ComponentInfo * data);

    nsresult ReadComponent(const nsAFlatCString & aFullComponentName, mozApocPolicyComponentData & pData, PRBool bClear = PR_FALSE);
    nsresult ReadComponentDOM(const nsAFlatCString & aFullComponentName, const nsAFlatCString & aShortComponentName, 
                                mozApocPolicyComponentDOMData *& newData);
    nsresult RefreshComponent(const char * aFullComponentName, PRBool bClear = PR_FALSE);
private:
    virtual void componentAdded(const char * component);
    virtual void componentModified(const char * component);
    virtual void componentRemoved(const char * component);
private:
    mozApocPolicyBackend(const mozApocPolicyBackend &);  // not implemented
    void operator=(const mozApocPolicyBackend &);  // not implemented
};

#endif

