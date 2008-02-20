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


#include "mozApocPolicyData.h"
#include "mozApocPolicyBackend.h"
#include "mozApocPathParser.h"
#include "nsReadableUtils.h"
#include "nsISupportsPrimitives.h"
#include "nsILocalFile.h"
#include "pref/nsIRelativeFilePref.h"
#include "nsIPrefLocalizedString.h"

static 
nsresult 
ParseComplexValue(const nsString & aData, mozApocEntryAttributes aAttributes, 
                  const nsIID & anID, /* [out, iid_is(anID)] */void * * aResult);

mozApocPolicyEntry::mozApocPolicyEntry()
: mData()
, mErrCode(NS_ERROR_NOT_INITIALIZED)
{
}

mozApocPolicyEntry::mozApocPolicyEntry(mozApocPolicyBackend & aBackend, char const * aPath, PRBool bPreferProp)
: mData()
, mErrCode(NS_OK)
{
    if (aPath == 0)
    {
        NS_WARNING("apoc - Policy Entry constructed on NULL path");
        mErrCode = NS_ERROR_INVALID_POINTER;
        return;
    }
    mozApocPathParser aParser(aPath);
    if (!aParser.HasData())
    {
        NS_WARNING("apoc - Policy Entry constructed on empty path");
        NS_WARNING(aPath ? aPath : "<null>");
        mErrCode = NS_ERROR_INVALID_ARG;
        return;
    }
    mData =  mozApocPolicyNodeData(aBackend.GetComponentData(aParser.GetComponent(), &mErrCode));
    if (HasError()) NS_WARNING("apoc - Error in GetComponentData - Policy Entry is invalid");
    FollowPath(aParser, bPreferProp);
}

mozApocPolicyEntry::mozApocPolicyEntry(mozApocPolicyComponentData const * aComponent, char const * aPath, PRBool bPreferProp)
: mData()
, mErrCode(NS_OK)
{
    mozApocPathParser aParser(aPath);
    if (aParser.HasData() && aParser.GetComponent().Equals(aComponent->GetComponentName()))
    {
        mData =  mozApocPolicyNodeData(aComponent);
    }
    else
    {
        NS_WARNING("apoc - Component data does not match path");
        mErrCode = NS_ERROR_INVALID_ARG;
    }
    FollowPath(aParser, bPreferProp);
}

mozApocPolicyEntry::mozApocPolicyEntry(mozApocPolicyEntry const & aEntry, char const * aRelativePath, PRBool bPreferProp)
: mData(aEntry.mData)
, mErrCode(aEntry.mErrCode)
{
    if (NS_FAILED(mErrCode)) return;
    
    if (aRelativePath == 0)
    {
        NS_WARNING("apoc - Policy Entry constructed on NULL path");
        SetError(NS_ERROR_INVALID_POINTER);
        return;
    }
    
    mozApocPathParser aParser(aRelativePath);
    if (!aParser.HasData())
    {
        NS_WARNING("apoc - Policy Entry constructed on empty path");
        SetError(NS_ERROR_INVALID_ARG);
        return;
    }
    
    FollowPath(aParser, bPreferProp);
}

void mozApocPolicyEntry::SetError(nsresult aErrCode)
{
    mErrCode = aErrCode;
    mData = mozApocPolicyNodeData();
}

static 
PRBool GotoFallbackChild(mozApocPolicyNodeData & aData, mozApocPathParser::partString const & aFallbackName)
{
    if (!aData.HasRealChild(aFallbackName))
    {
        return PR_FALSE;
    }
    
    nsresult result = aData.GotoChild(aFallbackName);
    if (NS_FAILED(result))
    {
        NS_WARNING("apoc - GotoChild failed for fallback");
        return PR_FALSE;
    }

    return PR_TRUE;
}

void mozApocPolicyEntry::FollowPath(mozApocPathParser & aParser, PRBool bPreferProp)
{
    while (mData.IsReal() && aParser.MoveNext() && NS_SUCCEEDED(mErrCode))
    {
        mozApocPathParser::partString aNextPart = aParser.GetCurrentPart();

        // prefer fallback, if the result should preferably be a Property 
        // (rather than a node with a conflicting name)
        if (bPreferProp || !mData.HasRealChild(aNextPart))
        {
            if (GotoFallbackChild(mData,aParser.GetRemainingPath()))
                break;
        }

        nsresult result = mData.GotoChild(aNextPart);
    
        if (NS_FAILED(result))
        {
            if (GotoFallbackChild(mData,aParser.GetRemainingPath()))
                break;

            NS_WARNING("apoc - GotoChild failed: Setting PolicyEntry to error state");
            SetError(result);
        }
    }
}

