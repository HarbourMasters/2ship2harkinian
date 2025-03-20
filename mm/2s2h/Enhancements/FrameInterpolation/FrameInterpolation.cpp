#include <libultraship/bridge.h>

#include <vector>
#include <map>
#include <unordered_map>
#include <math.h>
#include <cmath>

#include "FrameInterpolation.h"
#include "2s2h/BenPort.h"


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

/*
Frame interpolation.

The idea of this code is to interpolate all matrices.

The code contains two approaches. The first is to interpolate
all inputs in transformations, such as angles, scale and distances,
and then perform the same transformations with the interpolated values.
After evaluation for some reason some animations such rolling look strange.

The second approach is to simply interpolate the final matrices. This will
more or less simply interpolate the world coordinates for movements.
This will however make rotations ~180 degrees get the "paper effect".
The mitigation is to identify this case for actors and interpolate the
matrix but in model coordinates instead, by "removing" the rotation-
translation before interpolating, create a rotation matrix with the
interpolated angle which is then applied to the matrix.

Currently the code contains both methods but only the second one is currently
used.

Both approaches build a tree of instructions, containing matrices
at leaves. Every node is built from OPEN_DISPS/CLOSE_DISPS and manually
inserted FrameInterpolation_OpenChild/FrameInterpolation_Close child calls.
These nodes contain information that should suffice to identify the matrix,
so we can find it in an adjacent frame.

We can interpolate an arbitrary amount of frames between two original frames,
given a specific interpolation factor (0=old frame, 0.5=average of frames,
1.0=new frame).
*/

extern "C" {

void Matrix_Init(struct GameState* gameState);
void Matrix_Push(void);
void Matrix_Pop(void);
void Matrix_Get(MtxF* dest);
void Matrix_Put(MtxF* src);
void Matrix_Mult(MtxF* mf, u8 mode);
void Matrix_Translate(f32 x, f32 y, f32 z, u8 mode);
void Matrix_Scale(f32 x, f32 y, f32 z, u8 mode);
void Matrix_RotateXF(f32 x, u8 mode);
void Matrix_RotateYF(f32 y, u8 mode);
void Matrix_RotateZF(f32 z, u8 mode);
void Matrix_RotateZYX(s16 x, s16 y, s16 z, u8 mode);
void Matrix_TranslateRotateZYX(Vec3f* translation, Vec3s* rotation);
void Matrix_SetTranslateRotateYXZ(f32 translateX, f32 translateY, f32 translateZ, Vec3s* rot);
Mtx* Matrix_MtxFToMtx(MtxF* src, Mtx* dest, bool isViewMtx);
Mtx* Matrix_ToMtx(Mtx* dest, char* file, s32 line);
Mtx* Matrix_NewMtx(struct GraphicsContext* gfxCtx, char* file, s32 line);
Mtx* Matrix_MtxFToNewMtx(MtxF* src, struct GraphicsContext* gfxCtx);
void Matrix_MultVec3f(Vec3f* src, Vec3f* dest);
void Matrix_MtxFCopy(MtxF* dest, MtxF* src);
void Matrix_MtxToMtxF(Mtx* src, MtxF* dest);
void Matrix_MultVec3fExt(Vec3f* src, Vec3f* dest, MtxF* mf);
void Matrix_Transpose(MtxF* mf);
void Matrix_ReplaceRotation(MtxF* mf);
void Matrix_MtxFToYXZRotS(MtxF* mf, Vec3s* rotDest, s32 flag);
void Matrix_MtxFToZYXRotS(MtxF* mf, Vec3s* rotDest, s32 flag);
void Matrix_RotateAxisF(f32 angle, Vec3f* axis, u8 mode);
MtxF* Matrix_CheckFloats(MtxF* mf, char* file, s32 line);
void Matrix_SetTranslateScaleMtx2(Mtx* mtx, f32 scaleX, f32 scaleY, f32 scaleZ, f32 translateX, f32 translateY,
                                  f32 translateZ);

MtxF* Matrix_GetCurrent(void);
void guMtxIdentF(f32 mf[4][4]);

void SkinMatrix_MtxFMtxFMult(MtxF* mfA, MtxF* mfB, MtxF* dest);
}

static bool invert_matrix(const float m[16], float invOut[16]);

using namespace std;

namespace {

enum class Op {
    OpenChild,
    CloseChild,

