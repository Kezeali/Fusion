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

#include "FusionXML.h"

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>


namespace FusionEngine
{

	SetupXml::SetupXml()
	{
		xercesc::XMLPlatformUtils::Initialize();
	}

	SetupXml::~SetupXml()
	{
		xercesc::XMLPlatformUtils::Terminate();
	}

	DomReaderPtr OpenXml(const std::string filename, CL_VirtualDirectory vdir)
	{
		xercesc::XercesDOMParser* parser = new xercesc::XercesDOMParser();

		xercesc::ErrorHandler* errHandler = (xercesc::ErrorHandler*)new xercesc::HandlerBase();
		parser->setErrorHandler(errHandler);

		CL_IODevice in = vdir.open_file(filename, CL_File::open_existing, CL_File::access_read);
    size_t length(in.size());
    XMLByte* buffer(new XMLByte[length]);

    // if in.read() fails, buffer will be an empty string.
    memset(buffer, 0, length * sizeof(XMLByte));
    in.read(reinterpret_cast<char*>(buffer), length);

		std::tr1::shared_ptr<xercesc::MemBufInputSource> inputSrc(new xercesc::MemBufInputSource(buffer, length, filename.c_str(), true));
		parser->parse(*inputSrc);
	}

	String DomReader::toString(const XMLCh * ch)
    {
        TSharedArray<char> transcoded(XMLString::transcode(ch), XMLChReleaser());
        String result(transcoded.get());

        return result;
    }

    //-----------------------------------------------------------------------------
    Uint32 DomReader::getUint32Attribute(DOMElement* element, const char* attributeName)
    {
        return boost::lexical_cast<Uint32>(getStringAttribute(element, attributeName));
    }

    //-----------------------------------------------------------------------------
    ::Real DomReader::getRealAttribute(DOMElement* element, const char* attributeName)
    {
        return boost::lexical_cast< ::Real>(getStringAttribute(element, attributeName));
    }

    //-----------------------------------------------------------------------------
    String DomReader::getStringAttribute(DOMElement* element, const char* attributeName)
    {
        XMLChPtr temp(attributeName);
        return toString(element->getAttribute(temp.get()));
    }

    //-----------------------------------------------------------------------------
    bool DomReader::getBoolAttribute(DOMElement* element, const char* attributeName)
    {
        String temp = getStringAttribute(element, attributeName);
        return ((temp == "1") || (temp == "true"));
    }

    //==========================================================================
    //  DomReader::ElementIterator class implementation
    //==========================================================================
    //-----------------------------------------------------------------------------
    DomReader::ElementIterator::ElementIterator(XERCES_CPP_NAMESPACE::DOMDocument* doc, const char* tagName) :
       list(doc->getElementsByTagName(XMLChPtr(tagName).get())),
       index(0)
    {
    }

    //-----------------------------------------------------------------------------
    DomReader::ElementIterator::ElementIterator(XERCES_CPP_NAMESPACE::DOMElement* element, const char* tagName) :
       list(element->getElementsByTagName(XMLChPtr(tagName).get())),
       index(0)
    {
    }

    //-----------------------------------------------------------------------------
    DomReader::ElementIterator::ElementIterator(XERCES_CPP_NAMESPACE::DOMNodeList* _list) :
       list(_list),
       index(0)
    {
    }

    //-----------------------------------------------------------------------------
    XERCES_CPP_NAMESPACE::DOMElement* DomReader::ElementIterator::nextNode()
    {
        while (index < list->getLength())
        {
            DOMNode* node = list->item(index);
            ++index;
            if (node->getNodeType() == DOMNode::ELEMENT_NODE)
            {
                return static_cast<DOMElement*>(node);
            }
        }
        // if we get here, we're out of nodes
        return NULL;
    }

}