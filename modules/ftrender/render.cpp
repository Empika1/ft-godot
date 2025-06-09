#include "render.h"
#include "core\os\memory.h"
#include "scene\resources\image_texture.h"
#include "scene\2d\sprite_2d.h"
extern "C" {
    #include "modules\ftrender\embeds.h"
}

void FTRender::_bind_methods() {
	BIND_ENUM_CONSTANT(STATIC_RECT_BORDER);
	BIND_ENUM_CONSTANT(STATIC_RECT_INSIDE);
	BIND_ENUM_CONSTANT(STATIC_CIRC_BORDER);
	BIND_ENUM_CONSTANT(STATIC_CIRC_INSIDE);
	BIND_ENUM_CONSTANT(DYNAMIC_RECT_BORDER);
	BIND_ENUM_CONSTANT(DYNAMIC_RECT_INSIDE);
	BIND_ENUM_CONSTANT(DYNAMIC_CIRC_BORDER);
	BIND_ENUM_CONSTANT(DYNAMIC_CIRC_INSIDE);
	BIND_ENUM_CONSTANT(GP_RECT_BORDER);
	BIND_ENUM_CONSTANT(GP_RECT_INSIDE);
	BIND_ENUM_CONSTANT(GP_CIRC_BORDER);
	BIND_ENUM_CONSTANT(GP_CIRC_INSIDE);
	BIND_ENUM_CONSTANT(WOOD_BORDER);
	BIND_ENUM_CONSTANT(WOOD_INSIDE);
	BIND_ENUM_CONSTANT(WATER_BORDER);
	BIND_ENUM_CONSTANT(WATER_INSIDE);
	BIND_ENUM_CONSTANT(CW_BORDER);
	BIND_ENUM_CONSTANT(CW_INSIDE);
	BIND_ENUM_CONSTANT(CW_DECAL);
	BIND_ENUM_CONSTANT(CCW_BORDER);
	BIND_ENUM_CONSTANT(CCW_INSIDE);
	BIND_ENUM_CONSTANT(CCW_DECAL);
	BIND_ENUM_CONSTANT(UPW_BORDER);
	BIND_ENUM_CONSTANT(UPW_INSIDE);
	BIND_ENUM_CONSTANT(UPW_DECAL);
	BIND_ENUM_CONSTANT(BUILD_BORDER);
	BIND_ENUM_CONSTANT(BUILD_INSIDE);
	BIND_ENUM_CONSTANT(GOAL_BORDER);
	BIND_ENUM_CONSTANT(GOAL_INSIDE);
	BIND_ENUM_CONSTANT(JOINT_NORMAL);
	BIND_ENUM_CONSTANT(JOINT_WHEEL_CENTER);
    BIND_ENUM_CONSTANT(OBJ_TYPE_SIZE);

	BIND_ENUM_CONSTANT(STATIC_RECT);
	BIND_ENUM_CONSTANT(STATIC_CIRC);
	BIND_ENUM_CONSTANT(DYNAMIC_RECT);
	BIND_ENUM_CONSTANT(DYNAMIC_CIRC);
	BIND_ENUM_CONSTANT(GP_RECT);
	BIND_ENUM_CONSTANT(GP_CIRC);
	BIND_ENUM_CONSTANT(WOOD);
	BIND_ENUM_CONSTANT(WATER);
	BIND_ENUM_CONSTANT(CW);
	BIND_ENUM_CONSTANT(CCW);
	BIND_ENUM_CONSTANT(UPW);
	BIND_ENUM_CONSTANT(BUILD);
	BIND_ENUM_CONSTANT(GOAL);
    BIND_ENUM_CONSTANT(PIECE_TYPE_SIZE);

    BIND_CONSTANT(LAYER_COUNT);
    BIND_CONSTANT(LAYER_MULTIMESH_INSTANCE_COUNT);

    ClassDB::bind_method(D_METHOD("setColors", "colors"), &FTRender::setColors);
	ClassDB::bind_method(D_METHOD("getColors",), &FTRender::getColors);
	ClassDB::bind_method(D_METHOD("setColor", "objType", "color"), &FTRender::setColor);
	ClassDB::bind_method(D_METHOD("getColor", "objType"), &FTRender::getColor);

    ClassDB::bind_method(D_METHOD("setCornerRadii", "cornerRadii"), &FTRender::setCornerRadii);
    ClassDB::bind_method(D_METHOD("getCornerRadii"), &FTRender::getCornerRadii);
	ClassDB::bind_method(D_METHOD("setCornerRadius", "objType", "cornerRadius"), &FTRender::setCornerRadius);
    ClassDB::bind_method(D_METHOD("getCornerRadius", "objType"), &FTRender::getCornerRadius);

    ClassDB::bind_method(D_METHOD("setBorderThicknesses", "borderThicknesses"), &FTRender::setBorderThicknesses);
    ClassDB::bind_method(D_METHOD("getBorderThicknesses"), &FTRender::getBorderThicknesses);
	ClassDB::bind_method(D_METHOD("setBorderThickness", "objType", "borderThickness"), &FTRender::setBorderThickness);
    ClassDB::bind_method(D_METHOD("getBorderThickness", "objType"), &FTRender::getBorderThickness);

    ADD_PROPERTY(PropertyInfo(Variant::PACKED_COLOR_ARRAY, "colors"), "setColors", "getColors");
    ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT64_ARRAY, "cornerRadii"), "setCornerRadii", "getCornerRadii");
    ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT64_ARRAY, "borderThicknesses"), "setBorderThicknesses", "getBorderThicknesses");

    shader.instantiate();
    shader->set_code(renderShader);

    mesh.instantiate();
}

Ref<Shader> FTRender::shader;
Ref<QuadMesh> FTRender::mesh;

void FTRender::setColors(PackedColorArray colors_) {
    colors = colors_;
}

PackedColorArray FTRender::getColors() {
    return colors;
}

void FTRender::setColor(ObjType objType, Color color) {
	colors.set(objType, color);
}

Color FTRender::getColor(ObjType objType) {
    return colors[objType];
}

void FTRender::setCornerRadii(PackedFloat64Array cornerRadii_) {
    cornerRadii = cornerRadii_;
}

PackedFloat64Array FTRender::getCornerRadii() {
    return cornerRadii;
}

void FTRender::setCornerRadius(ObjType objType, double cornerRadius) {
    cornerRadii.set(objType, cornerRadius);
}

double FTRender::getCornerRadius(ObjType objType) {
    return cornerRadii[objType];
}

void FTRender::setBorderThicknesses(PackedFloat64Array borderThicknesses_) {
    borderThicknesses = borderThicknesses_;
}

PackedFloat64Array FTRender::getBorderThicknesses() {
    return borderThicknesses;
}

void FTRender::setBorderThickness(ObjType objType, double borderThickness) {
    borderThicknesses.set(objType, borderThickness);
}

double FTRender::getBorderThickness(ObjType objType) {
    return borderThicknesses[objType];
}

FTRender::FTRender() {
    shaderMaterial.instantiate();
    shaderMaterial->set_shader(shader);

    for(auto layer : layers) {
        layer = memnew(MultiMeshInstance2D);
        add_child(layer);

        layer->set_material(shaderMaterial);

        Ref<MultiMesh> mm;
        mm.instantiate();
        mm->set_instance_count(LAYER_MULTIMESH_INSTANCE_COUNT);
        mm->set_visible_instance_count(0);

        layer->set_multimesh(mm);
    }
}
