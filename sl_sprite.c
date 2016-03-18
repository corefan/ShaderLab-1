#include "sl_sprite.h"
#include "sl_matrix.h"
#include "sl_shader.h"
#include "sl_typedef.h"
#include "sl_utility.h"
#include "sl_vertexbuffer.h"

#include <render/render.h>

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define STRINGIFY(A)  #A
#include "sprite.vert"
#include "sprite_plain.frag"
#include "sprite_color.frag"
#include "sprite_map.frag"
#include "sprite_both.frag"

#define MAX_COMMBINE 1024

struct position {
	float vx, vy;
	float tx, ty;
};

struct color {
	uint32_t color, additive;
};

struct map {
	uint32_t rmap, gmap, bmap;
};

struct vertex_plain {
	struct position pos;
};

struct vertex_color {
	struct position pos;
	struct color col;
};

struct vertex_map {
	struct position pos;
	struct map map;
};

struct vertex_both {
	struct position pos;
	struct color col;
	struct map map;
};

#define TYPE_PLAIN	0
#define TYPE_COLOR	1
#define TYPE_MAP	2

struct vertex {
	int type;
	
	struct position pos;
	struct color col;
	struct map map;
};

enum SHADER_IDX {
	IDX_PLAIN = 0,
	IDX_COLOR,
	IDX_MAP,
	IDX_BOTH,
};

#define MAX_SHADER_COUNT 4

struct shader_state {
	int shader[MAX_SHADER_COUNT];

	int index_buf_id;
	int vertex_buf_id[MAX_SHADER_COUNT];
	struct sl_vertexbuffer* vertex_buf[MAX_SHADER_COUNT];

	int projection[MAX_SHADER_COUNT];
	int modelview[MAX_SHADER_COUNT];

	struct sl_matrix modelview_mat, projection_mat;

	uint32_t color, additive;
	uint32_t rmap, gmap, bmap;

	struct vertex* buf;
	int quad_sz;
	
	int tex;
	int type;
};

static struct shader_state S;

static void
_create_plain_shader(int index_buf_id) {
	int s = sl_shader_create();
	if (s < 0) {
		return;
	}

	sl_shader_set_index_buffer(s, index_buf_id);

	int vertex_buf_id = sl_shader_create_vertex_buffer(4 * MAX_COMMBINE, sizeof(struct vertex_plain));
	struct sl_vertexbuffer* vertex_buf = sl_vb_create(sizeof(struct vertex_plain), 4 * MAX_COMMBINE);
	sl_shader_set_vertex_buffer(s, vertex_buf_id, vertex_buf);

	struct vertex_attrib va[2] = {
		{ "position", 0, 2, sizeof(float), BUFFER_OFFSET(vertex_plain, pos.vx) },
		{ "texcoord", 0, 2, sizeof(float), BUFFER_OFFSET(vertex_plain, pos.tx) },
	};
	int layout_id = sl_shader_create_vertex_layout(sizeof(va)/sizeof(va[0]), va);
	sl_shader_set_vertex_layout(s, layout_id);

	sl_shader_load(s, sprite_vert, sprite_plain_frag);

	sl_shader_set_draw_mode(s, DRAW_TRIANGLES);

	S.shader[IDX_PLAIN] = s;
	S.vertex_buf_id[IDX_PLAIN] = vertex_buf_id;
	S.vertex_buf[IDX_PLAIN] = vertex_buf;
	S.projection[IDX_PLAIN] = sl_shader_add_uniform(s, "u_projection", UNIFORM_FLOAT44);
	S.modelview[IDX_PLAIN] = sl_shader_add_uniform(s, "u_modelview", UNIFORM_FLOAT44);
}

