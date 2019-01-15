#ifndef PAKKI_JSON
#define PAKKI_JSON

//#include <vector>
#include <string>
#include "Containers.h"

enum class ValueType
{
	Jbool = 0, 
	Jstring,
	Jchild,
	Jarray,
	Jnull,
	invalid,
	Jdouble,
	Jfloat,
	Jint,
	Juint,
	maxtypes
};



#define DEF_LENGHT 10000
struct Value
{
	ValueType type;
	union
	{
		bool            Jbool;
		float           Fnum;
		double          Dnum;
		int             Inum;
		unsigned int    UInum;
		int             string;
		int             child_offset;
		int 			numArray;
	};
};


class JsonToken;

struct JValueHandle final
{
	friend JsonToken;
	JValueHandle operator[] (const char* name);
	JValueHandle operator[] (int index);

	char* GetString()const
	{
		return type == ValueType::Jstring ? string : nullptr;
	}
	float GetFloat()const
	{
		return type == ValueType::Jfloat ? Fnum : 0;
	}
	int GetArraySize() const
	{
		return type == ValueType::Jarray ? val->numArray : 0;		
	}
	int GetInt()const
	{
		return type == ValueType::Jint ? Inum : 0;
	}
	double GetDouble()const
	{
		return type == ValueType::Jdouble ? Dnum : 0;
	}
	unsigned int GetUInt()const
	{
		return type == ValueType::Juint ? UInum : 0;
	}
	bool GetBool()const
	{
		return type == ValueType::Jbool ? Jbool : 0;
	}
	bool IsValid()const
	{
		return type != ValueType::invalid;
	}
	JsonToken* GetToken()const
	{
		return type == ValueType::Jchild ? handle : nullptr;
	}
	ValueType GetType() const
	{
		return type;
	}
	private:
	ValueType   type;
	union
	{
		JsonToken*      handle;
		float           Fnum;
		double          Dnum;
		int             Inum;
		unsigned int    UInum;
		bool            Jbool;
		char*           string;
		struct
		{
			Value*      val;
			JsonToken*  from;
			//			int 		SizeOfArray;
		};
	};
};

class JsonToken final
{
	public:
		friend JValueHandle;
		JsonToken();
		~JsonToken();
		JsonToken(const JsonToken&) = delete;
		JsonToken(const JsonToken&&) = delete;
		void operator=(const JsonToken&&) = delete;

		void ParseFile(const char* file);

		JValueHandle operator[] (const char* key);
		void ParseFromSrc(char* src);
		void GetKeys(CONTAINER::DynamicArray<char*>* buffer /*std::vector<std::string>* buffer*/);
		unsigned int GetNumberOfElements() const
		{
			return (unsigned int)m_keys.numobj;
		}
	private:
		struct key
		{
			int name_offset = 0;
			int value_offset = 0;
			//int numValues = 0;
		};
		JsonToken*          m_childs = NULL;
		unsigned int        m_child_indexes = 0;
		unsigned int        m_child_alloc_size;
		//std::vector<Value>  m_values;              // all values
		CONTAINER::DynamicArray<Value> m_values;
		//std::vector<key>    m_keys;                // all keys
		CONTAINER::DynamicArray<key>	m_keys;
		char*               m_strings = NULL;      // all strings
		int                 m_strings_lenght = 0;
		int                 m_strings_alloc_lenght = DEF_LENGHT;
	private:
		static JValueHandle m_create_return_value_from_value(Value* val, JsonToken* from); // init from malloc
		char*               m_parse_section(char* src);
		static void         m_init_malloc(JsonToken* ob);
};

//#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
//#include <vector>
#include <assert.h>
#include <string.h>
#include <iostream>
////#include "JsonParser.h"


JValueHandle JValueHandle::operator[] (const char* name)
{
	if (type != ValueType::invalid && type == ValueType::Jchild)
	{
		return (*handle)[name];
	}
	else
	{
		JValueHandle temp;
		temp.type = ValueType::invalid;
		return temp;
	}
}
JValueHandle JValueHandle::operator[] (int index)
{
	if (type == ValueType::Jarray)
	{
		return JsonToken::m_create_return_value_from_value(val + index + 1, from);
	}
	else
	{
		JValueHandle temp;
		temp.type = ValueType::invalid;
		return temp;
	}
}

JsonToken::JsonToken() //: m_values(100) , m_keys(100)
{
	m_strings = (char*)malloc(sizeof(char) * DEF_LENGHT);
	CONTAINER::init_dynamic_array(&m_keys,100);
	CONTAINER::init_dynamic_array(&m_values,100);
}

JsonToken::~JsonToken()
{
	free(m_strings);
	if (m_childs != NULL)
	{
		for (unsigned i = 0; i < m_child_indexes; i++)
		{
			m_childs[i].~JsonToken();
		}
		free(m_childs);
	}
}
void JsonToken::m_init_malloc(JsonToken* ob)
{
	ob = new(ob)JsonToken;
}


