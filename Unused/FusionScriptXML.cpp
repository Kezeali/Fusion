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

#include "FusionScriptXML.h"

#include "FusionXml.h"
#include "FusionScriptTypeRegistrationUtils.h"

#include "FusionRefCounted.h"

namespace FusionEngine
{
	namespace Scripting
	{

		/*class XmlNode : public RefCounted
		{
		public:
			XmlNode();
			XmlNode(const TiXmlNode &copy);
			virtual ~XmlNode();

		private:
			TiXmlNode* m_Node;
		};*/

		class XmlDocument : public RefCounted
		{
		public:
			XmlDocument();
			XmlDocument(TiXmlDocument *document);
			virtual ~XmlDocument();

			void Open(const std::string &filename);
			void Save(const std::string &filename);

			TiXmlDocument *m_Document;
		};

		//class XmlElement : public XmlNode, public RefCounted
		//{
		//public:
		//	XmlElement();
		//	XmlElement(const TiXmlElement &copy);
		//	virtual ~XmlElement();
		//};

		//XmlNode::XmlNode()
		//	: m_Node(NULL)
		//{
		//}

		//XmlNode::XmlNode(TiXmlNode *node)
		//	: m_Node(node)
		//{
		//}

		XmlDocument::XmlDocument()
		{
		}

		XmlDocument::XmlDocument(TiXmlDocument *document)
			: RefCounted(),
			m_Document(document)
		{
		}

		XmlDocument::~XmlDocument()
		{
			delete m_Document;
		}

		//XmlElement::XmlElement()
		//{
		//}

		//XmlElement::XmlElement(const TiXmlElement &copy)
		//	: TiXmlElement(copy)
		//{
		//}

		//! Opens a ref-counted XmlDocument
		XmlDocument* OpenXmlDocument(const std::string &filename)
		{
			XmlDocument *document = NULL;
			try
			{
				document = new XmlDocument( FusionEngine::OpenXml_PhysFS(filename) );
			}
			catch (FusionEngine::FileSystemException &ex)
			{
				SendToConsole(ex.ToString());
				document = NULL;
			}

			return document;
		}

		XmlDocument* XmlDocument_Factory()
		{
			return new XmlDocument(new TiXmlDocument());
		}

		XmlDocument* XmlDocument_Factory_Open(const std::string &filename)
		{
			return OpenXmlDocument(filename);
		}

		//template <class T>
		//void registerTiXmlRefCountedType(asIScriptEngine *engine, const char *name)
		//{
		//	int r;
		//	r = engine->RegisterObjectType(name, 0, asOBJ_REF); FSN_ASSERT(r >= 0);
		//}

		void registerTiXmlNodeMembers(asIScriptEngine *engine, const char *name)
		{
		}

		void registerTiXmlTypes(asIScriptEngine *engine)
		{
			//RefCounted::RegisterType<XmlNode>(engine, "XmlNode");
			RefCounted::RegisterType<XmlDocument>(engine, "XmlDocument");
			//RefCounted::RegisterType<XmlElement>(engine, "XmlElement");
		}

		void RegisterXmlIo(asIScriptEngine *engine)
		{
			registerTiXmlTypes(engine);
		}

	}
}
