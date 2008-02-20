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
 * $RCSfile: mozApocSaxWrapper.cpp,v $
 *
 * Description: 
 *
 * Last change: $Date: 2006/10/30 05:26:18 $ $Revision: 1.2 $
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved. Use of this 
 * product is subject to license terms. 
 *
 *******************************************************************************
 * Source Code Control System - Header
 *
 * $Header: /export/src/cvs/firefox-apoc-adapter/mozApocSaxWrapper.cpp,v 1.2 2006/10/30 05:26:18 brian Exp $
 *******************************************************************************
 */

#include "mozApocSaxWrapper.h"

#include "nsString.h"
#include "nsDependentString.h"
#include <stdio.h>

#define MOZAPOC_INDIRECT_HANDLER 0

const XML_Char k_NSSep = XML_Char('|');

static inline
PRInt32 SplitQName(nsDependentString & Name)
{
    PRInt32 SplitIndex = Name.FindChar(k_NSSep);
    
    Name.Rebind(Name.get() + SplitIndex + 1, Name.get() + Name.Length());

    return SplitIndex;
}

void mozApocSaxAttributeIterator::Next()
{
    NS_PRECONDITION( this->Is(), "apoc - Error: advancing an invalid Sax Attribute Iterator");
    m_data += 2;
}

PRBool mozApocSaxAttributeIterator::IsNamed(XML_String const & LocalName,
                                                            XML_String const * NamespaceURI,
                                                            PRBool IsNamespaceRequired) const
{
    NS_PRECONDITION( this->Is(), "apoc - Error: dereferencing an invalid Sax Attribute Iterator");

    nsDependentString ActualName( *m_data );
    PRInt32 SplitIndex = SplitQName(ActualName);

    if (!ActualName.Equals(LocalName)) return PR_FALSE;

    if (SplitIndex >= 0) // found namespace
    {
        if (NamespaceURI)
        {
            nsAutoString FoundURI(*m_data, PRUint32(SplitIndex));
            return NamespaceURI->Equals(FoundURI);
        }
    }
    else 
    {
        if (!NamespaceURI)
            return !PR_TRUE;
    }

    // one URI is there, the other isn't
    return !IsNamespaceRequired;
}

XML_Char const * mozApocSaxAttributeIterator::GetLocalName() const
{
    NS_PRECONDITION( this->Is(), "apoc - Error: dereferencing an invalid Sax Attribute Iterator");

    nsDependentString LocalName( *m_data );
    SplitQName(LocalName);
    return LocalName.get();
}

XML_Char const * mozApocSaxAttributeIterator::GetAttributeValue() const
{
    NS_PRECONDITION( this->Is(), "apoc - Error: dereferencing an invalid Sax Attribute Iterator");

    return m_data[1];
}
    
XML_Char const * mozApocSaxAttributeIterator::FindAttributeValue(XML_String const & LocalName, 
                                    XML_String const * NamespaceURI,
                                    PRBool IsNamespaceRequired) const
{
    for (mozApocSaxAttributeIterator it(m_data); it.Is(); it.Next())
    {
        if (it.IsNamed(LocalName,NamespaceURI,IsNamespaceRequired))
            return it.GetAttributeValue();
    }
    return nsnull;
}


static 
inline
mozApocSaxHandler * GetCallbackHandler(void * callbackdata)
{
#if MOZAPOC_INDIRECT_HANDLER
    XML_Parser parser = NS_STATIC_CAST(XML_Parser, callbackdata);
    NS_ASSERTION(parser,"apoc - Expat passing NULL parser in callback");
    void * handler = XML_GetUserData(parser);
#else
    void * handler = callbackdata;
#endif
 
    NS_WARN_IF_FALSE(handler, "apoc - SAX callback invoked without a handler");

    return NS_STATIC_CAST(mozApocSaxHandler *, handler);
}

