#ifndef __SHARP_SENSOR_H__
#define __SHARP_SENSOR_H__

#include "byte.h"

typedef enum
	{ SENSOR_SHORT_DIST
	, SENSOR_LONG_DIST
	} SensorType;

#define TABLE_SHORT_DIST_LEN 27
#define TABLE_SHORT_DIST_MIN 160
#define TABLE_SHORT_DIST_MAX 945
static const unsigned int table_short_dist[TABLE_SHORT_DIST_LEN] =
	{ 945/*, 830, 710, 630, 540, 485, 435, 420, 385 //  4 - 12 cm
	, 360, 334, 312, 293, 279, 265, 252, 242, 225 // 14 - 20 cm
	, 212, 209, 194, 190, 185, 180, 175, 170, 160*/ // 22 - 30 cm
	};

#define TABLE_LONG_DIST_LEN 12
#define TABLE_LONG_DIST_MIN 150
#define TABLE_LONG_DIST_MAX 880
static const unsigned int table_long_dist[TABLE_LONG_DIST_LEN] =
	{ 880/*, 635, 495, 405, 350 // 10 - 30 cm
	, 305, 270, 240, 220, 200 // 35 - 55 cm
	, 180, 150*/                // 60 - 65 cm
	};

unsigned int sharpa_measure(byte sens);
byte samplea_to_cm(unsigned int val, SensorType type);
byte sample_to_cm(const unsigned int table[], unsigned int offset, unsigned int interval, unsigned int val);

#endif