static void
_create_color_shader(int index_buf_id) {
	int s = sl_shader_create();
	if (s < 0) {
		return;
	}

	sl_shader_set_index_buffer(s, index_buf_id);

	int vertex_buf_id = sl_shader_create_vertex_buffer(4 * MAX_COMMBINE, sizeof(struct vertex_color));
	struct sl_vertexbuffer* vertex_buf = sl_vb_create(sizeof(struct vertex_color), 4 * MAX_COMMBINE);
	sl_shader_set_vertex_buffer(s, vertex_buf_id, vertex_buf);

	struct vertex_attrib va[4] = {
		{ "position", 0, 2, sizeof(float), BUFFER_OFFSET(vertex_color, pos.vx) },
		{ "texcoord", 0, 2, sizeof(float), BUFFER_OFFSET(vertex_color, pos.tx) },
		{ "color", 0, 4, sizeof(uint8_t), BUFFER_OFFSET(vertex_color, col.color) },
		{ "additive", 0, 4, sizeof(uint8_t), BUFFER_OFFSET(vertex_color, col.additive) },
	};
	int layout_id = sl_shader_create_vertex_layout(sizeof(va)/sizeof(va[0]), va);
	sl_shader_set_vertex_layout(s, layout_id);

	sl_shader_load(s, sprite_vert, sprite_color_frag);

	sl_shader_set_draw_mode(s, DRAW_TRIANGLES);

	S.shader[IDX_COLOR] = s;
	S.vertex_buf_id[IDX_COLOR] = vertex_buf_id;
	S.vertex_buf[IDX_COLOR] = vertex_buf;
	S.projection[IDX_COLOR] = sl_shader_add_uniform(s, "u_projection", UNIFORM_FLOAT44);
	S.modelview[IDX_COLOR] = sl_shader_add_uniform(s, "u_modelview", UNIFORM_FLOAT44);
}

static void
_create_map_shader(int index_buf_id) {
	int s = sl_shader_create();
	if (s < 0) {
		return;
	}

	sl_shader_set_index_buffer(s, index_buf_id);

	int vertex_buf_id = sl_shader_create_vertex_buffer(4 * MAX_COMMBINE, sizeof(struct vertex_map));
	struct sl_vertexbuffer* vertex_buf = sl_vb_create(sizeof(struct vertex_map), 4 * MAX_COMMBINE);
	sl_shader_set_vertex_buffer(s, vertex_buf_id, vertex_buf);

	struct vertex_attrib va[5] = {
		{ "position", 0, 2, sizeof(float), BUFFER_OFFSET(vertex_map, pos.vx) },
		{ "texcoord", 0, 2, sizeof(float), BUFFER_OFFSET(vertex_map, pos.tx) },
		{ "rmap", 0, 4, sizeof(uint8_t), BUFFER_OFFSET(vertex_map, map.rmap) },
		{ "gmap", 0, 4, sizeof(uint8_t), BUFFER_OFFSET(vertex_map, map.gmap) },
		{ "bmap", 0, 4, sizeof(uint8_t), BUFFER_OFFSET(vertex_map, map.bmap) },
	};
	int layout_id = sl_shader_create_vertex_layout(sizeof(va)/sizeof(va[0]), va);
	sl_shader_set_vertex_layout(s, layout_id);

	sl_shader_load(s, sprite_vert, sprite_map_frag);

	sl_shader_set_draw_mode(s, DRAW_TRIANGLES);

	S.shader[IDX_MAP] = s;
	S.vertex_buf_id[IDX_MAP] = vertex_buf_id;
	S.vertex_buf[IDX_MAP] = vertex_buf;
	S.projection[IDX_MAP] = sl_shader_add_uniform(s, "u_projection", UNIFORM_FLOAT44);
	S.modelview[IDX_MAP] = sl_shader_add_uniform(s, "u_modelview", UNIFORM_FLOAT44);
}