nsresult mozApocPolicyEntry::GetBoolValue(PRBool * _retval) const
{
    NS_ENSURE_ARG_POINTER(_retval);
    
    NS_ENSURE_FALSE(HasError(), NS_ERROR_UNEXPECTED);
    
    // to do: check and handle type issues
    nsAutoString value;
    nsresult rv = mData.GetValue(value);
    if (NS_SUCCEEDED(rv))
    {
        if (value.Equals(NS_LITERAL_STRING("false")))
            *_retval = PR_FALSE;

        else if (value.Equals(NS_LITERAL_STRING("true")))
            *_retval = PR_TRUE;

        else 
        {
            NS_WARNING("apoc - GetBoolValue: Invalid data - not a boolean value");
            rv = NS_ERROR_UNEXPECTED;
        }
    }
    
    return rv;
}

nsresult mozApocPolicyEntry::GetIntValue(PRInt32 * _retval) const
{
    NS_ENSURE_ARG_POINTER(_retval);
    
    NS_ENSURE_FALSE(HasError(), NS_ERROR_UNEXPECTED);
    
    // to do: check and handle type issues
    nsAutoString value;
    nsresult rv = mData.GetValue(value);
    if (NS_SUCCEEDED(rv))
    {
        PRInt32 errcode = NS_OK;
        PRInt32 intVal = value.ToInteger(&errcode,kRadix10);

        rv = NS_STATIC_CAST(nsresult,errcode);
        if (NS_SUCCEEDED(rv))
            *_retval = intVal;
        else
            NS_WARNING("apoc -  - GetIntValue: Invalid data - not a numeric value");
    }
    
    return rv;
}

nsresult mozApocPolicyEntry::GetCharValue(char **_retval) const
{
    NS_ENSURE_ARG_POINTER(_retval);
    
    NS_ENSURE_FALSE(HasError(), NS_ERROR_UNEXPECTED);
    
    // to do: check and handle type issues
    nsAutoString value;
    nsresult rv = mData.GetValue(value);
    if (NS_SUCCEEDED(rv))
    {
        *_retval = ToNewUTF8String(value);
        if (!*_retval)
            rv = NS_ERROR_OUT_OF_MEMORY;
    }
    
    return rv;
}

nsresult mozApocPolicyEntry::GetUnicodeValue(PRUnichar **_retval) const
{
    NS_ENSURE_ARG_POINTER(_retval);
    
    NS_ENSURE_FALSE(HasError(), NS_ERROR_UNEXPECTED);
    
    // to do: check and handle type issues
    nsAutoString value;
    nsresult rv = mData.GetValue(value);
    if (NS_SUCCEEDED(rv))
    {
        *_retval = ToNewUnicode(value);
        if (!*_retval)
            rv = NS_ERROR_OUT_OF_MEMORY;
    }
    
    return rv;
}

nsresult mozApocPolicyEntry::GetComplexValue(const nsIID & aType, void * *aValue) const
{
    NS_ENSURE_ARG_POINTER(aValue);
    
    NS_ENSURE_FALSE(HasError(), NS_ERROR_UNEXPECTED);
    
    // to do: check and handle type issues
    nsAutoString value;
    nsresult rv = mData.GetValue(value);
    if (NS_SUCCEEDED(rv))
    {
        rv = ParseComplexValue(value, mData.GetAttributes(), aType, aValue);
        NS_WARN_IF_FALSE( NS_SUCCEEDED(rv), "apoc -  - GetComplexValue: "
                "Invalid data or request - could not create object of expected type from data.");
    }
    
    return rv;
}


static const char k_mozApocFlatSettingComponent[] = "ooc";

mozApocPolicyData::mozApocPolicyData()
: mBackend(NULL)
, mRootEntry()
{
}

mozApocPolicyData::mozApocPolicyData(mozApocPolicyBackend & aBackend)
: mBackend(&aBackend)
, mRootEntry() // to be initialized later
{
}

mozApocPolicyData::mozApocPolicyData(mozApocPolicyEntry const & aRootEntry)
: mBackend(NULL)
, mRootEntry(aRootEntry)
{
}

mozApocPolicyData::~mozApocPolicyData()
{
}

PRBool mozApocPolicyData::HasError() const 
{ 
    // support lazy init
    if (mBackend && mRootEntry.GetError() == NS_ERROR_NOT_INITIALIZED)
        return PR_FALSE;
    
    return mRootEntry.HasError(); 
}

nsresult mozApocPolicyData::GetError() const 
{ 
    // support lazy init
    if (mBackend && mRootEntry.GetError() == NS_ERROR_NOT_INITIALIZED)
        return NS_OK;

    return mRootEntry.GetError(); 
}

mozApocPolicyEntry mozApocPolicyData::GetEntry(char const * path, PRBool bPreferProp) const
{
    if (mBackend)
    {
        mozApocPolicyEntry aResult(*mBackend,path,bPreferProp);

        bool bUseOOC = !aResult.IsReal() && path && 
                        mozApocPathParser(path).TotalDepth() == 1;
        if (!bUseOOC) 
            return aResult;

        // do lazy initialization
        if (mRootEntry.GetError() == NS_ERROR_NOT_INITIALIZED)
            mRootEntry = mozApocPolicyEntry(*mBackend,k_mozApocFlatSettingComponent,PR_FALSE);

        NS_ASSERTION(mRootEntry.GetError() != NS_ERROR_NOT_INITIALIZED,
                    "apoc - Unexpected error from reading policy data: change lazy initialisation");
    }
    return mozApocPolicyEntry(mRootEntry,path,bPreferProp);
}

