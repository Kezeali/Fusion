#include "PrecompiledHeaders.h"

#include "FusionAssert.h"

#include <string.h> // strstr
#include <boost/lexical_cast.hpp>
#include "scriptstring.h"
using namespace std;

//--------------
// constructors
//--------------

CScriptString::CScriptString()
{
	// Count the first reference
	refCount = 1;
}

CScriptString::CScriptString(const char *s, unsigned int length)
{
	refCount = 1;
	buffer.assign(s, length);
}

CScriptString::CScriptString(const string &s)
{
	refCount = 1;
	buffer = s;
}

CScriptString::CScriptString(const CScriptString &s)
{
	refCount = 1;
	buffer = s.buffer;
}

CScriptString::~CScriptString()
{
	//FSN_ASSERT( refCount == 0 );
}

//--------------------
// reference counting
//--------------------

void CScriptString::AddRef() const
{
	refCount++;
}

static void StringAddRef_Generic(asIScriptGeneric *gen)
{
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	thisPointer->AddRef();
}

void CScriptString::Release() const
{
	if( --refCount == 0 )
		delete this;
}

static void StringRelease_Generic(asIScriptGeneric *gen)
{
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	thisPointer->Release();
}

//-----------------
// string = string
//-----------------

CScriptString &CScriptString::operator=(const CScriptString &other)
{
	// Copy only the buffer, not the reference counter
	buffer = other.buffer;

	// Return a reference to this object
	return *this;
}

static void AssignString_Generic(asIScriptGeneric *gen)
{
	CScriptString *a = (CScriptString*)gen->GetArgAddress(0);
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	*thisPointer = *a;
	gen->SetReturnAddress(thisPointer);
}

//------------------
// string += string
//------------------

CScriptString &CScriptString::operator+=(const CScriptString &other)
{
	buffer += other.buffer;
	return *this;
}

static void AddAssignString_Generic(asIScriptGeneric *gen)
{
	CScriptString *a = (CScriptString*)gen->GetArgAddress(0);
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	*thisPointer += *a;
	gen->SetReturnAddress(thisPointer);
}

//-----------------
// string opCmp string
//-----------------

static int StringCmp(const string &a, const string &b)
{
	int cmp = 0;
	if( a < b ) cmp = -1;
	else if( a > b ) cmp = 1;
	return cmp;
}

static void StringCmp_Generic(asIScriptGeneric * gen) 
{
  string * a = static_cast<string *>(gen->GetObject());
  string * b = static_cast<string *>(gen->GetArgAddress(0));

  int cmp = 0;
  if( *a < *b ) cmp = -1;
  else if( *a > *b ) cmp = 1;

  *(int*)gen->GetAddressOfReturnLocation() = cmp;
}

//-----------------
// string + string
//-----------------

CScriptString *operator+(const CScriptString &a, const CScriptString &b)
{
	// Return a new object as a script handle
	CScriptString *str = new CScriptString();

	// Avoid unnecessary memory copying by first reserving the full memory buffer, then concatenating
	str->buffer.reserve(a.buffer.length() + b.buffer.length());
	str->buffer += a.buffer;
	str->buffer += b.buffer;

	return str;
}

static void ConcatenateStrings_Generic(asIScriptGeneric *gen)
{
	CScriptString *a = (CScriptString*)gen->GetObject();
	CScriptString *b = (CScriptString*)gen->GetArgAddress(0);
	CScriptString *out = *a + *b;
	gen->SetReturnAddress(out);
}

//----------------
// string = value
//----------------

static CScriptString &AssignUIntToString(unsigned int i, CScriptString &dest)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%u", i);
#else
	sprintf(buf, "%u", i);
#endif
	dest.buffer = buf;
	return dest;
}

static void AssignUIntToString_Generic(asIScriptGeneric *gen)
{
	unsigned int i = gen->GetArgDWord(0);
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	AssignUIntToString(i, *thisPointer);
	gen->SetReturnAddress(thisPointer);
}

static CScriptString &AssignIntToString(int i, CScriptString &dest)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%d", i);
#else
	sprintf(buf, "%d", i);
#endif
	dest.buffer = buf;
	return dest;
}

static void AssignIntToString_Generic(asIScriptGeneric *gen)
{
	int i = gen->GetArgDWord(0);
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	AssignIntToString(i, *thisPointer);
	gen->SetReturnAddress(thisPointer);
}

