#include "render.h"
#include "core/math/vector2.h"
#include "core/os/memory.h"
#include "scene/2d/sprite_2d.h"
#include "scene/resources/image_texture.h"
#include "core/string/print_string.h"
#include <cmath>
#include <cstdint>

float uintBitsToFloat(uint32_t u) {
	union Pun {
		uint32_t u;
		float f;
	};
	Pun p;
	p.u = u;
	return p.f;
}

// data layout:
// size: 48 bytes as fixed point | center: 48 bytes as fixed point | type: 32 bytes as enum (lots of space to spare)
Color packDataToColor(Vector2 size, Vector2 center, ObjType::Type type) {
	uint32_t sizeX = std::lroundf(size.x * 16), sizeY = std::lroundf(size.y * 16);
	uint32_t centerX = std::lroundf(center.x * 16), centerY = std::lroundf(center.y * 16);
	uint32_t i1 = sizeX << 8 | sizeY >> 16;
	uint32_t i2 = sizeY << 16 | centerX >> 8;
	uint32_t i3 = centerX << 24 | centerY;
	uint32_t i4 = static_cast<uint32_t>(type);
	return Color{ uintBitsToFloat(i1), uintBitsToFloat(i2), uintBitsToFloat(i3), uintBitsToFloat(i4) };
}

void RenderLayer::addRenderObject(Vector2 pos, Vector2 size, float rotation, Vector2 center, ObjType::Type type) {
	ERR_FAIL_COND_MSG(renderCount >= LAYER_MULTIMESH_INSTANCE_COUNT, "Too many objects to render!");
	sizes.set(renderCount, size);
	rotations.set(renderCount, rotation);
	poses.set(renderCount, pos);
	centers.set(renderCount, center);
	objTypes.set(renderCount, type);

	renderCount++;
}

void RenderLayer::resetRender() {
	renderCount = 0;
}

void RenderLayer::renderPartial(float scale, Vector2 shift, float aaWidth, Ref<Image>& renderImg) {
	Ref<MultiMesh> mm = mmi->get_multimesh();
	mmi->set_instance_shader_parameter("scale", scale);
	mmi->set_instance_shader_parameter("aaWidth", aaWidth);
	mm->set_visible_instance_count(renderCount);
	for (int i = 0; i < renderCount; i++) {
		Transform2D transform(rotations[i],
				(sizes[i] + Vector2{ aaWidth, aaWidth }) * scale,
				0,
				poses[i] * scale + shift);
		mm->set_instance_transform_2d(i, transform);
		Color data = packDataToColor(sizes[i] * scale, centers[i] * scale, objTypes[i]);
		renderImg->set_pixel(i % 128 + layerID * 128, i / 128, data);
	}
}

void RenderLayer::init(MultiMeshInstance2D *mmi_, uint32_t layerID_) {
    layerID = layerID_;
	mmi = mmi_;
	mmi->set_instance_shader_parameter("layerID", layerID_);

    sizes.resize(LAYER_MULTIMESH_INSTANCE_COUNT);
    rotations.resize(LAYER_MULTIMESH_INSTANCE_COUNT);
    poses.resize(LAYER_MULTIMESH_INSTANCE_COUNT);
    centers.resize(LAYER_MULTIMESH_INSTANCE_COUNT);
    objTypes.resize(LAYER_MULTIMESH_INSTANCE_COUNT);
}

