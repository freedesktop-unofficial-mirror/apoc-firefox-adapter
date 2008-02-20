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


#include "mozApocPathParser.h"

nsCAutoString mozApocPathParser::GetComponent() const
{
    NS_ASSERTION(mPosition == 0, "apoc - Requesting component from path parser while not at start of path");
    nsCAutoString aResult (GetCurrentPart());
    return aResult;
}

PRBool mozApocPathParser::MoveNext()
{
    NS_PRECONDITION(HasData(), "apoc - Calling PathParser::MoveNext, when no components are left");

    PRUint32 oldPosition = NS_STATIC_CAST(PRUint32,mPosition);
    mPosition = mPathString.FindChar(k_mozApocPathDelimiter, oldPosition);

    if (mPosition < 0)
        return PR_FALSE;
    
    PRUint32 newPosition = NS_STATIC_CAST(PRUint32,mPosition);
    if (newPosition >= mPathString.Length())
    {
        mPosition = -1;
        return PR_FALSE;
    }

        ++mPosition; // move after the dot

    return PR_TRUE;
}

//brian nsDependentSingleFragmentCSubstring mozApocPathParser::GetCurrentPart() const
nsDependentCSubstring mozApocPathParser::GetCurrentPart() const
{
    NS_PRECONDITION(HasData(), "apoc - Calling PathParser::GetCurrentPart, when no components are left");

    PRUint32 curPosition = NS_STATIC_CAST(PRUint32,mPosition);

    PRInt32 nextPosition = mPathString.FindChar(k_mozApocPathDelimiter, curPosition);
    if (nextPosition < 0)
        nextPosition = mPathString.Length();

    PRUint32 curLength = NS_STATIC_CAST(PRUint32,nextPosition)-curPosition;

    return Substring(mPathString,curPosition,curLength);
}

//brian nsDependentSingleFragmentCSubstring mozApocPathParser::GetRemainingPath() const
nsDependentCSubstring mozApocPathParser::GetRemainingPath() const
{
    PRUint32 endPosition = mPathString.Length();
    PRUint32 curPosition = this->HasData() ? NS_STATIC_CAST(PRUint32,mPosition) : endPosition;

    return Substring(mPathString,curPosition,endPosition-curPosition);
}

nsCAutoString mozApocBuildPath(const nsACString & aPrefix, const nsACString & aLocalName)
{
    nsCAutoString aResult( aPrefix );
    if (!aPrefix.IsEmpty())
        aResult.Append(k_mozApocPathDelimiter);

    aResult.Append(aLocalName);
    return aResult;
}

