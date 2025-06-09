#ifndef FTRENDER_H
#define FTRENDER_H

#include "core/object/ref_counted.h"
#include "scene/main/node.h"
#include "core\math\color.h"
#include "scene\resources\material.h"
#include "core\variant\variant.h"
#include "scene\2d\multimesh_instance_2d.h"
#include "scene\resources\mesh.h"
#include "scene/resources/3d/primitive_meshes.h"

class FTRender : public Node {
	GDCLASS(FTRender, Node);

protected:
	static void _bind_methods();

public:
    enum ObjType {
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

    enum PieceType {
        STATIC_RECT, STATIC_CIRC, DYNAMIC_RECT, DYNAMIC_CIRC, GP_RECT, GP_CIRC, WOOD, WATER, CW, CCW, UPW, BUILD, GOAL, PIECE_TYPE_SIZE
    };

    static const int LAYER_COUNT = 3;
    static const int LAYER_MULTIMESH_INSTANCE_COUNT = 16384;

private:
    PackedColorArray colors;
    PackedFloat64Array cornerRadii;
    PackedFloat64Array borderThicknesses;

    static Ref<Shader> shader;
    Ref<ShaderMaterial> shaderMaterial;

    static Ref<QuadMesh> mesh;
    MultiMeshInstance2D* layers[LAYER_COUNT]; //0: areas, 1: borders, 2: insides

public:
    void setColors(PackedColorArray colors_);
    PackedColorArray getColors();

    void setColor(ObjType objType, Color color);
    Color getColor(ObjType objType);

    void setCornerRadii(PackedFloat64Array cornerRadii_);
    PackedFloat64Array getCornerRadii();

    void setCornerRadius(ObjType objType, double cornerRadius);
    double getCornerRadius(ObjType objType);

    void setBorderThicknesses(PackedFloat64Array borderThicknesses_);
    PackedFloat64Array getBorderThicknesses();

    void setBorderThickness(ObjType objType, double borderThickness);
    double getBorderThickness(ObjType objType);

	FTRender();
};

VARIANT_ENUM_CAST(FTRender::ObjType);
VARIANT_ENUM_CAST(FTRender::PieceType);

#endif // FTRENDER_H