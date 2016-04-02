#include "sl_sprite3.h"
#include "sl_shader.h"
#include "sl_buffer.h"

#include <sm.h>
#include <ds_array.h>

#include <string.h>
#include <stdlib.h>

#define STRINGIFY(A)  #A
#include "sprite3.vert"
#include "sprite3.frag"

#define MAX_VERTICES		4096

#define BUFFER_OFFSET(f) ((intptr_t)&(((struct vertex *)NULL)->f))

struct vertex {
	float vx, vy, vz;
	float tx, ty;
	uint32_t color, additive;
};

struct shader_state {
	int shader;

	int vertex_buf_id;
	struct sl_buffer* vertex_buf;

	int projection_id, modelview_id;
	union sm_mat4 modelview_mat, projection_mat;

	uint32_t color, additive;

	struct vertex* buf;
	int vertices_sz;

	int tex;
};

static struct shader_state S;

void 
sl_sprite3_load() {
	int s = sl_shader_create();
	if (s < 0) {
		return;
	}

	int vertex_buf_id = sl_shader_create_vertex_buffer(MAX_VERTICES, sizeof(struct vertex));
	struct sl_buffer* vertex_buf = sl_buf_create(sizeof(struct vertex), MAX_VERTICES);
	sl_shader_set_vertex_buffer(s, vertex_buf_id, vertex_buf);

	struct vertex_attrib va[4] = {
		{ "position", 0, 3, sizeof(float), BUFFER_OFFSET(vx) },
		{ "texcoord", 0, 2, sizeof(float), BUFFER_OFFSET(tx) },
		{ "color", 0, 4, sizeof(uint8_t), BUFFER_OFFSET(color) },
		{ "additive", 0, 4, sizeof(uint8_t), BUFFER_OFFSET(additive) },
	};
	int layout_id = sl_shader_create_vertex_layout(sizeof(va)/sizeof(va[0]), va);
	sl_shader_set_vertex_layout(s, layout_id);

	sl_shader_load(s, sprite3_vert, sprite3_frag);

	sl_shader_set_draw_mode(s, DRAW_TRIANGLES);

	S.shader = s;

	S.vertex_buf_id = vertex_buf_id;
	S.vertex_buf = vertex_buf;

	S.projection_id = sl_shader_add_uniform(s, "u_projection", UNIFORM_FLOAT44);
	S.modelview_id = sl_shader_add_uniform(s, "u_modelview", UNIFORM_FLOAT44);
	sm_mat4_identity(&S.projection_mat);
	sm_mat4_identity(&S.modelview_mat);

	S.color = 0xffffffff;
	S.additive = 0x00000000;

	S.buf = (struct vertex*)malloc(sizeof(struct vertex) * MAX_VERTICES);
	S.vertices_sz = 0;

	S.tex = 0;

	sl_shader_set_clear_flag(MASKC);
}

void 
sl_sprite3_unload() {
	sl_shader_release_vertex_buffer(S.vertex_buf_id);
	sl_buf_release(S.vertex_buf);
	sl_shader_unload(S.shader);

	free(S.buf); S.buf = NULL;

	memset(&S, 0, sizeof(struct shader_state));
}

void 
sl_sprite3_bind() {
	sl_shader_bind(S.shader);
}

void 
sl_sprite3_unbind() {
	sl_sprite3_commit();
}

void 
sl_sprite3_projection(const union sm_mat4* mat) {
	memcpy(&S.projection_mat, mat, sizeof(*mat));
	sl_shader_set_uniform(S.shader, S.projection_id, UNIFORM_FLOAT44, S.projection_mat.x);
}

void 
sl_sprite3_modelview(const union sm_mat4* mat) {
	memcpy(&S.modelview_mat, mat, sizeof(*mat));	
	sl_shader_set_uniform(S.shader, S.modelview_id, UNIFORM_FLOAT44, S.modelview_mat.x);
}

void 
sl_sprite3_set_color(uint32_t color, uint32_t additive) {
	S.color = color;
	S.additive = additive;	
}

void 
sl_sprite3_draw(const float* positions, const float* texcoords, int texid, int vertices_count) {
	if (S.vertices_sz + vertices_count > MAX_VERTICES ||
		(texid != S.tex && S.tex != 0)) {
		sl_sprite3_commit();
	}
	S.tex = texid;

	for (int i = 0; i < vertices_count; ++i) {
		struct vertex* v = &S.buf[S.vertices_sz++];
		v->vx = positions[i * 3];
		v->vy = positions[i * 3 + 1];
		v->vz = positions[i * 3 + 2];
		v->tx = texcoords[i * 2];
		v->ty = texcoords[i * 2 + 1];
		v->color = S.color;
		v->additive = S.additive;
	}
}

void 
sl_sprite3_commit() {
	if (S.vertices_sz == 0) {
		return;
	}

	sl_shader_set_texture(S.tex, 0);

	sl_shader_draw(S.shader, S.buf, S.vertices_sz, 0);
	S.vertices_sz = 0;

	sl_shader_flush();
}