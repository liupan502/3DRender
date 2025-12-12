#include <QCoreApplication>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <ibl/CubemapUtils.h>
#include <ibl/CubemapSH.h>
#include <ibl/CubemapIBL.h>
#include <utils/JobSystem.h>
#include <math/mat3.h>
#include <math/scalar.h>

using namespace filament::ibl;

void load_image(const std::string& image_path, Image** ppimg) {
    int x = 0;
    int y = 0;
    int com = 0;
    stbi_uc* data = stbi_load(image_path.c_str(), &x, &y, &com, 3);
    *ppimg = new Image(x, y);
    filament::ibl::Image& img = **ppimg;

    float* img_data = (float*)(img.getData());
    // memcpy(img_data, data, sizeof(unsigned char) * x * y * 3);
    for (uint16_t i = 0; i < y; i++) {
        for (uint16_t j = 0; j < x; j++) {
            uint32_t base_idx = (i * x + j) * 3;
            img_data[base_idx] = data[base_idx] / 255.0f;
            img_data[base_idx + 1] = data[base_idx + 1] / 255.0f;
            img_data[base_idx + 2] = data[base_idx + 2] / 255.0f;
        }
    }
    stbi_image_free(data);
    data = nullptr;
}

void save_png_image(const std::string& png_image_path, const Image& image) {
    float* img_out_data = (float*)(image.getData());
    uint16_t x = image.getWidth();
    uint16_t y = image.getHeight();
    unsigned char* buf = new unsigned char[x * y * 4];
    for (uint16_t i = 0; i < y; i++) {
        for (uint16_t j = 0; j < x; j++) {
            uint32_t base_idx = (i * x + j);
            buf[base_idx * 4] = (unsigned char)(img_out_data[base_idx * 3] * 255.0f);
            buf[base_idx * 4 + 1] = (unsigned char)(img_out_data[base_idx * 3 + 1] * 255.0f);
            buf[base_idx * 4 + 2] = (unsigned char)(img_out_data[base_idx * 3 + 2] * 255.0f);
            buf[base_idx * 4 + 3] = 255;
        }
    }

    stbi_write_png(png_image_path.c_str(), x, y, 4, buf, x * 4);
    delete [] buf;
    buf = nullptr;
}

void compute_sh(const std::string& cube_map_path) {

    filament::ibl::Image* pimg = nullptr;
    load_image(cube_map_path, &pimg);
    Image& img = *pimg;
    uint16_t x = img.getWidth();
    uint16_t y = img.getHeight();
    Cubemap cube_map = CubemapUtils::create(img, img.getWidth() / 4);

    utils::JobSystem js(0, 1);
    js.adopt();
    uint8_t band_num = 3;
    auto sh = CubemapSH::computeSH(js, cube_map, band_num, true);
    for (uint16_t i = 0; i < band_num * band_num; i++) {
        printf("%f, %f, %f,\n", sh[i][0], sh[i][1], sh[i][2]);
    }
    fflush(stdout);
    filament::ibl::Image img_out(x, y);

    Cubemap cube_map_out = CubemapUtils::create(img_out, x / 4);

    CubemapSH::renderSH(js, cube_map_out, sh, band_num);



    save_png_image("/home/liup/Downloads/cubemap_renderer.png", img_out);

    CubemapSH::preprocessSHForShader(sh);

    filament::ibl::Image pre_img_out(x, y);

    Cubemap pre_cube_map_out = CubemapUtils::create(pre_img_out, x / 4);

    CubemapSH::renderPreScaledSH3Bands(js, cube_map_out, sh);



    save_png_image("/home/liup/Downloads/cubemap_renderer_pre.png", pre_img_out);
}

void create_dfg_img(const std::string& dfg_img_path, uint16_t dfg_dim) {
    filament::ibl::Image dfg_img(dfg_dim, dfg_dim);
    utils::JobSystem js(0, 1);
    js.adopt();
    CubemapIBL::DFG(js, dfg_img,false, false);
    save_png_image(dfg_img_path, dfg_img);
}

void create_prefiltered_cubemap(const std::vector<std::string>& cubemap_paths, float roughness,
                                const std::string& prefiltered_cubemap_path, uint16_t dim) {
    std::vector<Cubemap> cubemaps;
    std::vector<Image*> images(cubemap_paths.size(), nullptr);
    for (uint16_t i = 0; i < cubemap_paths.size(); i++) {

        load_image(cubemap_paths[i], &images[i]);
        // memset(images[i]->getData(), 0, images[i]->getBytesPerRow() * images[i]->getHeight());
        Cubemap cube_map = CubemapUtils::create(*images[i], (*images[i]).getWidth() / 4);
        cubemaps.emplace_back(std::move(cube_map));
    }

    Image img(dim * 4, dim * 3);

    memset(img.getData(), 0, img.getBytesPerRow() * img.getHeight());
    Cubemap prefiltered_cube_map = CubemapUtils::create(img, img.getWidth() / 4);
    utils::JobSystem js(0, 1);
    js.adopt();

    CubemapIBL::roughnessFilter(js, prefiltered_cube_map, cubemaps,roughness, 1024,
                                 filament::math::float3(1.0, 1.0, 1.0), true);
    save_png_image(prefiltered_cubemap_path, img);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    compute_sh("/home/liup/Downloads/cubemaps/cubemap4.jpg");
    create_dfg_img("/home/liup/Downloads/dfg.png", 512);
    std::vector<std::string> cubemap_paths = {
        "/home/liup/Downloads/cubemaps/1.jpg",
        "/home/liup/Downloads/cubemaps/2.jpg",
        "/home/liup/Downloads/cubemaps/4.jpg",
        "/home/liup/Downloads/cubemaps/4.jpg",
        "/home/liup/Downloads/cubemaps/4.jpg"
    };

    char buf[256] = {0};
    int tmp = 1;
    for (uint8_t i = 0; i< 5; i++) {
//        if (i != 3) {
//            tmp *= 2;
//            continue;
//        }
        float roughness = 0.2 * i;
        memset(buf, 0, 256);
        sprintf(buf, "/home/liup/Downloads/cubemaps/prefiltered_%d.png", tmp);
        std::string path(buf);
        create_prefiltered_cubemap(cubemap_paths, roughness, path, 256 / tmp);
        tmp *= 2;
    }
    return a.exec();
}