nsresult TranslateSaxError(XML_Parser parser)
{
    XML_Error errcode = XML_GetErrorCode(parser);

    if (errcode != XML_ERROR_NONE)
      ::fprintf(stderr, "apoc - SAX Error (Code=%d)\n", int(errcode));

    switch (errcode)
    {
    case XML_ERROR_NONE: 
        return NS_OK;

    case XML_ERROR_NO_MEMORY: 
        return NS_ERROR_OUT_OF_MEMORY;

    default:
        return NS_ERROR_FAILURE;
    }
}

static
//brian int StartElementDispatch(void * callbackdata, const XML_Char * name, const XML_Char ** atts)
void StartElementDispatch(void * callbackdata, const XML_Char * name, const XML_Char ** atts)
{
    mozApocSaxHandler * Handler = GetCallbackHandler(callbackdata);
    if (Handler)
    {
        nsDependentString LocalName(name);
        PRInt32 LengthNS = SplitQName(LocalName);
        if (LengthNS >= 0)
        {
            nsAutoString NamespaceURI(name,PRUint32(LengthNS));
            Handler->StartElement( LocalName, &NamespaceURI, mozApocSaxAttributeIterator(atts) );
        }
        else
            Handler->StartElement( LocalName, nsnull, mozApocSaxAttributeIterator(atts) );

        //brian return XML_ERROR_NONE;
        return ;
    }
    else
        //brian return XML_ERROR_NO_MEMORY;
        return ;
}

static
//brian int EndElementDispatch(void * callbackdata, const XML_Char * name)
void EndElementDispatch(void * callbackdata, const XML_Char * name)
{
    mozApocSaxHandler * Handler = GetCallbackHandler(callbackdata);
    if (Handler)
    {
        nsDependentString LocalName(name);
        PRInt32 LengthNS = SplitQName(LocalName);
        if (LengthNS >= 0)
        {
            nsAutoString NamespaceURI(name,PRUint32(LengthNS));
            Handler->EndElement( LocalName, &NamespaceURI );
        }
        else
            Handler->EndElement( LocalName, nsnull );

       //brian  return XML_ERROR_NONE;
       return;
    }
    else
        //brian return XML_ERROR_NO_MEMORY;
        return;
}

static
void CharacterDataDispatch(void * callbackdata, const XML_Char * characters, int len)
{
    mozApocSaxHandler * Handler = GetCallbackHandler(callbackdata);
    if (Handler)
    {
        NS_ASSERTION(len >= 0, "apoc - Expat passing negative character count");

        nsAutoString Data(characters, PRUint32(len));
        Handler->CharacterData( Data );
    }
}

mozApocSaxParser::mozApocSaxParser(PRBool bUseNamespaces, const XML_Char * encoding)
{
    XML_Char tmp[2];
    *tmp = k_NSSep;

    if (bUseNamespaces)
        m_parser = XML_ParserCreate_MM(encoding, NULL,tmp); 
    else
        m_parser = XML_ParserCreate_MM(encoding,NULL,NULL); 

    if (m_parser) 
    {
#if MOZAPOC_INDIRECT_HANDLER
        XML_UseParserAsHandlerArg(m_parser);
#endif
        XML_SetElementHandler(m_parser, StartElementDispatch, EndElementDispatch);
        XML_SetCharacterDataHandler(m_parser, CharacterDataDispatch);
    }
    else
        NS_WARNING("apoc - Creating Expat parser failed");
}

mozApocSaxParser::~mozApocSaxParser()
{
    if (m_parser) XML_ParserFree(m_parser);
}

nsresult mozApocSaxParser::Parse(mozApocSaxHandler & handler, const char * data, PRInt32 len) const
{
    if (!m_parser) return NS_ERROR_OUT_OF_MEMORY;

    XML_SetUserData(m_parser, &handler);

    nsresult rv = NS_OK;
    if (! XML_Parse(m_parser, data, len, PR_TRUE) )
        rv = TranslateSaxError( m_parser );
    
    XML_SetUserData(m_parser,0);

    return rv;
}