    MatrixPush,
    MatrixPop,
    MatrixPut,
    MatrixMult,
    MatrixTranslate,
    MatrixScale,
    MatrixRotate1Coord,
    MatrixRotateZYX,
    MatrixTranslateRotateZYX,
    MatrixSetTranslateRotateYXZ,
    MatrixMtxFToMtx,
    MatrixToMtx,
    MatrixReplaceRotation,
    MatrixRotateAxis,
    SkinMatrixMtxFToMtx
};

typedef pair<const void*, int> label;

union Data {
    Data() {
    }

    struct {
        MtxF src;
    } matrix_put;

    struct {
        MtxF mf;
        u8 mode;
    } matrix_mult;

    struct {
        f32 x, y, z;
        u8 mode;
    } matrix_translate, matrix_scale;

    struct {
        u32 coord;
        f32 value;
        u8 mode;
    } matrix_rotate_1_coord;

    struct {
        s16 x, y, z;
        u8 mode;
    } matrix_rotate_zyx;

    struct {
        Vec3f translation;
        Vec3s rotation;
    } matrix_translate_rotate_zyx;

    struct {
        f32 translateX, translateY, translateZ;
        Vec3s rot;
        // MtxF mtx;
        bool has_mtx;
        bool interpolate_wider_angles;
    } matrix_set_translate_rotate_yxz;

    struct {
        MtxF src;
		Mtx* dest;
		bool isViewMtx;
    } matrix_mtxf_to_mtx;

    struct {
        Mtx* dest;
        MtxF src;
        bool has_adjusted;
    } matrix_to_mtx;

    struct {
        MtxF mf;
    } matrix_replace_rotation;

    struct {
        f32 angle;
        Vec3f axis;
        u8 mode;
    } matrix_rotate_axis;

    struct {
        label key;
        size_t idx;
    } open_child;
};

struct Path {
    map<label, vector<Path>> children;
    map<Op, vector<Data>> ops;
    vector<pair<Op, size_t>> items;
};

struct Recording {
    Path root_path;
};

bool is_recording;
vector<Path*> current_path;
uint32_t camera_epoch;
uint32_t previous_camera_epoch;
Recording current_recording;
Recording previous_recording;

bool interpolate_wider_angles = false;

bool next_is_actor_pos_rot_matrix;
bool has_inv_actor_mtx;
bool ignore_inv_actor_mtx;
size_t ignore_inv_actor_mtx_path_index;
MtxF inv_actor_mtx;
size_t inv_actor_mtx_path_index;

Data& append(Op op) {
    auto& m = current_path.back()->ops[op];
    current_path.back()->items.emplace_back(op, m.size());
    return m.emplace_back();
}

struct InterpolateCtx {
    float step;
    float w;
    unordered_map<Mtx*, MtxF> mtx_replacements;
    MtxF tmp_mtxf, tmp_mtxf2;
    Vec3f tmp_vec3f;
    Vec3s tmp_vec3s;
    MtxF actor_mtx;

    MtxF* new_replacement(Mtx* addr) {
        return &mtx_replacements[addr];
    }

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

	static void combine(float* a, float* b, float* res, float a1, float b1)
	{
		res[0] = (a[0] * a1) + (b[0] * b1);
		res[1] = (a[1] * a1) + (b[1] * b1);
		res[2] = (a[2] * a1) + (b[2] * b1);
	}

	static glm::mat4x4 ComposeRotationMatrix(glm::quat& quaternion)
	{
		return glm::toMat4(quaternion);
	}

	static MtxF ComposeScaleMatrix(glm::vec3* scales)
	{
		MtxF scaleMtx;

		memset(&scaleMtx, 0.0f, 4 * 4 * sizeof(float));
		scaleMtx.mf[0][0] = scales->x;
		scaleMtx.mf[1][1] = scales->y;
		scaleMtx.mf[2][2] = scales->z;
		scaleMtx.mf[3][3] = 1.0f;

		return scaleMtx;
	}

	static void ComposePerspective(MtxF* m, glm::vec4* perspective)
	{
		m->mf[0][3] = perspective->x;
		m->mf[1][3] = perspective->y;
		m->mf[2][3] = perspective->z;
		m->mf[3][3] = perspective->w;
	}

	static void ComposeTranslation(MtxF* m, glm::vec3* translate)
	{
		m->mf[3][0] = translate->x;
		m->mf[3][1] = translate->y;
		m->mf[3][2] = translate->z;
	}

