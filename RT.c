//
// Created by huang825172 on 2020/2/21.
//

#include "RTStructs.h"

#define ABS(i) (i>0)?i:-i

float Det(float *col1, float *col2, float *col3) {
  return col1[0]*col2[1]*col3[2]+col2[0]*col3[1]*col1[2]+col3[0]*col1[1]*col2[2] -
      (col1[2]*col2[1]*col3[0]+col2[2]*col3[1]*col1[0]+col3[2]*col1[1]*col2[0]);
}

/*
 * Ray tracing kernel.
 * ray+mesh => RayCast -> Reflect => new ray
 * Until all rays' depth==1 (clear_flag==0)
 */
__kernel
void RayTrace(__global struct Ray *ray,
              __global struct Triangle *mesh,
              __global struct Config *config,
              __global struct Summary *summary) {
  const float kTOLERATE = 1e-5;
  const int wi_pos_x = get_global_id(0);
  const int wi_pos_y = get_global_id(1);
  const int ray_index = wi_pos_y * config->screen_width + wi_pos_x;
  if (ray[ray_index].depth == -1) return;
  if (ray[ray_index].depth == 0) {
    ray[ray_index].dir[0] = wi_pos_x + config->ray_offset_x,
    ray[ray_index].dir[1] = config->ray_offset_y - wi_pos_y,
    ray[ray_index].dir[2] = config->near * -1;
  }
  float RCTemp[4][3];
  float RCTempDet[4];
  float RCTempPara[3];
  for(int i=0; i<config->mesh_size; i++) {
    RCTemp[0][0] = mesh[i].vertex[0][0] - mesh[i].vertex[1][0];
    RCTemp[0][1] = mesh[i].vertex[0][1] - mesh[i].vertex[1][1];
    RCTemp[0][2] = mesh[i].vertex[0][2] - mesh[i].vertex[1][2];
    RCTemp[1][0] = mesh[i].vertex[0][0] - mesh[i].vertex[2][0];
    RCTemp[1][1] = mesh[i].vertex[0][1] - mesh[i].vertex[2][1];
    RCTemp[1][2] = mesh[i].vertex[0][2] - mesh[i].vertex[2][2];
    RCTemp[2][0] = ray[ray_index].dir[0];
    RCTemp[2][1] = ray[ray_index].dir[1];
    RCTemp[2][2] = ray[ray_index].dir[2];
    RCTemp[3][0] = mesh[i].vertex[0][0];
    RCTemp[3][1] = mesh[i].vertex[0][1];
    RCTemp[3][2] = mesh[i].vertex[0][2];
    RCTempDet[0] = Det(RCTemp[0], RCTemp[1], RCTemp[2]);
    if (ABS(RCTempDet[0])-0<kTOLERATE) {
      continue;
    }
    RCTempDet[1] = Det(RCTemp[3], RCTemp[1], RCTemp[2]);
    RCTempDet[2] = Det(RCTemp[0], RCTemp[3], RCTemp[2]);
    RCTempDet[3] = Det(RCTemp[0], RCTemp[1], RCTemp[3]);
    RCTempPara[0] = RCTempDet[1] / RCTempDet[0];
    RCTempPara[1] = RCTempDet[2] / RCTempDet[0];
    RCTempPara[2] = RCTempDet[3] / RCTempDet[0];
    if ((RCTempPara[0]+RCTempPara[1]) > 1 ||
        RCTempPara[0]<0 || RCTempPara[0]>1 ||
        RCTempPara[1]<0 || RCTempPara[1]>1) {
      continue;
    }
    ++ray[ray_index].depth;
    ray[ray_index].color[0] = mesh[i].color[0][0];
    ray[ray_index].color[1] = mesh[i].color[0][1];
    ray[ray_index].color[2] = mesh[i].color[0][2];
    ray[ray_index].color[3] = mesh[i].color[0][3];
    float RFTemp[3];
    RFTemp[0] = ray[ray_index].dir[1]*mesh[i].normal[2] -
              ray[ray_index].dir[2]*mesh[i].normal[1];
    RFTemp[1] = ray[ray_index].dir[2]*mesh[i].normal[0] -
              ray[ray_index].dir[0]*mesh[i].normal[2];
    RFTemp[2] = ray[ray_index].dir[0]*mesh[i].normal[1] -
              ray[ray_index].dir[1]*mesh[i].normal[0];
    RFTemp[0] *= -2;
    RFTemp[1] *= -2;
    RFTemp[2] *= -2;
    RFTemp[0] = RFTemp[1]*mesh[i].normal[2] -
              RFTemp[i]*mesh[i].normal[1];
    RFTemp[1] = RFTemp[2]*mesh[i].normal[0] -
              RFTemp[i]*mesh[i].normal[2];
    RFTemp[2] = RFTemp[i]*mesh[i].normal[1] -
              RFTemp[i]*mesh[i].normal[0];
    ray[ray_index].dir[0] += RFTemp[0];
    ray[ray_index].dir[1] += RFTemp[1];
    ray[ray_index].dir[2] += RFTemp[2];
    if (ray[ray_index].depth >= config->max_depth) {
      ray[ray_index].depth = -1;
    } else {
      summary->clear_flag = 1;
    }
    return;
  }
  if (ray[ray_index].depth == 0) {
    ray[ray_index].color[0] = config->clear_color[0];
    ray[ray_index].color[1] = config->clear_color[1];
    ray[ray_index].color[2] = config->clear_color[2];
    ray[ray_index].color[3] = config->clear_color[3];
    ray[ray_index].depth = -1;
    return;
  } else {
    ray[ray_index].color[0] *= config->env_color[0];
    ray[ray_index].color[1] *= config->env_color[1];
    ray[ray_index].color[2] *= config->env_color[2];
    ray[ray_index].color[3] *= config->env_color[3];
    ray[ray_index].depth = -1;
  }
}