//
// Created by huang825172 on 2020/2/23.
//

#include "icosahedron.h"

namespace icosahedron {
v3 operator+(const v3 &lhs, const v3 &rhs) {
  return v3{lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2]};
}

v3 normalize(const v3 &rhs) {
  float length = sqrt(rhs[0] * rhs[0] + rhs[1] * rhs[1] + rhs[2] * rhs[2]);
  return v3{rhs[0] / length, rhs[1] / length, rhs[2] / length};
}

Index vertex_for_edge(Lookup &lookup, VertexList &vertices, Index first, Index second) {
  Lookup::key_type key(first, second);
  if (key.first > key.second)
    std::swap(key.first, key.second);

  auto inserted = lookup.insert({key, vertices.size()});
  if (inserted.second) {
    auto &edge0 = vertices[first];
    auto &edge1 = vertices[second];
    auto point = normalize(edge0 + edge1);
    vertices.push_back(point);
  }

  return inserted.first->second;
}

TriangleList subdivide(VertexList &vertices,
                       const TriangleList &triangles) {
  Lookup lookup;
  TriangleList result;

  for (auto &&each:triangles) {
    std::array<Index, 3> mid{};
    for (int edge = 0; edge < 3; ++edge) {
      mid[edge] = vertex_for_edge(lookup, vertices,
                                  each.vertex[edge], each.vertex[(edge + 1) % 3]);
    }

    result.push_back({each.vertex[0], mid[0], mid[2]});
    result.push_back({each.vertex[1], mid[1], mid[0]});
    result.push_back({each.vertex[2], mid[2], mid[1]});
    result.push_back({mid[0], mid[1], mid[2]});
  }

  return result;
}

IndexedMesh make_icosphere(int subdivisions) {
  VertexList vertices = icosahedron::vertices;
  TriangleList triangles = icosahedron::triangles;

  for (int i = 0; i < subdivisions; ++i) {
    triangles = subdivide(vertices, triangles);
  }

  return {vertices, triangles};
}

v3 get_normal(const VertexList &vl, const Triangle &tri) {
  v3 sum = vl[tri.vertex[0]] + vl[tri.vertex[1]] + vl[tri.vertex[2]];
  return normalize(sum);
}
}