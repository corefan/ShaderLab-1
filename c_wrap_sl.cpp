#include "c_wrap_sl.h"
#include "shader/ShaderMgr.h"
#include "shader/ShapeShader.h"
#include "shader/SpriteShader.h"

namespace sl
{

extern "C"
void sl_create(int max_texture)
{
	ShaderMgr::Instance()->CreateContext(max_texture);
}

extern "C"
void sl_release()
{
	ShaderMgr::Instance()->ReleaseContext();
}

extern "C"
void sl_create_shader(enum SHADER_TYPE type)
{
	ShaderMgr* mgr = ShaderMgr::Instance();
	RenderContext* rc = mgr->GetContext();

	ShaderType sl_type = MAX_SHADER;
	Shader* shader = NULL;
	switch (type)
	{
	case ST_SHAPE:
		sl_type = SHAPE;
		shader = new ShapeShader(rc);
		break;
	case ST_SPRITE:
		sl_type = SPRITE;
		shader = new SpriteShader(rc);
		break;
	}
	if (shader) {
		mgr->CreateShader(sl_type, shader);
	}
}

extern "C"
void sl_set_shader(enum SHADER_TYPE type)
{
	ShaderType sl_type = MAX_SHADER;
	switch (type)
	{
	case ST_SHAPE:
		sl_type = SHAPE;
		break;
	case ST_SPRITE:
		sl_type = SPRITE;
		break;
	}
	if (type != MAX_SHADER) {
		ShaderMgr::Instance()->SetShader(sl_type);
	}
}

extern "C"
void sl_flush() 
{
	Shader* shader = ShaderMgr::Instance()->GetShader();
	if (shader) {
		shader->Commit();
	}
}

extern "C"
void sl_shape_color(uint32_t color) 
{
	ShapeShader* shader = static_cast<ShapeShader*>(
		ShaderMgr::Instance()->GetShader(SHAPE));
	if (shader) {
		shader->SetColor(color);
	}
}

extern "C"
void sl_shape_type(int type)
{
	ShapeShader* shader = static_cast<ShapeShader*>(
		ShaderMgr::Instance()->GetShader(SHAPE));
	if (shader) {
		shader->SetType(type);
	}
}

extern "C"
void sl_shape_draw(const float* positions, int count)
{
	ShapeShader* shader = static_cast<ShapeShader*>(
		ShaderMgr::Instance()->GetShader(SHAPE));
	if (shader) {
		shader->Draw(positions, count);
	}
}

extern "C"
void sl_shape_draw_node(float x, float y, bool dummy)
{
	ShapeShader* shader = static_cast<ShapeShader*>(
		ShaderMgr::Instance()->GetShader(SHAPE));
	if (shader) {
		shader->Draw(x, y, dummy);
	}
}

extern "C"
void sl_sprite_set_color(uint32_t color, uint32_t additive)
{
	SpriteShader* shader = static_cast<SpriteShader*>(
		ShaderMgr::Instance()->GetShader(SPRITE));
	if (shader) {
		shader->SetColor(color, additive);
	}
}

extern "C"
void sl_sprite_set_map_color(uint32_t rmap, uint32_t gmap, uint32_t bmap)
{
	SpriteShader* shader = static_cast<SpriteShader*>(
		ShaderMgr::Instance()->GetShader(SPRITE));
	if (shader) {
		shader->SetColorMap(rmap, gmap, bmap);
	}
}

extern "C"
void sl_sprite_draw(const float* positions, const float* texcoords, int texid)
{
	SpriteShader* shader = static_cast<SpriteShader*>(
		ShaderMgr::Instance()->GetShader(SPRITE));
	if (shader) {
		shader->Draw(positions, texcoords, texid);
	}
}

}