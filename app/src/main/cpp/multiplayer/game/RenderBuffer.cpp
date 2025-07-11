//
// Created on 20.09.2023.
//

#include "RenderBuffer.h"
#include "../util/patch.h"

void RenderBuffer::RenderStuffInBuffer() {
    Render(rwPRIMTYPETRILIST, nullptr, rwIM3D_VERTEXUV);
}

void RenderBuffer::StartStoring(
        int32 nIndicesNeeded,
        int32 nVerticesNeeded,
        RwImVertexIndex*& outPtrFirstIndex,
        RwIm3DVertex*& outPtrFirstVertex
) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x5BBA80 + 1 : 0x6E0370), nIndicesNeeded, nVerticesNeeded, outPtrFirstIndex, outPtrFirstVertex);
}

void RenderBuffer::StopStoring() {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x5BBB24 + 1 : 0x6E0428));
}

void RenderBuffer::ClearRenderBuffer() {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005BADE4 + 1 : 0x6DF5CC));
}

// NOTSA
void RenderBuffer::Render(RwPrimitiveType primType, RwMatrix *ltm, RwUInt32 flags, bool isIndexed) {
    if (uiTempBufferVerticesStored) {
        if (RwIm3DTransform(aTempBufferVertices, uiTempBufferVerticesStored, ltm, flags)) {
            if (isIndexed) {
              //  assert(aTempBufferIndices);
                RwIm3DRenderIndexedPrimitive(primType, aTempBufferIndices, uiTempBufferIndicesStored);
            } else {
                RwIm3DRenderPrimitive(primType);
            }
            RwIm3DEnd();
        }
    }
    ClearRenderBuffer();
}

bool RenderBuffer::CanFitVertices(int32 nVtxNeeded) {
    return uiTempBufferVerticesStored + nVtxNeeded <= VtxBufferSize;
}

RwIm3DVertex *RenderBuffer::PushVertex(CVector pos, CRGBA color) {
    const auto vtx = &aTempBufferVertices[uiTempBufferVerticesStored++];

    RwIm3DVertexSetPos(vtx, pos.x, pos.y, pos.z);
    RwIm3DVertexSetRGBA(vtx, color.r, color.g, color.b, color.a);

    return vtx;
}

// notsa
RwIm3DVertex* RenderBuffer::PushVertex(CVector pos, CVector2D uv, CRGBA color) {
    const auto vtx = PushVertex(pos, color);

    RwIm3DVertexSetU(vtx, uv.x);
    RwIm3DVertexSetV(vtx, uv.y);

    return vtx;
}

void RenderBuffer::InjectHooks() {
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00679D04 : 0x851A20), &uiTempBufferVerticesStored);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00676504 : 0x84AA78), &uiTempBufferIndicesStored);
}

// notsa
void PushIndex(int32 idx, bool useCurrentVtxAsBase) {
    idx = useCurrentVtxAsBase
          ? (int32)uiTempBufferVerticesStored + idx
          : idx;
    assert(idx >= 0);
    aTempBufferIndices[uiTempBufferIndicesStored++] = idx;
}

void RenderBuffer::PushIndices(std::initializer_list<int32> idxs, bool useCurrentVtxAsBase) {
    for (auto idx : idxs) {
        PushIndex(idx, useCurrentVtxAsBase);
    }
}
