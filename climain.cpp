// ImGui - standalone example application for Glfw + OpenGL 2, using fixed pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.

#include "imgui/imgui.h"
#include "imgui_backend/imgui_impl_glfw.h"
#include <stdio.h>
#include <GLFW/glfw3.h>

extern "C"{
#include "configure/stdafx.h"

#include "zhwkre/bss.h"
#include "zhwkre/list.h"
#include "zhwkre/concurrent.h"

#include "clinet/clinet.h"
}

typedef struct q_Messeage_st{unsigned char qid;binary_safe_string payload;} Messeage;

#define PUSH(num) (((ui)1)<<(num))
#define SETSTAT(stat) (curr_status |= (stat))
#define CLRSTAT(stat) (curr_status &= ~(stat))
#define CHKSTAT(stat) (curr_status & (stat))

typedef unsigned int ui;

ui curr_status = 0;
// some_consts
const ui uistat_login = PUSH(0);
const ui uistat_process_login = PUSH(1);

static void error_callback(int error, const char* description){
    fprintf(stderr, "Error %d: %s\n", error, description);
}


int main(int argc, char** argv)
{
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "CSMcli -- powered by ImGui/OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    // Setup ImGui binding
    ImGui_ImplGlfw_Init(window, true);
    { // fonts reference
        // Load Fonts
        // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
        //ImGuiIO& io = ImGui::GetIO();
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
        //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
        //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
        //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    }
    ImVec4 clear_color = ImColor(114, 144, 154);
    // define basic globalvars
        char usernamebuffer[256],passwordbuffer[256];
        char serverbuffer[256];
    // setup basic envirs
    {
        SETSTAT(uistat_login);
        memset(usernamebuffer,0,256);
        memset(passwordbuffer,0,256);
        memset(serverbuffer,0,256);
        qList_initdesc(ui_notifier);
        qList_initdesc(network_notifier);
        ui_noti_lock = qMutex_constructor();
        net_noti_lock = qMutex_constructor();
        qRun(handle_network,NULL);// start process
    }

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplGlfw_NewFrame();
        /* references
        {
            // 1. Show a simple window
            // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
            {
                static float f = 0.0f;
                ImGui::Text("Hello, world!");
                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
                ImGui::ColorEdit3("clear color", (float*)&clear_color);
                if (ImGui::Button("Test Window")) show_test_window ^= 1;
                if (ImGui::Button("Another Window")) show_another_window ^= 1;
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            }

            // 2. Show another simple window, this time using an explicit Begin/End pair
            if (show_another_window)
            {
                ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
                ImGui::Begin("Another Window", &show_another_window);
                ImGui::Text("Hello");
                ImGui::End();
            }

            // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
            if (show_test_window)
            {
                ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
                ImGui::ShowTestWindow(&show_test_window);
            }
        }*/

        if(CHKSTAT(uistat_login)){
            ImGui::Begin("Login or Register");
            ImGui::Text("Server format: 127.0.0.1:1992");
            ImGui::InputText("Server",serverbuffer,256);
            ImGui::InputText("Username",usernamebuffer,256);
            ImGui::InputText("Password",passwordbuffer,256,ImGuiInputTextFlags_Password);
            if(ImGui::Button("Login")){
                fprintf(stderr,"Attempt login with %s + %s to %s\n",usernamebuffer,passwordbuffer,serverbuffer);
                // TODO:send request to network notifier
                CLRSTAT(uistat_login);
                SETSTAT(uistat_process_login);
            }
            ImGui::SameLine();
            if(ImGui::Button("Register")){
                fprintf(stderr,"Attempt register with %s + %s to %s\n",usernamebuffer,passwordbuffer,serverbuffer);
                // TODO:send request to network handle
                CLRSTAT(uistat_login);
                SETSTAT(uistat_process_login);
            }
            ImGui::End();
        }
        if(CHKSTAT(uistat_process_login)){
            ImGui::Begin("Please Wait");
            ImGui::Text("Connecting to server..");
            ImGui::End();
        }

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfw_Shutdown();
    glfwTerminate();

    return 0;
}
