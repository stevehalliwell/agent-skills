/* Bounded macOS C99 consumer check for actual GLFW/OpenGL3 RichMd dylib.
 * Usage: bridge-check <assets-folder> <output.ppm>
 * Owns one hidden window; never opens a project or changes source records. */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
/* OpenGL core test calls resolve through GLFW, as normal extension loading does.
 * Keep C99 consumer independent of Apple's C++/framework header search support.
 * Scalar types/tokens match SDK OpenGL/gl3.h; production renderer uses SDK headers. */
typedef unsigned int GLenum, GLuint;
typedef int GLint;
#define GL_NO_ERROR 0
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_UNPACK_SKIP_ROWS 0x0CF3
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_PACK_ROW_LENGTH 0x0D02
#define GL_PACK_SKIP_ROWS 0x0D03
#define GL_PACK_SKIP_PIXELS 0x0D04
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_STATIC_DRAW 0x88E4
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
static void (*glBindBuffer)(GLenum, GLuint);
static void (*glBufferData)(GLenum, ptrdiff_t, const void *, GLenum);
static void (*glPixelStorei)(GLenum, GLint);
static void (*glGetIntegerv)(GLenum, GLint *);
static GLenum (*glGetError)(void);
static void (*glGenBuffers)(int, GLuint *);
static void (*glDeleteBuffers)(int, const GLuint *);
static int load_gl(void) {
    glBindBuffer = (void (*)(GLenum, GLuint))glfwGetProcAddress("glBindBuffer");
    glBufferData = (void (*)(GLenum, ptrdiff_t, const void *, GLenum))glfwGetProcAddress("glBufferData");
    glPixelStorei = (void (*)(GLenum, GLint))glfwGetProcAddress("glPixelStorei");
    glGetIntegerv = (void (*)(GLenum, GLint *))glfwGetProcAddress("glGetIntegerv");
    glGetError = (GLenum (*)(void))glfwGetProcAddress("glGetError");
    glGenBuffers = (void (*)(int, GLuint *))glfwGetProcAddress("glGenBuffers");
    glDeleteBuffers = (void (*)(int, const GLuint *))glfwGetProcAddress("glDeleteBuffers");
    return glBindBuffer && glBufferData && glPixelStorei && glGetIntegerv && glGetError && glGenBuffers && glDeleteBuffers;
}
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include "richmd_c.h"

static void glfw_error(int code, const char *message) {
    fprintf(stderr, "GLFW %d: %s\n", code, message);
}
/* Deliberately bind undersized PBO and nondefault layout. Without adapter
 * normalization, transfers fail or treat CPU pointers as PBO offsets. */
