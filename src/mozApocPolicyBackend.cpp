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


#include "mozApocPolicyBackend.h"
#include "mozApocPolicyComponentDOMData.h"
#include "mozApocPolicyModelFromDOM.h"
#include "mozApocPolicyModelFromXML.h"
#include "prthread.h"

#ifndef MOZAPOC_DEBUGSUPPORT
#define MOZAPOC_DEBUGSUPPORT 0
#endif

#ifndef MOZAPOC_BACKEND_TRACE
#ifdef DEBUG
#define MOZAPOC_BACKEND_TRACE 2
#else
#define MOZAPOC_BACKEND_TRACE MOZAPOC_DEBUGSUPPORT
#endif
#endif

#define BETRACE(lvl) if ((lvl) > (MOZAPOC_BACKEND_TRACE)); else ::fprintf
#define BACKENDTRACE BETRACE(1)
#define REQUESTTRACE BETRACE(2)
#define DETAILTRACE  BETRACE(3)

#define k_mozApocPackagePrefix "org.mozilla."

mozApocPolicyBackend::mozApocPolicyBackend()
: mPolicyStore()
, mGlobalListenerId(nsnull)
, mCache()
, mEnableCache(PR_FALSE)
, mUpdateLock(PR_NewMonitor())
, mNotifier(nsnull)
{
    mEnableCache = mCache.Init(PL_DHASH_MIN_SIZE);
    
    NS_WARN_IF_FALSE(mPolicyStore.IsConnected(),"apoc - Backend: Connecting to PAPI failed - no policy data available");
    NS_WARN_IF_FALSE(mUpdateLock,"apoc - Backend: Could not create lock for cache - will not run threadsafe");
    if (mEnableCache && mPolicyStore.IsConnected())
    {
        mPolicyStore.AddListener(k_mozApocPackagePrefix,
                                 this,
                                 mGlobalListenerId) ;
    }
}

mozApocPolicyBackend::~mozApocPolicyBackend()
{
    if (mGlobalListenerId != nsnull)
    {
        mPolicyStore.RemoveListener(mGlobalListenerId) ;
    }
    if (mUpdateLock) PR_DestroyMonitor(mUpdateLock);
}

// tracks currently active parse(s) to prevent reentrant calls
class mozApoc_ActiveComponent
{
    nsACString const & name;
    mozApoc_ActiveComponent * next;

    static mozApoc_ActiveComponent * top;
public:
    explicit
    mozApoc_ActiveComponent(nsACString const * name_)
    : name(*name_)
    , next(top)
    { top = this; }

    ~mozApoc_ActiveComponent()
    { top = next; }

    static PRBool findAny() { return top != nsnull; }
    static PRBool find(nsACString const & name_)
    {
        for (mozApoc_ActiveComponent* entry = top; entry; entry = entry->next)
        {
            if (name_.Equals(entry->name))
                return PR_TRUE;
        }
        return PR_FALSE;
    }
};
mozApoc_ActiveComponent * mozApoc_ActiveComponent::top = nsnull;

// -------------------------------------------------------------------------------------------

mozApocPolicyComponentData const * 
mozApocPolicyBackend::GetComponentData(const nsACString & aComponentName, nsresult * _rv)
{
    ComponentInfo * pInfo = nsnull;
    nsCAutoString aFullComponentName( NS_LITERAL_CSTRING(k_mozApocPackagePrefix) + aComponentName);

    nsresult rv = NS_OK;
    if (!mEnableCache || !mCache.Get(aFullComponentName,&pInfo))
    {
        if (mUpdateLock) PR_EnterMonitor(mUpdateLock);

        if (!mCache.Get(aFullComponentName,&pInfo))
        {
            REQUESTTRACE(stderr,"\napoc: Request to read component '%s' from backend.\n", 
                        PromiseFlatCString(aComponentName).get());

            // Prevent infinite recursion for reentrant use of parser
            PRBool isComponentAlreadyBeingParsed = mozApoc_ActiveComponent::find(aFullComponentName);
            if (isComponentAlreadyBeingParsed)
            {
                NS_WARNING("apoc: Component is already being parsed - aborting recursive access\n");
                rv = NS_ERROR_UNEXPECTED;
            }
            else
            {
                mozApoc_ActiveComponent nowActive(&aFullComponentName);

                pInfo = new ComponentInfo(aComponentName);
                if (pInfo)
                {
                    if (mEnableCache)
                        mCache.Put(aFullComponentName,pInfo);

                    rv = ReadComponent(aFullComponentName,*pInfo);
                    if (NS_FAILED(rv))
                    {
                        if (mEnableCache)
                            mCache.Remove(aFullComponentName);
                        else 
                            delete pInfo;
                        pInfo = nsnull;
                    }
                }
                else
                    rv = NS_ERROR_OUT_OF_MEMORY;
            }
        }

        if (mUpdateLock) PR_ExitMonitor(mUpdateLock);
    }

    if (_rv) *_rv = rv;

    return pInfo;
}

