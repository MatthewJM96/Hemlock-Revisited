#include "stdafx.h"

#include "graphics/font/font.h"
#include "graphics/sprite/string_drawer.h"

#include "graphics/sprite/batcher.h"

using namespace hg::f;

#define VERTICES_PER_QUAD 4
#define INDICES_PER_QUAD  6

hg::s::SpriteBatcher::SpriteBatcher() :
    m_is_initialised(false),
    m_vao(0), m_vbo(0), m_ibo(0),
    m_usage_hint(GL_STATIC_DRAW),
    m_index_count(0),
    m_default_texture(0),
    m_active_shader(nullptr),
    m_shader_cache(nullptr),
    m_font_cache(nullptr)
{
    // Empty.
}

void hg::s::SpriteBatcher::init(ShaderCache* shader_cache, FontCache* font_cache /*= nullptr*/, bool is_dynamic /*= false*/) {
    if (m_is_initialised) return;
    m_is_initialised = true;

    m_shader_cache = shader_cache;
    m_font_cache   = font_cache;
    m_usage_hint   = is_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    /*****************************\
     * Create a default shader . *
    \*****************************/

    // Create a default shader program.
    m_default_shader.init(m_shader_cache);

    // Set each attribute's corresponding index.
    m_default_shader.set_attribute("vPosition",         SpriteShaderAttribID::POSITION);
    m_default_shader.set_attribute("vRelativePosition", SpriteShaderAttribID::RELATIVE_POSITION);
    m_default_shader.set_attribute("vUVDimensions",     SpriteShaderAttribID::UV_DIMENSIONS);
    m_default_shader.set_attribute("vColour",           SpriteShaderAttribID::COLOUR);

    // TODO(Matthew): Handle errors.
    // Add the shaders to the program.
    m_default_shader.add_shaders("shaders/DefaultSprite.vert", "shaders/DefaultSprite.frag");

    // Link program (i.e. send to GPU).
    m_default_shader.link();

    // Set default shader as active shader.
    m_active_shader = &m_default_shader;

    /******************\
     * Create the VAO *
    \******************/

    // Gen the vertex array object and bind it.
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Generate the associated vertex & index buffers - these are the bits of memory that will be populated within the GPU storing information
    // about the graphics we want to draw.
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ibo);

    // Bind those buffers
    //    OpenGL generally follows a pattern of generate an ID corresponding to some memory on the GPU, bind said memory, link some properties to them 
    //    (such as how the data in the memory correspond to variables inside our shader programs).
    glBindBuffer(GL_ARRAY_BUFFER,         m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

    // Enable the attributes in our shader.
    m_default_shader.enable_vertex_attrib_arrays();

    // Connect the vertex attributes in the shader (e.g. vPosition) to its corresponding chunk of memory inside the SpriteVertex struct.
    //     We first tell OpenGL the ID of the attribute within the shader (as we set earlier), then the number of values and their type.
    //
    //     After that, we tell OpenGL whether that data should be normalised (e.g. unsigned bytes that need normalising will be converted 
    //     to a float divided through by 255.0f and by OpenGL - so that colours, e.g., are represented by values between 0.0f and 1.0f 
    //     per R/G/B/A channel rather than the usual 0 to 255).
    //
    //     We then pass the size of the data representing a vertex followed by how many bytes into that data the value is stored - we use offset rather than 
    //     manually writing this to give us flexibility in changing the order of the SpriteVertex struct.
    glVertexAttribPointer(SpriteShaderAttribID::POSITION,          3, GL_FLOAT,         false, sizeof(SpriteVertex), reinterpret_cast<void*>(offsetof(SpriteVertex, position)));
    glVertexAttribPointer(SpriteShaderAttribID::RELATIVE_POSITION, 2, GL_FLOAT,         false, sizeof(SpriteVertex), reinterpret_cast<void*>(offsetof(SpriteVertex, relative_position)));
    glVertexAttribPointer(SpriteShaderAttribID::UV_DIMENSIONS,     4, GL_FLOAT,         false, sizeof(SpriteVertex), reinterpret_cast<void*>(offsetof(SpriteVertex, uv_rect)));
    glVertexAttribPointer(SpriteShaderAttribID::COLOUR,            4, GL_UNSIGNED_BYTE, true,  sizeof(SpriteVertex), reinterpret_cast<void*>(offsetof(SpriteVertex, colour)));

    // Clean everything up, unbinding each of our buffers and the vertex array.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /***********************************\
     * Create a default white texture. *
    \***********************************/

    // Generate and bind texture.
    glGenTextures(1, &m_default_texture);
    glBindTexture(GL_TEXTURE_2D, m_default_texture);

    // Set texture to be just a 1x1 image of a pure white pixel.
    ui32 pix = 0xffffffff;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pix);

    // Set texture parameters to repeat our pixel as needed and to not do any averaging of pixels.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,     GL_REPEAT);

    // Unbind our complete texture.
    glBindTexture(GL_TEXTURE_2D, 0);
}