static CScriptString &AssignFloatToString(float f, CScriptString &dest)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%g", f);
#else
	sprintf(buf, "%g", f);
#endif
	dest.buffer = buf;
	return dest;
}

static void AssignFloatToString_Generic(asIScriptGeneric *gen)
{
	float f = gen->GetArgFloat(0);
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	AssignFloatToString(f, *thisPointer);
	gen->SetReturnAddress(thisPointer);
}

static CScriptString &AssignDoubleToString(double f, CScriptString &dest)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%g", f);
#else
	sprintf(buf, "%g", f);
#endif
	dest.buffer = buf;
	return dest;
}

static void AssignDoubleToString_Generic(asIScriptGeneric *gen)
{
	double f = gen->GetArgDouble(0);
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	AssignDoubleToString(f, *thisPointer);
	gen->SetReturnAddress(thisPointer);
}

static CScriptString &AssignBoolToString(bool b, CScriptString &dest)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%s", b ? "true" : "false");
#else
	sprintf(buf, "%s", b ? "true" : "false");
#endif
	dest.buffer = buf;
	return dest;
}

static void AssignBoolToString_Generic(asIScriptGeneric *gen)
{
	bool b = gen->GetArgByte(0) ? true : false;
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	AssignBoolToString(b, *thisPointer);
	gen->SetReturnAddress(thisPointer);
}

//-----------------
// string += value
//-----------------

static CScriptString &AddAssignUIntToString(unsigned int i, CScriptString &dest)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%u", i);
#else
	sprintf(buf, "%u", i);
#endif
	dest.buffer += buf;
	return dest;
}

static void AddAssignUIntToString_Generic(asIScriptGeneric *gen)
{
	unsigned int i = gen->GetArgDWord(0);
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	AddAssignUIntToString(i, *thisPointer);
	gen->SetReturnAddress(thisPointer);
}

static CScriptString &AddAssignIntToString(int i, CScriptString &dest)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%d", i);
#else
	sprintf(buf, "%d", i);
#endif
	dest.buffer += buf;
	return dest;
}

static void AddAssignIntToString_Generic(asIScriptGeneric *gen)
{
	int i = gen->GetArgDWord(0);
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	AddAssignIntToString(i, *thisPointer);
	gen->SetReturnAddress(thisPointer);
}

static CScriptString &AddAssignFloatToString(float f, CScriptString &dest)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%g", f);
#else
	sprintf(buf, "%g", f);
#endif
	dest.buffer += buf;
	return dest;
}

static void AddAssignFloatToString_Generic(asIScriptGeneric *gen)
{
	float f = gen->GetArgFloat(0);
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	AddAssignFloatToString(f, *thisPointer);
	gen->SetReturnAddress(thisPointer);
}

static CScriptString &AddAssignDoubleToString(double f, CScriptString &dest)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%g", f);
#else
	sprintf(buf, "%g", f);
#endif
	dest.buffer += buf;
	return dest;
}

static void AddAssignDoubleToString_Generic(asIScriptGeneric *gen)
{
	double f = gen->GetArgDouble(0);
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	AddAssignDoubleToString(f, *thisPointer);
	gen->SetReturnAddress(thisPointer);
}

static CScriptString &AddAssignBoolToString(bool b, CScriptString &dest)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%s", b ? "true" : "false");
#else
	sprintf(buf, "%s", b ? "true" : "false");
#endif
	dest.buffer += buf;
	return dest;
}

static void AddAssignBoolToString_Generic(asIScriptGeneric *gen)
{
	bool b = gen->GetArgByte(0) ? true : false;
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	AddAssignBoolToString(b, *thisPointer);
	gen->SetReturnAddress(thisPointer);
}

//----------------
// string + value
//----------------

static CScriptString *AddStringUInt(const CScriptString &str, unsigned int i)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%u", i);
#else
	sprintf(buf, "%u", i);
#endif
	return new CScriptString(str.buffer + buf);
}

static void AddStringUInt_Generic(asIScriptGeneric *gen)
{
	CScriptString *str = (CScriptString*)gen->GetObject();
	unsigned int i = gen->GetArgDWord(0);
	CScriptString *out = AddStringUInt(*str, i);
	gen->SetReturnAddress(out);
}