void mozApocPolicyBackend::Reset()
{
    // TODO: Unregister listeners
    if (mEnableCache)
        mCache.Clear();
}

nsresult mozApocPolicyBackend::ReadComponent(const nsAFlatCString & aFullComponentName, mozApocPolicyComponentData & aData, PRBool bClear)
{
    nsresult rv = NS_OK;

    mozApocPolicyComponentDOMData * pDOMData = nsnull;
    if (!bClear) rv = ReadComponentDOM(aFullComponentName, aData.GetComponentName(), pDOMData);
    
    if (NS_SUCCEEDED(rv))
    {
        mozApocBuildPolicyModelFromDOM aBuilder(pDOMData);
        rv = aData.UpdateData(aBuilder, mNotifier);
    }
    else
        NS_WARNING("apoc - Refreshing component data failed: parser error");

    delete pDOMData;

    return rv;
}

nsresult mozApocPolicyBackend::RefreshComponent(const char * component, PRBool bClear)
{
    NS_PRECONDITION(component, "apoc - Received PAPI notification with NULL component name");
    
    if (mUpdateLock) PR_EnterMonitor(mUpdateLock);

    NS_ASSERTION(mEnableCache, "apoc - Notification is being invoked altough cache is not in use.");
    NS_ASSERTION(!mozApoc_ActiveComponent::findAny(), "apoc - component refresh being executed while a request is in progress"
                                                      " (Locking problem ?)");

    nsresult rv = NS_OK;

    nsCAutoString aFullComponentName(component);

    ComponentInfo * pInfo = nsnull;
    if (mCache.Get(aFullComponentName,&pInfo))
    {
        NS_ASSERTION(pInfo,"apoc - error: Found NULL cache entry");

        REQUESTTRACE(stderr,"\napoc: Refreshing to component '%s' from backend for notification.\n", 
                        aFullComponentName.get());

        mozApoc_ActiveComponent nowActive(&aFullComponentName);

        rv = ReadComponent(aFullComponentName,*pInfo,bClear);
    }

    if (mUpdateLock) PR_ExitMonitor(mUpdateLock);

    return rv;
}

nsresult mozApocPolicyBackend::ReadComponentDOM(const nsAFlatCString & aFullComponentName, const nsAFlatCString & aShortComponentName, mozApocPolicyComponentDOMData *& newData)
{
    if (!mPolicyStore.IsConnected())
    {
        DETAILTRACE(stderr,"apoc DEBUG: No data - not connected");
        return NS_OK;
    }
    
    mozApocPAPILayerList aList;

    nsresult rv = mPolicyStore.ReadComponentLayers(aFullComponentName.get(),aList);
    if (aList.IsEmpty())
    {
        DETAILTRACE(stderr,"apoc DEBUG: No data in PAPI (rv = %0#8x)",unsigned(rv));
        return rv;
    }

    BACKENDTRACE(stderr,"\napoc: Processing policy data for component '%s': ", aFullComponentName.get());

    nsAutoPtr<mozApocPolicyComponentDOMData> apData( new mozApocPolicyComponentDOMData(aShortComponentName) );
    if (!apData)
        return NS_ERROR_OUT_OF_MEMORY;
    
    mozApocPAPILayerListEnumerator aIterator = aList.Enum();
    while (aIterator.HasMore())
    {
        BACKENDTRACE(stderr,".");
        mozApocPAPILayer aData = aIterator.Next();

        mozApocBuildPolicyModelFromXML aParser(aData.GetData(), aData.GetLength());
        mozApocParsedLayerData aLayerData; 
        aParser.UpdateModel(aShortComponentName, nsnull, aLayerData);

        if (NS_SUCCEEDED(rv))
        {
            apData->AddLayer(aLayerData);
            BACKENDTRACE(stderr,"+");
        }
        else
            NS_WARNING("apoc - Parsing a layer failed: layer is ignored");
    }
    BACKENDTRACE(stderr," - done. %d layers added.\n", int(apData->GetLayerCount()));
    
    newData = apData.forget();
    return NS_OK;
}

nsresult mozApocPolicyBackend::RegisterForNotfication(const nsAFlatCString & aFullComponentName, ComponentInfo * data)
{
    NS_PRECONDITION(data,"apoc - error: trying to register a notification for a NULL component info");
    
    nsresult rv = mPolicyStore.AddListener(aFullComponentName .get(), this, data->ListenerId);

    return rv;
}

void mozApocPolicyBackend::componentAdded(const char * component)
{
    this->RefreshComponent(component);
}

void mozApocPolicyBackend::componentModified(const char * component)
{
    this->RefreshComponent(component);
}

void mozApocPolicyBackend::componentRemoved(const char * component)
{
    this->RefreshComponent(component,PR_TRUE);
}


