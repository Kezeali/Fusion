/*
  Copyright (c) 2009 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.


	File Author(s):

		Elliot Hayward
*/

#include "FusionCommon.h"

#include <xercesc/dom/DOM.hpp>
#include <xqilla/xqilla-simple.hpp>

#ifndef Header_FusionEngine_Xml
#define Header_FusionEngine_Xml

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{
	typedef AutoRelease<xercesc::DOMDocument> DocHandle;
	typedef xercesc::DOMDocument DomDocument;
	typedef std::tr1::shared_ptr<DomReader> DomReaderPtr;

	class SetupXml
	{
	public:
		//! Constructor
		SetupXml();
		//! Destructor
		~SetupXml();
	};

	class DomReader
	{    
    public:
        /*! \brief Default constructor.
        */
        DomReader();

				/*! \brief Parsing constructor
				*/
				DomReader(const std::string& filename, CL_VirtualDirectory vdir);
    
        /*! \brief Default destructor.
        */
        ~DomReader();

				DomDocument* Parse(const std::string& filename);

        void ReleaseDocument();

				/*! \brief Returns the XMLChar as a string
				*/
				static CL_String toString(const XMLCh * Ch);

        /** @brief: Returns the value of the specified attribute
         */
        static Uint32 getUint32Attribute(XERCES_CPP_NAMESPACE::DOMElement* element, const char* attributeName);

        /** @brief: Returns the value of the specified attribute
         */
         static Real getRealAttribute(XERCES_CPP_NAMESPACE::DOMElement* element, const char* attributeName);

        /** @brief: Returns the value of the specified attribute
         */
         static String getStringAttribute(XERCES_CPP_NAMESPACE::DOMElement* element, const char* attributeName);

         /** @brief: Returns the value of the specified attribute
         */
         static bool getBoolAttribute(XERCES_CPP_NAMESPACE::DOMElement* element, const char* attributeName);

         /** @brief: walks the child subelements (with name tagName) of specified object
         */
         class XENOCIDE_BASE_API ElementIterator
         {
         public:
             ElementIterator(XERCES_CPP_NAMESPACE::DOMDocument* doc,     const char* tagName);
             ElementIterator(XERCES_CPP_NAMESPACE::DOMElement*  element, const char* tagName);
             ElementIterator(XERCES_CPP_NAMESPACE::DOMNodeList* list);

             XERCES_CPP_NAMESPACE::DOMElement* nextNode();
         private:
             XERCES_CPP_NAMESPACE::DOMNodeList* list;
             Uint32                             index;
         };
	};

}

#endif