	static MtxF ComposeMatrix(glm::vec3* translate, glm::vec3* scales, glm::vec4* skew, glm::vec4* perspective, glm::quat* quaternion)
	{
		MtxF m;
		guMtxIdentF(m.mf);

		// Apply Perspective and Translation
		ComposePerspective(&m, perspective);
		ComposeTranslation(&m, translate);

		// Apply Rotation
		glm::mat4x4 rotationMatrix = ComposeRotationMatrix(*quaternion);
		multiply(m.mf, &rotationMatrix, m.mf);

		// Apply Skew
		float temp[4][4];
		guMtxIdentF(temp);

		if (skew->z)
		{
			temp[2][1] = skew->z;
			multiply(m.mf, temp, m.mf);
			guMtxIdentF(temp);
		}

		if (skew->y)
		{
			temp[2][1] = 0;
			temp[2][0] = skew->y;
			multiply(m.mf, temp, m.mf);
			guMtxIdentF(temp);
		}

		if (skew->x)
		{
			temp[2][0] = 0;
			temp[1][0] = skew->x;
			multiply(m.mf, temp, m.mf);
			guMtxIdentF(temp);
		}

		// Apply Scale
		multiply(m.mf, ComposeScaleMatrix(scales).mf, m.mf);

		return m;
	}

	static void DecomposeScaleAndSkew(MtxF* mtx, glm::mat3x3& row, glm::vec3* scale, glm::vec4* skew)
	{
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; ++j)
				row[i][j] = mtx->mf[i][j];

		// Compute X scale factor and normalize first row.
		scale->x = glm::length(row[0]);
		row[0] = glm::normalize(row[0]);

		// Compute XY shear factor and make 2nd row orthogonal to 1st.
		skew->x = glm::dot(row[0], row[1]);
		combine((float*)&row[1], (float*)&row[0], (float*)&row[1], 1.0, -skew->x);

		// Now, compute Y scale and normalize 2nd row.
		scale->y = glm::length(row[1]);
		row[1] = glm::normalize(row[1]);
		skew->x /= scale->y;

		// Compute XZ and YZ shears, orthogonalize 3rd row
		skew->y = glm::dot(row[0], row[2]);
		combine((float*)&row[2], (float*)&row[0], (float*)&row[2], 1.0, -skew->y);
		skew->z = glm::dot(row[1], row[2]);
		combine((float*)&row[2], (float*)&row[1], (float*)&row[2], 1.0, -skew->z);

		// Next, get Z scale and normalize 3rd row.
		scale->z = glm::length(row[2]);
		row[2] = glm::normalize(row[2]);
		skew->y /= scale->z;
		skew->z /= scale->z;
	}

	static void DecomposeRotation(glm::mat3x3& row, glm::quat& quaternion)
	{
		quaternion[0] = 0.5 * sqrt(MAX(1 + row[0][0] - row[1][1] - row[2][2], 0));
		quaternion[1] = 0.5 * sqrt(MAX(1 - row[0][0] + row[1][1] - row[2][2], 0));
		quaternion[2] = 0.5 * sqrt(MAX(1 - row[0][0] - row[1][1] + row[2][2], 0));
		quaternion[3] = 0.5 * sqrt(MAX(1 + row[0][0] + row[1][1] + row[2][2], 0));

		if (row[2][1] > row[1][2])
			quaternion[0] = -quaternion[0];
		if (row[0][2] > row[2][0])
			quaternion[1] = -quaternion[1];
		if (row[1][0] > row[0][1])
			quaternion[2] = -quaternion[2];
	}

	static void DecomposeTranslate(MtxF* mtx, glm::vec3* translate)
	{
		translate->x = mtx->mf[3][0];
		translate->y = mtx->mf[3][1];
		translate->z = mtx->mf[3][2];
	}