void hg::s::SpriteBatcher::dispose() {
    if (!m_is_initialised) return;
    m_is_initialised = false;

    // Clean up buffer objects before vertex array.
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }

    if (m_ibo != 0) {
        glDeleteBuffers(1, &m_ibo);
        m_ibo = 0;
        m_index_count = 0;
    }

    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }

    // Delete our default texture.
    if (m_default_texture != 0) {
        glDeleteTextures(1, &m_default_texture);
        m_default_texture = 0;
    }

    // Reset properties and stored sprites & batches.
    m_usage_hint  = GL_STATIC_DRAW;
    m_index_count = 0;

    Sprites().swap(m_sprites);
    SpritePtrs().swap(m_sprite_ptrs);
    Batches().swap(m_batches);
}

void hg::s::SpriteBatcher::reserve(size_t count) {
    m_sprites.reserve(count);
    m_sprite_ptrs.reserve(count);
}

void hg::s::SpriteBatcher::begin() {
    m_sprites.clear();
    m_batches.clear();
}

void hg::s::SpriteBatcher::end(SpriteSortMode sort_mode /*= SpriteSortMode::TEXTURE*/) {
    // Reserve the right amount of space to then assign a pointer for each sprite.
    if (m_sprite_ptrs.size() != m_sprites.size()) {
        m_sprite_ptrs.resize(m_sprites.size());
    }
    for (size_t i = 0; i < m_sprites.size(); ++i) {
        m_sprite_ptrs[i] = &m_sprites[i];
    }

    // Sort the sprites - we sort the vector of pointers only for speed.
    sort_sprites(sort_mode);

    // Generate the batches to use for draw calls.
    generate_batches();
}

void hg::s::SpriteBatcher::add_sprite(Sprite&& sprite) {
    m_sprites.emplace_back(std::forward<Sprite>(sprite));

    Sprite& sprite_ref = m_sprites.back();
    if (sprite_ref.texture == 0) {
        sprite_ref.texture = m_default_texture;
    }
}

void hg::s::SpriteBatcher::add_sprite(Sprite* sprites, ui32 sprite_count) {
    for (ui32 i = 0; i < sprite_count; ++i) {
        add_sprite(std::forward<Sprite>(sprites[i]));
    }
}

void hg::s::SpriteBatcher::add_sprite(
                 QuadBuilder builder,
                      GLuint texture,
                       f32v2 position,
                       f32v2 size,
                     colour4 c1       /*= { 255, 255, 255, 255 }*/,
                     colour4 c2       /*= { 255, 255, 255, 255 }*/,
                    Gradient gradient /*= Gradient::NONE*/,
                         f32 depth    /*= 0.0f*/,
                       f32v4 uv_rect  /*= f32v4(0.0f, 0.0f, 1.0f, 1.0f)*/
) {
    m_sprites.emplace_back(Sprite{
        builder,
        texture,
        position,
        size,
        depth,
        uv_rect,
        c1,
        c2,
        gradient
    });
}

