#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _shaderlab_wrap_c_h_
#define _shaderlab_wrap_c_h_

#include <stdint.h>

/**
 *  @note
 *    the values should same as sl::ShaderType
 */
enum SHADER_TYPE {
	ST_SHAPE2 = 0,
	ST_SHAPE3,
	ST_SPRITE2,
	ST_SPRITE3,
	ST_BLEND,
	ST_FILTER,
	ST_MODEL3,

	ST_MAX_SHADER
};

union sm_mat4;

/**
 *  @brief
 *    common
 */

int  sl_create(int max_texture);
void sl_release();

void sl_create_shader(enum SHADER_TYPE type);
void sl_release_shader(enum SHADER_TYPE type);

void sl_set_shader(enum SHADER_TYPE type);
int  sl_is_shader(enum SHADER_TYPE type);

void sl_on_projection2(int w, int h);
void sl_on_projection3(const union sm_mat4*);
void sl_on_modelview2(float x, float y, float sx, float sy);
void sl_on_modelview3(const union sm_mat4*);

void sl_set_texture(int id);
int  sl_get_texture();
void sl_set_target(int id);
int  sl_get_target();

void sl_set_blend(int m1, int m2);
void sl_set_default_blend();

void sl_render_clear(unsigned long argb);
int  sl_render_version();
void sl_enable_scissor(int enable);

struct render* sl_get_ej_render();

void sl_flush();

/**
 *  @brief
 *    shape2 shader
 */
void sl_shape2_color(uint32_t color);
void sl_shape2_type(int type);
void sl_shape2_draw(const float* positions, int count);
void sl_shape2_draw_node(float x, float y, int dummy);

/**
 *  @brief
 *    shape3 shader
 */
void sl_shape3_color(uint32_t color);
void sl_shape3_type(int type);
void sl_shape3_draw(const float* positions, int count);
void sl_shape3_draw_node(float x, float y, float z, int dummy);

/**
 *  @brief
 *    sprite2 shader
 */
void sl_sprite2_set_color(uint32_t color, uint32_t additive);
void sl_sprite2_set_map_color(uint32_t rmap, uint32_t gmap, uint32_t bmap);
void sl_sprite2_draw(const float* positions, const float* texcoords, int texid);

/**
 *  @brief
 *    sprite3 shader
 */
void sl_sprite3_set_color(uint32_t color, uint32_t additive);
void sl_sprite3_set_map_color(uint32_t rmap, uint32_t gmap, uint32_t bmap);
void sl_sprite3_draw(const float* positions, const float* texcoords, int texid);

/**
 *  @brief
 *    filter shader
 *
 *  @note
 *    the values should same as sl::FILTER_MODE
 */
enum SL_FILTER_MODE {
	SLFM_NULL				= 0,
	SLFM_NORMAL				= 1,

	SLFM_EDGE_DETECTION		= 10,
	SLFM_RELIEF,
	SLFM_OUTLINE,

	SLFM_GRAY				= 20,
	SLFM_BLUR,
	SLFM_GAUSSIAN_BLUR,

	SLFM_HEAT_HAZE			= 30,
	SLFM_SHOCK_WAVE,
	SLFM_SWIRL,
	SLFM_BURNING_MAP,
};
void sl_filter_set_mode(int mode);
void sl_filter_set_heat_haze_factor(float distortion, float rise);
void sl_filter_set_heat_haze_texture(int id);
void sl_filter_set_burning_map_upper_texture(int id);
void sl_filter_set_burning_map_height_texture(int id);
void sl_filter_set_burning_map_border_texture(int id);
void sl_filter_update(float dt);
void sl_filter_clear_time();
void sl_filter_draw(const float* positions, const float* texcoords, int texid);

/**
 *  @brief
 *    blend shader
 */
void sl_blend_set_mode(int mode);
void sl_blend_set_color(uint32_t color, uint32_t additive);
void sl_blend_draw(const float* positions, const float* texcoords_blend, 
				   const float* texcoords_base, int tex_blend, int tex_base);

#endif // _shaderlab_wrap_c_h_

#ifdef __cplusplus
}
#endif