	static bool DecomposePerspectiveMatrix(MtxF* mtx, glm::mat4x4& perspectiveMatrix, glm::vec4* perspective)
	{
		memcpy(&perspectiveMatrix, mtx->mf, 4 * 4 * sizeof(float));

		for (int i = 0; i < 3; i++)
			perspectiveMatrix[i][3] = 0.0f;

		perspectiveMatrix[3][3] = 1;

		if (glm::determinant(perspectiveMatrix) == 0)
			return false;

		// First, isolate perspective.
		if (mtx->mf[0][3] != 0 || mtx->mf[1][3] != 0 || mtx->mf[2][3] != 0)
		{
			// Note: In practice, it seems we don't really work with perspective matrices...
			glm::vec4 rightHandSide;

			// rightHandSide is the right hand side of the equation.
			rightHandSide.x = mtx->mf[0][3];
			rightHandSide.y = mtx->mf[1][3];
			rightHandSide.z = mtx->mf[2][3];
			rightHandSide.w = mtx->mf[3][3];

			// Solve the equation by inverting perspectiveMatrix and multiplying
			// rightHandSide by the inverse.
			glm::mat4x4 inversePerspectiveMatrix = glm::inverse(perspectiveMatrix);
			glm::mat4x4 transposedInversePerspectiveMatrix = glm::transpose(inversePerspectiveMatrix);

			*perspective = transposedInversePerspectiveMatrix * rightHandSide;
		}
		else
		{
			// No perspective.
			perspective->x = 0;
			perspective->y = 0;
			perspective->z = 0;
			perspective->w = 1;
		}

		return true;
	}

	// Implementation based on Graphics Gems II
	static bool DecomposeMatrix(MtxF* mtx, glm::vec3* translate, glm::vec3* scale, glm::vec4* skew, glm::vec4* perspective, glm::quat& quaternion)
	{
		// Normalize the matrix.
		if (mtx->mf[3][3] == 0)
			return false;

		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				mtx->mf[i][j] /= mtx->mf[3][3];

		glm::mat4x4 perspectiveMatrix;

		if (!DecomposePerspectiveMatrix(mtx, perspectiveMatrix, perspective))
			return false;
		
		// Next take care of translation
		DecomposeTranslate(mtx, translate);

		// Now get scale and shear. 'row' is a 3 element array of 3 component vectors
		glm::mat3x3 row;
		DecomposeScaleAndSkew(mtx, row, scale, skew);

		// At this point, the matrix (in rows) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.
		glm::vec3 pdum3 = glm::cross(row[1], row[2]);

		if (glm::dot(row[0], pdum3) < 0)
		{
			for (int i = 0; i < 3; i++)
			{
				scale[i] *= -1;

				row[i][0] *= -1;
				row[i][1] *= -1;
				row[i][2] *= -1;
			}
		}

		// Now, get the rotations out
		DecomposeRotation(row, quaternion);

		return true;
	}

	static void multiply(float a[4][4], float b[4][4], float result[4][4]) 
	{
		SkinMatrix_MtxFMtxFMult((MtxF*)a, (MtxF*)b, (MtxF*)result);
	}

	static void multiply(float a[4][4], glm::mat4x4* b, float result[4][4])
	{
		SkinMatrix_MtxFMtxFMult((MtxF*)a, (MtxF*)b, (MtxF*)result);
	}

	void InterpolateMatricesLegacy(MtxF* res, MtxF* o, MtxF* n)
	{
		for (size_t i = 0; i < 4; i++)
			for (size_t j = 0; j < 4; j++)
				res->mf[i][j] = w * o->mf[i][j] + step * n->mf[i][j];
	}


    void interpolate_mtxf(MtxF* res, MtxF* o, MtxF* n, bool isViewMtx)
	{
		glm::vec3 translateO, translateN;
		glm::vec3 scalesO, scalesN;
		glm::vec4 skewO, skewN;
		glm::vec4 perspectiveO, perspectiveN;
		glm::quat quaternionO, quaternionN;

		// View matrices won't work with the new interpolation technique, 
		// so we need to rely on how it was done before
		if (isViewMtx)
		{
			InterpolateMatricesLegacy(res, o, n);
			return;
		}

		bool success = DecomposeMatrix(o, &translateO, &scalesO, &skewO, &perspectiveO, quaternionO);

		if (!success)
		{
			InterpolateMatricesLegacy(res, o, n);
			return;
		}

		bool success2 = DecomposeMatrix(n, &translateN, &scalesN, &skewN, &perspectiveN, quaternionN);

		if (!success2)
		{
			InterpolateMatricesLegacy(res, o, n);
			return;
		}

		// Take the shortest route
		if (glm::dot(quaternionO, quaternionN) < 0.0f)
			quaternionN = -quaternionN;

		quaternionO = glm::slerp(quaternionO, quaternionN, step);

		translateO = glm::mix(translateO, translateN, step);
		scalesO = glm::mix(scalesO, scalesN, step);
		skewO = glm::mix(skewO, skewN, step);
		perspectiveO = glm::mix(perspectiveO, perspectiveN, step);

		MtxF m = ComposeMatrix(&translateO, &scalesO, &skewO, &perspectiveO, &quaternionO);
		memcpy(res->mf, &m.mf, 4 * 4 * sizeof(float));
    }