void hg::s::SpriteBatcher::add_sprite(
                 QuadBuilder builder,
                       f32v2 position,
                       f32v2 size,
                     colour4 c1       /*= { 255, 255, 255, 255 }*/,
                     colour4 c2       /*= { 255, 255, 255, 255 }*/,
                    Gradient gradient /*= Gradient::NONE*/,
                         f32 depth    /*= 0.0f*/,
                       f32v4 uv_rect  /*= f32v4(0.0f, 0.0f, 1.0f, 1.0f)*/
) {
    m_sprites.emplace_back(Sprite{
        builder,
        m_default_texture,
        position,
        size,
        depth,
        uv_rect,
        c1,
        c2,
        gradient
    });
}

void hg::s::SpriteBatcher::add_sprite(
                 GLuint texture,
                  f32v2 position,
                  f32v2 size,
                colour4 c1       /*= { 255, 255, 255, 255 }*/,
                colour4 c2       /*= { 255, 255, 255, 255 }*/,
               Gradient gradient /*= Gradient::NONE*/,
                    f32 depth    /*= 0.0f*/,
                  f32v4 uv_rect  /*= f32v4(0.0f, 0.0f, 1.0f, 1.0f)*/
) {
    m_sprites.emplace_back(Sprite{
        &impl::basic_build_quad,
        texture,
        position,
        size,
        depth,
        uv_rect,
        c1,
        c2,
        gradient
    });
}

void hg::s::SpriteBatcher::add_sprite(
                 f32v2 position,
                 f32v2 size,
               colour4 c1       /*= { 255, 255, 255, 255 }*/,
               colour4 c2       /*= { 255, 255, 255, 255 }*/,
              Gradient gradient /*= Gradient::NONE*/,
                   f32 depth    /*= 0.0f*/,
                 f32v4 uv_rect  /*= f32v4(0.0f, 0.0f, 1.0f, 1.0f)*/
) {
    m_sprites.emplace_back(Sprite{
        &impl::basic_build_quad,
        m_default_texture,
        position,
        size,
        depth,
        uv_rect,
        c1,
        c2,
        gradient
    });
}


void hg::s::SpriteBatcher::add_string(
                 CALLER_DELETE const char* str,
                                     f32v4 rect,
                              StringSizing sizing,
                                   colour4 tint,
                        const std::string& font_name,
                                  FontSize font_size,
                                 TextAlign align        /*= TextAlign::TOP_LEFT*/,
                                  WordWrap wrap         /*= WordWrap::NONE*/,
                                       f32 depth        /*= 0.0f*/,
                                 FontStyle style        /*= FontStyle::NORMAL*/,
                           FontRenderStyle render_style /*= FontRenderStyle::BLENDED*/
) {
    add_string(
        str, rect, sizing, tint,
        m_font_cache->fetch(font_name)
                    ->get_instance(font_size, style, render_style),
        align, wrap, depth
    );
}

void hg::s::SpriteBatcher::add_string(
                 CALLER_DELETE const char* str,
                                     f32v4 rect,
                              StringSizing sizing,
                                   colour4 tint,
                        const std::string& font_name,
                                 TextAlign align        /*= TextAlign::TOP_LEFT*/,
                                  WordWrap wrap         /*= WordWrap::NONE*/,
                                       f32 depth        /*= 0.0f*/,
                                 FontStyle style        /*= FontStyle::NORMAL*/,
                           FontRenderStyle render_style /*= FontRenderStyle::BLENDED*/
) {
    add_string(
        str, rect, sizing, tint,
        m_font_cache->fetch(font_name)
                    ->get_instance(style, render_style),
        align, wrap, depth
    );
}

void hg::s::SpriteBatcher::add_string(
                 CALLER_DELETE const char* str,
                                     f32v4 rect,
                              StringSizing sizing,
                                   colour4 tint,
                              FontInstance font_instance,
                                 TextAlign align /*= TextAlign::TOP_LEFT*/,
                                  WordWrap wrap  /*= WordWrap::NONE*/,
                                       f32 depth /*= 0.0f*/
) {
    add_string(
        DrawableStringComponent{
            str, {
                font_instance,
                sizing,
                tint
            }
        },
        rect, align, wrap, depth
    );
}

