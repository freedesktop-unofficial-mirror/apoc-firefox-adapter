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


#include "mozApocNotificationImpl.h"
#include "mozApocPathParser.h" // for BuildPath

//Brian++ #include "nsIServiceManagerUtils.h"
#include "nsServiceManagerUtils.h"
#include "nsIProxyObjectManager.h"

#ifdef DEBUG
#define NOTIFICATION_TRACE ::fprintf
#else
#define NOTIFICATION_TRACE if (1) ; else ::fprintf
#endif

struct mozApocNativeSettingsData
{
    mozApocNativeSettingsData() : PrefType(nsIPrefBranch::PREF_INVALID), Locked(PR_FALSE) {}
    ~mozApocNativeSettingsData() { Clear(); }

    nsresult Read(const nsCOMPtr<nsIPrefBranch> & aBranch, const char * aPath);
    nsresult Restore(const nsCOMPtr<nsIPrefBranch> & aBranch, const char * aPath) const;
    nsresult Clear();
    
    PRInt32 PrefType;
    PRBool Locked;
    union
    {
        char * CharValue;
        PRInt32 IntValue;
        PRBool BoolValue;
    };
};

nsresult mozApocNativeSettingsData::Read(const nsCOMPtr<nsIPrefBranch> & aBranch, const char * aPath)
{
    NS_PRECONDITION( PrefType == nsIPrefBranch::PREF_INVALID, "apoc - Native Settings not cleared before reading (may leak memory)");
    NS_PRECONDITION( aBranch, "apoc - Native Settings operation on NULL branch");
    NS_PRECONDITION( aPath, "apoc - Native Settings operation on NULL path");

    nsresult rv = aBranch->GetPrefType(aPath, &PrefType);
    if (NS_FAILED(rv))
    {
        NS_ASSERTION( PrefType == nsIPrefBranch::PREF_INVALID, "apoc - Failed GetPrefType has returned unexpected type" );
        return rv;
    }

    switch (PrefType)
    {
    case nsIPrefBranch::PREF_STRING:
        CharValue = nsnull;
        rv = aBranch->GetCharPref(aPath, &CharValue); 
        break;

    case nsIPrefBranch::PREF_INT:
        rv = aBranch->GetIntPref(aPath, &IntValue); 
        break;

    case nsIPrefBranch::PREF_BOOL:
        rv = aBranch->GetBoolPref(aPath, &BoolValue); 
        break;

    default:    
        NS_WARNING("apoc - Unknown PrefType returned by GetPrefType !?");
        rv = NS_ERROR_UNEXPECTED;
        // fall through

    case nsIPrefBranch::PREF_INVALID: 
        // no data
        break;
    }

    if (NS_FAILED(rv))
    {
        NS_WARNING("apoc - Could not get native value for later restore");
        PrefType = nsIPrefBranch::PREF_INVALID;
    }

    if (PrefType != nsIPrefBranch::PREF_INVALID)
    {
        nsresult rv2 = aBranch->PrefIsLocked(aPath,&Locked);
        if ( NS_FAILED(rv2) )
        {
            NS_WARNING("apoc - Could not determine native lock state for later restore");
            Locked = PR_FALSE;
        }
    }
    
    return rv;
}

nsresult mozApocNativeSettingsData::Restore(const nsCOMPtr<nsIPrefBranch> & aBranch, const char * aPath) const
{
    NS_PRECONDITION( aBranch, "apoc - Native Settings operation on NULL branch");
    NS_PRECONDITION( aPath, "apoc - Native Settings operation on NULL path");

    nsresult rv = Locked ? rv = aBranch->LockPref(aPath) : rv = aBranch->UnlockPref(aPath);
    if (NS_FAILED(rv) && PrefType != nsIPrefBranch::PREF_INVALID)
        NS_WARNING("apoc - Could not restore native lock state");
        
    switch (PrefType)
    {
    case nsIPrefBranch::PREF_STRING:
        rv = aBranch->SetCharPref(aPath, CharValue); 
        break;

    case nsIPrefBranch::PREF_INT:
        rv = aBranch->SetIntPref(aPath, IntValue); 
        break;

    case nsIPrefBranch::PREF_BOOL:
        rv = aBranch->SetBoolPref(aPath, BoolValue); 
        break;

    default:    
        NS_WARNING("apoc - Unknown PrefType returned by GetPrefType !?");
        rv = NS_ERROR_UNEXPECTED;
        break;

    case nsIPrefBranch::PREF_INVALID: 
        aBranch->DeleteBranch(aPath);
        rv = NS_OK;
        break;
    }
    
    NS_WARN_IF_FALSE(NS_SUCCEEDED(rv),"apoc - Could not restore native value");
    return rv;
}