static CScriptString *AddStringInt(const CScriptString &str, int i)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%d", i);
#else
	sprintf(buf, "%d", i);
#endif
	return new CScriptString(str.buffer + buf);
}

static void AddStringInt_Generic(asIScriptGeneric *gen)
{
	CScriptString *str = (CScriptString*)gen->GetObject();
	int i = gen->GetArgDWord(0);
	CScriptString *out = AddStringInt(*str, i);
	gen->SetReturnAddress(out);
}

static CScriptString *AddStringFloat(const CScriptString &str, float f)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%g", f);
#else
	sprintf(buf, "%g", f);
#endif
	return new CScriptString(str.buffer + buf);
}

static void AddStringFloat_Generic(asIScriptGeneric *gen)
{
	CScriptString *str = (CScriptString*)gen->GetObject();
	float f = gen->GetArgFloat(0);
	CScriptString *out = AddStringFloat(*str, f);
	gen->SetReturnAddress(out);
}

static CScriptString *AddStringDouble(const CScriptString &str, double f)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%g", f);
#else
	sprintf(buf, "%g", f);
#endif
	return new CScriptString(str.buffer + buf);
}

static void AddStringDouble_Generic(asIScriptGeneric *gen)
{
	CScriptString *str = (CScriptString*)gen->GetObject();
	double f = gen->GetArgDouble(0);
	CScriptString *out = AddStringDouble(*str, f);
	gen->SetReturnAddress(out);
}

static CScriptString *AddStringBool(const CScriptString &str, bool b)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%s", b ? "true" : "false");
#else
	sprintf(buf, "%s", b ? "true" : "false");
#endif
	return new CScriptString(str.buffer + buf);
}

static void AddStringBool_Generic(asIScriptGeneric *gen)
{
	CScriptString *str = (CScriptString*)gen->GetObject();
	bool b = gen->GetArgByte(0) ? true : false;
	CScriptString *out = AddStringBool(*str, b);
	gen->SetReturnAddress(out);
}

//----------------
// value + string
//----------------

static CScriptString *AddIntString(int i, const CScriptString &str)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%d", i);
#else
	sprintf(buf, "%d", i);
#endif
	return new CScriptString(buf + str.buffer);
}

static void AddIntString_Generic(asIScriptGeneric *gen)
{
	int i = gen->GetArgDWord(0);
	CScriptString *str = (CScriptString*)gen->GetObject();
	CScriptString *out = AddIntString(i, *str);
	gen->SetReturnAddress(out);
}

static CScriptString *AddUIntString(unsigned int i, const CScriptString &str)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%u", i);
#else
	sprintf(buf, "%u", i);
#endif
	return new CScriptString(buf + str.buffer);
}

static void AddUIntString_Generic(asIScriptGeneric *gen)
{
	unsigned int i = gen->GetArgDWord(0);
	CScriptString *str = (CScriptString*)gen->GetObject();
	CScriptString *out = AddUIntString(i, *str);
	gen->SetReturnAddress(out);
}

static CScriptString *AddFloatString(float f, const CScriptString &str)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%g", f);
#else
	sprintf(buf, "%g", f);
#endif
	return new CScriptString(buf + str.buffer);
}

static void AddFloatString_Generic(asIScriptGeneric *gen)
{
	float f = gen->GetArgFloat(0);
	CScriptString *str = (CScriptString*)gen->GetObject();
	CScriptString *out = AddFloatString(f, *str);
	gen->SetReturnAddress(out);
}

static CScriptString *AddDoubleString(double f, const CScriptString &str)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%g", f);
#else
	sprintf(buf, "%g", f);
#endif
	return new CScriptString(buf + str.buffer);
}

static void AddDoubleString_Generic(asIScriptGeneric *gen)
{
	double f = gen->GetArgDouble(0);
	CScriptString *str = (CScriptString*)gen->GetObject();
	CScriptString *out = AddDoubleString(f, *str);
	gen->SetReturnAddress(out);
}

static CScriptString *AddBoolString(bool b, const CScriptString &str)
{
	char buf[100];
#if _MSC_VER >= 1400 // MSVC 8.0 / 2005
	sprintf_s(buf, 100, "%s", b ? "true" : "false");
#else
	sprintf(buf, "%s", b ? "true" : "false");
#endif
	return new CScriptString(buf + str.buffer);
}