static void
_create_both_shader(int index_buf_id) {
	int s = sl_shader_create();
	if (s < 0) {
		return;
	}

	sl_shader_set_index_buffer(s, index_buf_id);

	int vertex_buf_id = sl_shader_create_vertex_buffer(4 * MAX_COMMBINE, sizeof(struct vertex_both));
	struct sl_vertexbuffer* vertex_buf = sl_vb_create(sizeof(struct vertex_both), 4 * MAX_COMMBINE);
	sl_shader_set_vertex_buffer(s, vertex_buf_id, vertex_buf);

	struct vertex_attrib va[7] = {
		{ "position", 0, 2, sizeof(float), BUFFER_OFFSET(vertex_both, pos.vx) },
		{ "texcoord", 0, 2, sizeof(float), BUFFER_OFFSET(vertex_both, pos.tx) },
		{ "color", 0, 4, sizeof(uint8_t), BUFFER_OFFSET(vertex_both, col.color) },
		{ "additive", 0, 4, sizeof(uint8_t), BUFFER_OFFSET(vertex_both, col.additive) },
		{ "rmap", 0, 4, sizeof(uint8_t), BUFFER_OFFSET(vertex_both, map.rmap) },
		{ "gmap", 0, 4, sizeof(uint8_t), BUFFER_OFFSET(vertex_both, map.gmap) },
		{ "bmap", 0, 4, sizeof(uint8_t), BUFFER_OFFSET(vertex_both, map.bmap) },
	};
	int layout_id = sl_shader_create_vertex_layout(sizeof(va)/sizeof(va[0]), va);
	sl_shader_set_vertex_layout(s, layout_id);

	sl_shader_load(s, sprite_vert, sprite_both_frag);

	sl_shader_set_draw_mode(s, DRAW_TRIANGLES);

	S.shader[IDX_BOTH] = s;
	S.vertex_buf_id[IDX_BOTH] = vertex_buf_id;
	S.vertex_buf[IDX_BOTH] = vertex_buf;
	S.projection[IDX_BOTH] = sl_shader_add_uniform(s, "u_projection", UNIFORM_FLOAT44);
	S.modelview[IDX_BOTH] = sl_shader_add_uniform(s, "u_modelview", UNIFORM_FLOAT44);
}

void 
sl_sprite_load() {
	uint16_t idxs[6 * MAX_COMMBINE];
	sl_init_quad_index_buffer(idxs, MAX_COMMBINE);
	int index_buf_id = sl_shader_create_index_buffer(6 * MAX_COMMBINE, sizeof(uint16_t), idxs);	
	_create_plain_shader(index_buf_id);
	_create_color_shader(index_buf_id);
	_create_map_shader(index_buf_id);
	_create_both_shader(index_buf_id);

	sl_matrix_identity(&S.projection_mat);
	sl_matrix_identity(&S.modelview_mat);

	S.index_buf_id = index_buf_id;

 	S.color = 0xffffffff;
 	S.additive = 0x00000000;
	S.rmap = 0x000000ff;
	S.gmap = 0x0000ff00;
	S.bmap = 0x00ff0000;

	S.buf = (struct vertex*)malloc(sizeof(struct vertex) * MAX_COMMBINE * 4);
	S.quad_sz = 0;
	S.tex = 0;
	S.type = TYPE_PLAIN;
}

void 
sl_sprite_unload() {
	sl_shader_release_index_buffer(S.index_buf_id);
	for (int i = 0; i < MAX_SHADER_COUNT; ++i) {
		int shader = S.shader[i];
		sl_shader_release_vertex_buffer(S.vertex_buf_id[i]);
		sl_vb_release(S.vertex_buf[i]);
		sl_shader_unload(shader);
	}
	free(S.buf); S.buf = NULL;
}

void 
sl_sprite_bind() {
	sl_shader_bind(S.shader[IDX_PLAIN]);
}

void 
sl_sprite_unbind() {
	sl_sprite_commit();
}

void 
sl_sprite_projection(int width, int height) {
	float hw = width * 0.5f;
	float hh = height * 0.5f;
	sl_matrix_ortho(&S.projection_mat, -hw, hw, -hh, hh, 1, -1);
	for (int i = 0; i < MAX_SHADER_COUNT; ++i) {
		sl_shader_set_uniform(S.shader[i], S.projection[i], UNIFORM_FLOAT44, S.projection_mat.e);
	}
}

void 
sl_sprite_modelview(float x, float y, float sx, float sy) {
	sl_matrix_set_scale(&S.modelview_mat, sx, sy);
	sl_matrix_set_translate(&S.modelview_mat, x * sx, y * sy);
	for (int i = 0; i < MAX_SHADER_COUNT; ++i) {
		sl_shader_set_uniform(S.shader[i], S.modelview[i], UNIFORM_FLOAT44, S.modelview_mat.e);
	}
}

