#ifdef __cplusplus
extern "C"
{
#endif

#ifndef shader_lab_sprite3_h
#define shader_lab_sprite3_h

void sl_sprite3_load();
void sl_sprite3_unload();

void sl_sprite3_bind();
void sl_sprite3_unbind();

void sl_sprite3_projection(int width, int height, float near, float far);
void sl_sprite3_modelview(float x, float y, float sx, float sy);

void sl_sprite3_draw(const float* positions, const float* texcoords, int texid, int vertices_count);

void sl_sprite3_commit();

#endif // shader_lab_sprite3_h

#ifdef __cplusplus
}
#endif