static void AddBoolString_Generic(asIScriptGeneric *gen)
{
	bool b = gen->GetArgByte(0) ? true : false;
	CScriptString *str = (CScriptString*)gen->GetObject();
	CScriptString *out = AddBoolString(b, *str);
	gen->SetReturnAddress(out);
}

//----------
// string[]
//----------

static char *StringCharAt(unsigned int i, CScriptString &str)
{
	if( i >= str.buffer.size() )
	{
		// Set a script exception
		asIScriptContext *ctx = asGetActiveContext();
		ctx->SetException("Out of range");

		// Return a null pointer
		return 0;
	}

	return &str.buffer[i];
}

static void StringCharAt_Generic(asIScriptGeneric *gen)
{
	unsigned int i = gen->GetArgDWord(0);
	CScriptString *str = (CScriptString*)gen->GetObject();
	char *ch = StringCharAt(i, *str);
	gen->SetReturnAddress(ch);
}

//------------
// conversion
//------------
template <typename T>
T ParseValueOrDefault(const std::string &str, const T &default_value)
{
	try
	{
		return boost::lexical_cast<T>(str);
	}
	catch (boost::bad_lexical_cast&)
	{
		return default_value;
	}

	return default_value;
}

template <typename T>
T StringParseValue(CScriptString &str)
{
	try
	{
		return boost::lexical_cast<T>(str.buffer);
	}
	catch (boost::bad_lexical_cast &ex)
	{
		asIScriptContext *ctx = asGetActiveContext();
		if (ctx != NULL)
		{
			std::string message =
				"Failed to parse " + std::string(ex.target_type().name()) + " value from string."
				" Consider using one of the exception-safe conversion functions,"
				" e.g. string::parseInt instead of string::toInt."
				" The string's current value is: " + str.buffer;
			ctx->SetException(message.c_str());
		}
	}

	return T();
}

template <typename T>
bool StringParseValue_Checked(T& result, CScriptString &str)
{
	try
	{
		result = boost::lexical_cast<T>(str.buffer);
	}
	catch (boost::bad_lexical_cast &)
	{
		return false;
	}

	return true;
}

bool CScriptString::ToBool(bool default_value) const
{
	return ParseValueOrDefault<bool>(buffer, default_value);
}

int CScriptString::ToInt(int default_value) const
{
	return ParseValueOrDefault<int>(buffer, default_value);
}

unsigned int CScriptString::ToUInt(unsigned int default_value) const
{
	return ParseValueOrDefault<unsigned int>(buffer, default_value);
}

float CScriptString::ToFloat(float default_value) const
{
	return ParseValueOrDefault<float>(buffer, default_value);
}

double CScriptString::ToDouble(double default_value) const
{
	return ParseValueOrDefault<double>(buffer, default_value);
}

//-----------------------
// AngelScript functions
//-----------------------

// This is the string factory that creates new strings for the script based on string literals
static CScriptString *StringFactory(asUINT length, const char *s)
{
	return new CScriptString(s, length);
}

static void StringFactory_Generic(asIScriptGeneric *gen)
{
	asUINT length = gen->GetArgDWord(0);
	const char *s = (const char*)gen->GetArgAddress(1);
	CScriptString *str = StringFactory(length, s);
	gen->SetReturnAddress(str);
}

// This is the default string factory, that is responsible for creating empty string objects, e.g. when a variable is declared
static CScriptString *StringDefaultFactory()
{
	// Allocate and initialize with the default constructor
	return new CScriptString();
}

static CScriptString *StringCopyFactory(const CScriptString &other)
{
	// Allocate and initialize with the copy constructor
	return new CScriptString(other);
}

static void StringDefaultFactory_Generic(asIScriptGeneric *gen)
{
	*(CScriptString**)gen->GetAddressOfReturnLocation() = StringDefaultFactory();
}

static void StringCopyFactory_Generic(asIScriptGeneric *gen)
{
	CScriptString *other = (CScriptString *)gen->GetArgObject(0);
	*(CScriptString**)gen->GetAddressOfReturnLocation() = StringCopyFactory(*other);
}

static void StringEqual_Generic(asIScriptGeneric *gen)
{
	string *a = (string*)gen->GetArgAddress(0);
	string *b = (string*)gen->GetArgAddress(1);
	bool r = *a == *b;
    *(bool*)gen->GetAddressOfReturnLocation() = r;
}

