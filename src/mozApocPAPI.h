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

 
#include "papi/papilib.h"
#include "nsError.h"

class mozApocPAPILayer
{
    PAPILayerList * layer_;
public:
    mozApocPAPILayer(PAPILayerList * layer)
    : layer_(layer)
    {}
    
    PRBool Is() const { return layer_ != nsnull; }
    PRBool HasData() const { return layer_ && layer_->data; }

    const unsigned char * GetData() const 
    { return layer_ ? layer_->data : nsnull; }
    
    PRInt32 GetLength() const 
    { return layer_ ? layer_->size : 0; }
};

class mozApocPAPILayerListEnumerator
{
    PAPILayerList * next_;
public:
    mozApocPAPILayerListEnumerator(PAPILayerList * start)
    : next_(start)
    {}
    
    PRBool HasMore() const { return next_ != nsnull; }
    
    mozApocPAPILayer Next()
    {
        PAPILayerList * data = next_;
        if (next_) next_ = next_->next;
        return data;
    }
};

class mozApocPAPILayerList
{
    PAPILayerList * list_;
public:
    mozApocPAPILayerList()
    : list_(nsnull)
    {}
    
    ~mozApocPAPILayerList()
    { Free(); }

    PRBool IsEmpty() const { return list_ == nsnull; }

    mozApocPAPILayerListEnumerator Enum() const { return list_; }
private:
    friend class mozApocPAPI;
    void SetData(PAPILayerList * list) { Free(); list_ = list; }
    void Free() { if (list_) gPAPILib.papiFreeLayerList(list_); }
private:
    mozApocPAPILayerList(const mozApocPAPILayerList &); // not implemented
    void operator=(const mozApocPAPILayerList &); // not implemented
};

class mozApocPAPIStringListEnumerator
{
    PAPIStringList * next_;
public:
    mozApocPAPIStringListEnumerator(PAPIStringList * start)
    : next_(start)
    {}
    
    PRBool HasMore() const { return next_ != nsnull; }
    
    const char * Next()
    {
        if (!next_) return nsnull;
        
        const char * data = next_->data;
        next_ = next_->next;
        return data;
    }
};

class mozApocPAPIStringList
{
    PAPIStringList * list_;
public:
    mozApocPAPIStringList()
    : list_(nsnull)
    {}
    
    ~mozApocPAPIStringList()
    { Free(); }

    PRBool IsEmpty() const { return list_ == nsnull; }

    mozApocPAPIStringListEnumerator Enum() const { return list_; }
private:
    friend class mozApocPAPI;
    void SetData(PAPIStringList * list) { Free(); list_ = list; }
    void Free() { if (list_) gPAPILib.papiFreeStringList(list_); }
private:
    mozApocPAPIStringList(const mozApocPAPIStringList &); // not implemented
    void operator=(const mozApocPAPIStringList &); // not implemented
};

struct mozApocPAPIListener
{
    virtual void componentAdded(const char * component) = 0;
    virtual void componentModified(const char * component) = 0;
    virtual void componentRemoved(const char * component) = 0;
};

typedef PAPIListenerId mozApocPAPIListenerId;

class mozApocPAPI
{
    PAPI * mPAPI;
public:
    explicit
    mozApocPAPI(PAPIStatus * outStatus = nsnull)
    : mPAPI( gPAPILib.papiInitialise(nsnull, outStatus) )
    {}

    explicit
    mozApocPAPI(const char * inEntityId, PAPIStatus * outStatus = nsnull)
    : mPAPI( gPAPILib.papiInitialise(inEntityId, outStatus) )
    {}

    ~mozApocPAPI()
    {
        if (mPAPI) gPAPILib.papiDeinitialise(mPAPI,nsnull); 
    }

    PRBool IsConnected() const { return mPAPI != nsnull; }
    
    nsresult ReadComponentLayers( const char * inComponentName, mozApocPAPILayerList & outList) const;
    
    nsresult ListComponentNames( const char * inFilter, mozApocPAPIStringList & outList ) const;

    nsresult AddListener(const char * inComponentName, mozApocPAPIListener * inListener, mozApocPAPIListenerId & outId);
    nsresult RemoveListener(mozApocPAPIListenerId inListenerId);
    
    // NYI: SetOffline, IsOffline

    static nsresult ConvertResult(PAPIStatus eStatus);

private:
    mozApocPAPI(const mozApocPAPI&); // not implemented
    void operator=(const mozApocPAPI&); // not implemented
};