void hg::s::SpriteBatcher::add_string(
                 CALLER_DELETE DrawableStringComponent str_component,
                                                 f32v4 rect,
                                             TextAlign align /*= TextAlign::TOP_LEFT*/,
                                              WordWrap wrap  /*= WordWrap::NONE*/,
                                                   f32 depth /*= 0.0f*/
) {
    switch(wrap) {
        case WordWrap::NONE:
            add_string_no_wrap(this, str_component, rect, align, depth);
            break;
        case WordWrap::QUICK:
            add_string_quick_wrap(this, str_component, rect, align, depth);
            break;
        case WordWrap::GREEDY:
            add_string_greedy_wrap(this, str_component, rect, align, depth);
            break;
        case WordWrap::MINIMUM_RAGGEDNESS:
            // add_string_minimum_raggedness_wrap(this, str_component, rect, align, depth);
            break;
    }
}

void hg::s::SpriteBatcher::add_string(
                 CALLER_DELETE DrawableStringComponents str_components,
                                                   ui32 num_components,
                                                  f32v4 rect,
                                              TextAlign align /*= TextAlign::TOP_LEFT*/,
                                               WordWrap wrap  /*= WordWrap::NONE*/,
                                                    f32 depth /*= 0.0f*/
) {
    switch(wrap) {
        case WordWrap::NONE:
            add_string_no_wrap(this, str_components, num_components, rect, align, depth);
            break;
        case WordWrap::QUICK:
            add_string_quick_wrap(this, str_components, num_components, rect, align, depth);
            break;
        case WordWrap::GREEDY:
            add_string_greedy_wrap(this, str_components, num_components, rect, align, depth);
            break;
        case WordWrap::MINIMUM_RAGGEDNESS:
            // add_string_minimum_raggedness_wrap(this, str_components, num_components, rect, align, depth);
            break;
    }
}

bool hg::s::SpriteBatcher::set_shader(GLSLProgram* shader /*= nullptr*/) {
    if (shader == nullptr) {
        m_active_shader = &m_default_shader;
    } else {
        if (!shader->initialised()) return false;

        if (!shader->linked()) {
            shader->set_attribute("vPosition",         SpriteShaderAttribID::POSITION);
            shader->set_attribute("vRelativePosition", SpriteShaderAttribID::RELATIVE_POSITION);
            shader->set_attribute("vUVDimensions",     SpriteShaderAttribID::UV_DIMENSIONS);
            shader->set_attribute("vColour",           SpriteShaderAttribID::COLOUR);

            if (shader->link() != ShaderLinkResult::SUCCESS) return false;
        }

        m_active_shader = shader;
    }

    return true;
}

void hg::s::SpriteBatcher::render(f32m4 world_projection, f32m4 view_projection) {
    // Activate the shader.
    m_active_shader->use();

    // Upload our projection matrices.
    glUniformMatrix4fv(m_active_shader->uniform_location("WorldProjection"), 1, false, &world_projection[0][0]);
    glUniformMatrix4fv(m_active_shader->uniform_location("ViewProjection"),  1, false, &view_projection[0][0]);

    // Bind our vertex array.
    glBindVertexArray(m_vao);

    // Activate the zeroth texture slot in OpenGL, and pass the index to the texture uniform in our shader.
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_active_shader->uniform_location("SpriteTexture"), 0);

    // For each batch, bind its texture, set the sampler state (have to do this each time), and draw the triangles in that batch.
    for (auto& batch : m_batches) {
        glBindTexture(GL_TEXTURE_2D, batch.texture);

        // Note that we pass an offset as the final argument despite glDrawElements expecting a pointer as we have already uploaded
        // the data to the buffer on the GPU - we only need to pass an offset in bytes from the beginning of this buffer rather than
        // the address of a buffer in RAM.
        glDrawElements(GL_TRIANGLES, batch.index_count , GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(batch.index_offset * sizeof(ui32)));
    }

    // Unbind out vertex array.
    glBindVertexArray(0);

    // Deactivate our shader.
    m_active_shader->unuse();
}