static 
nsresult ParseComplexValue(const nsString & aData, mozApocEntryAttributes aAttributes, 
                           const nsIID & aType, void * * aResult)
{
    NS_WARN_IF_FALSE(mozApocGetEntryType(aAttributes) != MOZAPOC_ENTRY_TYPE_BINARY, 
                     "apoc - ParseComplexValue: support for binary (serialized) data is not yet available");
    
    if (aType.Equals(NS_GET_IID(nsISupportsString))) 
    {
        nsresult rv = NS_ERROR_FAILURE;
        nsCOMPtr<nsISupportsString> theString(do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID, &rv));
        NS_ENSURE_SUCCESS(rv,rv);

        rv = theString->SetData(aData);
        NS_ENSURE_SUCCESS(rv,rv);
            
        nsISupportsString * temp = theString;

        NS_ADDREF(temp);
        *aResult = temp;
        return rv;
    }

    if (aType.Equals(NS_GET_IID(nsIPrefLocalizedString))) 
    {
        nsresult rv = NS_ERROR_FAILURE;
        nsCOMPtr<nsIPrefLocalizedString> theString(do_CreateInstance(NS_PREFLOCALIZEDSTRING_CONTRACTID, &rv));
        NS_ENSURE_SUCCESS(rv,rv);

        rv = theString->SetDataWithLength(aData.Length(),aData.get());
        NS_ENSURE_SUCCESS(rv,rv);
            
        nsIPrefLocalizedString * temp = theString;

        NS_ADDREF(temp);
        *aResult = temp;
        return rv;
    }
    
    if (aType.Equals(NS_GET_IID(nsILocalFile))) 
    {
        // rv = file->SetPersistentDescriptor(utf8String); //--> to be used for binary data
            
        nsCOMPtr<nsILocalFile> file;
        nsresult rv = NS_NewLocalFile(aData, PR_FALSE, getter_AddRefs(file));
        NS_ENSURE_SUCCESS(rv,rv);

        nsILocalFile * temp = file;

        NS_ADDREF(temp);
        *aResult = temp;
        return rv;
    }
    
    if (aType.Equals(NS_GET_IID(nsIRelativeFilePref))) 
    {
        nsresult rv = NS_ERROR_FAILURE;
        
        nsAString::const_iterator keyBegin, strEnd;
        aData.BeginReading(keyBegin);
        aData.EndReading(strEnd);    

        // The pref has the format: [fromKey]a/b/c
        NS_ENSURE_TRUE(*keyBegin++ == PRUnichar('['), rv);
    
        nsAString::const_iterator keyEnd(keyBegin);
        NS_ENSURE_TRUE( FindCharInReadable(PRUnichar(']'), keyEnd, strEnd), rv);
    
        // assumption: directory key will only ever be ASCII
        NS_LossyConvertUCS2toASCII key(Substring(keyBegin, keyEnd));
    
        nsCOMPtr<nsIProperties> directoryService(do_GetService(NS_DIRECTORY_SERVICE_CONTRACTID, &rv));
        NS_ENSURE_SUCCESS(rv,rv);

        nsCOMPtr<nsILocalFile> theFile;
        rv = directoryService->Get(key.get(), NS_GET_IID(nsILocalFile), getter_AddRefs(theFile));
        NS_ENSURE_SUCCESS(rv,rv);
    
        rv = theFile->AppendRelativePath(Substring(++keyEnd, strEnd));
        NS_ENSURE_SUCCESS(rv,rv);
    
        nsCOMPtr<nsIRelativeFilePref> relativePref;
        rv = NS_NewRelativeFilePref(theFile, key, getter_AddRefs(relativePref));
        NS_ENSURE_SUCCESS(rv,rv);

        nsIRelativeFilePref * temp = relativePref;

        NS_ADDREF(temp);
        *aResult = temp;
        return rv;
    }
    
#if 0 // No support for this with unicode strings
    // This is deprecated and you should not be using it
    if (aType.Equals(NS_GET_IID(nsIFileSpec))) 
    {
        nsresult rv = NS_ERROR_FAILURE;
        nsCOMPtr<nsIFileSpec> file(do_CreateInstance(NS_FILESPEC_CONTRACTID, &rv));
        NS_ENSURE_SUCCESS(rv,rv);

        PRBool      valid;
        file->SetPersistentDescriptorString(utf8String);  // only returns NS_OK
        file->IsValid(&valid);
        if (!valid) {
            /* if the string wasn't a valid persistent descriptor, it might be a valid native path */
            file->SetNativePath(utf8String);
        }

        nsIFileSpec *temp = file;

        NS_ADDREF(temp);
        *aResult = temp;
        return rv;
    }
#endif

    NS_WARNING("apoc - ParseComplexValue - Unsupported interface type");
    return NS_NOINTERFACE;
}

