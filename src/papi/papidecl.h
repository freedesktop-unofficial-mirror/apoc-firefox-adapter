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

/* papidecl.h - Policy Access API library function declaration helper */

#ifndef PAPI_DECLARATION
#error Must #define PAPI_DECLARATION( resulttype, functionname, parameterlist ) before including papidecl.h
#endif

/* PAPI functions */

/* don't include offline interface, unless requested */
#ifndef PAPI_OFFLINE_CONTROL
#define PAPI_OFFLINE_CONTROL 0
#endif

/* Main PAPI functions */

/*
 * Obtain a new PAPI handle for the specified entity
 * Specifying inEntityId=NULL will result in the entity id being calculated
 * from the current user id.
 */
PAPI_DECLARATION( PAPI *, papiInitialise, (const char * inEntityId, PAPIStatus * outStatus) )

/*
 * Invalidate and free all resoures associated with a PAPI handle
 * previously returned by papiInitialise()
 */
PAPI_DECLARATION( void, papiDeinitialise, (PAPI * inPAPI, PAPIStatus * outStatus) )

/*
 * Retrieve all layers for a specified component
 * Returns a PAPILayerList ( See papiTypes.h ) ordered according to mergepath
 * 
 * The caller must "free" the returned PAPILayerList using papiFreeLayerList()
 */
PAPI_DECLARATION( PAPILayerList *, papiReadComponentLayers, 
                    (const PAPI * inPAPI, const char * inComponentName, PAPIStatus * outStatus) )

/*
 * List all available components which comply with the specified filter
 * Returns a PAPIStringList ( See papiTypes.h )
 *
 * The caller must "free" the returned PAPIStringList using papiFreeStringList()
 * 
 * TBD: specify the supported filter syntax
 */
PAPI_DECLARATION( PAPIStringList *, papiListComponentNames, 
                    (const PAPI * inPAPI, const char * inFilter, PAPIStatus * outStatus) )

#if PAPI_OFFLINE_CONTROL
/*
 * Mark the specified component as offline or online
 */
PAPI_DECLARATION( void, papiSetOffline,
                    (const PAPI * inPAPI, const char * inComponentName, int inOffline, PAPIStatus * outStatus) )

/*
 * Query the online/offline status of a specified component
 */
PAPI_DECLARATION( int, papiIsOffline,
                    (const PAPI * inPAPI, const char * inComponentName, int * outOffline, PAPIStatus * outStatus) )
#endif
    
/*
 * Add a listener for changes to the specified component
 */
PAPI_DECLARATION( PAPIListenerId, papiAddListener,
                    ( const PAPI * inPAPI, const char * inComponentName, 
                      PAPIListener inListener, void * inUserData, PAPIStatus * outStatus) )

/*
 * Remove a listener previously added by papiAddListener()
 */
PAPI_DECLARATION( void, papiRemoveListener, ( const PAPI * inPAPI, PAPIListenerId inListenerId, PAPIStatus * outStatus) )

/*
 * Free all resources associated with the specified list
 */
PAPI_DECLARATION( void, papiFreeLayerList, ( PAPILayerList * inList ) )
PAPI_DECLARATION( void, papiFreeStringList, ( PAPIStringList * inList ) )