void FTRender::_bind_methods() {
	BIND_ENUM_CONSTANT(ObjType::STATIC_RECT_BORDER);
	BIND_ENUM_CONSTANT(ObjType::STATIC_RECT_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::STATIC_CIRC_BORDER);
	BIND_ENUM_CONSTANT(ObjType::STATIC_CIRC_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::DYNAMIC_RECT_BORDER);
	BIND_ENUM_CONSTANT(ObjType::DYNAMIC_RECT_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::DYNAMIC_CIRC_BORDER);
	BIND_ENUM_CONSTANT(ObjType::DYNAMIC_CIRC_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::GP_RECT_BORDER);
	BIND_ENUM_CONSTANT(ObjType::GP_RECT_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::GP_CIRC_BORDER);
	BIND_ENUM_CONSTANT(ObjType::GP_CIRC_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::WOOD_BORDER);
	BIND_ENUM_CONSTANT(ObjType::WOOD_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::WATER_BORDER);
	BIND_ENUM_CONSTANT(ObjType::WATER_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::CW_BORDER);
	BIND_ENUM_CONSTANT(ObjType::CW_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::CW_DECAL);
	BIND_ENUM_CONSTANT(ObjType::CCW_BORDER);
	BIND_ENUM_CONSTANT(ObjType::CCW_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::CCW_DECAL);
	BIND_ENUM_CONSTANT(ObjType::UPW_BORDER);
	BIND_ENUM_CONSTANT(ObjType::UPW_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::UPW_DECAL);
	BIND_ENUM_CONSTANT(ObjType::BUILD_BORDER);
	BIND_ENUM_CONSTANT(ObjType::BUILD_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::GOAL_BORDER);
	BIND_ENUM_CONSTANT(ObjType::GOAL_INSIDE);
	BIND_ENUM_CONSTANT(ObjType::JOINT_NORMAL);
	BIND_ENUM_CONSTANT(ObjType::JOINT_WHEEL_CENTER);
	BIND_ENUM_CONSTANT(ObjType::OBJ_TYPE_SIZE);

	BIND_ENUM_CONSTANT(PieceType::STATIC_RECT);
	BIND_ENUM_CONSTANT(PieceType::STATIC_CIRC);
	BIND_ENUM_CONSTANT(PieceType::DYNAMIC_RECT);
	BIND_ENUM_CONSTANT(PieceType::DYNAMIC_CIRC);
	BIND_ENUM_CONSTANT(PieceType::GP_RECT);
	BIND_ENUM_CONSTANT(PieceType::GP_CIRC);
	BIND_ENUM_CONSTANT(PieceType::WOOD);
	BIND_ENUM_CONSTANT(PieceType::WATER);
	BIND_ENUM_CONSTANT(PieceType::CW);
	BIND_ENUM_CONSTANT(PieceType::CCW);
	BIND_ENUM_CONSTANT(PieceType::UPW);
	BIND_ENUM_CONSTANT(PieceType::BUILD);
	BIND_ENUM_CONSTANT(PieceType::GOAL);
	BIND_ENUM_CONSTANT(PieceType::PIECE_TYPE_SIZE);

	BIND_CONSTANT(LAYER_COUNT);
	BIND_CONSTANT(LAYER_MULTIMESH_INSTANCE_COUNT);

	ClassDB::bind_method(D_METHOD("setColors", "colors"), &FTRender::setColors);
	ClassDB::bind_method(D_METHOD("getColors"), &FTRender::getColors);
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

	ClassDB::bind_method(D_METHOD("setScale", "scale"), &FTRender::setScale);
	ClassDB::bind_method(D_METHOD("getScale"), &FTRender::getScale);

	ClassDB::bind_method(D_METHOD("setShift", "shift"), &FTRender::setShift);
	ClassDB::bind_method(D_METHOD("getShift"), &FTRender::getShift);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale"), "setScale", "getScale");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "shift"), "setShift", "getShift");

	ClassDB::bind_method(D_METHOD("resetRender"), &FTRender::resetRender);
	ClassDB::bind_method(D_METHOD("render"), &FTRender::render);

	ClassDB::bind_method(D_METHOD("addStaticRect", "pos", "size", "rotation"), &FTRender::addStaticRect);
	ClassDB::bind_method(D_METHOD("addStaticCirc", "pos", "radius", "rotation"), &FTRender::addStaticCirc);
	ClassDB::bind_method(D_METHOD("addDynamicRect", "pos", "size", "rotation"), &FTRender::addDynamicRect);
	ClassDB::bind_method(D_METHOD("addDynamicCirc", "pos", "radius", "rotation"), &FTRender::addDynamicCirc);
	ClassDB::bind_method(D_METHOD("addGPRect", "pos", "size", "rotation"), &FTRender::addGPRect);
	ClassDB::bind_method(D_METHOD("addGPCirc", "pos", "radius", "rotation"), &FTRender::addGPCirc);
	ClassDB::bind_method(D_METHOD("addWood", "pos", "size", "rotation"), &FTRender::addWood);
	ClassDB::bind_method(D_METHOD("addWater", "pos", "size", "rotation"), &FTRender::addWater);
	ClassDB::bind_method(D_METHOD("addCW", "pos", "radius", "rotation"), &FTRender::addCW);
	ClassDB::bind_method(D_METHOD("addCCW", "pos", "radius", "rotation"), &FTRender::addCCW);
	ClassDB::bind_method(D_METHOD("addUPW", "pos", "radius", "rotation"), &FTRender::addUPW);
	ClassDB::bind_method(D_METHOD("addBuildArea", "pos", "size", "rotation"), &FTRender::addBuildArea);
	ClassDB::bind_method(D_METHOD("addGoalArea", "pos", "size", "rotation"), &FTRender::addGoalArea);

	ClassDB::bind_method(D_METHOD("init", "shaderMaterial", "mmiAreas", "mmiBorders", "mmiInsides"), &FTRender::init);

    setupPieceArrays();
}

