#include <assert.h>
#include <string.h> // strstr
#include <boost/lexical_cast.hpp>
#include "scriptstring.h"
using namespace std;

BEGIN_AS_NAMESPACE

//--------------
// constructors
//--------------

CScriptString::CScriptString()
{
	// Count the first reference
	refCount = 1;
}

CScriptString::CScriptString(const char *s)
{
	refCount = 1;
	buffer = s;
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
	assert( refCount == 0 );
}

//--------------------
// reference counting
//--------------------

void CScriptString::AddRef()
{
	refCount++;
}

static void StringAddRef_Generic(asIScriptGeneric *gen)
{
	CScriptString *thisPointer = (CScriptString*)gen->GetObject();
	thisPointer->AddRef();
}

void CScriptString::Release()
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
// string + string
//-----------------

CScriptString *operator+(const CScriptString &a, const CScriptString &b)
{
	// Return a new object as a script handle
	return new CScriptString(a.buffer + b.buffer);
}

static void ConcatenateStrings_Generic(asIScriptGeneric *gen)
{
	CScriptString *a = (CScriptString*)gen->GetArgAddress(0);
	CScriptString *b = (CScriptString*)gen->GetArgAddress(1);
	CScriptString *out = *a + *b;
	gen->SetReturnAddress(out);
}

//----------------
// string = value
//----------------

