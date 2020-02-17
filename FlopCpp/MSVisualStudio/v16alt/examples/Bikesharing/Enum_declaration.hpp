#ifndef Enum_declaration_h__
#define Enum_declaration_h__

#include "enum_factory.hpp"
#define STATIONS_ENUM(XX) \
XX(s1020,) \
XX(s1021,) \
XX(s1022,) \
XX(s1023,) \
XX(s1024,) \
XX(s1025,) \
XX(s1026,) \
XX(s1027,) \
XX(s1028,) \
XX(s1029,) \
XX(s1030,) \
XX(s1031,) \
XX(s1032,) \
XX(s1033,) \
XX(s1034,) \
XX(s1035,) \
XX(s1036,) \
XX(s1037,) \
XX(s1038,) \
XX(s1039,) \
XX(s1041,) \
XX(s1042,) \
XX(s1043,) \
XX(s1044,) \
XX(s1045,) \
XX(s1046,) \
XX(s1048,) \
XX(s1049,) \
XX(s1050,) \
XX(s1051,) \
XX(s1052,) \
XX(s1053,) \
XX(s1054,) \
XX(s1055,) \
XX(s1056,) \
XX(s1057,) \
XX(s1058,) \
XX(s1059,) \
XX(s1060,) \
XX(s1061,) \
XX(s1062,) \
XX(s1063,) \
XX(s1064,) \
XX(s1065,) \
XX(s1066,) \
XX(s1067,) \
XX(s1068,) \
XX(s1069,) \
XX(s1071,) \
XX(s1072,) \
XX(s1073,) \
XX(s1074,) \
XX(s1075,) \
XX(s1076,) \
XX(s1077,) \
XX(s1078,) \
XX(s1080,) \
XX(s1082,) \
XX(s1083,) \
XX(number_stations,) 
DECLARE_ENUM(stations,STATIONS_ENUM)

#define PERIODS_ENUM(XX) \
XX(p0,) \
XX(p1,) \
XX(p2,) \
XX(p3,) \
XX(p4,) \
XX(p5,) \
XX(p6,) \
XX(p7,) \
XX(p8,) \
XX(p9,) \
XX(p10,) \
XX(p11,) \
XX(p12,) \
XX(p13,) \
XX(p14,) \
XX(p15,) \
XX(p16,) \
XX(p17,) \
XX(p18,) \
XX(p19,) \
XX(p20,) \
XX(p21,) \
XX(p22,) \
XX(p23,) \
XX(p24,) \
XX(number_periods,)
DECLARE_ENUM(periods,PERIODS_ENUM)

#define SCENARIOS_ENUM(XX) \
    XX(number_scenarios,=186)
DECLARE_ENUM(scenarios,SCENARIOS_ENUM)

#endif // Enum_declaration_h__