PackedColorArray FTRender::getDefaultColors() {
	PackedColorArray colors;
	colors.resize(ObjType::OBJ_TYPE_SIZE);
	colors.set(ObjType::STATIC_RECT_BORDER, Color("#008009"));
	colors.set(ObjType::STATIC_RECT_INSIDE, Color("#00be01"));
	colors.set(ObjType::STATIC_CIRC_BORDER, Color("#008009"));
	colors.set(ObjType::STATIC_CIRC_INSIDE, Color("#00be01"));
	colors.set(ObjType::DYNAMIC_RECT_BORDER, Color("#c6560c"));
	colors.set(ObjType::DYNAMIC_RECT_INSIDE, Color("#f9da2f"));
	colors.set(ObjType::DYNAMIC_CIRC_BORDER, Color("#c6560c"));
	colors.set(ObjType::DYNAMIC_CIRC_INSIDE, Color("#f9da2f"));
	colors.set(ObjType::GP_RECT_BORDER, Color("#bb6666"));
	colors.set(ObjType::GP_RECT_INSIDE, Color("#ff6666"));
	colors.set(ObjType::GP_CIRC_BORDER, Color("#bb6666"));
	colors.set(ObjType::GP_CIRC_INSIDE, Color("#ff6666"));
	colors.set(ObjType::WOOD_BORDER, Color("#b55900"));
	colors.set(ObjType::WOOD_INSIDE, Color("#6b3400"));
	colors.set(ObjType::WATER_BORDER, Color("#ffffff"));
	colors.set(ObjType::WATER_INSIDE, Color("#0000ff"));
	colors.set(ObjType::CW_BORDER, Color("#fc8003"));
	colors.set(ObjType::CW_INSIDE, Color("#ffec00"));
	colors.set(ObjType::CW_DECAL, Color("#fc8003"));
	colors.set(ObjType::CCW_BORDER, Color("#d147a5"));
	colors.set(ObjType::CCW_INSIDE, Color("#ffcccc"));
	colors.set(ObjType::CCW_DECAL, Color("#d147a5"));
	colors.set(ObjType::UPW_BORDER, Color("#0a69fd"));
	colors.set(ObjType::UPW_INSIDE, Color("#89fae3"));
	colors.set(ObjType::UPW_DECAL, Color("#4a69fd"));
	colors.set(ObjType::BUILD_BORDER, Color("#7777ee"));
	colors.set(ObjType::BUILD_INSIDE, Color("#bcdbf9"));
	colors.set(ObjType::GOAL_BORDER, Color("#bb6666"));
	colors.set(ObjType::GOAL_INSIDE, Color("#f19191"));
	colors.set(ObjType::JOINT_NORMAL, Color("#838383"));
	colors.set(ObjType::JOINT_WHEEL_CENTER, Color("#ffffff"));
	return colors;
}

PackedFloat32Array FTRender::getDefaultCornerRadii() {
	PackedFloat32Array cornerRadii;
	cornerRadii.resize(ObjType::OBJ_TYPE_SIZE);
	cornerRadii.set(ObjType::STATIC_RECT_BORDER, 3);
	cornerRadii.set(ObjType::DYNAMIC_RECT_BORDER, 3);
	cornerRadii.set(ObjType::GP_RECT_BORDER, 3);
	cornerRadii.set(ObjType::WOOD_BORDER, 2);
	cornerRadii.set(ObjType::WATER_BORDER, 2);
	cornerRadii.set(ObjType::BUILD_BORDER, 2);
	cornerRadii.set(ObjType::GOAL_BORDER, 2);
	return cornerRadii;
}