static CScriptString &AssignUIntToString(unsigned int i, CScriptString &dest)
{
	char buf[100];
	sprintf(buf, "%u", i);
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
	sprintf(buf, "%d", i);
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
	sprintf(buf, "%g", f);
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
	sprintf(buf, "%g", f);
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

//-----------------
// string += value
//-----------------

static CScriptString &AddAssignUIntToString(unsigned int i, CScriptString &dest)
{
	char buf[100];
	sprintf(buf, "%u", i);
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
	sprintf(buf, "%d", i);
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
	sprintf(buf, "%g", f);
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
	sprintf(buf, "%g", f);
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

//----------------
// string + value
//----------------

static CScriptString *AddStringUInt(const CScriptString &str, unsigned int i)
{
	char buf[100];
	sprintf(buf, "%u", i);
	return new CScriptString(str.buffer + buf);
}

static void AddStringUInt_Generic(asIScriptGeneric *gen)
{
	CScriptString *str = (CScriptString*)gen->GetArgAddress(0);
	unsigned int i = gen->GetArgDWord(1);
	CScriptString *out = AddStringUInt(*str, i);
	gen->SetReturnAddress(out);
}

static CScriptString *AddStringInt(const CScriptString &str, int i)
{
	char buf[100];
	sprintf(buf, "%d", i);
	return new CScriptString(str.buffer + buf);
}

static void AddStringInt_Generic(asIScriptGeneric *gen)
{
	CScriptString *str = (CScriptString*)gen->GetArgAddress(0);
	int i = gen->GetArgDWord(1);
	CScriptString *out = AddStringInt(*str, i);
	gen->SetReturnAddress(out);
}

static CScriptString *AddStringFloat(const CScriptString &str, float f)
{
	char buf[100];
	sprintf(buf, "%g", f);
	return new CScriptString(str.buffer + buf);
}

static void AddStringFloat_Generic(asIScriptGeneric *gen)
{
	CScriptString *str = (CScriptString*)gen->GetArgAddress(0);
	float f = gen->GetArgFloat(1);
	CScriptString *out = AddStringFloat(*str, f);
	gen->SetReturnAddress(out);
}

static CScriptString *AddStringDouble(const CScriptString &str, double f)
{
	char buf[100];
	sprintf(buf, "%g", f);
	return new CScriptString(str.buffer + buf);
}

static void AddStringDouble_Generic(asIScriptGeneric *gen)
{
	CScriptString *str = (CScriptString*)gen->GetArgAddress(0);
	double f = gen->GetArgDouble(1);
	CScriptString *out = AddStringDouble(*str, f);
	gen->SetReturnAddress(out);
}

//----------------
// value + string
//----------------

static CScriptString *AddIntString(int i, const CScriptString &str)
{
	char buf[100];
	sprintf(buf, "%d", i);
	return new CScriptString(buf + str.buffer);
}

static void AddIntString_Generic(asIScriptGeneric *gen)
{
	int i = gen->GetArgDWord(0);
	CScriptString *str = (CScriptString*)gen->GetArgAddress(1);
	CScriptString *out = AddIntString(i, *str);
	gen->SetReturnAddress(out);
}

static CScriptString *AddUIntString(unsigned int i, const CScriptString &str)
{
	char buf[100];
	sprintf(buf, "%u", i);
	return new CScriptString(buf + str.buffer);
}

static void AddUIntString_Generic(asIScriptGeneric *gen)
{
	unsigned int i = gen->GetArgDWord(0);
	CScriptString *str = (CScriptString*)gen->GetArgAddress(1);
	CScriptString *out = AddUIntString(i, *str);
	gen->SetReturnAddress(out);
}

static CScriptString *AddFloatString(float f, const CScriptString &str)
{
	char buf[100];
	sprintf(buf, "%g", f);
	return new CScriptString(buf + str.buffer);
}

static void AddFloatString_Generic(asIScriptGeneric *gen)
{
	float f = gen->GetArgFloat(0);
	CScriptString *str = (CScriptString*)gen->GetArgAddress(1);
	CScriptString *out = AddFloatString(f, *str);
	gen->SetReturnAddress(out);
}

static CScriptString *AddDoubleString(double f, const CScriptString &str)
{
	char buf[100];
	sprintf(buf, "%g", f);
	return new CScriptString(buf + str.buffer);
}

static void AddDoubleString_Generic(asIScriptGeneric *gen)
{
	double f = gen->GetArgDouble(0);
	CScriptString *str = (CScriptString*)gen->GetArgAddress(1);
	CScriptString *out = AddDoubleString(f, *str);
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
static CScriptString *StringFactory(asUINT /*length*/, const char *s)
{
	return new CScriptString(s);
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
	gen->SetReturnDWord((asUINT)l);
}

static void StringResize_Generic(asIScriptGeneric *gen)
{
	string *s = (string*)gen->GetObject();
	size_t v = *(size_t*)gen->GetAddressOfArg(0);
	s->resize(v);
}

// This is where we register the string type
int RegisterScriptString_Native(asIScriptEngine *engine)
{
	int r, typeId;

	// Register the type
	typeId = engine->RegisterObjectType("string", sizeof(CScriptString), asOBJ_REF); assert( typeId >= 0 );

	// Register the object operator overloads
	// Note: We don't have to register the destructor, since the object uses reference counting
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_FACTORY,    "string @f()",                 asFUNCTION(StringDefaultFactory), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_FACTORY,    "string @f(const string &in)", asFUNCTION(StringCopyFactory), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADDREF,     "void f()",                    asMETHOD(CScriptString,AddRef), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_RELEASE,    "void f()",                    asMETHOD(CScriptString,Release), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(const string &in)", asMETHODPR(CScriptString, operator =, (const CScriptString&), CScriptString&), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(const string &in)", asMETHODPR(CScriptString, operator+=, (const CScriptString&), CScriptString&), asCALL_THISCALL); assert( r >= 0 );

	// Register the factory to return a handle to a new string
	// Note: We must register the string factory after the basic behaviours,
	// otherwise the library will not allow the use of object handles for this type
	r = engine->RegisterStringFactory("string@", asFUNCTION(StringFactory), asCALL_CDECL); assert( r >= 0 );

	// Register the global operator overloads
	// Note: We can use std::string's methods directly because the
	// internal std::string is placed at the beginning of the class
	r = engine->RegisterGlobalBehaviour(asBEHAVE_EQUAL,       "bool f(const string &in, const string &in)",    asFUNCTIONPR(operator==, (const string &, const string &), bool), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_NOTEQUAL,    "bool f(const string &in, const string &in)",    asFUNCTIONPR(operator!=, (const string &, const string &), bool), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_LEQUAL,      "bool f(const string &in, const string &in)",    asFUNCTIONPR(operator<=, (const string &, const string &), bool), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_GEQUAL,      "bool f(const string &in, const string &in)",    asFUNCTIONPR(operator>=, (const string &, const string &), bool), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_LESSTHAN,    "bool f(const string &in, const string &in)",    asFUNCTIONPR(operator <, (const string &, const string &), bool), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_GREATERTHAN, "bool f(const string &in, const string &in)",    asFUNCTIONPR(operator >, (const string &, const string &), bool), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(const string &in, const string &in)", asFUNCTIONPR(operator +, (const CScriptString &, const CScriptString &), CScriptString*), asCALL_CDECL); assert( r >= 0 );

	// Register the index operator, both as a mutator and as an inspector
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_INDEX, "uint8 &f(uint)", asFUNCTION(StringCharAt), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_INDEX, "const uint8 &f(uint) const", asFUNCTION(StringCharAt), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Register the object methods
	if( sizeof(size_t) == 4 )
	{
		r = engine->RegisterObjectMethod("string", "uint length() const", asMETHOD(string,size), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("string", "void resize(uint)", asMETHODPR(string,resize,(size_t),void), asCALL_THISCALL); assert( r >= 0 );
	}
	else
	{
		r = engine->RegisterObjectMethod("string", "uint64 length() const", asMETHOD(string,size), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("string", "void resize(uint64)", asMETHODPR(string,resize,(size_t),void), asCALL_THISCALL); assert( r >= 0 );
	}

    // TODO: Add factory  string(const string &in str, int repeatCount)

	// TODO: Add explicit type conversion via constructor and value cast

	r = engine->RegisterObjectMethod("string", "bool toBool() const", asFUNCTION(StringParseValue<bool>), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int toInt() const", asFUNCTION(StringParseValue<int>), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "uint toUInt() const", asFUNCTION(StringParseValue<unsigned int>), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "float toFloat() const", asFUNCTION(StringParseValue<float>), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "double toDouble() const", asFUNCTION(StringParseValue<double>), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "bool toBool(bool) const", asMETHODPR(CScriptString,ToBool,(bool) const,bool), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int toInt(int) const", asMETHODPR(CScriptString,ToInt,(int) const,int), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "uint toUInt(uint) const", asMETHODPR(CScriptString,ToUInt,(unsigned int) const,unsigned int), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "float toFloat(float) const", asMETHODPR(CScriptString,ToFloat,(float) const,float), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "double toDouble(double) const", asMETHODPR(CScriptString,ToDouble,(double) const,double), asCALL_THISCALL); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "bool parseBool(bool&out) const", asFUNCTIONPR(StringParseValue_Checked, (bool&, CScriptString&), bool), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool parseInt(int&out) const", asFUNCTIONPR(StringParseValue_Checked, (int&, CScriptString&), bool), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool parseUInt(uint&out) const", asFUNCTIONPR(StringParseValue_Checked, (unsigned int&, CScriptString&), bool), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool parseFloat(float&out) const", asFUNCTIONPR(StringParseValue_Checked, (float&, CScriptString&), bool), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool parseDouble(double&out) const", asFUNCTIONPR(StringParseValue_Checked, (double&, CScriptString&), bool), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Automatic conversion from values
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(double)", asFUNCTION(AssignDoubleToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(double)", asFUNCTION(AddAssignDoubleToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(const string &in, double)", asFUNCTION(AddStringDouble), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(double, const string &in)", asFUNCTION(AddDoubleString), asCALL_CDECL); assert( r >= 0 );

	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(float)", asFUNCTION(AssignFloatToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(float)", asFUNCTION(AddAssignFloatToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(const string &in, float)", asFUNCTION(AddStringFloat), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(float, const string &in)", asFUNCTION(AddFloatString), asCALL_CDECL); assert( r >= 0 );

	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(int)", asFUNCTION(AssignIntToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(int)", asFUNCTION(AddAssignIntToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(const string &in, int)", asFUNCTION(AddStringInt), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(int, const string &in)", asFUNCTION(AddIntString), asCALL_CDECL); assert( r >= 0 );

	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(uint)", asFUNCTION(AssignUIntToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(uint)", asFUNCTION(AddAssignUIntToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(const string &in, uint)", asFUNCTION(AddStringUInt), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(uint, const string &in)", asFUNCTION(AddUIntString), asCALL_CDECL); assert( r >= 0 );

	return typeId;
}

int RegisterScriptString_Generic(asIScriptEngine *engine)
{
	int r, typeId;

	// Register the type
	typeId = engine->RegisterObjectType("string", sizeof(CScriptString), asOBJ_REF); assert( typeId >= 0 );

	// Register the object operator overloads
	// Note: We don't have to register the destructor, since the object uses reference counting
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_FACTORY,    "string @f()",                 asFUNCTION(StringDefaultFactory_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_FACTORY,    "string @f(const string &in)", asFUNCTION(StringCopyFactory_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADDREF,     "void f()",                    asFUNCTION(StringAddRef_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_RELEASE,    "void f()",                    asFUNCTION(StringRelease_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(const string &in)", asFUNCTION(AssignString_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(const string &in)", asFUNCTION(AddAssignString_Generic), asCALL_GENERIC); assert( r >= 0 );

	// Register the factory to return a handle to a new string
	// Note: We must register the string factory after the basic behaviours,
	// otherwise the library will not allow the use of object handles for this type
	r = engine->RegisterStringFactory("string@", asFUNCTION(StringFactory_Generic), asCALL_GENERIC); assert( r >= 0 );

	// Register the global operator overloads
	// Note: We can use std::string's methods directly because the
	// internal std::string is placed at the beginning of the class
	r = engine->RegisterGlobalBehaviour(asBEHAVE_EQUAL,       "bool f(const string &in, const string &in)",    asFUNCTION(StringEqual_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_NOTEQUAL,    "bool f(const string &in, const string &in)",    asFUNCTION(StringNotEqual_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_LEQUAL,      "bool f(const string &in, const string &in)",    asFUNCTION(StringLesserOrEqual_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_GEQUAL,      "bool f(const string &in, const string &in)",    asFUNCTION(StringGreaterOrEqual_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_LESSTHAN,    "bool f(const string &in, const string &in)",    asFUNCTION(StringLesser_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_GREATERTHAN, "bool f(const string &in, const string &in)",    asFUNCTION(StringGreater_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(const string &in, const string &in)", asFUNCTION(ConcatenateStrings_Generic), asCALL_GENERIC); assert( r >= 0 );

	// Register the index operator, both as a mutator and as an inspector
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_INDEX, "uint8 &f(uint)", asFUNCTION(StringCharAt_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_INDEX, "const uint8 &f(uint) const", asFUNCTION(StringCharAt_Generic), asCALL_GENERIC); assert( r >= 0 );

	// Register the object methods
	if( sizeof(size_t) == 4 )
	{
		r = engine->RegisterObjectMethod("string", "uint length() const", asFUNCTION(StringLength_Generic), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("string", "void resize(uint)", asFUNCTION(StringResize_Generic), asCALL_GENERIC); assert( r >= 0 );
	}
	else
	{
		r = engine->RegisterObjectMethod("string", "uint64 length() const", asFUNCTION(StringLength_Generic), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("string", "void resize(uint64)", asFUNCTION(StringResize_Generic), asCALL_GENERIC); assert( r >= 0 );
	}

	// Automatic conversion from values
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(double)", asFUNCTION(AssignDoubleToString_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(double)", asFUNCTION(AddAssignDoubleToString_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(const string &in, double)", asFUNCTION(AddStringDouble_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(double, const string &in)", asFUNCTION(AddDoubleString_Generic), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(float)", asFUNCTION(AssignFloatToString_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(float)", asFUNCTION(AddAssignFloatToString_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(const string &in, float)", asFUNCTION(AddStringFloat_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(float, const string &in)", asFUNCTION(AddFloatString_Generic), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(int)", asFUNCTION(AssignIntToString_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(int)", asFUNCTION(AddAssignIntToString_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(const string &in, int)", asFUNCTION(AddStringInt_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(int, const string &in)", asFUNCTION(AddIntString_Generic), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(uint)", asFUNCTION(AssignUIntToString_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(uint)", asFUNCTION(AddAssignUIntToString_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(const string &in, uint)", asFUNCTION(AddStringUInt_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string@ f(uint, const string &in)", asFUNCTION(AddUIntString_Generic), asCALL_GENERIC); assert( r >= 0 );

	return typeId;
}

int RegisterScriptString(asIScriptEngine *engine)
{
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
		return RegisterScriptString_Generic(engine);
	else
		return RegisterScriptString_Native(engine);
}

END_AS_NAMESPACE


