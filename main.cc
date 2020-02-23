//
// Created by huang825172 on 2020/2/22.
//

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 220

#include <CL/cl2.hpp>
#include <SDL.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include "RTStructs.h"
#include "icosahedron.h"

constexpr float kPI = 3.1415926535;

float D2R(float degree) {
  return degree * kPI / 180;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
int main() {
  const int screen_width = 640;
  const int screen_height = 480;
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Event event;
  bool sdl_quit = false;
  if (SDL_Init(SDL_INIT_VIDEO) != 0){
    std::cout << SDL_GetError() << std::endl;
    return -1;
  }
  window = SDL_CreateWindow(
	  "PRT",
	  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	  screen_width, screen_height,
	  SDL_WINDOW_SHOWN
  );
  if (window == nullptr) return -1;
  renderer = SDL_CreateRenderer(window, -1, 0);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
							  SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
  const auto kernel_file = "RT.c";
  const float fov = 90;
  auto ray_set = std::vector<Ray>(screen_width * screen_height);
  for (auto &ray: ray_set) { ray.depth = 0; }
  auto mesh_set = std::vector<Triangle>();
  auto ico_vec_tri = icosahedron::make_icosphere(1);
  for (auto &vec_tri: ico_vec_tri.second) {
	icosahedron::v3 normal = icosahedron::get_normal(ico_vec_tri.first, vec_tri);
	mesh_set.push_back(Triangle{
		0,
		{
			{
				ico_vec_tri.first[vec_tri.vertex[0]][0],
				ico_vec_tri.first[vec_tri.vertex[0]][1],
				ico_vec_tri.first[vec_tri.vertex[0]][2] - 2.0f
			},
			{
				ico_vec_tri.first[vec_tri.vertex[1]][0],
				ico_vec_tri.first[vec_tri.vertex[1]][1],
				ico_vec_tri.first[vec_tri.vertex[1]][2] - 2.0f
			},
			{
				ico_vec_tri.first[vec_tri.vertex[2]][0],
				ico_vec_tri.first[vec_tri.vertex[2]][1],
				ico_vec_tri.first[vec_tri.vertex[2]][2] - 2.0f
			}
		},
		{
			{1.0f, 0.0f, 0.0f, 1.0f},
			{0.0f, 1.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f, 1.0f}
		},
		{normal[0], normal[1], normal[2]}
	});
  }
  int mesh_size = mesh_set.size();
  Config config = {
	  screen_width,
	  screen_height,
	  screen_width / -2,
	  screen_height / 2,
	  static_cast<int>((screen_width / 2.0f) / tan(static_cast<double>(D2R(fov / 2)))),
	  mesh_size,
	  {0.5f, 0.5f, 0.5f, 1.0f},
	  {0.0f, 0.0f, 0.0f, 1.0f},
	  2
  };
  Summary summary = {0};
  try {
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	std::vector<cl::Device> devices;
#ifdef __APPLE__
	platforms[0].getDevices(CL_DEVICE_TYPE_ALL, &devices);
#else
	platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
#endif
	cl::Context context(devices);
	cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);
	cl::Buffer ray_buffer = cl::Buffer(context, CL_MEM_READ_WRITE,
									   sizeof(Ray) * ray_set.size());
	cl::Buffer mesh_buffer = cl::Buffer(context, CL_MEM_READ_ONLY,
										sizeof(Triangle) * mesh_set.size());
	cl::Buffer config_buffer = cl::Buffer(context, CL_MEM_READ_ONLY,
										  sizeof(Config));
	cl::Buffer summary_buffer = cl::Buffer(context, CL_MEM_WRITE_ONLY,
										   sizeof(Summary));
	queue.enqueueWriteBuffer(ray_buffer, CL_TRUE, 0,
							 sizeof(Ray) * ray_set.size(), ray_set.data());
	queue.enqueueWriteBuffer(mesh_buffer, CL_TRUE, 0,
							 sizeof(Triangle) * mesh_set.size(), mesh_set.data());
	queue.enqueueWriteBuffer(config_buffer, CL_TRUE, 0,
							 sizeof(Config), &config);
	queue.enqueueWriteBuffer(summary_buffer, CL_TRUE, 0,
							 sizeof(Summary), &summary);
	std::ifstream sourceFile(kernel_file);
	std::string sourceCode(std::istreambuf_iterator<char>(sourceFile),
						   (std::istreambuf_iterator<char>()));
	cl::Program::Sources source = {sourceCode};
	cl::Program program = cl::Program(context, source);
	try {
	  program.build(devices);
	} catch (cl::Error &error) {
	  std::cout << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;
	  return error.err();
	}
	int clear_flag = 1;
	while (clear_flag != 0) {
	  summary.clear_flag = 0;
	  queue.enqueueWriteBuffer(summary_buffer, CL_TRUE, 0,
							   sizeof(Summary), &summary);
	  cl::Kernel ray_trace_kernel(program, "RayTrace");
	  ray_trace_kernel.setArg(0, ray_buffer);
	  ray_trace_kernel.setArg(1, mesh_buffer);
	  ray_trace_kernel.setArg(2, config_buffer);
	  ray_trace_kernel.setArg(3, summary_buffer);
	  cl::NDRange global(screen_width, screen_height);
	  queue.enqueueNDRangeKernel(ray_trace_kernel, cl::NullRange, global, cl::NullRange);
	  queue.enqueueReadBuffer(summary_buffer, CL_TRUE, 0, sizeof(summary), &summary);
	  clear_flag = (int) summary.clear_flag;
	}
	queue.enqueueReadBuffer(ray_buffer, CL_TRUE, 0,
							sizeof(Ray) * ray_set.size(), ray_set.data());
	auto pixel_out = std::vector<char>(screen_width * screen_height * 4);
	for (int y = 0; y < screen_height; ++y) {
	  for (int x = 0; x < screen_width; ++x) {
		int ray_index = y * screen_width + x;
		pixel_out[ray_index * 4 + 0] = (char) (ray_set[ray_index].color[0] * 255);
		pixel_out[ray_index * 4 + 1] = (char) (ray_set[ray_index].color[1] * 255);
		pixel_out[ray_index * 4 + 2] = (char) (ray_set[ray_index].color[2] * 255);
		pixel_out[ray_index * 4 + 3] = (char) (ray_set[ray_index].color[3] * 255);
	  }
	}
	while (!sdl_quit) {
	  SDL_PollEvent(&event);
	  switch (event.type) {
		case SDL_QUIT:sdl_quit = true;
		  break;
		default:break;
	  }
	  SDL_UpdateTexture(texture, nullptr, pixel_out.data(), screen_width * 4);
	  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	  SDL_RenderPresent(renderer);
	}
  } catch (cl::Error &error) {
	std::cout << error.what() << "(" << error.err() << ")" << std::endl;
  }
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  return 0;
}
#pragma clang diagnostic pop
