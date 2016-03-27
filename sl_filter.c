#include "sl_filter.h"
#include "sl_math.h"
#include "sl_utility.h"
#include "sl_shader.h"
#include "sl_buffer.h"
#include "sl_typedef.h"

#include <stdint.h>
#include <stdlib.h>

#define STRINGIFY(A)  #A
#include "filter.vert"
#include "edge_detect.frag"
#include "relief.frag"
#include "outline.frag"
#include "blur.frag"
#include "gray.frag"
#include "heat_haze.frag"
#include "shock_wave.frag"

#define MAX_COMMBINE 256

struct vertex {
	float vx, vy;
	float tx, ty;
};

struct shader_state {
	int shader[SLFM_MAX_COUNT];

	int index_buf_id, vertex_buf_id;
	struct sl_buffer *index_buf, *vertex_buf;

	int projection[SLFM_MAX_COUNT];
	int modelview[SLFM_MAX_COUNT];

	union sl_mat4 modelview_mat, projection_mat;

	struct vertex* buf;
	int quad_sz;

	int tex;
	enum SL_FILTER_MODE mode;

	float time;

	// edge detect
	int edge_detect_val;
	// blur
	int blur_val;
	// heat haze
	int heat_haze_time, heat_haze_distortion, heat_haze_rise;
	int heat_haze_tex;
	// shock_wave
	int shock_wave_time, shock_wave_center, shock_wave_params;
};

static struct shader_state S;

static void
_create_shader(int idx, const char* vs, const char* fs, 
			   int index_buf_id, struct sl_buffer* index_buf,
			   int vertex_buf_id, struct sl_buffer* vertex_buf, 
			   int layout_id) {
	int s = sl_shader_create();
	if (s < 0) {
		return;
	}

	sl_shader_set_index_buffer(s, index_buf_id, index_buf);
	sl_shader_set_vertex_buffer(s, vertex_buf_id, vertex_buf);
	sl_shader_set_vertex_layout(s, layout_id);

	sl_shader_load(s, vs, fs);

	sl_shader_set_draw_mode(s, DRAW_TRIANGLES);

	S.shader[idx] = s;
	S.projection[idx] = sl_shader_add_uniform(s, "u_projection", UNIFORM_FLOAT44);
	S.modelview[idx] = sl_shader_add_uniform(s, "u_modelview", UNIFORM_FLOAT44);
}

static void
_init_edge_detect_uniforms() {
	S.edge_detect_val = sl_shader_add_uniform(S.shader[SLFM_EDGE_DETECTION], "u_blend", UNIFORM_FLOAT1);
	sl_filter_set_edge_detect_val(0.5f);
}

static void
_init_blur_uniforms() {
	S.blur_val = sl_shader_add_uniform(S.shader[SLFM_BLUR], "u_radius", UNIFORM_FLOAT1);
	sl_filter_set_blur_val(1);
}

static void
_init_heat_haze_uniforms() {
	S.heat_haze_time = sl_shader_add_uniform(S.shader[SLFM_HEAT_HAZE], "u_time", UNIFORM_FLOAT1);
	S.heat_haze_distortion = sl_shader_add_uniform(S.shader[SLFM_HEAT_HAZE], "u_distortion_factor", UNIFORM_FLOAT1);
	S.heat_haze_rise = sl_shader_add_uniform(S.shader[SLFM_HEAT_HAZE], "u_rise_factor", UNIFORM_FLOAT1);
	sl_filter_set_heat_haze_val(0.1f, 0.5f);
	int tex0 = sl_shader_add_uniform(S.shader[SLFM_HEAT_HAZE], "u_current_tex", UNIFORM_INT1);
	if (tex0 >= 0) {
		float sample = 0;
		sl_shader_set_uniform(S.shader[SLFM_HEAT_HAZE], tex0, UNIFORM_INT1, &sample);
	}
	int tex1 = sl_shader_add_uniform(S.shader[SLFM_HEAT_HAZE], "u_distortion_map_tex", UNIFORM_INT1);
	if (tex1 >= 0) {
		float sample = 1;
		sl_shader_set_uniform(S.shader[SLFM_HEAT_HAZE], tex1, UNIFORM_INT1, &sample);
	}
}

