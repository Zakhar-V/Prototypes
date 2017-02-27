
struct TypeAttributes
{

};

struct TypeInfo
{

};


class Object
{

	static const TypeInfo* GetTypeInfoStatic(void);

	virtual const TypeInfo* GetTypeInfo();
	uint GetType();
	const TypeAttributes* GetAttributes();
	const String& GetTypeName();
	const TypeInfo* GetBase();

};


////////////////////////////


class Object
{
	uint Type();
	string TypeName();
	ClassInfo* TypeInfo();

};