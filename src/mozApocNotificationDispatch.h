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


#ifndef MOZAPOC_NOTIFICATIONDISPATCH_H_INCLUDED
#define MOZAPOC_NOTIFICATIONDISPATCH_H_INCLUDED 

#include "nsString.h"

typedef struct mozApocNativeSettingsData const * mozApocNativeSettingsMemento;
extern void mozApocDiscardNativeSettings(mozApocNativeSettingsMemento aMemento);

struct mozApocNotifier
{
    // dispatch a notification
    virtual void NotifyValueChange(const char * aPath, nsAString const & aValue, PRBool bLocked) = 0;
    virtual void NotifyLockChange(const char * aPath, PRBool bLocked) = 0;
    
    // help for remembering/restoring the native value
    virtual mozApocNativeSettingsMemento RememberNativeValue(const char * aPath) const = 0;
    virtual void RestoreAndNotifyNativeValue(const char * aPath, mozApocNativeSettingsMemento aMemento)= 0;
    
protected:
    // to silence warnings
    virtual ~mozApocNotifier() {}
};

class mozApocNotificationDispatcher
{
    nsCString Path;
    mozApocNotifier * Notifier;
public:
    mozApocNotificationDispatcher(nsACString const & aPath, mozApocNotifier * pNotifier)
    : Path(aPath)
    , Notifier(pNotifier)
    {}

    mozApocNotificationDispatcher(mozApocNotificationDispatcher const & aParent, nsACString const & aChildName);

    mozApocNotificationDispatcher GetChildNotifier(nsACString const & aChildName) const
    { return mozApocNotificationDispatcher(*this,aChildName); }

    void NotifyValueChange(nsAString const & aValue, PRBool bLocked) const
    { if (Notifier) Notifier->NotifyValueChange(Path.get(),aValue,bLocked); }

    void NotifyLockChange(PRBool bLocked) const
    { if (Notifier) Notifier->NotifyLockChange(Path.get(),bLocked); }
    
    // help for remembering/restoring the native value
    mozApocNativeSettingsMemento RememberNativeValue() const
    { return Notifier ? Notifier->RememberNativeValue(Path.get()) : nsnull; }
    
    void RestoreAndNotifyNativeValue(mozApocNativeSettingsMemento aMemento) const
    { if (Notifier && aMemento) Notifier->RestoreAndNotifyNativeValue(Path.get(),aMemento); }
    
};

#endif

