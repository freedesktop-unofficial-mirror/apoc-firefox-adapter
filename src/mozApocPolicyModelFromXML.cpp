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

/* vim:expandtab:shiftwidth=4:tabstop=4:
 */
/*
 *******************************************************************************
 * $RCSfile: mozApocPolicyModelFromXML.cpp,v $
 *
 * Description: 
 *
 * Last change: $Date: 2006/10/30 03:23:53 $ $Revision: 1.1.1.1 $
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved. Use of this 
 * product is subject to license terms. 
 *
 *******************************************************************************
 * Source Code Control System - Header
 *
 * $Header: /export/src/cvs/firefox-apoc-adapter/mozApocPolicyModelFromXML.cpp,v 1.1.1.1 2006/10/30 03:23:53 sunop Exp $
 *******************************************************************************
 */

#include "mozApocPolicyModelFromXML.h"
#include "mozApocSaxWrapper.h"
#include "mozApocXML.h"
#include "nsString.h"

class mozApocBuildPolicyModelFromXML::ParserState
{
public:
    enum ParseContext
    {
        PARSE_FAILED,
        ROOT_ELEMENT,
        NODE_ELEMENT,
        PROP_ELEMENT,
        VALUE_ELEMENT,
        SKIPPED_ELEMENT,
        UNKNOWN_ELEMENT = SKIPPED_ELEMENT
    };
public:
    ParserState(nsACString const & aComponent);
    ~ParserState();

    void SetErrorState(nsresult errcode);
    nsresult GetParseResult() const;

    nsACString const &          GetElementName() const { return m_name; }
    mozApocEntryAttributes GetElementAttributes() const { return m_attributes; }

    nsAString const &           GetLastValue() const { return m_value; }
    void AddToCurrentValue(const nsAString & Data) { m_value += Data; }
    
    ParseContext StartElement(nsAString const & Name, nsAString const * NamespaceURI, 
                                mozApocSaxAttributeIterator Attributes);
    
    ParseContext EndElement(nsAString const & Name, nsAString const * NamespaceURI);
private:
    void CollectAttributes(mozApocSaxAttributeIterator attributes, nsCString & outName);

    ParseContext IdentifyElement(nsAString const & Name, nsAString const * NamespaceURI);

    PRBool IsInProperty() const { return mozApocIsProperty(m_attributes); }
    PRBool IsInValue() const { return mozApocHasValue(m_attributes); }
    
    void EnterRoot() { mozApocSetFlags(m_attributes, MOZAPOC_ENTRY_EXISTS_IN_POLICY); }
    void LeaveRoot() { mozApocResetFlags(m_attributes, MOZAPOC_ENTRY_EXISTS_IN_POLICY); }
    
    void EnterProp() { mozApocSetFlags(m_attributes, MOZAPOC_ENTRY_PROPERTY); }
    void LeaveProp() 
    { 
        mozApocResetFlags(m_attributes, MOZAPOC_ENTRY_PROPERTY); 
        mozApocResetFlags(m_attributes, MOZAPOC_ENTRY_TYPE_MASK);
    }
    
    void EnterValue() { mozApocSetFlags(m_attributes, MOZAPOC_ENTRY_HASVALUE); }
    void LeaveValue() { mozApocResetFlags(m_attributes, MOZAPOC_ENTRY_HASVALUE); }

    ParseContext StartElement(ParseContext NewContext,mozApocSaxAttributeIterator attributes); 
    ParseContext EndElement(ParseContext OldContext);
    ParseContext Fail() { SetErrorState(NS_ERROR_UNEXPECTED); return PARSE_FAILED; }
private:
    nsCString   m_name;
    nsString    m_value;
    PRUint32    m_depth;
    PRUint32    m_skipped;
    nsresult    m_errcode;
    mozApocEntryAttributes m_attributes;
};

nsresult mozApocBuildPolicyModelFromXML::DoBuildModel(const nsACString & aComponent)
{
    if (!m_DataLength) // allow empty data
        return NS_OK;

    NS_ENSURE_TRUE(m_RawData, NS_ERROR_NULL_POINTER);

    // Prevent signed integer overflow
    PRInt32 SignedDataLength = PRInt32(m_DataLength);
    NS_ENSURE_TRUE(SignedDataLength > 0, NS_ERROR_FAILURE);
    
    NS_ENSURE_TRUE(!m_ParserState, NS_ERROR_UNEXPECTED);
    
    m_ParserState = new ParserState(aComponent);

    NS_ENSURE_TRUE(m_ParserState, NS_ERROR_OUT_OF_MEMORY);
    
    mozApocSaxParser aParser(PR_TRUE);
    
    nsresult rv = aParser.Parse(*this, NS_STATIC_CAST(const char *, m_RawData), SignedDataLength);

    if (NS_SUCCEEDED(rv))
        rv = m_ParserState->GetParseResult();

    delete m_ParserState;

    return rv;
}