nsresult mozApocNativeSettingsData::Clear()
{
    nsresult rv = NS_OK;
    switch (PrefType)
    {
    case nsIPrefBranch::PREF_STRING:
        nsMemory::Free(CharValue);
        break;

    case nsIPrefBranch::PREF_INT:
    case nsIPrefBranch::PREF_BOOL:
        // nothing to do
        break;

    default:    
        NS_WARNING("apoc - Illegal PrefType found in native settings data !?");
        rv = NS_ERROR_UNEXPECTED;
        // fall through
    case nsIPrefBranch::PREF_INVALID: 
        // no data
        break;
    }
    NS_WARN_IF_FALSE( NS_SUCCEEDED(rv), "apoc - error discarding settings data");

    PrefType = nsIPrefBranch::PREF_INVALID;
    Locked = PR_FALSE;

    return rv;
}

void mozApocDiscardNativeSettings(mozApocNativeSettingsMemento aMemento)
{
    delete aMemento;
}

mozApocNotificationDispatcher::mozApocNotificationDispatcher(mozApocNotificationDispatcher const & aParent, 
                                                              nsACString const & aChildName)
: Path( mozApocBuildPath(aParent.Path,aChildName) )
, Notifier(aParent.Notifier)
{}

// This function is used to lazyly set up mDispatchBranch as UI-Thread proxy for mNativeBranch
// This is needed to ensure that non-threadsafe components (e.g. XPConnect objects) are notfied on the UI thread
//
// Creation of the proxy requires that the event queue of the UI thread is created
// If a notification is dispatched before that time, we simply use mNativeBranch directly
// This is valid, as clients that exist before the UI thread, can operate outside it... 
nsCOMPtr< nsIPrefBranch > const & mozApocBranchNotifier::getTargetBranch()
{
    // TODO: use a mutex to protect mDispatchBranch manipulation
    if (!mDispatchBranch.get())
    {
        // no proxy yet - attempt to create it
        static NS_DEFINE_CID(kProxyObjectManagerCID, NS_PROXYEVENT_MANAGER_CID);

        nsresult rv = NS_OK;
        nsCOMPtr<nsIPrefBranch> aBranchProxy;

        nsCOMPtr<nsIProxyObjectManager> proxyObjectManager(do_GetService(kProxyObjectManagerCID, &rv));
        if (NS_SUCCEEDED(rv))
        {
            rv = proxyObjectManager->GetProxyForObject(NS_UI_THREAD_EVENTQ, NS_GET_IID(nsIPrefBranch), 
                                                       mNativeBranch, PROXY_SYNC | PROXY_ALWAYS,
                                                       getter_AddRefs(aBranchProxy));
        }
        if (NS_FAILED(rv)) // cannot create proxy (yet)
        {
            if (rv == NS_ERROR_NOT_AVAILABLE)
                NS_WARNING("apoc: Cannot create proxy for notifcation: UI thread event queue is not available");
            else
                NS_WARNING("apoc: Cannot create proxy for notifcation: Unexpected failure");

            // .. so using the non-proxied branch object instead
            return mNativeBranch;
        }
        NS_ASSERTION(aBranchProxy.get() != nsnull, "apoc: GetProxyForObject succeeded, but still no proxy available ?");
        mDispatchBranch = aBranchProxy;
    }
    return mDispatchBranch;
}

