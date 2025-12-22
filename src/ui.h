#ifndef UI_H
#define UI_H

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ImGuiZmo.h"
#include "build.h"

#include <GLFW/glfw3.h>

bool showBuildWindow=false;


class UI
{
    public:
    // ===== ImGui init =====
    void UIinit(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); 
        (void)io;
        io.FontGlobalScale = 1.5f; 
        
        ImGui::StyleColorsDark();

        // 绑定到 GLFW + OpenGL3
        ImGui_ImplGlfw_InitForOpenGL(window, false);
        ImGui_ImplOpenGL3_Init("#version 330");
    }
    void BeginUI()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

    }

    void DrawUI(BuildSystem& build)
    {
        
        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("Window"))
            {
                ImGui::MenuItem("build",nullptr,&showBuildWindow);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        if(showBuildWindow)
        {
            ImGui::Begin("build",&showBuildWindow);
            if(ImGui::Button("cube"))
            {
                SceneObject obj;
                obj.id=build.nextId++;
                obj.model=glm::mat4(1.0f);
                obj.selected=false;
                build.objects.push_back(obj);
            }
            ImGui::End();
        }
    }
    void EndUI()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    //释放imgui
    void ReleaseUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

};



#endif