static void
_init_shock_wave_uniforms() {
	S.shock_wave_time = sl_shader_add_uniform(S.shader[SLFM_SHOCK_WAVE], "u_time", UNIFORM_FLOAT1);
	S.shock_wave_center = sl_shader_add_uniform(S.shader[SLFM_SHOCK_WAVE], "u_center", UNIFORM_FLOAT2);
	S.shock_wave_params = sl_shader_add_uniform(S.shader[SLFM_SHOCK_WAVE], "u_params", UNIFORM_FLOAT3);

	float center[2] = { 0.5f, 0.5f };
	sl_filter_set_shock_wave_center(center);
	float params[3] = { 10, 0.8f, 0.1f };
	sl_filter_set_shock_wave_params(params);
}

void 
sl_filter_load() {
	uint16_t idxs[6 * MAX_COMMBINE];
	sl_init_quad_index_buffer(idxs, MAX_COMMBINE);
	int index_buf_id = sl_shader_create_index_buffer(6 * MAX_COMMBINE, sizeof(uint16_t));
	struct sl_buffer* index_buf = sl_buf_create(sizeof(uint16_t), 6 * MAX_COMMBINE);
	sl_buf_add(index_buf, idxs, 6 * MAX_COMMBINE);
	sl_shader_update_buffer(index_buf_id, index_buf);
	index_buf->n = 0;

	int vertex_buf_id = sl_shader_create_vertex_buffer(4 * MAX_COMMBINE, sizeof(struct vertex));
	struct sl_buffer* vertex_buf = sl_buf_create(sizeof(struct vertex), 4 * MAX_COMMBINE);

	struct vertex_attrib va[2] = {
		{ "position", 0, 2, sizeof(float), BUFFER_OFFSET(vertex, vx) },
		{ "texcoord", 0, 2, sizeof(float), BUFFER_OFFSET(vertex, tx) },
	};
	int layout_id = sl_shader_create_vertex_layout(sizeof(va)/sizeof(va[0]), va);

	_create_shader(SLFM_EDGE_DETECTION, filter_vert, edge_detect_frag, index_buf_id, index_buf, vertex_buf_id, vertex_buf, layout_id);
	_create_shader(SLFM_RELIEF, filter_vert, relief_frag, index_buf_id, index_buf, vertex_buf_id, vertex_buf, layout_id);
	_create_shader(SLFM_OUTLINE, filter_vert, outline_frag, index_buf_id, index_buf, vertex_buf_id, vertex_buf, layout_id);
	_create_shader(SLFM_BLUR, filter_vert, blur_frag, index_buf_id, index_buf, vertex_buf_id, vertex_buf, layout_id);
	_create_shader(SLFM_GRAY, filter_vert, gray_frag, index_buf_id, index_buf, vertex_buf_id, vertex_buf, layout_id);
	_create_shader(SLFM_HEAT_HAZE, filter_vert, heat_haze_frag, index_buf_id, index_buf, vertex_buf_id, vertex_buf, layout_id);
	_create_shader(SLFM_SHOCK_WAVE, filter_vert, shock_wave_frag, index_buf_id, index_buf, vertex_buf_id, vertex_buf, layout_id);

	sl_mat4_identity(&S.projection_mat);
	sl_mat4_identity(&S.modelview_mat);

	S.index_buf_id = index_buf_id;
	S.vertex_buf_id = vertex_buf_id;
	S.index_buf = index_buf;
	S.vertex_buf = vertex_buf;

	S.buf = (struct vertex*)malloc(sizeof(struct vertex) * MAX_COMMBINE * 4);
	S.quad_sz = 0;
	S.tex = 0;
	S.mode = SLFM_MAX_COUNT;
	S.time = 0;

	_init_edge_detect_uniforms();
	_init_blur_uniforms();
	_init_heat_haze_uniforms();
	_init_shock_wave_uniforms();
}

void 
sl_filter_unload() {
	sl_shader_release_index_buffer(S.index_buf_id);
	sl_shader_release_vertex_buffer(S.vertex_buf_id);
	sl_buf_release(S.index_buf);
	sl_buf_release(S.vertex_buf);
	for (int i = 0; i < SLFM_MAX_COUNT; ++i) {
		sl_shader_unload(S.shader[i]);
	}
	free(S.buf); S.buf = NULL;
}

void 
sl_filter_bind() {
	if (S.mode != SLFM_MAX_COUNT) {
		sl_shader_bind(S.shader[S.mode]);
	}
}