    float lerp(f32 o, f32 n) {
        return w * o + step * n;
    }

    void lerp_vec3f(Vec3f* res, Vec3f* o, Vec3f* n) {
        res->x = lerp(o->x, n->x);
        res->y = lerp(o->y, n->y);
        res->z = lerp(o->z, n->z);
    }

    float interpolate_angle(f32 o, f32 n) {
        if (o == n)
            return n;
        o = fmodf(o, 2 * M_PI);
        if (o < 0.0f) {
            o += 2 * M_PI;
        }
        n = fmodf(n, 2 * M_PI);
        if (n < 0.0f) {
            n += 2 * M_PI;
        }
        if (fabsf(o - n) > M_PI) {
            if (o < n) {
                o += 2 * M_PI;
            } else {
                n += 2 * M_PI;
            }
        }
        if (fabsf(o - n) > M_PI / 2) {
            // return n;
        }
        return lerp(o, n);
    }

    s16 interpolate_angle(s16 os, s16 ns) {
        if (os == ns)
            return ns;
        int o = (u16)os;
        int n = (u16)ns;
        u16 res;
        int diff = o - n;
        if (-0x8000 <= diff && diff <= 0x8000) {
            if (diff < -0x4000 || diff > 0x4000) {
                // Wider angle cut off values are just slightly larger than when Deku Link enters a flower
                if (!interpolate_wider_angles || diff < -0x5700 || diff > 0x5700) {
                    return ns;
                }
            }
            res = (u16)(w * o + step * n);
        } else {
            if (o < n) {
                o += 0x10000;
            } else {
                n += 0x10000;
            }
            diff = o - n;
            if (diff < -0x4000 || diff > 0x4000) {
                if (!interpolate_wider_angles || diff < -0x5700 || diff > 0x5700) {
                    return ns;
                }
            }
            res = (u16)(w * o + step * n);
        }
        if (os / 327 == ns / 327 && (s16)res / 327 != os / 327) {
            int bp = 0;
        }
        return res;
    }

    void interpolate_angles(Vec3s* res, Vec3s* o, Vec3s* n) {
        res->x = interpolate_angle(o->x, n->x);
        res->y = interpolate_angle(o->y, n->y);
        res->z = interpolate_angle(o->z, n->z);
    }

