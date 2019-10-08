#ifndef enum_factory_h__
#define enum_factory_h__

#include <string>
// Taken from http://stackoverflow.com/questions/147267/easy-way-to-use-variables-of-enum-types-as-string-in-c, and adapted a bit

// expansion macro for enum value definition
#define ENUM_VALUE(name,assign) name assign,

// expansion macro for enum to string conversion
#define ENUM_CASE(name,assign) case name: return #name;

// expansion macro for string to enum conversion
#define ENUM_STRCMP(name,assign) if (!strcmp(str,#name)) return name;

/// declare the access function and define enum values
#define DECLARE_ENUM(EnumType,ENUM_DEF) \
  enum EnumType { \
    ENUM_DEF(ENUM_VALUE) \
  }; \
  const char *GetString(EnumType dummy); \
  EnumType GetEnumValue(const char *string,EnumType foo); \

/// define the access function names
#define DEFINE_ENUM(EnumType,ENUM_DEF) \
  const char *GetString(EnumType value) \
  { \
    switch(value) \
    { \
      ENUM_DEF(ENUM_CASE) \
      default: return ""; /* handle input error */ \
    } \
  } \
  EnumType GetEnumValue(const char *str,EnumType foo) \
  { \
    ENUM_DEF(ENUM_STRCMP) \
    return (EnumType)0; /* handle input error */ \
  } \

#endif // enum_factory_h__