PackedFloat32Array FTRender::getDefaultBorderThicknesses() {
	PackedFloat32Array borderThicknesses;
    borderThicknesses.resize(ObjType::OBJ_TYPE_SIZE);
	borderThicknesses.set(ObjType::STATIC_RECT_BORDER, 4);
	borderThicknesses.set(ObjType::STATIC_CIRC_BORDER, 4);
	borderThicknesses.set(ObjType::DYNAMIC_RECT_BORDER, 4);
	borderThicknesses.set(ObjType::DYNAMIC_CIRC_BORDER, 4);
	borderThicknesses.set(ObjType::GP_RECT_BORDER, 4);
	borderThicknesses.set(ObjType::GP_CIRC_BORDER, 4);
	borderThicknesses.set(ObjType::WOOD_BORDER, 3);
	borderThicknesses.set(ObjType::WATER_BORDER, 3);
	borderThicknesses.set(ObjType::BUILD_BORDER, 4);
	borderThicknesses.set(ObjType::GOAL_BORDER, 4);
	borderThicknesses.set(ObjType::CW_BORDER, 4);
	borderThicknesses.set(ObjType::CCW_BORDER, 4);
	borderThicknesses.set(ObjType::UPW_BORDER, 4);
	borderThicknesses.set(ObjType::JOINT_NORMAL, 2);
	borderThicknesses.set(ObjType::JOINT_WHEEL_CENTER, 2);
	return borderThicknesses;
}

ObjType::Type FTRender::pieceBorders[PieceType::PIECE_TYPE_SIZE];
ObjType::Type FTRender::pieceInsides[PieceType::PIECE_TYPE_SIZE];
ObjType::Type FTRender::pieceDecals[PieceType::PIECE_TYPE_SIZE];

void FTRender::setupPieceBorders() {
	pieceBorders[PieceType::STATIC_RECT] = ObjType::STATIC_RECT_BORDER;
	pieceBorders[PieceType::STATIC_CIRC] = ObjType::STATIC_CIRC_BORDER;
	pieceBorders[PieceType::DYNAMIC_RECT] = ObjType::DYNAMIC_RECT_BORDER;
	pieceBorders[PieceType::DYNAMIC_CIRC] = ObjType::DYNAMIC_CIRC_BORDER;
	pieceBorders[PieceType::GP_RECT] = ObjType::GP_RECT_BORDER;
	pieceBorders[PieceType::GP_CIRC] = ObjType::GP_CIRC_BORDER;
	pieceBorders[PieceType::WOOD] = ObjType::WOOD_BORDER;
	pieceBorders[PieceType::WATER] = ObjType::WATER_BORDER;
	pieceBorders[PieceType::CW] = ObjType::CW_BORDER;
	pieceBorders[PieceType::CCW] = ObjType::CCW_BORDER;
	pieceBorders[PieceType::UPW] = ObjType::UPW_BORDER;
	pieceBorders[PieceType::BUILD] = ObjType::BUILD_BORDER;
	pieceBorders[PieceType::GOAL] = ObjType::GOAL_BORDER;
}

void FTRender::setupPieceInsides() {
	pieceInsides[PieceType::STATIC_RECT] = ObjType::STATIC_RECT_INSIDE;
	pieceInsides[PieceType::STATIC_CIRC] = ObjType::STATIC_CIRC_INSIDE;
	pieceInsides[PieceType::DYNAMIC_RECT] = ObjType::DYNAMIC_RECT_INSIDE;
	pieceInsides[PieceType::DYNAMIC_CIRC] = ObjType::DYNAMIC_CIRC_INSIDE;
	pieceInsides[PieceType::GP_RECT] = ObjType::GP_RECT_INSIDE;
	pieceInsides[PieceType::GP_CIRC] = ObjType::GP_CIRC_INSIDE;
	pieceInsides[PieceType::WOOD] = ObjType::WOOD_INSIDE;
	pieceInsides[PieceType::WATER] = ObjType::WATER_INSIDE;
	pieceInsides[PieceType::CW] = ObjType::CW_INSIDE;
	pieceInsides[PieceType::CCW] = ObjType::CCW_INSIDE;
	pieceInsides[PieceType::UPW] = ObjType::UPW_INSIDE;
	pieceInsides[PieceType::BUILD] = ObjType::BUILD_INSIDE;
	pieceInsides[PieceType::GOAL] = ObjType::GOAL_INSIDE;
}

