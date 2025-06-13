#ifndef FTRENDER_H
#define FTRENDER_H

#include "core/object/ref_counted.h"
#include "scene/main/node.h"
#include "core/math/color.h"
#include "scene/resources/material.h"
#include "core/variant/variant.h"
#include "scene/2d/multimesh_instance_2d.h"
#include "scene/resources/mesh.h"
#include "scene/resources/3d/primitive_meshes.h"
#include "scene/resources/image_texture.h"

struct ObjType {
    enum Type : uint8_t {
        STATIC_RECT_BORDER, STATIC_RECT_INSIDE,
        STATIC_CIRC_BORDER, STATIC_CIRC_INSIDE,
        DYNAMIC_RECT_BORDER, DYNAMIC_RECT_INSIDE,
        DYNAMIC_CIRC_BORDER, DYNAMIC_CIRC_INSIDE,
        GP_RECT_BORDER, GP_RECT_INSIDE,
        GP_CIRC_BORDER, GP_CIRC_INSIDE,
        WOOD_BORDER, WOOD_INSIDE,
        WATER_BORDER, WATER_INSIDE,
        CW_BORDER, CW_INSIDE, CW_DECAL,
        CCW_BORDER, CCW_INSIDE, CCW_DECAL,
        UPW_BORDER, UPW_INSIDE, UPW_DECAL,
        BUILD_BORDER, BUILD_INSIDE,
        GOAL_BORDER, GOAL_INSIDE,
        JOINT_NORMAL, JOINT_WHEEL_CENTER,
        OBJ_TYPE_SIZE,
    };
};

struct PieceType {
    enum Type : uint8_t {
        STATIC_RECT, STATIC_CIRC, DYNAMIC_RECT, DYNAMIC_CIRC, GP_RECT, GP_CIRC, WOOD, WATER, CW, CCW, UPW, BUILD, GOAL, PIECE_TYPE_SIZE
    };
};

static const int LAYER_COUNT = 3;
static const int LAYER_MULTIMESH_INSTANCE_COUNT = 16384;
static const Vector2i LAYER_DATA_IMAGE_SIZE{128, 128};
static const float AA_WIDTH = 0.5;

static const float JOINT_RADIUS = 4;
static const float INNER_JOINT_THRESHOLD_RADIUS = 20;
static const Vector2 WOOD_SIZE_PADDING{-2, 2};
static const Vector2 WATER_SIZE_PADDING{-2, 6};
static const float GHOST_ROD_PADDING = 1;

struct RenderLayer {
    MultiMeshInstance2D* mmi;
    PackedVector2Array sizes;
    PackedFloat32Array rotations;
    PackedVector2Array poses;
    PackedVector2Array centers;
    Vector<ObjType::Type> objTypes;
    uint32_t layerID;

    int32_t renderCount = 0;
    void addRenderObject(Vector2 pos, Vector2 size, float rotation, Vector2 center, ObjType::Type type);

    void resetRender();

    void renderPartial(float scale, Vector2 shift, float aaWidth, Ref<Image>& renderImg);

    void init(MultiMeshInstance2D* mmi_, uint32_t layerID_);
};

class FTRender : public Node {
	GDCLASS(FTRender, Node);

protected:
	static void _bind_methods();

private:
    PackedColorArray colors;
    PackedFloat32Array cornerRadii;
    PackedFloat32Array borderThicknesses;

    static PackedColorArray getDefaultColors();
    static PackedFloat32Array getDefaultCornerRadii();
    static PackedFloat32Array getDefaultBorderThicknesses();

    static ObjType::Type pieceBorders[PieceType::PIECE_TYPE_SIZE];
    static ObjType::Type pieceInsides[PieceType::PIECE_TYPE_SIZE];
    static ObjType::Type pieceDecals[PieceType::PIECE_TYPE_SIZE];

    static void setupPieceBorders();
    static void setupPieceInsides();
    static void setupPieceDecals();
    static void setupPieceArrays();

    float scale = 1;
    Vector2 shift{0, 0};

    Ref<ShaderMaterial> shaderMaterial;

    void updateShaderColors();
    void updateShaderCornerRadii();
    void updateShaderBorderThicknesses();

    RenderLayer layers[LAYER_COUNT]; //0: areas, 1: borders, 2: insides

    Ref<Image> renderImg;
    Ref<ImageTexture> renderTex;
    void setupRenderData();

public:
    void setColors(PackedColorArray colors_);
    PackedColorArray getColors();

    void setColor(ObjType::Type objType, Color color);
    Color getColor(ObjType::Type objType);

    void setCornerRadii(PackedFloat32Array cornerRadii_);
    PackedFloat32Array getCornerRadii();

    void setCornerRadius(ObjType::Type objType, double cornerRadius);
    double getCornerRadius(ObjType::Type objType);

    void setBorderThicknesses(PackedFloat32Array borderThicknesses_);
    PackedFloat32Array getBorderThicknesses();

    void setBorderThickness(ObjType::Type objType, double borderThickness);
    double getBorderThickness(ObjType::Type objType);

    void setScale(float scale_);
    float getScale();

    void setShift(Vector2 shift_);
    Vector2 getShift();

    void resetRender();

    void render();

private:
    void addRoundedRect(Vector2 pos, Vector2 size, float rotation, PieceType::Type type,
            RenderLayer & borderLayer, RenderLayer & insideLayer);
    void addRoundedRectPiece(Vector2 pos, Vector2 size, float rotation, PieceType::Type type);
    void addArea(Vector2 pos, Vector2 size, float rotation, PieceType::Type type);
    void addCirclePiece(Vector2 pos, float radius, float rotation, PieceType::Type type);
    void addJoint(Vector2 pos, float rotation, ObjType::Type type);
    void addRectJoints(Vector2 pos, Vector2 size, float rotation);
    void addJointedRect(Vector2 pos, Vector2 size, float rotation, PieceType::Type type);
    void addRodJoints(Vector2 pos, Vector2 size, float rotation);
    void addJointedRod(Vector2 pos, Vector2 size, float rotation, PieceType::Type type);
    void addCircleJoints(Vector2 pos, float radius, float rotation, PieceType::Type type);
    void addJointedCircle(Vector2 pos, float radius, float rotation, PieceType::Type type);
    void addDecalCircle(Vector2 pos, float radius, float rotation, PieceType::Type type);

public:
    void addStaticRect(Vector2 pos, Vector2 size, float rotation);
    void addStaticCirc(Vector2 pos, float radius, float rotation);
    void addDynamicRect(Vector2 pos, Vector2 size, float rotation);
    void addDynamicCirc(Vector2 pos, float radius, float rotation);
    void addGPRect(Vector2 pos, Vector2 size, float rotation);
    void addGPCirc(Vector2 pos, float radius, float rotation);
    void addWood(Vector2 pos, Vector2 size, float rotation);
    void addWater(Vector2 pos, Vector2 size, float rotation);
    void addCW(Vector2 pos, float radius, float rotation);
    void addCCW(Vector2 pos, float radius, float rotation);
    void addUPW(Vector2 pos, float radius, float rotation);
    void addBuildArea(Vector2 pos, Vector2 size, float rotation);
    void addGoalArea(Vector2 pos, Vector2 size, float rotation);

    void init(Ref<ShaderMaterial> shaderMaterial_, MultiMeshInstance2D* mmiAreas, MultiMeshInstance2D* mmiBorders, MultiMeshInstance2D* mmiInsides);
	//FTRender();
};

VARIANT_ENUM_CAST(ObjType::Type);
VARIANT_ENUM_CAST(PieceType::Type);

#endif // FTRENDER_H