    void interpolate_branch(Path* old_path, Path* new_path) {
        for (auto& item : new_path->items) {
            Data& new_op = new_path->ops[item.first][item.second];

            if (item.first == Op::OpenChild) {
                if (auto it = old_path->children.find(new_op.open_child.key);
                    it != old_path->children.end() && new_op.open_child.idx < it->second.size()) {
                    interpolate_branch(&it->second[new_op.open_child.idx],
                                       &new_path->children.find(new_op.open_child.key)->second[new_op.open_child.idx]);
                } else {
                    interpolate_branch(&new_path->children.find(new_op.open_child.key)->second[new_op.open_child.idx],
                                       &new_path->children.find(new_op.open_child.key)->second[new_op.open_child.idx]);
                }
                continue;
            }

            if (auto it = old_path->ops.find(item.first); it != old_path->ops.end()) {
                if (item.second < it->second.size()) {
                    Data& old_op = it->second[item.second];
                    switch (item.first) {
                        case Op::OpenChild:
                            break;
                        case Op::CloseChild:
                            break;

                        case Op::MatrixPush:
                            Matrix_Push();
                            break;

                        case Op::MatrixPop:
                            Matrix_Pop();
                            break;

                        case Op::MatrixPut:
                            interpolate_mtxf(&tmp_mtxf, &old_op.matrix_put.src, &new_op.matrix_put.src, false);
                            Matrix_Put(&tmp_mtxf);
                            break;

                        case Op::MatrixMult:
                            interpolate_mtxf(&tmp_mtxf, &old_op.matrix_mult.mf, &new_op.matrix_mult.mf, false);
                            Matrix_Mult(&tmp_mtxf, new_op.matrix_mult.mode);
                            break;

                        case Op::MatrixTranslate:
                            Matrix_Translate(lerp(old_op.matrix_translate.x, new_op.matrix_translate.x),
                                             lerp(old_op.matrix_translate.y, new_op.matrix_translate.y),
                                             lerp(old_op.matrix_translate.z, new_op.matrix_translate.z),
                                             new_op.matrix_translate.mode);
                            break;

                        case Op::MatrixScale:
                            Matrix_Scale(lerp(old_op.matrix_scale.x, new_op.matrix_scale.x),
                                         lerp(old_op.matrix_scale.y, new_op.matrix_scale.y),
                                         lerp(old_op.matrix_scale.z, new_op.matrix_scale.z), new_op.matrix_scale.mode);
                            break;

                        case Op::MatrixRotate1Coord: {
                            float v = interpolate_angle(old_op.matrix_rotate_1_coord.value,
                                                        new_op.matrix_rotate_1_coord.value);
                            u8 mode = new_op.matrix_rotate_1_coord.mode;
                            switch (new_op.matrix_rotate_1_coord.coord) {
                                case 0:
                                    Matrix_RotateXF(v, mode);
                                    break;

                                case 1:
                                    Matrix_RotateYF(v, mode);
                                    break;

                                case 2:
                                    Matrix_RotateZF(v, mode);
                                    break;
                            }
                            break;
                        }

                        case Op::MatrixRotateZYX:
                            Matrix_RotateZYX(interpolate_angle(old_op.matrix_rotate_zyx.x, new_op.matrix_rotate_zyx.x),
                                             interpolate_angle(old_op.matrix_rotate_zyx.y, new_op.matrix_rotate_zyx.y),
                                             interpolate_angle(old_op.matrix_rotate_zyx.z, new_op.matrix_rotate_zyx.z),
                                             new_op.matrix_rotate_zyx.mode);
                            break;

                        case Op::MatrixTranslateRotateZYX:
                            lerp_vec3f(&tmp_vec3f, &old_op.matrix_translate_rotate_zyx.translation,
                                       &new_op.matrix_translate_rotate_zyx.translation);
                            interpolate_angles(&tmp_vec3s, &old_op.matrix_translate_rotate_zyx.rotation,
                                               &new_op.matrix_translate_rotate_zyx.rotation);
                            Matrix_TranslateRotateZYX(&tmp_vec3f, &tmp_vec3s);
                            break;

                        case Op::MatrixSetTranslateRotateYXZ:
                            interpolate_wider_angles = new_op.matrix_set_translate_rotate_yxz.interpolate_wider_angles;
                            interpolate_angles(&tmp_vec3s, &old_op.matrix_set_translate_rotate_yxz.rot,
                                               &new_op.matrix_set_translate_rotate_yxz.rot);
                            Matrix_SetTranslateRotateYXZ(lerp(old_op.matrix_set_translate_rotate_yxz.translateX,
                                                              new_op.matrix_set_translate_rotate_yxz.translateX),
                                                         lerp(old_op.matrix_set_translate_rotate_yxz.translateY,
                                                              new_op.matrix_set_translate_rotate_yxz.translateY),
                                                         lerp(old_op.matrix_set_translate_rotate_yxz.translateZ,
                                                              new_op.matrix_set_translate_rotate_yxz.translateZ),
                                                         &tmp_vec3s);
                            if (new_op.matrix_set_translate_rotate_yxz.has_mtx &&
                                old_op.matrix_set_translate_rotate_yxz.has_mtx) {
                                actor_mtx = *Matrix_GetCurrent();
                            }
                            interpolate_wider_angles = false;
                            break;

                        case Op::MatrixMtxFToMtx:
                            interpolate_mtxf(new_replacement(new_op.matrix_mtxf_to_mtx.dest),
                                             &old_op.matrix_mtxf_to_mtx.src, &new_op.matrix_mtxf_to_mtx.src, old_op.matrix_mtxf_to_mtx.isViewMtx);
                            break;

                        case Op::MatrixToMtx: {
                            //*new_replacement(new_op.matrix_to_mtx.dest) = *Matrix_GetCurrent();
                            if (old_op.matrix_to_mtx.has_adjusted && new_op.matrix_to_mtx.has_adjusted) {
                                interpolate_mtxf(&tmp_mtxf, &old_op.matrix_to_mtx.src, &new_op.matrix_to_mtx.src, false);
                                SkinMatrix_MtxFMtxFMult(&actor_mtx, &tmp_mtxf,
                                                        new_replacement(new_op.matrix_to_mtx.dest));
                            } else {
                                interpolate_mtxf(new_replacement(new_op.matrix_to_mtx.dest), &old_op.matrix_to_mtx.src,
                                                 &new_op.matrix_to_mtx.src, false);
                            }
                            break;
                        }

                        case Op::MatrixReplaceRotation:
                            interpolate_mtxf(&tmp_mtxf, &old_op.matrix_replace_rotation.mf,
                                             &new_op.matrix_replace_rotation.mf, false);
                            Matrix_ReplaceRotation(&tmp_mtxf);
                            break;

                        case Op::MatrixRotateAxis:
                            lerp_vec3f(&tmp_vec3f, &old_op.matrix_rotate_axis.axis, &new_op.matrix_rotate_axis.axis);
                            Matrix_RotateAxisF(
                                interpolate_angle(old_op.matrix_rotate_axis.angle, new_op.matrix_rotate_axis.angle),
                                &tmp_vec3f, new_op.matrix_rotate_axis.mode);
                            break;

                        case Op::SkinMatrixMtxFToMtx:
                            break;
                    }
                }
            }
        }
    }
};

} // anonymous namespace

