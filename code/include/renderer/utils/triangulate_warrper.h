#pragma once
#include <vector>


namespace zr{
    namespace utils{
        /**
         1. 底层代码来自 http://gamma.cs.unc.edu/SEIDEL/
         2. vertices 按照逆时针方向排列
         3. 分割成功时，tri_indices 中存储对应的三角片顶点的索引， 三个索引构成一个三角片
         4. 返回值为0时，表示分割成功
         5. 目前最多支持800个边
         6. 仅支持没有洞的简单多边形
         7. 预期时间复杂度为 O(NlogN)
        */
        int triangulate_polygon_warrper(const std::vector<float>& vertices, std::vector<int>& tri_indcies);
    }
};