void FTRender::setupPieceDecals() {
	pieceDecals[PieceType::CW] = ObjType::CW_DECAL;
	pieceDecals[PieceType::CCW] = ObjType::CCW_DECAL;
	pieceDecals[PieceType::UPW] = ObjType::UPW_DECAL;
}

void FTRender::setupPieceArrays() {
	setupPieceBorders();
	setupPieceInsides();
	setupPieceDecals();
}

void FTRender::updateShaderColors() {
	shaderMaterial->set_shader_parameter("colorsGlobal", colors);
}

void FTRender::updateShaderCornerRadii() {
	shaderMaterial->set_shader_parameter("cornerRadiiGlobal", cornerRadii);
}

void FTRender::updateShaderBorderThicknesses() {
	shaderMaterial->set_shader_parameter("borderThicknessesGlobal", borderThicknesses);
}

void FTRender::setupRenderData() {
	renderImg = Image::create_empty(LAYER_DATA_IMAGE_SIZE.x * LAYER_COUNT, LAYER_DATA_IMAGE_SIZE.y,
			false, Image::FORMAT_RGBAF);
    renderTex = ImageTexture::create_from_image(renderImg);
}

void FTRender::setColors(PackedColorArray colors_) {
	colors = colors_;
}

PackedColorArray FTRender::getColors() {
	return colors;
}

void FTRender::setColor(ObjType::Type objType, Color color) {
	colors.set(objType, color);
}

Color FTRender::getColor(ObjType::Type objType) {
	return colors[objType];
}

void FTRender::setCornerRadii(PackedFloat32Array cornerRadii_) {
	cornerRadii = cornerRadii_;
}

PackedFloat32Array FTRender::getCornerRadii() {
	return cornerRadii;
}

void FTRender::setCornerRadius(ObjType::Type objType, double cornerRadius) {
	cornerRadii.set(objType, cornerRadius);
}

double FTRender::getCornerRadius(ObjType::Type objType) {
	return cornerRadii[objType];
}

void FTRender::setBorderThicknesses(PackedFloat32Array borderThicknesses_) {
	borderThicknesses = borderThicknesses_;
}

PackedFloat32Array FTRender::getBorderThicknesses() {
	return borderThicknesses;
}

void FTRender::setBorderThickness(ObjType::Type objType, double borderThickness) {
	borderThicknesses.set(objType, borderThickness);
}

double FTRender::getBorderThickness(ObjType::Type objType) {
	return borderThicknesses[objType];
}

void FTRender::setScale(float scale_) {
	scale = scale_;
}

float FTRender::getScale() {
	return scale;
}

void FTRender::setShift(Vector2 shift_) {
	shift = shift_;
}

Vector2 FTRender::getShift() {
	return shift;
}

void FTRender::resetRender() {
	for (auto &layer : layers) {
		layer.resetRender();
	}
}

void FTRender::render() {
	for (int i = 0; i < LAYER_COUNT; i++) {
		layers[i].renderPartial(scale, shift, AA_WIDTH, renderImg);
	}
    renderTex->update(renderImg);
	shaderMaterial->set_shader_parameter("data", renderTex);
}

float getRealInsideSize(float size, float borderThickness) {
	return abs(size - 2 * borderThickness);
}

float getRealBorderSize(float size, float insideSize) {
	return MAX(size, insideSize + GHOST_ROD_PADDING * 2);
}

