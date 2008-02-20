

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


#ifndef MOZAPOC_SAXHANDLER_H_INCLUDED
#define MOZAPOC_SAXHANDLER_H_INCLUDED

//brian #include "nsAFlatString.h"
#include "nsAString.h"

typedef PRUnichar       XML_Char;
typedef nsAFlatString   XML_String;

class mozApocSaxAttributeIterator
{
    XML_Char const * const * m_data;
public:
    explicit
    mozApocSaxAttributeIterator(XML_Char const * const * data)
    : m_data(data)
    {}

    PRBool Is() const { return m_data && *m_data; }

    void Next();
    
    PRBool IsNamed( XML_String const & LocalName,
                                    XML_String const * NamespaceURI = 0,
                                    PRBool IsNamespaceRequired = PR_FALSE) const;
    
    XML_Char const * GetLocalName() const;
    XML_Char const * GetAttributeValue() const;
        
    XML_Char const * FindAttributeValue(XML_String const & LocalName, 
                                        XML_String const * NamespaceURI = 0,
                                        PRBool IsNamespaceRequired = PR_FALSE) const;
};

struct mozApocSaxHandler
{
    virtual void StartElement(XML_String const & Name, XML_String const * NamespaceURI, mozApocSaxAttributeIterator Attributes) = 0;
    virtual void EndElement(XML_String const & Name, XML_String const * NamespaceURI) = 0;
    virtual void CharacterData(XML_String const & Characters) = 0;
};
    
#endif

