#pragma once
#include "stm32wbxx_hal.h"
#include "messages.h"

#define NUM_SENSORS 1
#define VEC3D_SIZE 3

// Stored definition of Base Stations
typedef struct BaseStationGeometryDef {
	float origin[VEC3D_SIZE];  // Origin point
    float mat[9];  // Normalized rotation matrix.
} BaseStationGeometryDef;

typedef struct SensorLocalGeometry {
    uint32_t input_idx;
    float pos[VEC3D_SIZE];  // Position of the sensor relative to the object.
} SensorLocalGeometry;

// Parent, abstract class for GeometryBuilders.
typedef struct GeometryBuilder{
    BaseStationGeometryDef base_stations_[2];
    SensorLocalGeometry sensors[NUM_SENSORS];
    VIVEVars vive_vars_;
} GeometryBuilder;


void _GeometryBuilder(GeometryBuilder *self);
void consume_angles(GeometryBuilder * self, const SensorAnglesFrame * f);
void vec_cross_product(float *a, float *b, float *res);
float vec_length(float *vec);
void calc_ray_vec(const BaseStationGeometryDef * bs, float angle1, float angle2, float *res, float *origin);
uint8_t intersect_lines(float *orig1, float *vec1, float *orig2, float *vec2, float *res, float *dist);