unordered_map<Mtx*, MtxF> FrameInterpolation_Interpolate(float step) {
    InterpolateCtx ctx;
    ctx.step = step;
    ctx.w = 1.0f - step;
    ctx.interpolate_branch(&previous_recording.root_path, &current_recording.root_path);
    return ctx.mtx_replacements;
}

bool camera_interpolation = false;

void FrameInterpolation_ShouldInterpolateFrame(bool shouldInterpolate) {
    camera_interpolation = shouldInterpolate;
}

void FrameInterpolation_StartRecord(void) {
    previous_recording = move(current_recording);
    current_recording = {};
    current_path.clear();
    current_path.push_back(&current_recording.root_path);
    has_inv_actor_mtx = false;
    interpolate_wider_angles = false;
    ignore_inv_actor_mtx = false;

    if (!camera_interpolation) {
        // default to interpolating
        camera_interpolation = true;
        is_recording = false;
        return;
    }
    if (OTRGlobals::Instance->GetInterpolationFPS() != 20) {
        is_recording = true;
    }
}

void FrameInterpolation_StopRecord(void) {
    previous_camera_epoch = camera_epoch;
    is_recording = false;
}

void FrameInterpolation_RecordOpenChild(const void* a, int b) {
    if (!is_recording)
        return;
    label key = { a, b };
    auto& m = current_path.back()->children[key];
    append(Op::OpenChild).open_child = { key, m.size() };
    current_path.push_back(&m.emplace_back());
}

void FrameInterpolation_RecordCloseChild(void) {
    if (!is_recording)
        return;
    // append(Op::CloseChild);
    if (has_inv_actor_mtx && current_path.size() == inv_actor_mtx_path_index) {
        has_inv_actor_mtx = false;
    }
    if (ignore_inv_actor_mtx && current_path.size() == ignore_inv_actor_mtx_path_index) {
        ignore_inv_actor_mtx = false;
    }
    current_path.pop_back();
}

void FrameInterpolation_DontInterpolateCamera(void) {
    camera_epoch = previous_camera_epoch + 1;
}

int FrameInterpolation_GetCameraEpoch(void) {
    return (int)camera_epoch;
}

// Marks the current record path and its children to not apply the matrix result
// against the recorded actor inverted matrix
void FrameInterpolation_IgnoreActorMtx() {
    if (!is_recording)
        return;
    ignore_inv_actor_mtx = true;
    ignore_inv_actor_mtx_path_index = current_path.size();
}

// Allows interpolating from angle changes that are up to 123º for the next SetTranslateRotateYXZ
void FrameInterpolation_InterpolateWiderAngles() {
    if (!is_recording)
        return;
    interpolate_wider_angles = true;
}

void FrameInterpolation_RecordActorPosRotMatrix(void) {
    if (!is_recording)
        return;
    next_is_actor_pos_rot_matrix = true;
}

void FrameInterpolation_RecordMatrixPush(void) {
    if (!is_recording)
        return;
    append(Op::MatrixPush);
}

void FrameInterpolation_RecordMatrixPop(void) {
    if (!is_recording)
        return;
    append(Op::MatrixPop);
}

void FrameInterpolation_RecordMatrixPut(MtxF* src) {
    if (!is_recording)
        return;
    append(Op::MatrixPut).matrix_put = { *src };
}

void FrameInterpolation_RecordMatrixMult(MtxF* mf, u8 mode) {
    if (!is_recording)
        return;
    append(Op::MatrixMult).matrix_mult = { *mf, mode };
}

void FrameInterpolation_RecordMatrixTranslate(f32 x, f32 y, f32 z, u8 mode) {
    if (!is_recording)
        return;
    append(Op::MatrixTranslate).matrix_translate = { x, y, z, mode };
}