JValueHandle  JsonToken::operator[] (const char* key)
{
	int i = 0;
	for (; i < (int)m_keys.numobj ;i++)
	{
		if (!strcmp(&m_strings[m_keys.buffer[i].name_offset], key))
		{
			return m_create_return_value_from_value(&m_values.buffer[m_keys.buffer[i].value_offset], this);
		}
	}
	JValueHandle temp;
	temp.handle = NULL;
	temp.type = ValueType::invalid;
	return temp;
}

void JsonToken::GetKeys(CONTAINER::DynamicArray<char*>* buffer /*std::vector<std::string>* buffer*/)
{
	for (unsigned int i = 0; i < m_keys.numobj; i++)    //key& k : m_keys)
	{
		CONTAINER::push_back_dynamic_array(buffer,&m_strings[m_keys.buffer[i].name_offset]); //k.name_offset]);
	}
}
char* read_file(const char* file)
{
	char *source = NULL;
	FILE *fp = fopen(file, "r");
	if (fp != NULL) {
		/* Go to the end of the file. */
		if (fseek(fp, 0L, SEEK_END) == 0) {
			/* Get the size of the file. */
			long bufsize = ftell(fp);
			if (bufsize == -1) { /* Error */ }

			/* Allocate our buffer to that size. */
			source = (char*)malloc(sizeof(char) * (bufsize + 1));

			/* Go back to the start of the file. */
			if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

			/* Read the entire file into memory. */
			size_t newLen = fread(source, sizeof(char), bufsize, fp);
			if (ferror(fp) != 0) {
				fputs("Error reading file", stderr);
			}
			else {
				source[newLen++] = '\0'; /* Just to be safe. */
			}
		}
		fclose(fp);
	}
	return source;
}
inline void CopyWord(const char* source, char* dest, const int size)
{
	for (int i = 0; i < size; i++)
	{
		dest[i] = source[i];
	}
}

int ensure_size(char** buf, int size, int pushSize, int AllocSize)
{
	if (size + pushSize >= AllocSize)
	{
		AllocSize *= 2;
		char* temp = *buf;
		*buf = (char*)realloc(*buf, AllocSize);
		if (!(*buf))
		{
			*buf = temp;
			return AllocSize / 2;
		}
	}
	return AllocSize;
}

