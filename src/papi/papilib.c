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

/* papilib.c - Policy Access API library stub implementations */

#include "papilib.h"

/* declare stub functions for all interfaces */
#define PAPI_DECLARATION( resulttype, functionname, parameterlist ) \
    static resulttype functionname ## Stub parameterlist;

#include "papidecl.h"
#undef PAPI_DECLARATION

/* define the global PAPILib instance */
struct PAPILib gPAPILib = 
{
/* initialize function pointers for all interfaces to the stub functions */
#define PAPI_DECLARATION( resulttype, functionname, parameterlist ) \
    functionname ## Stub,

#include "papidecl.h"
#undef PAPI_DECLARATION

    0 /* no module */
};


/* Initialize/deinitialize PAPI functions */

/* Provided by the context: */
int PAPILib_loadLibrary(struct PAPILib * libdata);

/*
 * Load the PAPI functions if not yet done
 */
static PAPI * papiInitialiseStub(const char * inEntityId, PAPIStatus * outStatus)
{
    if (!PAPILib_loadLibrary(&gPAPILib))
    {
        if (outStatus) *outStatus = PAPIConnectionFailure;
        return 0;
    }
    
    return gPAPILib.papiInitialise(inEntityId,outStatus);
}

/*
 * Invalidate and free all resoures associated with a PAPI handle
 * previously returned by papiInitialise()
 */
static void papiDeinitialiseStub(PAPI * , PAPIStatus * outStatus)
{
    if (outStatus) *outStatus = PAPIConnectionFailure;
}

/* other PAPI functions - empty stubs */
static PAPILayerList * papiReadComponentLayersStub(const PAPI * , const char * , PAPIStatus * outStatus)
{
    if (outStatus) *outStatus = PAPIConnectionFailure;
    return 0;
}

static PAPIStringList * papiListComponentNamesStub(const PAPI * , const char * , PAPIStatus * outStatus)
{
    if (outStatus) *outStatus = PAPIConnectionFailure;
    return 0;
}

#if PAPI_OFFLINE_CONTROL
static void papiSetOfflineStub(const PAPI * , const char * , int , PAPIStatus * outStatus)
{
    if (outStatus) *outStatus = PAPIConnectionFailure;
}

static int papiIsOfflineStub(const PAPI * , const char * , int * , PAPIStatus * outStatus)
{
    if (outStatus) *outStatus = PAPIConnectionFailure;
    return 0;
}
#endif

static PAPIListenerId papiAddListenerStub( const PAPI * , const char * , PAPIListener , void * , PAPIStatus * outStatus)
{
    if (outStatus) *outStatus = PAPIConnectionFailure;

    return 0;
}

static void papiRemoveListenerStub( const PAPI * , PAPIListenerId , PAPIStatus * outStatus)
{
    if (outStatus) *outStatus = PAPIConnectionFailure;
}

static void papiFreeLayerListStub( PAPILayerList * ) 
{}

static void papiFreeStringListStub( PAPIStringList * ) 
{}
    