void mozApocBuildPolicyModelFromXML::StartElement(XML_String const & Name, XML_String const * NamespaceURI, 
                                                    mozApocSaxAttributeIterator Attributes)
{
    NS_ASSERTION(m_ParserState,"apoc - Policy DOM Builder called without active parser");

    switch (m_ParserState->StartElement(Name, NamespaceURI, Attributes)) 
    {
    case ParserState::ROOT_ELEMENT:
    case ParserState::NODE_ELEMENT:
    case ParserState::PROP_ELEMENT:
        m_ParserState->SetErrorState( this->Enter( m_ParserState->GetElementName(), m_ParserState->GetElementAttributes() ) );
        break;

    default:
        break;
    }
}

void mozApocBuildPolicyModelFromXML::EndElement(XML_String const & Name, XML_String const * NamespaceURI)
{
    NS_ASSERTION(m_ParserState,"apoc - Policy DOM Builder called without active parser");

    switch (m_ParserState->EndElement(Name, NamespaceURI)) 
    {
    case ParserState::ROOT_ELEMENT:
    case ParserState::NODE_ELEMENT:
    case ParserState::PROP_ELEMENT:
        m_ParserState->SetErrorState( this->Leave() );
        break;

    case ParserState::VALUE_ELEMENT:
        m_ParserState->SetErrorState( this->AddPropertyValue(m_ParserState->GetLastValue()) );
        break;

    default:
        break;
    }
}

void mozApocBuildPolicyModelFromXML::CharacterData(XML_String const & Characters)
{
    NS_ASSERTION(m_ParserState,"apoc - Policy DOM Builder called without active parser");

    m_ParserState->AddToCurrentValue(Characters);
}

mozApocBuildPolicyModelFromXML::ParserState::ParserState(nsACString const & aComponent)
: m_name(aComponent)
, m_value()
, m_depth(0)
, m_skipped(0)
, m_errcode()
, m_attributes()
{
}


mozApocBuildPolicyModelFromXML::ParserState::~ParserState()
{
}


void mozApocBuildPolicyModelFromXML::ParserState::SetErrorState(nsresult errcode)
{
    if (NS_SUCCEEDED(m_errcode)) m_errcode = errcode;
}


nsresult mozApocBuildPolicyModelFromXML::ParserState::GetParseResult() const
{
    if (NS_SUCCEEDED(m_errcode))
    {
        // check, if parse finished successfully
        NS_ENSURE_TRUE(m_depth == 0 && m_skipped == 0, NS_ERROR_FAILURE);
    }
    return m_errcode;
}

mozApocBuildPolicyModelFromXML::ParserState::ParseContext 
    mozApocBuildPolicyModelFromXML::ParserState::
        StartElement(ParseContext NewContext, mozApocSaxAttributeIterator Attributes)
{
    NS_ASSERTION(mozApocExistsInPolicy(m_attributes),"apoc - SAX builder: attributes were not prepared properly");

    NS_ENSURE_TRUE(!IsInValue(), Fail());

    nsCAutoString NewName;
    CollectAttributes(Attributes,NewName);

    NS_ENSURE_TRUE(!NewName.IsEmpty(),Fail());

    if (NewContext == ROOT_ELEMENT)
        NS_ENSURE_TRUE( m_name.Equals(NewName), Fail() );
    
    m_name = NewName;

    ++m_depth;
    return NewContext;
}

mozApocBuildPolicyModelFromXML::ParserState::ParseContext 
    mozApocBuildPolicyModelFromXML::ParserState::
        StartElement(nsAString const & Name, nsAString const * NamespaceURI, mozApocSaxAttributeIterator Attributes)
{
    ParseContext NewContext = IdentifyElement(Name, NamespaceURI);
    switch (NewContext)
    {
    case ROOT_ELEMENT:
        NS_ENSURE_TRUE(m_depth == 0, Fail());
        EnterRoot();
        return StartElement(NewContext, Attributes);

    case NODE_ELEMENT:
        NS_ENSURE_TRUE(m_depth != 0, Fail());
        NS_ENSURE_TRUE(!IsInProperty(), Fail());
        return StartElement(NewContext, Attributes);

    case PROP_ELEMENT:
        NS_ENSURE_TRUE(m_depth != 0, Fail());
        NS_ENSURE_TRUE(!IsInProperty(), Fail());
        EnterProp();
        return StartElement(NewContext, Attributes);

    case VALUE_ELEMENT:
        NS_ENSURE_TRUE(IsInProperty(), Fail());
        NS_ENSURE_TRUE(!IsInValue(), Fail());
        EnterValue();
        m_value.Truncate();
        break;
        
    case SKIPPED_ELEMENT:
        if (!m_skipped) NS_WARNING("apoc - SAX parser: Skipping unknown element");
        ++m_skipped;
        break;

    case PARSE_FAILED:
        break;
        
    default: 
        NS_NOTREACHED("apoc - Illegal Parser Context");
        return Fail();
    }
    return NewContext;
}