void FTRender::addRoundedRect(Vector2 pos, Vector2 size, float rotation, PieceType::Type type,
		RenderLayer &borderLayer, RenderLayer &insideLayer) {
	ObjType::Type borderType = pieceBorders[type];
	ObjType::Type insideType = pieceInsides[type];
	Vector2 borderThickness{ borderThicknesses[borderType], borderThicknesses[borderType] };
	Vector2 insideSize{ getRealInsideSize(size.x, borderThickness.x), getRealInsideSize(size.y, borderThickness.y) };
	Vector2 borderSize{ getRealBorderSize(size.x, insideSize.x), getRealBorderSize(size.y, insideSize.y) };
	borderLayer.addRenderObject(pos, borderSize, rotation, borderSize * 0.5, borderType);
	insideLayer.addRenderObject(pos, insideSize, rotation, insideSize * 0.5, insideType);
}

void FTRender::addRoundedRectPiece(Vector2 pos, Vector2 size, float rotation, PieceType::Type type) {
	addRoundedRect(pos, size, rotation, type, layers[1], layers[2]);
}

void FTRender::addArea(Vector2 pos, Vector2 size, float rotation, PieceType::Type type) {
	addRoundedRect(pos, size, rotation, type, layers[0], layers[0]);
}

void FTRender::addCirclePiece(Vector2 pos, float radius, float rotation, PieceType::Type type) {
	addRoundedRect(pos, Vector2{ radius, radius } * 2, rotation, type, layers[1], layers[2]);
}

void FTRender::addJoint(Vector2 pos, float rotation, ObjType::Type type) {
	static const Vector2 size = Vector2{ JOINT_RADIUS, JOINT_RADIUS } * 2;
	layers[2].addRenderObject(pos, size, rotation, size * 0.5, type);
}