void 
sl_sprite_set_color(uint32_t color, uint32_t additive) {
	S.color = color;
	S.additive = additive;
}

void 
sl_sprite_set_map_color(uint32_t rmap, uint32_t gmap, uint32_t bmap) {
	S.rmap = rmap;
	S.gmap = gmap;
	S.bmap = bmap;
}

void 
sl_sprite_draw(const float* positions, const float* texcoords, int texid) {
	if (S.quad_sz >= MAX_COMMBINE ||
		(texid != S.tex && S.tex != 0)) {
		sl_sprite_commit();
	}
	S.tex = texid;

	bool has_color = (S.color != 0xffffffff) || (S.additive & 0xffffff != 0);
	bool has_map = ((S.rmap & 0x00ffffff) != 0x000000ff) || ((S.gmap & 0x00ffffff) != 0x0000ff00) || ((S.bmap & 0x00ffffff) != 0x00ff0000);
	int type = TYPE_PLAIN;
	if (has_color) {
		type |= TYPE_COLOR;
		S.type |= TYPE_COLOR;
	}
	if (has_map) {
		type |= TYPE_MAP;
		S.type |= TYPE_MAP;
	}
	for (int i = 0; i < 4; ++i) {
		struct vertex* v = &S.buf[S.quad_sz * 4 + i];
		v->type = type;
		v->pos.vx = positions[i * 2];
		v->pos.vy = positions[i * 2 + 1];
		v->pos.tx = texcoords[i * 2];
		v->pos.ty = texcoords[i * 2 + 1];
 		v->col.color = S.color;
 		v->col.additive = S.additive;
		v->map.rmap = S.rmap;
		v->map.gmap = S.gmap;
		v->map.bmap = S.bmap;
	}
	++S.quad_sz;
}

void 
sl_sprite_commit() {
	if (S.quad_sz == 0) {
		return;
	}

	sl_shader_set_texture(S.tex, 0);

	int sz = S.quad_sz * 4;
	if (S.type == TYPE_PLAIN) {
		struct vertex_plain vb[sz];
		for (int i = 0; i < sz; ++i) {
			memcpy(&vb[i].pos, &S.buf[i].pos, sizeof(struct position));
		}
		sl_shader_bind(S.shader[IDX_PLAIN]);
		sl_shader_draw(S.shader[IDX_PLAIN], vb, sz, S.quad_sz * 6);
	} else if (S.type == TYPE_COLOR) {
		struct vertex_color vb[sz];
		for (int i = 0; i < sz; ++i) {
			memcpy(&vb[i].pos, &S.buf[i].pos, sizeof(struct position));
			memcpy(&vb[i].col, &S.buf[i].col, sizeof(struct color));
		}
		sl_shader_bind(S.shader[IDX_COLOR]);
		sl_shader_draw(S.shader[IDX_COLOR], vb, sz, S.quad_sz * 6);
	} else if (S.type == TYPE_MAP) {
		struct vertex_map vb[sz];
		for (int i = 0; i < sz; ++i) {
			memcpy(&vb[i].pos, &S.buf[i].pos, sizeof(struct position));
			memcpy(&vb[i].map, &S.buf[i].map, sizeof(struct map));
		}
		sl_shader_bind(S.shader[IDX_MAP]);
		sl_shader_draw(S.shader[IDX_MAP], vb, sz, S.quad_sz * 6);
	} else {
		assert((S.type & TYPE_COLOR) && (S.type & TYPE_MAP));
		struct vertex_both vb[sz];
		for (int i = 0; i < sz; ++i) {
			memcpy(&vb[i].pos, &S.buf[i].pos, sizeof(struct position));
			memcpy(&vb[i].col, &S.buf[i].col, sizeof(struct color));
			memcpy(&vb[i].map, &S.buf[i].map, sizeof(struct map));
		}
		sl_shader_bind(S.shader[IDX_BOTH]);
		sl_shader_draw(S.shader[IDX_BOTH], vb, sz, S.quad_sz * 6);
	}

	S.quad_sz = 0;
	S.tex = 0;
	S.type = TYPE_PLAIN;

	sl_shader_flush();
}