mozApocBuildPolicyModelFromXML::ParserState::ParseContext 
    mozApocBuildPolicyModelFromXML::ParserState::
        EndElement(ParseContext OldContext)
{
    NS_ENSURE_TRUE(!IsInProperty(), Fail());
    NS_ENSURE_TRUE(m_depth > 0, Fail());
    --m_depth;

    m_value.Truncate(); // prevent accumulation of lots of whitespace
    mozApocResetFlags(m_attributes, MOZAPOC_LOCAL_PROTECTION_FLAGS);

    return OldContext;
}

mozApocBuildPolicyModelFromXML::ParserState::ParseContext
    mozApocBuildPolicyModelFromXML::ParserState::
        EndElement(nsAString const & Name, nsAString const * NamespaceURI)
{
    ParseContext OldContext = IdentifyElement(Name, NamespaceURI);
    switch (OldContext)
    {
    case ROOT_ELEMENT:
        NS_ENSURE_TRUE(m_depth == 1, Fail());
        LeaveRoot();
        return EndElement(OldContext);

    case NODE_ELEMENT:
        NS_ENSURE_TRUE(m_depth > 1, Fail());
        return EndElement(OldContext);

    case PROP_ELEMENT:
        NS_ENSURE_TRUE(IsInProperty(), Fail());
        NS_ENSURE_TRUE(!IsInValue(), Fail());
        LeaveProp();
        return EndElement(OldContext);

    case VALUE_ELEMENT:
        NS_ENSURE_TRUE(IsInValue(), Fail());
        LeaveValue();
        break;
        
    case SKIPPED_ELEMENT:
        NS_ENSURE_TRUE(m_skipped > 0, Fail());
        --m_skipped;
        break;

    case PARSE_FAILED:
        break;
        
    default: 
        NS_NOTREACHED("apoc - Illegal Parser Context");
        return Fail();
    }
    return OldContext;
}


static PRBool BooleanAttributeValue(const PRUnichar * aAttributeValue)
{
    if (NS_LITERAL_STRING("true").Equals(aAttributeValue))
        return PR_TRUE;

    return PR_FALSE;
}
static mozApocEntryFlagValues TypeAttributeValue(const PRUnichar * aAttributeValue)
{
    if (NS_LITERAL_STRING(k_mozApocXMLTypeValueString).Equals(aAttributeValue))
        return MOZAPOC_ENTRY_TYPE_STRING;
    if (NS_LITERAL_STRING(k_mozApocXMLTypeValueBool).Equals(aAttributeValue))
        return MOZAPOC_ENTRY_TYPE_BOOL;
    if (NS_LITERAL_STRING(k_mozApocXMLTypeValueInt).Equals(aAttributeValue))
        return MOZAPOC_ENTRY_TYPE_INT;
    if (NS_LITERAL_STRING(k_mozApocXMLTypeValueShort).Equals(aAttributeValue))
        return MOZAPOC_ENTRY_TYPE_INT;
    if (NS_LITERAL_STRING(k_mozApocXMLTypeValueBinary).Equals(aAttributeValue))
        return MOZAPOC_ENTRY_TYPE_BINARY;
    return MOZAPOC_ENTRY_TYPE_OTHER;
}

