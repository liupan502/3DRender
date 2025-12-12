#include <utils/interface.h>
#include <utils/triangulate_warrper.h>


using namespace zr::utils;

int zr::utils::triangulate_polygon_warrper(const std::vector<float>& vertices, std::vector<int>& tri_indcies) {
    
    int vertex_count = vertices.size() / 2;
    std::vector<double> in_buf((vertex_count + 1) * 2, 0.0);
    for (int i = 1; i <= vertex_count; i++) {
        in_buf[i * 2] = vertices[i * 2 -2];
        in_buf[i * 2 + 1] = vertices[i * 2 - 1];
    }
    tri_indcies.resize((vertex_count - 2) * 3);

    int ret_num = triangulate_polygon(1, &vertex_count, 
            (double(*)[2])in_buf.data(), (int(*)[3])tri_indcies.data());

    if (ret_num == 0) {
        for (int i = 0; i < tri_indcies.size(); i++) {
            --tri_indcies[i];
        }
    }
    return ret_num;
}