static void StringEquals_Generic(asIScriptGeneric * gen) 
{
	string * a = static_cast<string *>(gen->GetObject());
	string * b = static_cast<string *>(gen->GetArgAddress(0));
	*(bool*)gen->GetAddressOfReturnLocation() = (*a == *b);
}

static void StringNotEqual_Generic(asIScriptGeneric *gen)
{
	string *a = (string*)gen->GetArgAddress(0);
	string *b = (string*)gen->GetArgAddress(1);
	bool r = *a != *b;
    *(bool*)gen->GetAddressOfReturnLocation() = r;
}

static void StringLesserOrEqual_Generic(asIScriptGeneric *gen)
{
	string *a = (string*)gen->GetArgAddress(0);
	string *b = (string*)gen->GetArgAddress(1);
	bool r = *a <= *b;
    *(bool*)gen->GetAddressOfReturnLocation() = r;
}

static void StringGreaterOrEqual_Generic(asIScriptGeneric *gen)
{
	string *a = (string*)gen->GetArgAddress(0);
	string *b = (string*)gen->GetArgAddress(1);
	bool r = *a >= *b;
    *(bool*)gen->GetAddressOfReturnLocation() = r;
}

static void StringLesser_Generic(asIScriptGeneric *gen)
{
	string *a = (string*)gen->GetArgAddress(0);
	string *b = (string*)gen->GetArgAddress(1);
	bool r = *a < *b;
    *(bool*)gen->GetAddressOfReturnLocation() = r;
}

static void StringGreater_Generic(asIScriptGeneric *gen)
{
	string *a = (string*)gen->GetArgAddress(0);
	string *b = (string*)gen->GetArgAddress(1);
	bool r = *a > *b;
    *(bool*)gen->GetAddressOfReturnLocation() = r;
}

static void StringLength_Generic(asIScriptGeneric *gen)
{
	string *s = (string*)gen->GetObject();
	size_t l = s->size();
	if( sizeof(size_t) == 4 )
		gen->SetReturnDWord((asUINT)l);
	else
		gen->SetReturnQWord((asQWORD)l);
}

static void StringResize_Generic(asIScriptGeneric *gen)
{
	string *s = (string*)gen->GetObject();
	size_t v = *(size_t*)gen->GetAddressOfArg(0);
	s->resize(v);
}

std::string &string_erasefrom(std::string::size_type offset, std::string *obj)
{
	return obj->erase(offset);
}