void mozApocBuildPolicyModelFromXML::ParserState:: 
    CollectAttributes(mozApocSaxAttributeIterator attributes, nsCString & outName)
{
    mozApocResetFlags(m_attributes, MOZAPOC_LOCAL_PROTECTION_FLAGS);
    NS_ASSERTION(!mozApocAreAnyFlagsSet(m_attributes,MOZAPOC_ENTRY_PROTECTED_FLAGS),
                "apoc - Inherited protection flag found in SAX Node builder");
    NS_ASSERTION(!mozApocAreAnyFlagsSet(m_attributes,MOZAPOC_ENTRY_TYPE_MASK),
                "apoc - Attributes have a type when starting node");
        
    //Brian++ const NS_NAMED_LITERAL_STRING( OORNamespaceURI, k_mozApocXMLNamespaceOOR );
    NS_NAMED_LITERAL_STRING( OORNamespaceURI, k_mozApocXMLNamespaceOOR );
    //Brian++ const NS_NAMED_LITERAL_STRING( AttributeName,      k_mozApocXMLAttributeName );
    NS_NAMED_LITERAL_STRING( AttributeName,      k_mozApocXMLAttributeName );
    //Brian++ const NS_NAMED_LITERAL_STRING( AttributeFinalized, k_mozApocXMLAttributeFinalized );
    NS_NAMED_LITERAL_STRING( AttributeFinalized, k_mozApocXMLAttributeFinalized );
    //Brian++ const NS_NAMED_LITERAL_STRING( AttributeMandatory, k_mozApocXMLAttributeMandatory );
    NS_NAMED_LITERAL_STRING( AttributeMandatory, k_mozApocXMLAttributeMandatory );

    for (mozApocSaxAttributeIterator CurrentAttribute = attributes; CurrentAttribute.Is(); CurrentAttribute.Next())
    {
        if (CurrentAttribute.IsNamed(AttributeName, &OORNamespaceURI, PR_TRUE))
        {
            outName.AssignWithConversion( CurrentAttribute.GetAttributeValue() );
        }
        else if (CurrentAttribute.IsNamed(AttributeFinalized, &OORNamespaceURI, PR_TRUE))
        {
            if ( BooleanAttributeValue(CurrentAttribute.GetAttributeValue()) )
                mozApocSetFlags(m_attributes,MOZAPOC_ENTRY_FINALIZED);
        }
        else if (CurrentAttribute.IsNamed(AttributeMandatory, &OORNamespaceURI, PR_TRUE))
        {
            if ( BooleanAttributeValue(CurrentAttribute.GetAttributeValue()) )
                mozApocSetFlags(m_attributes,MOZAPOC_ENTRY_MANDATORY);
        }
            
        else if (mozApocIsProperty(m_attributes))
        {                            
            if (CurrentAttribute.IsNamed(NS_LITERAL_STRING(k_mozApocXMLAttributeType), &OORNamespaceURI, PR_TRUE))
                mozApocSetFlags(m_attributes,TypeAttributeValue(CurrentAttribute.GetAttributeValue()));
        }
    }
}


mozApocBuildPolicyModelFromXML::ParserState::ParseContext 
    mozApocBuildPolicyModelFromXML::ParserState::
        IdentifyElement(nsAString const & Name, nsAString const * NamespaceURI)
{
    if (NS_FAILED(m_errcode)) return PARSE_FAILED;

    if (m_skipped) return SKIPPED_ELEMENT;

    if (NamespaceURI)
    {
        //Brian++ const NS_NAMED_LITERAL_STRING( OORNamespaceURI, k_mozApocXMLNamespaceOOR );
        NS_NAMED_LITERAL_STRING( OORNamespaceURI, k_mozApocXMLNamespaceOOR );
        if (!NamespaceURI->Equals(OORNamespaceURI))
            return UNKNOWN_ELEMENT;
    }
    
    //Brian++ const NS_NAMED_LITERAL_STRING( OORTagRoot, k_mozApocXMLTagRoot );
    NS_NAMED_LITERAL_STRING( OORTagRoot, k_mozApocXMLTagRoot );
    if (Name.Equals(OORTagRoot)) return ROOT_ELEMENT;

    //Brian++ const NS_NAMED_LITERAL_STRING( OORTagNode, k_mozApocXMLTagNode );
    NS_NAMED_LITERAL_STRING( OORTagNode, k_mozApocXMLTagNode );
    if (Name.Equals(OORTagNode)) return NODE_ELEMENT;

    //Brian++ const NS_NAMED_LITERAL_STRING( OORTagProp, k_mozApocXMLTagProp );
    NS_NAMED_LITERAL_STRING( OORTagProp, k_mozApocXMLTagProp );
    if (Name.Equals(OORTagProp)) return PROP_ELEMENT;

    //Brian++ const NS_NAMED_LITERAL_STRING( OORTagValue, k_mozApocXMLTagValue );
    NS_NAMED_LITERAL_STRING( OORTagValue, k_mozApocXMLTagValue );
    if (Name.Equals(OORTagValue)) return VALUE_ELEMENT;

    return UNKNOWN_ELEMENT;
}


