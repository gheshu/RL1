#include "init.h"

#include <time.h>
#include <stdlib.h>

#include "macro.h"
#include "window.h"
#include "camera.h"
#include "pipeline.h"
#include "shader.h"
#include "transform.h"
#include "physics.h"
#include "buffer.h"
#include "image.h"
#include "component.h"
#include "task.h"
#include "prng.h"

#include "bsp.h"

#include "imguishim.h"

#include "sokol_gfx.h"
#include "sokol_time.h"

#include "shaders/textured.h"

Window window;
Camera camera;

void Init()
{
    SRand(time(0) ^ (uint64_t)&puts);

    window.Init("RL1", false);
    Window::SetActive(&window);
    camera.Init(window.m_width, window.m_height);
    camera.move(vec3(0.0f, 0.0f, -3.0f));
    Camera::SetActive(&camera);

    sg_desc desc = {0};
    sg_setup(&desc);
    stm_setup();

    ImGuiShim::Init(window.m_window);

    Components::Init();
    TaskManager::Init();
    Physics::Init();
    
    sg_shader_desc shadesc = {0};

    shadesc.vs.source = textured_vs;
    shadesc.vs.uniform_blocks[0].size = sizeof(VSUniform);
    shadesc.vs.uniform_blocks[0].uniforms[0] = { "MVP", SG_UNIFORMTYPE_MAT4 };
    shadesc.vs.uniform_blocks[0].uniforms[1] = { "M",   SG_UNIFORMTYPE_MAT4 };

    int32_t u = 0;
    shadesc.fs.source = textured_fs;
    shadesc.fs.uniform_blocks[0].size = sizeof(FSUniform);
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "Eye",             SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "LightDir",        SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "LightRad",        SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "Pal0",            SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "Pal1",            SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "Pal2",            SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "PalCenter",       SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "RoughnessOffset", SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "MetalnessOffset", SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.uniform_blocks[0].uniforms[u++] = { "Seed",            SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.images[0].name = "MatTex";
    shadesc.fs.images[0].type = SG_IMAGETYPE_2D;
    shadesc.fs.images[1].name = "NorTex";
    shadesc.fs.images[1].type = SG_IMAGETYPE_2D;

    slot shaderSlot = Shaders::Create(shadesc);

    sg_pipeline_desc pdesc = {0};
    pdesc.shader = Shaders::Get(shaderSlot);
    pdesc.index_type = SG_INDEXTYPE_UINT32;
    pdesc.layout.attrs[0].name = "position";
    pdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pdesc.layout.attrs[1].name = "normal";
    pdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
    pdesc.layout.attrs[2].name = "uv0";
    pdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
    pdesc.depth_stencil.depth_write_enabled = true;
    pdesc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS;
    pdesc.rasterizer.cull_mode = SG_CULLMODE_BACK;
    pdesc.rasterizer.face_winding = SG_FACEWINDING_CCW;
    
    slot pipeslot       = Pipelines::Create(pdesc);

    const float cubef[] = 
    {
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
    };

    const int32_t cubeindf[] = 
    {
        0,  2,  1,      0,  3,  2,
        6,  4,  5,      7,  4,  6,
        8,  10, 9,      8,  11, 10,
        14, 12, 13,     15, 12, 14,
        16, 18, 17,     16, 19, 18,
        22, 20, 21,     23, 20, 22
    };
    const Array<vec3> cube((const vec3*)cubef, NELEM(cubef) / 3);
    const Array<int32_t> ind(cubeindf, NELEM(cubeindf));

    {
        slot ent = Components::Create();
        Components::Add<TransformComponent>(ent);
        RenderComponent* rc = Components::GetAdd<RenderComponent>(ent);
        PhysicsComponent* pc = Components::GetAdd<PhysicsComponent>(ent);

        pc->Init(0.0f, vec3(0.0f, 0.0f, 0.0f), vec3(10.0f, 0.33f, 10.0f));

        BspTree t3 = BspTree(cube, ind)
            .Subtract(
                BspTree(cube, ind).Transform(glm::translate(mat4(1.0f), vec3(0.5f))));
        
        Array<Vertex> verts;
        Array<int32_t> inds;
        t3.ToVertices(camera.m_eye, inds, verts);

        printf("%d; %d\n", verts.count(), inds.count());
        for(int32_t x : inds)
        {
            printf("%d, ", x);
        }
        puts("");

        BufferData bd;
        bd.vertices     = verts.begin();
        bd.vertCount    = verts.count();
        bd.indices      = (uint32_t*)inds.begin();
        bd.indexCount   = inds.count();

        Buffer buf = Buffers::Create(bd);
        slot bslot = Buffers::Create(BufferString("csg_test"), buf);
        
        rc->m_buffer    = bslot; //Buffers::Load(BufferString("sphere"));
        rc->m_material  = Images::Load(ImageString("bumpy_PRMA"));
        rc->m_normal   = Images::Load(ImageString("bumpy_normal"));
        rc->m_pipeline  = pipeslot;
    }
}