void FTRender::addRectJoints(Vector2 pos, Vector2 size, float rotation) {
	addJoint(pos, 0, ObjType::JOINT_WHEEL_CENTER);
	addJoint(Vector2(size.x * 0.5, size.y * 0.5).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
	addJoint(Vector2(-size.x * 0.5, size.y * 0.5).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
	addJoint(Vector2(size.x * 0.5, -size.y * 0.5).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
	addJoint(Vector2(-size.x * 0.5, -size.y * 0.5).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
}

void FTRender::addJointedRect(Vector2 pos, Vector2 size, float rotation, PieceType::Type type) {
	addRoundedRectPiece(pos, size, rotation, type);
	addRectJoints(pos, size, rotation);
}

void FTRender::addRodJoints(Vector2 pos, Vector2 size, float rotation) {
	addJoint(Vector2(size.x * 0.5, 0).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
	addJoint(Vector2(-size.x * 0.5, 0).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
}

void FTRender::addJointedRod(Vector2 pos, Vector2 size, float rotation, PieceType::Type type) {
	addRoundedRectPiece(pos, size, rotation, type);
	addRodJoints(pos, size, rotation);
}

void FTRender::addCircleJoints(Vector2 pos, float radius, float rotation, PieceType::Type type) {
	addJoint(pos, 0, ObjType::JOINT_WHEEL_CENTER);
	addJoint(Vector2(radius, 0).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
	addJoint(Vector2(-radius, 0).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
	addJoint(Vector2(0, radius).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
	addJoint(Vector2(0, -radius).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
	if (radius > INNER_JOINT_THRESHOLD_RADIUS) {
		addJoint(Vector2(INNER_JOINT_THRESHOLD_RADIUS, 0).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
		addJoint(Vector2(-INNER_JOINT_THRESHOLD_RADIUS, 0).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
		addJoint(Vector2(0, INNER_JOINT_THRESHOLD_RADIUS).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
		addJoint(Vector2(0, -INNER_JOINT_THRESHOLD_RADIUS).rotated(rotation) + pos, 0, ObjType::JOINT_NORMAL);
	}
}

void FTRender::addJointedCircle(Vector2 pos, float radius, float rotation, PieceType::Type type) {
	addCirclePiece(pos, radius, rotation, type);
	addCircleJoints(pos, radius, rotation, type);
}

void FTRender::addDecalCircle(Vector2 pos, float radius, float rotation, PieceType::Type type) {
	addCirclePiece(pos, radius, rotation, type);
	ObjType::Type borderType = pieceBorders[type];
	ObjType::Type decalType = pieceDecals[type];
	float borderThickness = borderThicknesses[borderType];
	float insideDiameter = getRealInsideSize(radius * 2, borderThickness);
	Vector2 insideSize{ insideDiameter, insideDiameter };
	layers[2].addRenderObject(pos, insideSize, rotation, insideSize * 0.5, decalType);
	addCircleJoints(pos, radius, rotation, type);
}

void FTRender::addStaticRect(Vector2 pos, Vector2 size, float rotation) {
	addRoundedRectPiece(pos, size, rotation, PieceType::STATIC_RECT);
}

void FTRender::addStaticCirc(Vector2 pos, float radius, float rotation) {
	addCirclePiece(pos, radius, rotation, PieceType::STATIC_CIRC);
}

void FTRender::addDynamicRect(Vector2 pos, Vector2 size, float rotation) {
	addRoundedRectPiece(pos, size, rotation, PieceType::DYNAMIC_RECT);
}

void FTRender::addDynamicCirc(Vector2 pos, float radius, float rotation) {
	addCirclePiece(pos, radius, rotation, PieceType::DYNAMIC_CIRC);
}

void FTRender::addGPRect(Vector2 pos, Vector2 size, float rotation) {
	addJointedRect(pos, size, rotation, PieceType::GP_RECT);
}

void FTRender::addGPCirc(Vector2 pos, float radius, float rotation) {
	addJointedCircle(pos, radius, rotation, PieceType::GP_CIRC);
}

void FTRender::addWood(Vector2 pos, Vector2 size, float rotation) {
	addRoundedRectPiece(pos, size + WOOD_SIZE_PADDING, rotation, PieceType::WOOD);
	addRodJoints(pos, size, rotation);
}

void FTRender::addWater(Vector2 pos, Vector2 size, float rotation) {
	addRoundedRectPiece(pos, size + WATER_SIZE_PADDING, rotation, PieceType::WATER);
	addRodJoints(pos, size, rotation);
}

void FTRender::addCW(Vector2 pos, float radius, float rotation) {
	addDecalCircle(pos, radius, rotation, PieceType::CW);
}

void FTRender::addCCW(Vector2 pos, float radius, float rotation) {
	addDecalCircle(pos, radius, rotation, PieceType::CCW);
}

void FTRender::addUPW(Vector2 pos, float radius, float rotation) {
	addDecalCircle(pos, radius, rotation, PieceType::UPW);
}

void FTRender::addBuildArea(Vector2 pos, Vector2 size, float rotation) {
	addArea(pos, size, rotation, PieceType::BUILD);
}

void FTRender::addGoalArea(Vector2 pos, Vector2 size, float rotation) {
	addArea(pos, size, rotation, PieceType::GOAL);
}

void FTRender::init(Ref<ShaderMaterial> shaderMaterial_, MultiMeshInstance2D* mmiAreas, MultiMeshInstance2D* mmiBorders, MultiMeshInstance2D* mmiInsides) {
    colors = getDefaultColors();
	cornerRadii = getDefaultCornerRadii();
	borderThicknesses = getDefaultBorderThicknesses();

	shaderMaterial = shaderMaterial_;

	updateShaderColors();
	updateShaderCornerRadii();
	updateShaderBorderThicknesses();

    layers[0].init(mmiAreas, 0);
    layers[1].init(mmiBorders, 1);
    layers[2].init(mmiInsides, 2);

    setupRenderData();
}

// FTRender::FTRender() {
// 	colors = getDefaultColors();
// 	cornerRadii = getDefaultCornerRadii();
// 	borderThicknesses = getDefaultBorderThicknesses();

// 	shaderMaterial.instantiate();
// 	shaderMaterial->set_shader(shader);

// 	updateShaderColors();
// 	updateShaderCornerRadii();
// 	updateShaderBorderThicknesses();

// 	for (int i = 0; i < LAYER_COUNT; i++) {
// 		MultiMeshInstance2D *mmi = memnew(MultiMeshInstance2D);
// 		add_child(mmi);
// 		mmi->set_material(shaderMaterial);

// 		Ref<MultiMesh> mm;
// 		mm.instantiate();
// 		mm->set_instance_count(LAYER_MULTIMESH_INSTANCE_COUNT);
// 		mm->set_visible_instance_count(0);
//         mm->set_mesh(mesh);

// 		mmi->set_multimesh(mm);

// 		layers[i].init(mmi, i);
// 	}

// 	setupRenderDataArr();
// }
