//
// Created by huang825172 on 2020/2/22.
//

#ifndef PRT__RTSTRUCTS_H_
#define PRT__RTSTRUCTS_H_

/*
 * Data struct for RAY BUFFER.
 * Length: 29 Bytes.
 * Init:
 *  depth = 0;
 * If no collision occur when depth==0, depth = -1;
 * If no collision occur when depth!=0, color = environment color, depth = -1;
 * If collision occur when depth==max depth, depth = -1;
 * If collision occur when depth<max depth, color calc, ++depth;
 */
struct Ray {
  float dir[3];
  float color[4];
  char depth;
};

/*
 * Data struct for MESH BUFFER.
 * Length: 100 Bytes.
 * normal MUST be unitized.
 */
struct Triangle {
  unsigned int id;
  float vertex[3][3];
  float color[3][4];
  float normal[3];
};

/*
 * Data struct for RAY TRACING CONFIG.
 * Length: 44 Bytes.
 */
struct Config {
  int screen_width;
  int screen_height;
  int ray_offset_x;
  int ray_offset_y;
  int near;
  int mesh_size;
  float clear_color[4];
  float env_color[4];
  char max_depth;
};

/*
 * Data struct for TRACING OUTPUT.
 * Init:
 *  clear_flag = 0;
 * If any output ray's depth!=-1, clear_flag = 1;
 */
struct Summary {
  char clear_flag;
};

#endif //PRT__RTSTRUCTS_H_