static void transfer_state(GLenum target, GLuint buffer) {
    int unpack = target == GL_PIXEL_UNPACK_BUFFER;
    glBindBuffer(target, buffer);
    if (buffer) glBufferData(target, 4, NULL, GL_STATIC_DRAW);
    glPixelStorei(unpack ? GL_UNPACK_ALIGNMENT : GL_PACK_ALIGNMENT, buffer ? 8 : 4);
    glPixelStorei(unpack ? GL_UNPACK_ROW_LENGTH : GL_PACK_ROW_LENGTH, buffer ? 19 : 0);
    glPixelStorei(unpack ? GL_UNPACK_SKIP_ROWS : GL_PACK_SKIP_ROWS, buffer ? 1 : 0);
    glPixelStorei(unpack ? GL_UNPACK_SKIP_PIXELS : GL_PACK_SKIP_PIXELS, buffer ? 2 : 0);
}
static int transfer_restored(GLenum target, GLuint buffer) {
    int unpack = target == GL_PIXEL_UNPACK_BUFFER;
    const GLenum properties[] = {unpack ? GL_UNPACK_ALIGNMENT : GL_PACK_ALIGNMENT,
        unpack ? GL_UNPACK_ROW_LENGTH : GL_PACK_ROW_LENGTH,
        unpack ? GL_UNPACK_SKIP_ROWS : GL_PACK_SKIP_ROWS,
        unpack ? GL_UNPACK_SKIP_PIXELS : GL_PACK_SKIP_PIXELS,
        unpack ? GL_PIXEL_UNPACK_BUFFER_BINDING : GL_PIXEL_PACK_BUFFER_BINDING};
    const GLint expected[] = {8, 19, 1, 2, (GLint)buffer};
    size_t i;
    for (i = 0; i < sizeof(properties) / sizeof(properties[0]); ++i) {
        GLint value = 0;
        glGetIntegerv(properties[i], &value);
        if (value != expected[i]) return 0;
    }
    return glGetError() == GL_NO_ERROR;
}
int main(int argc, char **argv) {
    GLFWwindow *window = NULL;
    ImGuiContext *context = NULL;
    GLuint transfer_buffer = 0;
    int initialized = 0, ok = 0, frames = 0, image_seen = 0;
    const char *markdown = "# Native bridge\n\nC99 calls **RichMd** through native dylib.\n\n![Image](images/world.png)\n";
    const size_t layout[] = {sizeof(ImGuiStoragePair), offsetof(ImGuiStoragePair, value),
        sizeof(ImGuiStyleMod), offsetof(ImGuiStyleMod, backup),
        sizeof(ImGuiInputEvent), offsetof(ImGuiInputEvent, event)};
    if (argc != 3) { fprintf(stderr, "Usage: bridge-check <assets-folder> <output.ppm>\n"); return 2; }
    glfwSetErrorCallback(glfw_error);
    if (!glfwInit()) goto cleanup;
    initialized = 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    window = glfwCreateWindow(640, 480, "Attendant bridge check", NULL, NULL);
    if (!window) goto cleanup;
    glfwMakeContextCurrent(window);
    if (!load_gl()) goto cleanup;
    context = igCreateContext(NULL);
    if (!context) goto cleanup;
    igGetIO_Nil()->IniFilename = NULL;
    if (!igDebugCheckVersionAndDataLayout(igGetVersion(), sizeof(ImGuiIO), sizeof(ImGuiStyle),
        sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(ImDrawIdx)) ||
        !crmd_check_cimgui_layout(layout, sizeof(layout) / sizeof(layout[0])) ||
        !crmd_init(window, argv[1], NULL, NULL)) goto cleanup;
    glGenBuffers(1, &transfer_buffer);
    if (!transfer_buffer) goto cleanup;
    for (frames = 0; frames < 4; ++frames) {
        crmd_preview_selection selection;
        glfwPollEvents();
        if (!crmd_begin_frame()) goto cleanup;
        igSetNextWindowPos((ImVec2){0, 0}, ImGuiCond_Always, (ImVec2){0, 0});
        igSetNextWindowSize((ImVec2){640, 480}, ImGuiCond_Always);
        igBegin("Bridge", NULL, ImGuiWindowFlags_NoDecoration);
        transfer_state(GL_PIXEL_UNPACK_BUFFER, transfer_buffer);
        if (!crmd_render_preview("bridge", 1, markdown, strlen(markdown), &selection, sizeof(selection)) ||
            !transfer_restored(GL_PIXEL_UNPACK_BUFFER, transfer_buffer)) goto cleanup;
        transfer_state(GL_PIXEL_UNPACK_BUFFER, 0);
        igEnd();
        /* Asymmetric markers prove P6 row orientation, not only color diversity. */
        ImDrawList_AddRectFilled(igGetForegroundDrawList_Nil(), (ImVec2){10, 10}, (ImVec2){30, 30}, 0xff0000ffu, 0, 0);
        ImDrawList_AddRectFilled(igGetForegroundDrawList_Nil(), (ImVec2){10, 450}, (ImVec2){30, 470}, 0xffff0000u, 0, 0);
        if (!crmd_end_frame() || crmd_vertex_count() <= 0) goto cleanup;
        image_seen |= crmd_live_images() > 0;
        if (frames == 3) {
            transfer_state(GL_PIXEL_PACK_BUFFER, transfer_buffer);
            if (!crmd_screenshot(argv[2]) || !transfer_restored(GL_PIXEL_PACK_BUFFER, transfer_buffer)) goto cleanup;
            transfer_state(GL_PIXEL_PACK_BUFFER, 0);
        }
        if (!crmd_present()) goto cleanup;
    }
    if (!image_seen) { fprintf(stderr, "Expected live Markdown image texture\n"); goto cleanup; }
    /* Failed nested render must abandon frame; shutdown must free image textures. */
    if (!crmd_begin_frame()) goto cleanup;
    igBegin("Failure", NULL, 0);
    if (crmd_check_render_failure() || !crmd_render_failed() || crmd_end_frame() || crmd_present()) goto cleanup;
    ok = 1;
cleanup:
    if (!ok) fprintf(stderr, "Bridge check failed: %s\n", crmd_error());
    if (!crmd_shutdown() || crmd_live_images() != 0) ok = 0;
    if (transfer_buffer) glDeleteBuffers(1, &transfer_buffer);
    if (context) igDestroyContext(context);
    if (window) glfwDestroyWindow(window);
    if (initialized) glfwTerminate();
    printf("{\"ok\":%s,\"frames\":%d,\"image_seen\":%s,\"live_images\":%d}\n",
        ok ? "true" : "false", frames, image_seen ? "true" : "false", crmd_live_images());
    return ok ? 0 : 1;
}
