/*
 * geometry.c
 *
 *  Created on: Nov 29, 2019
 *      Author: giand
 */
#include "stm32wbxx_hal.h"
#include "geometry.h"
#include <math.h>
#include <assert.h>
#include <arm_math.h>
#include "UART_Print.h"

void _GeometryBuilder(GeometryBuilder *self){
	BaseStationGeometryDef bs_0 = {{0.682646, 1.712605, 0.298152},
			  	  	  	  	  	  {0.356806, -0.017381, 0.934017, 0.001791, 0.999838, 0.017922, -0.934177, -0.004722, 0.356779}};

	BaseStationGeometryDef bs_1 = {{0.780941, 2.300994, -0.204002},
	  	  	  	  	  	  	  	  {-0.184830, -0.411017, 0.892694, 0.104180, 0.895032, 0.433664, -0.977233, 0.173155, -0.122609}};
	SensorLocalGeometry s_loc_geo = {0, {0.0, 0.0, 0.0}};

	self->base_stations_[0] = bs_0;
	self->base_stations_[1] = bs_1;
	self->sensors[0] = s_loc_geo;
}

void consume_angles(GeometryBuilder * self, const SensorAnglesFrame * f) {
    // First 2 angles - x, y of station B; second 2 angles - x, y of station C.
    // Coordinate system: Y - Up;  X ->  Z v  (to the viewer)
    // Station 'looks' to inverse Z axis (vector 0;0;-1).
    if (f->fix_level >= kCycleSynced) {
        SensorLocalGeometry *sens_def = &self->sensors[0];
        SensorAngles *sens = &f->sensors[sens_def->input_idx];

        // Check angles are fresh enough.
        uint32_t max_stale = 0;
        for (int i = 0; i < NUM_CYCLE_PHASES; i++){
        	//Get time since we last updated the angles for phase i
        	int stale = f->cycle_idx - sens->updated_cycles[i];
        	max_stale = (stale > max_stale) ? stale : max_stale;
        }

        if (max_stale < NUM_CYCLE_PHASES * 3) {  // We tolerate stale angles up to 2 cycles old.
            float ray1[VEC3D_SIZE], ray2[VEC3D_SIZE], origin1[VEC3D_SIZE], origin2[VEC3D_SIZE];
            calc_ray_vec(&self->base_stations_[0], sens->angles[0], sens->angles[1], ray1, origin1);
            calc_ray_vec(&self->base_stations_[1], sens->angles[2], sens->angles[3], ray2, origin2);

            intersect_lines(origin1, ray1, origin2, ray2, self->vive_vars_.pos, &self->vive_vars_.pos_delta);

            // Translate object position depending on the position of sensor relative to object.
            for (int i = 0; i < VEC3D_SIZE; i++){
            	self->vive_vars_.pos[i] -= sens_def->pos[i];
            }

            self->vive_vars_.time_ms = HAL_GetTick();


            //UART_Print_float(self->vive_vars_.pos[0]);
        }
        else {
            // Angles too stale - cannot calculate position anymore.

        }
    }
}

/*
void do_work_geometry_builder(GeometryBuilder * self) {
    // TODO: Make compatible with multiple geometry objects.
    //set_led_state(self->pos_.fix_level >= kStaleFix ? kFixFound : kNoFix);
}
*/

void vec_cross_product(float *a, float *b, float *res) {
    res[0] = a[1] * b[2] - a[2] * b[1];
    res[1] = a[2] * b[0] - a[0] * b[2];
    res[2] = a[0] * b[1] - a[1] * b[0];
}

float vec_length(float *vec) {
    float pow, res;

    arm_power_f32(vec, VEC3D_SIZE, &pow); // returns sum of squares
    arm_sqrt_f32(pow, &res);

    return res;
}

void calc_ray_vec(const BaseStationGeometryDef * bs, float angle1, float angle2, float *res, float *origin) {
    float a[VEC3D_SIZE] = {arm_cos_f32(angle1), 0, -arm_sin_f32(angle1)};  // Normal vector to X plane
    float b[VEC3D_SIZE] = {0, arm_cos_f32(angle2), arm_sin_f32(angle2)};   // Normal vector to Y plane

    float ray[VEC3D_SIZE] = {};
    vec_cross_product(b, a, ray); // Intersection of two planes -> ray vector.
    float len = vec_length(ray);
    arm_scale_f32(ray, 1/len, ray, VEC3D_SIZE); // Normalize ray length.

    arm_matrix_instance_f32 source_rotation_matrix = {3, 3, (const float*) bs->mat};
    arm_matrix_instance_f32 ray_vec = {3, 1, ray};
    arm_matrix_instance_f32 ray_rotated_vec = {3, 1, res};
    arm_mat_mult_f32(&source_rotation_matrix, &ray_vec, &ray_rotated_vec);

    // TODO: Make geometry adjustments within base station.
    float rotated_origin_delta[VEC3D_SIZE] = {};
    //vec3d base_origin_delta = {-0.025f, -0.025f, 0.f};  // Rotors are slightly off center in base station.
    // arm_matrix_instance_f32 origin_vec = {3, 1, base_origin_delta};
    // arm_matrix_instance_f32 origin_rotated_vec = {3, 1, rotated_origin_delta};
    // arm_mat_mult_f32(&source_rotation_matrix, &origin_vec, &origin_rotated_vec);
    arm_add_f32((const float *) bs->origin, rotated_origin_delta, origin, VEC3D_SIZE);
}


uint8_t intersect_lines(float *orig1, float *vec1, float *orig2, float *vec2, float *res, float *dist) {
    // Algorithm: http://geomalgorithms.com/a07-_distance.html#Distance-between-Lines

    float w0[VEC3D_SIZE] = {};
    arm_sub_f32((const float *)orig1, (const float *)orig2, w0, VEC3D_SIZE);

    float a, b, c, d, e;
    arm_dot_prod_f32((const float *)vec1, (const float *)vec1, VEC3D_SIZE, &a);
    arm_dot_prod_f32((const float *)vec1, (const float *)vec2, VEC3D_SIZE, &b);
    arm_dot_prod_f32((const float *)vec2, (const float *)vec2, VEC3D_SIZE, &c);
    arm_dot_prod_f32((const float *)vec1, w0, VEC3D_SIZE, &d);
    arm_dot_prod_f32((const float *)vec2, w0, VEC3D_SIZE, &e);

    float denom = a * c - b * b;
    if (fabs(denom) < 1e-5f)
        return 0;

    // Closest point to 2nd line on 1st line
    float t1 = (b * e - c * d) / denom;
    float pt1[VEC3D_SIZE] = {};
    arm_scale_f32((const float *)vec1, t1, pt1, VEC3D_SIZE);
    arm_add_f32(pt1, (const float *)orig1, pt1, VEC3D_SIZE);

    // Closest point to 1st line on 2nd line
    float t2 = (a * e - b * d) / denom;
    float pt2[VEC3D_SIZE] = {};
    arm_scale_f32((const float *)vec2, t2, pt2, VEC3D_SIZE);
    arm_add_f32(pt2, (const float *)orig2, pt2, VEC3D_SIZE);

    // Result is in the middle
    float tmp[VEC3D_SIZE] = {};
    arm_add_f32(pt1, pt2, tmp, VEC3D_SIZE);
    arm_scale_f32(tmp, 0.5f, res, VEC3D_SIZE);

    // Dist is distance between pt1 and pt2
    arm_sub_f32(pt1, pt2, tmp, VEC3D_SIZE);
    *dist = vec_length(tmp);

    return 1;
}