void mozApocBranchNotifier::NotifyValueChange(const char * aPath, nsAString const & aValue, PRBool bLocked)
{
    NS_PRECONDITION(aPath,"apoc - Notifying for NULL path");
   
    NOTIFICATION_TRACE(stderr, "Notifying change of value at %s: Value = '%s', State=%s\n",
                       aPath, NS_ConvertUCS2toUTF8(aValue).get(), bLocked ? "LOCKED" : "NOT LOCKED"); 
    
    PRInt32 aPrefType = nsIPrefBranch::PREF_INVALID;

    // read access (to determine the type) does not need to be proxied ..
    nsresult rv = mNativeBranch->GetPrefType(aPath, &aPrefType);
    if (NS_FAILED(rv))
        NS_ASSERTION( aPrefType == nsIPrefBranch::PREF_INVALID, "apoc - Failed GetPrefType has returned unexpected type" );

    // ... but write access (causing notifications) does
    nsCOMPtr< nsIPrefBranch > const & aTargetBranch = this->getTargetBranch();
    switch (aPrefType)
    {
    case nsIPrefBranch::PREF_INVALID: 
        // TODO: pass attributes into here, for typing
        NS_WARNING("apoc - No native data for entry being notified: assuming type STRING");
        // fall through

    case nsIPrefBranch::PREF_STRING:
        rv = aTargetBranch->SetCharPref(aPath, NS_ConvertUCS2toUTF8(aValue).get()); 
        break;

    case nsIPrefBranch::PREF_INT:
        {
            nsAutoString aHelperValue(aValue); // need this for ToInteger
            
            PRInt32 Errcode = NS_OK;
            PRInt32 IntVal = aHelperValue.ToInteger(&Errcode,kRadix10);

            rv = NS_STATIC_CAST(nsresult,Errcode);
            if (NS_SUCCEEDED(rv))
                rv = aTargetBranch->SetIntPref(aPath, IntVal); 
            else
                NS_WARNING("apoc - Cannot notify value change - not a numeric value for Int pref");
        }
        break;

    case nsIPrefBranch::PREF_BOOL:
        if (aValue.Equals(NS_LITERAL_STRING("false")))
            rv = aTargetBranch->SetBoolPref(aPath, PR_FALSE); 

        else if (aValue.Equals(NS_LITERAL_STRING("true")))
            rv = aTargetBranch->SetBoolPref(aPath, PR_TRUE); 

        else 
        {
            NS_WARNING("apoc - Cannot notify value change - not a boolean value for Bool pref");
            rv = NS_ERROR_UNEXPECTED;
        }
        break;

    default:    
        NS_WARNING("apoc - Unknown PrefType returned by GetPrefType !?");
        rv = NS_ERROR_UNEXPECTED;
        break;
    }

}

void mozApocBranchNotifier:: NotifyLockChange(const char * aPath, PRBool bLocked)
{
    NS_PRECONDITION(aPath,"apoc - Notifying for NULL path");
   
    NOTIFICATION_TRACE(stderr, "Notifying change of lock state at %s: New State=%s\n",
                       aPath, bLocked ? "LOCKED" : "NOT LOCKED"); 
    
    nsCOMPtr< nsIPrefBranch > const & aTargetBranch = this->getTargetBranch();

    nsresult rv = bLocked ? aTargetBranch->LockPref(aPath) : aTargetBranch->UnlockPref(aPath);
    if (NS_FAILED(rv))
        NS_WARNING("apoc - Could not notify lock change");
}

mozApocNativeSettingsMemento mozApocBranchNotifier::RememberNativeValue(const char * aPath) const
{
    mozApocNativeSettingsData * aSettings = new mozApocNativeSettingsData();
    if (aSettings)
    {
        // read access does not need to be proxied 
        nsresult rv = aSettings->Read(mNativeBranch, aPath);
        if (NS_FAILED(rv))
        {
            delete aSettings, aSettings = nsnull; 
        }
    }
    else
        NS_WARNING("apoc - Could not remember native value for later restore: OUT OF MEMORY");

    return aSettings;
}

void mozApocBranchNotifier::RestoreAndNotifyNativeValue(const char * aPath, mozApocNativeSettingsMemento aMemento)
{
    if (aMemento) 
    {
        NOTIFICATION_TRACE(stderr, "Notifying restore of value at %s\n", aPath);
        // write access must be proxied, if possible
        aMemento->Restore(this->getTargetBranch(), aPath);
    }
}


