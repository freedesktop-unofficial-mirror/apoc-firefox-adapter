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


#include "mozApocPAPI.h"
#include "nsDebug.h"
#include "prenv.h"

extern "C"
{
static 
void mozApocPAPIListenerDispatch( const PAPIEvent * inEvent, void * inUserData )
{
    NS_ASSERTION(inUserData, "apoc - NULL user data passed to listener");
    NS_ASSERTION(inEvent,    "apoc - NULL event passed to listener");
    NS_ASSERTION(inEvent->componentName, "apoc - NULL component name passed to listener");
    
    mozApocPAPIListener * listener = NS_STATIC_CAST(mozApocPAPIListener *, inUserData);
    switch (inEvent->eventType)
    {
    case PAPIComponentAdd: 
        listener->componentAdded(inEvent->componentName); 
        break;
    case PAPIComponentModify: 
        listener->componentModified(inEvent->componentName); 
        break;
    case PAPIComponentRemove: 
        listener->componentRemoved(inEvent->componentName); 
        break;
    default:
        NS_NOTREACHED("apoc - Unknown event type in PAPI notification");
    }
}
}

nsresult mozApocPAPI::ReadComponentLayers( const char * inComponentName, mozApocPAPILayerList & outList) const
{
    NS_ENSURE_STATE(mPAPI);
    
    PAPIStatus status = PAPISuccess;
    outList.SetData( gPAPILib.papiReadComponentLayers( mPAPI, inComponentName, &status ) );
    if (status == PAPINoSuchComponentFailure)
        return NS_OK;
    else
        return ConvertResult(status);
}

nsresult mozApocPAPI::ListComponentNames( const char * inFilter, mozApocPAPIStringList& outList ) const
{
    NS_ENSURE_STATE(mPAPI);
    
    PAPIStatus status = PAPISuccess;
    outList.SetData( gPAPILib.papiListComponentNames( mPAPI, inFilter, &status ) );
    return ConvertResult(status);
}

nsresult mozApocPAPI::AddListener(const char * inComponentName, mozApocPAPIListener * inListener, mozApocPAPIListenerId & outId)
{
    PAPIStatus status = PAPISuccess;
    outId = gPAPILib.papiAddListener( mPAPI, inComponentName, mozApocPAPIListenerDispatch, (void*)inListener, &status);
    return ConvertResult(status);
}

nsresult mozApocPAPI::RemoveListener(mozApocPAPIListenerId inListenerId)
{
    PAPIStatus status = PAPISuccess;
    gPAPILib.papiRemoveListener( mPAPI,  inListenerId, &status);
    return ConvertResult(status);
}

nsresult mozApocPAPI::ConvertResult(PAPIStatus eStatus)
{
    switch (eStatus)
    {
    case PAPISuccess:
        return NS_OK;

    case PAPIConnectionFailure:
        return NS_ERROR_NOT_AVAILABLE;

    case PAPIInvalidArg:
    case PAPINoSuchComponentFailure:
        return NS_ERROR_INVALID_ARG;
        
    case PAPIAuthenticationFailure:
    case PAPILDBFailure:
    case PAPIUnknownFailure:
        return NS_ERROR_FAILURE;
        
    case PAPIOutOfMemoryFailure:
        return NS_ERROR_OUT_OF_MEMORY;

    default:
        NS_WARNING("apoc - Unknown PAPI status code");
        return NS_ERROR_UNEXPECTED;
    }
}

// implement support for dynamically loading PAPI
#include "prlink.h"

// todo: adjust for porting
static const char kPAPILib_LibName[] = "libapoc.so";

static int PAPILib_loadLibrary(struct PAPILib * libdata)
{
   char* moz_in_install = PR_GetEnv("MOZ_IN_INSTALL");
                                                                                
    if (moz_in_install)
        return 0;  // this is during the quick silver installation
                   // don't load apoc library now.
                                                                                

    if (libdata->module_)
        return 0; // we have a module which failed before
    
    PRLibrary * libpapi = PR_LoadLibrary(kPAPILib_LibName);
    if (!libpapi)
        return 0;
        
    libdata->module_ = libpapi;
        
    int success = 1;

    // find the symbols for all interface functions in libpapi
#define PAPI_DECLARATION( resulttype, functionname, parameterlist ) \
    if (PRFuncPtr func = PR_FindFunctionSymbol( libpapi, #functionname)) \
        libdata->functionname = reinterpret_cast<functionname ## Ptr>(func); \
    else { \
        NS_WARNING("apoc - libpapi.so is missing function \"" #functionname "\""); \
        success = 0; \
    }

#include "papi/papidecl.h"
#undef PAPI_DECLARATION
    
    return success;
}

// implement the PAPI stub functionality here
#include "papi/papilib.c"

