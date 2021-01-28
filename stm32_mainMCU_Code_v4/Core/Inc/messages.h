#pragma once
#include "stm32wbxx_hal.h"
#include "cmsis_os2.h"

// Tunable constants
#define MAX_NUM_INPUTS 8	// Number of concurrent sensors supported.
#define MAX_BYTES_IN_DATA_FRAME 64	// Current DataFrame length is 33. This param should be larger.
#define MAX_BYTES_IN_DATA_CHUNK 64

// Not tunable: constant for Lighthouse system.
#define NUM_BASE_STATIONS 2
#define NUM_CYCLE_PHASES 4

typedef struct Pulse {
	uint8_t input_idx;
	uint16_t start_time;
	uint16_t pulse_len;
} Pulse;

enum FixLevel {
    kNoSignals      =    0,  // No signals visible at all.
    kCycleSyncing   =  100,  // Base station sync pulses are visible and we're syncing to them.
    kCycleSynced    =  200,  // We're synced to the base station sync pulses.
    kPartialVis     =  500,  // Some sensors/base stations don't have visibility and angles are stale. Position is invalid.
    kStaleFix       =  800,  // Position fix is valid, but uses angles from previous 1-2 cycles.
    kFullFix        = 1000,  // Position fix is valid and fresh.
};

//enum FixLevel {
//    kNoSignals      =    0,  // No signals visible at all.
//    kCycleSyncing   =  2,  // Base station sync pulses are visible and we're syncing to them.
//    kCycleSynced    =  4,  // We're synced to the base station sync pulses.
//    kPartialVis     =  8,  // Some sensors/base stations don't have visibility and angles are stale. Position is invalid.
//    kStaleFix       =  10,  // Position fix is valid, but uses angles from previous 1-2 cycles.
//    kFullFix        = 12,  // Position fix is valid and fresh.
//};

typedef struct SensorAngles {
    float angles[NUM_CYCLE_PHASES]; // Angles of base stations to sensor, -1/3 Pi to 1/3 Pi
    uint32_t updated_cycles[NUM_CYCLE_PHASES]; // Cycle id when this angle was last updated.
} SensorAngles;

// SensorAnglesFrame is produced by PulseProcessor every 4 cycles and consumed by GeometryBuilders. It contains
// a snapshot of angles visible by sensors.
typedef struct SensorAnglesFrame {
	uint16_t time;
    int fix_level;  // Up to kCycleSynced
    uint32_t cycle_idx;  // Increasing number of cycles since last fix.
    int32_t phase_id;    // 0..3
    SensorAngles sensors[MAX_NUM_INPUTS];
} SensorAnglesFrame;

// One data bit extracted from a long pulse from one base station. Produced by PulseProcessor and consumed by DataFrameDecoder.
typedef struct DataFrameBit {
	uint16_t time;
    uint32_t base_station_idx;
    uint32_t cycle_idx;
    uint8_t bit;
} DataFrameBit;

// Decoded data frame. Produced by DataFrameDecoder. 'bytes' array can be casted to DecodedDataFrame to get meaningful values.
typedef struct DataFrame {
	uint16_t time;
    uint32_t base_station_idx;
    uint8_t bytes[MAX_BYTES_IN_DATA_FRAME];
} DataFrame;

typedef struct VIVEVars{
	float pos[3];
	float pos_delta;
	uint32_t time_ms;
	uint32_t epoch;
} VIVEVars;


osMessageQueueId_t pulseQueueHandle;
osMessageQueueId_t viveQueueHandle;