void 
sl_filter_unbind() {
	sl_filter_commit();
}

void 
sl_filter_projection(int width, int height) {
	float hw = width * 0.5f;
	float hh = height * 0.5f;
	sl_mat4_ortho(&S.projection_mat, -hw, hw, -hh, hh, 1, -1);
	for (int i = 0; i < SLFM_MAX_COUNT; ++i) {
		sl_shader_set_uniform(S.shader[i], S.projection[i], UNIFORM_FLOAT44, S.projection_mat.x);
	}
}

void 
sl_filter_modelview(float x, float y, float sx, float sy) {
	sl_mat4_set_scale(&S.modelview_mat, sx, sy);
	sl_mat4_set_translate(&S.modelview_mat, x * sx, y * sy);
	for (int i = 0; i < SLFM_MAX_COUNT; ++i) {
		sl_shader_set_uniform(S.shader[i], S.modelview[i], UNIFORM_FLOAT44, S.modelview_mat.x);
	}
}

void 
sl_filter_set_mode(enum SL_FILTER_MODE mode) {
	if (mode != S.mode) {
		sl_filter_commit();
		S.mode = mode;
		sl_shader_bind(S.shader[S.mode]);
	}
}

void 
sl_filter_set_edge_detect_val(float val) {
	sl_shader_set_uniform(S.shader[SLFM_EDGE_DETECTION], S.edge_detect_val, UNIFORM_FLOAT1, &val);
}

void 
sl_filter_set_blur_val(float val) {
	sl_shader_set_uniform(S.shader[SLFM_BLUR], S.blur_val, UNIFORM_FLOAT1, &val);
}

void 
sl_filter_set_heat_haze_val(float distortion, float rise) {
 	sl_shader_set_uniform(S.shader[SLFM_HEAT_HAZE], S.heat_haze_distortion, UNIFORM_FLOAT1, &distortion);
 	sl_shader_set_uniform(S.shader[SLFM_HEAT_HAZE], S.heat_haze_rise, UNIFORM_FLOAT1, &rise);
}

void 
sl_filter_set_heat_haze_tex(int tex) {
	S.heat_haze_tex = tex;
}

void 
sl_filter_set_shock_wave_center(float center[2]) {
	sl_shader_set_uniform(S.shader[SLFM_SHOCK_WAVE], S.shock_wave_center, UNIFORM_FLOAT2, center);
}

void 
sl_filter_set_shock_wave_params(float params[3]) {
	sl_shader_set_uniform(S.shader[SLFM_SHOCK_WAVE], S.shock_wave_params, UNIFORM_FLOAT3, params);
}

void 
sl_filter_update(float dt) {
	S.time += dt;
	sl_shader_set_uniform(S.shader[SLFM_HEAT_HAZE], S.heat_haze_time, UNIFORM_FLOAT1, &S.time);
	sl_shader_set_uniform(S.shader[SLFM_SHOCK_WAVE], S.shock_wave_time, UNIFORM_FLOAT1, &S.time);
}

void 
sl_filter_draw(const float* positions, const float* texcoords, int texid) {
	if (S.quad_sz >= MAX_COMMBINE || (texid != S.tex && S.tex != 0)) {
		sl_filter_commit();
	}
	S.tex = texid;

	for (int i = 0; i < 4; ++i) {
		struct vertex* v = &S.buf[S.quad_sz * 4 + i];
		v->vx = positions[i * 2];
		v->vy = positions[i * 2 + 1];
		v->tx = texcoords[i * 2];
		v->ty = texcoords[i * 2 + 1];
	}
	++S.quad_sz;
}

void 
sl_filter_commit() {
	if (S.quad_sz == 0) {
		return;
	}

	sl_shader_set_texture(S.tex, 0);
	if (S.mode == SLFM_HEAT_HAZE) {
		sl_shader_set_texture(S.heat_haze_tex, 1);
	}

	sl_shader_draw(S.shader[S.mode], S.buf, S.quad_sz * 4, S.quad_sz * 6);

	S.quad_sz = 0;
	S.tex = 0;

	sl_shader_flush();
}

void 
sl_filter_on_size(int width, int height) {
	if (S.mode == SLFM_HEAT_HAZE) {
		sl_shader_set_texture(S.heat_haze_tex, 1);
	}
}