void hg::s::SpriteBatcher::render(f32m4 world_projection, f32v2 screen_size) {
    f32m4 view_projection = f32m4(
         2.0f / screen_size.x,  0.0f,                 0.0f, 0.0f,
         0.0f,                 -2.0f / screen_size.y, 0.0f, 0.0f,
         0.0f,                  0.0f,                 1.0f, 0.0f,
        -1.0f,                  1.0f,                 0.0f, 1.0f
    );

    render(world_projection, view_projection);
}

void hg::s::SpriteBatcher::render(f32v2 screen_size) {
    render(f32m4{1.0f}, screen_size);
}

void hg::s::SpriteBatcher::sort_sprites(SpriteSortMode sort_mode) {
    if (m_sprite_ptrs.empty()) return;

    // Sort the data according to mode.
    switch (sort_mode) {
    case SpriteSortMode::TEXTURE:
        std::stable_sort(m_sprite_ptrs.begin(), m_sprite_ptrs.end(), &impl::sort_texture);
        break;
    case SpriteSortMode::FRONT_TO_BACK:
        std::stable_sort(m_sprite_ptrs.begin(), m_sprite_ptrs.end(), &impl::sort_front_to_back);
        break;
    case SpriteSortMode::BACK_TO_FRONT:
        std::stable_sort(m_sprite_ptrs.begin(), m_sprite_ptrs.end(), &impl::sort_back_to_front);
        break;
    default:
        break;
    }
}

void hg::s::SpriteBatcher::generate_batches() {
    // If we have no sprites, just tell the GPU we have nothing.
    if (m_sprite_ptrs.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, m_usage_hint);
        return;
    }

    // Create a buffer of vertices to be populated and sent to the GPU.
    SpriteVertex* vertices = new SpriteVertex[VERTICES_PER_QUAD * m_sprite_ptrs.size()];

    // Some counts to help us know where we're at with populating the vertices.
    ui32 vertex_count = 0;
    ui32 index_count  = 0;

    // Create our first batch, which has 0 offset and texture the same as that of
    // the first sprite - as it defines the first batch.
    m_batches.emplace_back();
    m_batches.back().index_offset = 0;
    m_batches.back().texture      = m_sprite_ptrs[0]->texture;

    // For each sprite, we want to populate the vertex buffer with those for that
    // sprite. In the case that we are changing to a new texture, we need to 
    // start a new batch.
    for (auto& sprite : m_sprite_ptrs) {
        // Start a new batch with texture of the sprite we're currently working with
        // if that texture is different to the previous batch.
        if (sprite->texture != m_batches.back().texture) {
            // Now we are making a new batch, we can set the number of indices in 
            // the previous batch.
            m_batches.back().index_count  = index_count - m_batches.back().index_offset;
            m_batches.emplace_back();

            m_batches.back().index_offset = index_count;
            m_batches.back().texture      = sprite->texture;
        }

        // Builds the sprite's quad, i.e. adds the sprite's vertices to the vertex buffer.
        sprite->build(sprite, vertices + vertex_count);

        // Update our counts.
        vertex_count += VERTICES_PER_QUAD;
        index_count  += INDICES_PER_QUAD;
    }
    // Make sure to set the last batch's index count.
    m_batches.back().index_count = index_count - m_batches.back().index_offset;

    // If we need more indices than we have so far uploaded to the GPU, we must
    // generate more and update the index buffer on the GPU.
    //     For now the index pattern is the same for all sprites, as all sprites are
    //     treated as quads. If we want to support other geometries of sprite we will
    //     have to change this.
    if (m_index_count < index_count) {
        m_index_count = index_count;

        // Bind the index buffer.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        // Invalidate the old buffer data on the GPU so that when we write our new data we don't
        // need to wait for the old data to be unused by the GPU.
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_index_count * sizeof(ui32), nullptr, m_usage_hint);

        // Create a local index buffer we will upload to the GPU.
        ui32* indices = new ui32[m_index_count];
        ui32 i = 0; // Index cursor.
        ui32 v = 0; // Vertex cursor.
        while (i < m_index_count) {
            // For each quad, we have four vertices which we write 6 indices for - giving us two triangles.
            // The order of these indices is important - each triple should form a triangle correlating
            // to the build functions.
            indices[i++] = v;     // Top left vertex.
            indices[i++] = v + 2; // Bottom left vertex.
            indices[i++] = v + 3; // Bottom right vertex.
            indices[i++] = v + 3; // Bottom right vertex.
            indices[i++] = v + 1; // Top right vertex.
            indices[i++] = v;     // Top left vertex.

            v += 4;
        }

        // Send the indices over to the GPU.
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_index_count * sizeof(ui32), indices);
        // Unbind our buffer object.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    // Bind the vertex buffer and delete the old data from the GPU.
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // Invalidate the old buffer data on the GPU so that when we write our new data we don't
    // need to wait for the old data to be unused by the GPU.
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(SpriteVertex), nullptr, m_usage_hint);
    // Write our new data.
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * sizeof(SpriteVertex), vertices);
    // Unbind our buffer object.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Clear up memory.
    delete[] vertices;
}