inline int ParseWord(int buffIndex, char** iter, char** alloc, int *sizeAlloc)
{
	int i = 1;
	(*iter)++; // "
	char* temp = *iter;
	for (; *temp != '"'; temp++) { i++; }

	*sizeAlloc = ensure_size(alloc, buffIndex, i, *sizeAlloc);
	CopyWord(*iter, &((*alloc)[buffIndex]), i);
	*iter = temp;
	(*iter)++;
	((*alloc)[buffIndex + i - 1]) = '\0';
	return i;
}
JValueHandle JsonToken::m_create_return_value_from_value(Value* val, JsonToken* from)
{
	switch (val->type)
	{
		case ValueType::Jchild:
			{
				JValueHandle temp;
				temp.handle = &from->m_childs[val->child_offset];
				temp.type = ValueType::Jchild;
				return temp;
			}
		case ValueType::Jstring:
			{
				JValueHandle temp;
				char *k = &from->m_strings[val->string];
				temp.string = k;
				temp.type = ValueType::Jstring;
				return temp;
			}
		case ValueType::Jint:
			{
				JValueHandle temp;
				temp.Inum = val->Inum;
				temp.type = ValueType::Jint;
				return temp;
			}
		case ValueType::Jfloat:
			{
				JValueHandle temp;
				temp.Fnum = val->Fnum;
				temp.type = ValueType::Jfloat;
				return temp;
			}
		case ValueType::Jdouble:
			{
				JValueHandle temp;
				temp.Dnum = val->Dnum;
				temp.type = ValueType::Jdouble;
				return temp;
			}
		case ValueType::Juint:
			{
				JValueHandle temp;
				temp.UInum = val->UInum;
				temp.type = ValueType::Juint;
				return temp;
			}
		case ValueType::Jbool:
			{
				JValueHandle temp;
				temp.Jbool = val->Jbool;
				temp.type = ValueType::Jbool;
				return temp;
			}
		case ValueType::Jarray:
			{
				JValueHandle temp;
				temp.type = ValueType::Jarray;
				temp.val = val;
				temp.from = from;
				return temp;
			}
		default:
			JValueHandle temp;
			temp.handle = NULL;
			temp.type = ValueType::invalid;
			return temp;
	}


}
char* JsonToken::m_parse_section(char* source)
{
	if (!source) return source;
	bool GetValue = false;

	char numBuffer[10];
	bool GetArray = false;
	int StartOfArray = 0;
	for (char* iter = source; *iter; iter++)
	{
		bool nothingCaught = false;
		switch (*iter)
		{
			case '{':
				{
					if (GetValue) // add new token
					{

						if (m_childs == NULL)
						{
							m_childs = (JsonToken*)malloc(100 * sizeof(JsonToken));
							m_child_alloc_size = 100;
						}
						else if (m_child_indexes >= m_child_alloc_size)
						{
							JsonToken* temp = m_childs;
							m_child_alloc_size *= 2;
							m_childs = (JsonToken*)realloc(m_childs, m_child_alloc_size);
							if (!m_childs)
							{ 
                                m_childs = temp;
							}
						}

						JsonToken* ntoken = &m_childs[m_child_indexes++];

						Value t;
						t.type = ValueType::Jchild;
						t.child_offset = (int)(ntoken - m_childs);
						CONTAINER::push_back_dynamic_array(&m_values,t);
						//m_values.push_back(t);
						JsonToken::m_init_malloc(ntoken);
						iter = ntoken->m_parse_section(iter++);
						GetValue = GetArray ? true : false;
					}
					else // Syntax error if not int start of the file? 
					{
						continue;
					}
				}
				break;
			case '}':   // end token and return 
				{
					return iter++;
				}
				break;
			case '[':
				{
					//printf("Syntax error!\n");
					assert(GetValue);
					Value ar;
					ar.type = ValueType::Jarray;
					ar.numArray = 0;
					CONTAINER::push_back_dynamic_array(&m_values,ar);
					//m_values.push_back(ar);
					GetArray = true;
					StartOfArray = m_values.numobj - 1;
				}
				break;
			case ']':
				{
					//printf("Syntax error!\n");
					assert(GetValue);
					GetArray = false;
					GetValue = false;
					StartOfArray = 0;
				}
				break;
			case '\t': case'\n': case'\0':case ' ': case ',':
				{
					continue;
				}
				break;
			case '"':   // get string or key
				{
					if (GetValue)
					{
						Value val;
						val.string = m_strings_lenght;
						m_strings_lenght += ParseWord(m_strings_lenght, &iter, &m_strings, &m_strings_alloc_lenght);

						val.type = ValueType::Jstring;
						CONTAINER::push_back_dynamic_array(&m_values,val);
						//m_values.push_back_dynamic_array(val);
						iter--;
						GetValue = GetArray ? true : false;
						if(GetArray) m_values.buffer[StartOfArray].numArray++;
					}
					else
					{
						key k;
						k.name_offset = m_strings_lenght;
						m_strings_lenght += ParseWord(m_strings_lenght, &iter, &m_strings, &m_strings_alloc_lenght);
						k.value_offset = (int)m_values.numobj;
						CONTAINER::push_back_dynamic_array(&m_keys,k);
						//m_keys.push_back(k);
						while (*iter != ':')
						{
							assert(*iter == ' ');
							iter++;
						}
						GetValue = true;
					}
				}
				break;
			default:
				nothingCaught = true;
				break;
		}

		if (nothingCaught && GetValue)
		{
			if (*iter != ' ')
			{
				int i = 0;
				bool includesDot = false;
				bool includesU = false;
				bool includesF = false;
				for (; !(*iter == ',' || *iter == '}' || *iter == ']' || *iter == '\n' );iter++)//|| *iter == 'u' || *iter == 'f'); iter++)
				{
					if (*iter != ' ')
						numBuffer[i++] = *iter;
					if (*iter == '.')
						includesDot = true;


				}
				iter = *iter == ']' || *iter == '}' ? iter - 1 : iter;
				if (*iter == 'u')
				{
					includesU = true;
				}
				else if (*iter == 'f' && i > 0)
				{
					includesF = true;
				}
				numBuffer[i] = '\0';

				Value v;

				if ((!strcmp("true", numBuffer)))
				{
					v.type = ValueType::Jbool;
					v.Jbool = true;
				}
				else if ((!strcmp("false", numBuffer)))
				{
					v.type = ValueType::Jbool;
					v.Jbool = false;
				}
				else if ((!strcmp("null", numBuffer)))
				{
					v.type = ValueType::Jnull;
					v.Dnum = 0.00;
				}
				else
				{
					//numBuffer[i] = '\0';
					if (includesDot)//double or float
					{
						if (includesF)
						{
							float temp = strtof(numBuffer, NULL);
							v.type = ValueType::Jfloat;
							v.Fnum = temp;
						}
						else
						{
							double temp = strtod(numBuffer, NULL);
							v.type = ValueType::Jdouble;
							v.Dnum = temp;
						}
					}
					else // integer or unsigned
					{
						if (includesU)
						{
							unsigned int temp = strtoul(numBuffer, NULL, 0);
							v.type = ValueType::Juint;
							v.UInum = temp;
						}
						else
						{
							int temp = strtol(numBuffer, NULL, 0);
							v.type = ValueType::Jint;
							v.Inum = temp;

						}
					}



				}
				GetValue = GetArray ? true : false;
				if(GetArray) m_values.buffer[StartOfArray].numArray++;
				CONTAINER::push_back_dynamic_array(&m_values,v);
				//m_values.push_back(v);
			}
		}
	}
	return nullptr;
}


void JsonToken::ParseFile(const char* filename)
{
	char* source = read_file(filename);
	m_parse_section(source);
	free(source);
}
void JsonToken::ParseFromSrc(char* src)
{
	m_parse_section(src);
}
#endif // PAKKI_JSON