void FrameInterpolation_RecordMatrixScale(f32 x, f32 y, f32 z, u8 mode) {
    if (!is_recording)
        return;
    append(Op::MatrixScale).matrix_scale = { x, y, z, mode };
}

void FrameInterpolation_RecordMatrixRotate1Coord(u32 coord, f32 value, u8 mode) {
    if (!is_recording)
        return;
    append(Op::MatrixRotate1Coord).matrix_rotate_1_coord = { coord, value, mode };
}

void FrameInterpolation_RecordMatrixRotateZYX(s16 x, s16 y, s16 z, u8 mode) {
    if (!is_recording)
        return;
    append(Op::MatrixRotateZYX).matrix_rotate_zyx = { x, y, z, mode };
}

void FrameInterpolation_RecordMatrixTranslateRotateZYX(Vec3f* translation, Vec3s* rotation) {
    if (!is_recording)
        return;
    append(Op::MatrixTranslateRotateZYX).matrix_translate_rotate_zyx = { *translation, *rotation };
}

void FrameInterpolation_RecordMatrixSetTranslateRotateYXZ(f32 translateX, f32 translateY, f32 translateZ, Vec3s* rot) {
    if (!is_recording)
        return;
    auto& d = append(Op::MatrixSetTranslateRotateYXZ).matrix_set_translate_rotate_yxz = { translateX, translateY,
                                                                                          translateZ, *rot };
    if (next_is_actor_pos_rot_matrix) {
        d.has_mtx = true;
        d.interpolate_wider_angles = interpolate_wider_angles;
        interpolate_wider_angles = false;
        // d.mtx = *Matrix_GetCurrent();
        invert_matrix((const float*)Matrix_GetCurrent()->mf, (float*)inv_actor_mtx.mf);
        next_is_actor_pos_rot_matrix = false;
        has_inv_actor_mtx = true;
        inv_actor_mtx_path_index = current_path.size();
    }
}

void FrameInterpolation_RecordMatrixMtxFToMtx(MtxF* src, Mtx* dest, bool isViewMtx) {
    if (!is_recording)
        return;
    append(Op::MatrixMtxFToMtx).matrix_mtxf_to_mtx = { *src, dest, isViewMtx };
}

void FrameInterpolation_RecordMatrixToMtx(Mtx* dest, char* file, s32 line) {
    if (!is_recording)
        return;

    auto& d = append(Op::MatrixToMtx).matrix_to_mtx = { dest };
    if (has_inv_actor_mtx && !ignore_inv_actor_mtx) {
        d.has_adjusted = true;
        SkinMatrix_MtxFMtxFMult(&inv_actor_mtx, Matrix_GetCurrent(), &d.src);
    } else {
        d.src = *Matrix_GetCurrent();
    }
}

void FrameInterpolation_RecordMatrixReplaceRotation(MtxF* mf) {
    if (!is_recording)
        return;
    append(Op::MatrixReplaceRotation).matrix_replace_rotation = { *mf };
}

void FrameInterpolation_RecordMatrixRotateAxis(f32 angle, Vec3f* axis, u8 mode) {
    if (!is_recording)
        return;
    append(Op::MatrixRotateAxis).matrix_rotate_axis = { angle, *axis, mode };
}

void FrameInterpolation_RecordSkinMatrixMtxFToMtx(MtxF* src, Mtx* dest, bool isViewMtx) {
    if (!is_recording)
        return;
    FrameInterpolation_RecordMatrixMtxFToMtx(src, dest, isViewMtx);
}

// https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
static bool invert_matrix(const float m[16], float invOut[16]) {
    float inv[16], det;
    int i;

    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] +
             m[13] * m[6] * m[11] - m[13] * m[7] * m[10];

    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] -
             m[12] * m[6] * m[11] + m[12] * m[7] * m[10];

    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] +
             m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] -
              m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] -
             m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] +
             m[12] * m[2] * m[11] - m[12] * m[3] * m[10];

    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] -
             m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] +
              m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] +
             m[13] * m[2] * m[7] - m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] -
             m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] +
              m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] -
              m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] -
             m[9] * m[2] * m[7] + m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] - m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] -
              m[8] * m[1] * m[7] + m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] -
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0) {
        return false;
    }

    det = 1.0 / det;

    for (i = 0; i < 16; i++) {
        invOut[i] = inv[i] * det;
    }

    return true;
}