void hg::s::impl::basic_build_quad(const Sprite* sprite, SpriteVertex* vertices) {
    SpriteVertex& top_left     = vertices[0];
    top_left.position.x        = sprite->position.x;
    top_left.position.y        = sprite->position.y;
    top_left.position.z        = sprite->depth;
    top_left.relative_position = f32v2(0.0f, 0.0f);
    top_left.uv_rect           = sprite->uv_rect;

    SpriteVertex& top_right     = vertices[1];
    top_right.position.x        = sprite->position.x + sprite->size.x;
    top_right.position.y        = sprite->position.y;
    top_right.position.z        = sprite->depth;
    top_right.relative_position = f32v2(1.0f, 0.0f);
    top_right.uv_rect           = sprite->uv_rect;

    SpriteVertex& bottom_left     = vertices[2];
    bottom_left.position.x        = sprite->position.x;
    bottom_left.position.y        = sprite->position.y + sprite->size.y;
    bottom_left.position.z        = sprite->depth;
    bottom_left.relative_position = f32v2(0.0f, 1.0f);
    bottom_left.uv_rect           = sprite->uv_rect;

    SpriteVertex& bottom_right     = vertices[3];
    bottom_right.position.x        = sprite->position.x + sprite->size.x;
    bottom_right.position.y        = sprite->position.y + sprite->size.y;
    bottom_right.position.z        = sprite->depth;
    bottom_right.relative_position = f32v2(1.0f, 1.0f);
    bottom_right.uv_rect           = sprite->uv_rect;

    switch (sprite->gradient) {
        case Gradient::LEFT_TO_RIGHT:
            top_left.colour  = bottom_left.colour  = sprite->c1;
            top_right.colour = bottom_right.colour = sprite->c2;
            break;
        case Gradient::TOP_TO_BOTTOM:
            top_left.colour    = top_right.colour    = sprite->c1;
            bottom_left.colour = bottom_right.colour = sprite->c2;
            break;
        case Gradient::TOP_LEFT_TO_BOTTOM_RIGHT:
            top_left.colour     = sprite->c1;
            bottom_right.colour = sprite->c2;
            top_right.colour    = bottom_left.colour = lerp(sprite->c1, sprite->c2, 0.5);
            break;
        case Gradient::TOP_RIGHT_TO_BOTTOM_LEFT:
            top_right.colour   = sprite->c1;
            bottom_left.colour = sprite->c2;
            top_left.colour    = bottom_right.colour = lerp(sprite->c1, sprite->c2, 0.5);
            break;
        case Gradient::NONE:
            top_left.colour = top_right.colour = bottom_left.colour = bottom_right.colour = sprite->c1;
            break;
        default:
            puts("Invalid gradient type!");
            assert(false);
    }
}

bool hg::s::impl::sort_texture(Sprite* sprite1, Sprite* sprite2) {
    return sprite1->texture < sprite2->texture;
}

bool hg::s::impl::sort_front_to_back(Sprite* sprite1, Sprite* sprite2) {
    return sprite1->depth < sprite2->depth;
}

bool hg::s::impl::sort_back_to_front(Sprite* sprite1, Sprite* sprite2) {
    return sprite1->depth > sprite2->depth;
}