// This is where we register the string type
int RegisterScriptString_Native(asIScriptEngine *engine)
{
	int r, typeId;

	// Register the type
	r = engine->RegisterObjectType("string", 0, asOBJ_REF); FSN_ASSERT( r >= 0 );
	typeId = engine->GetTypeIdByDecl("string");

	// Register the object operator overloads
	// Note: We don't have to register the destructor, since the object uses reference counting
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_FACTORY,    "string @f()",                 asFUNCTION(StringDefaultFactory), asCALL_CDECL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_FACTORY,    "string @f(const string &in)", asFUNCTION(StringCopyFactory), asCALL_CDECL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADDREF,     "void f()",                    asMETHOD(CScriptString,AddRef), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_RELEASE,    "void f()",                    asMETHOD(CScriptString,Release), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAssign(const string &in)", asMETHODPR(CScriptString, operator =, (const CScriptString&), CScriptString&), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(const string &in)", asMETHODPR(CScriptString, operator+=, (const CScriptString&), CScriptString&), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

	// Register the factory to return a handle to a new string
	// Note: We must register the string factory after the basic behaviours,
	// otherwise the library will not allow the use of object handles for this type
	r = engine->RegisterStringFactory("string@", asFUNCTION(StringFactory), asCALL_CDECL); FSN_ASSERT( r >= 0 );

	r = engine->RegisterObjectMethod("string", "bool opEquals(const string &in)", asFUNCTIONPR(operator==, (const string &, const string &), bool), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int opCmp(const string &in)", asMETHODPR(std::string, compare, (const std::string&) const, int), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string@ opAdd(const string &in)", asFUNCTIONPR(operator +, (const CScriptString &, const CScriptString &), CScriptString*), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );

	// Register the index operator, both as a mutator and as an inspector
	r = engine->RegisterObjectMethod("string", "uint8 &opIndex(uint)", asFUNCTION(StringCharAt), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "const uint8 &opIndex(uint) const", asFUNCTION(StringCharAt), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

	// Register the object methods
	r = engine->RegisterObjectMethod("string", "uint length() const", asMETHOD(string,size), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool empty() const", asMETHOD(string,empty), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "void resize(uint)", asMETHODPR(string,resize,(size_t),void), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "void erase(uint)", asFUNCTION(string_erasefrom), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "void erase(uint, uint)", asMETHODPR(string, erase, (string::size_type, string::size_type),string&), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "void clear()", asMETHOD(string, clear), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

	// TODO: Add factory  string(const string &in str, int repeatCount)

	// TODO: Add explicit type conversion via constructor and value cast

	r = engine->RegisterObjectMethod("string", "bool toBool() const", asFUNCTION(StringParseValue<bool>), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int toInt() const", asFUNCTION(StringParseValue<int>), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "uint toUInt() const", asFUNCTION(StringParseValue<unsigned int>), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "float toFloat() const", asFUNCTION(StringParseValue<float>), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "double toDouble() const", asFUNCTION(StringParseValue<double>), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

	r = engine->RegisterObjectMethod("string", "bool toBool(bool) const", asMETHODPR(CScriptString,ToBool,(bool) const,bool), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int toInt(int) const", asMETHODPR(CScriptString,ToInt,(int) const,int), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "uint toUInt(uint) const", asMETHODPR(CScriptString,ToUInt,(unsigned int) const,unsigned int), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "float toFloat(float) const", asMETHODPR(CScriptString,ToFloat,(float) const,float), asCALL_THISCALL); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "double toDouble(double) const", asMETHODPR(CScriptString,ToDouble,(double) const,double), asCALL_THISCALL); FSN_ASSERT( r >= 0 );

	r = engine->RegisterObjectMethod("string", "bool parseBool(bool&out) const", asFUNCTIONPR(StringParseValue_Checked, (bool&, CScriptString&), bool), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool parseInt(int&out) const", asFUNCTIONPR(StringParseValue_Checked, (int&, CScriptString&), bool), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool parseUInt(uint&out) const", asFUNCTIONPR(StringParseValue_Checked, (unsigned int&, CScriptString&), bool), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool parseFloat(float&out) const", asFUNCTIONPR(StringParseValue_Checked, (float&, CScriptString&), bool), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool parseDouble(double&out) const", asFUNCTIONPR(StringParseValue_Checked, (double&, CScriptString&), bool), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

	// Automatic conversion from values
	r = engine->RegisterObjectMethod("string", "string &opAssign(double)", asFUNCTION(AssignDoubleToString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(double)", asFUNCTION(AddAssignDoubleToString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string@ opAdd(double) const", asFUNCTION(AddStringDouble), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string@ opAdd_r(double) const", asFUNCTION(AddDoubleString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(float)", asFUNCTION(AssignFloatToString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(float)", asFUNCTION(AddAssignFloatToString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string@ opAdd(float) const", asFUNCTION(AddStringFloat), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string@ opAdd_r(float) const", asFUNCTION(AddFloatString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(int)", asFUNCTION(AssignIntToString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(int)", asFUNCTION(AddAssignIntToString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string@ opAdd(int) const", asFUNCTION(AddStringInt), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string@ opAdd_r(int) const", asFUNCTION(AddIntString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(uint)", asFUNCTION(AssignUIntToString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(uint)", asFUNCTION(AddAssignUIntToString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string@ opAdd(uint) const", asFUNCTION(AddStringUInt), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string@ opAdd_r(uint) const", asFUNCTION(AddUIntString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(bool)", asFUNCTION(AssignBoolToString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(bool)", asFUNCTION(AddAssignBoolToString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string@ opAdd(bool) const", asFUNCTION(AddStringBool), asCALL_CDECL_OBJFIRST); FSN_ASSERT( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string@ opAdd_r(bool) const", asFUNCTION(AddBoolString), asCALL_CDECL_OBJLAST); FSN_ASSERT( r >= 0 );

	return typeId;
}

int RegisterScriptString(asIScriptEngine *engine)
{
	if( strcmp(asGetLibraryOptions(), "AS_MAX_PORTABILITY") != 0 )
		return RegisterScriptString_Native(engine);
	return -1;
}

