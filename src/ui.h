#ifndef UI_H
#define UI_H

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ImGuizmo.h"
#include "build.h"

#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

bool showBuildWindow=false;
bool showTransformWindow=false;


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

    void DrawUI(BuildSystem& build,ImGuizmo::OPERATION& gizmoOp,ImGuizmo::MODE& gizmoMode)
    {
        
        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("Window"))
            {
                ImGui::MenuItem("build",nullptr,&showBuildWindow);
                ImGui::MenuItem("transform",nullptr,&showTransformWindow);
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
        if(showTransformWindow)
        {
            ImGui::SetNextWindowPos(ImVec2(10,40),ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(120,140),ImGuiCond_Always);
            ImGui::Begin("transform",&showTransformWindow,
                            ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove);

            if(ImGui::Button("move",ImVec2(-1,0))) gizmoOp=ImGuizmo::TRANSLATE;
            if(ImGui::Button("rotate",ImVec2(-1,0))) gizmoOp=ImGuizmo::ROTATE;
            if(ImGui::Button("scale",ImVec2(-1,0))) gizmoOp=ImGuizmo::SCALE;

            ImGui::End();
        }
        if(build.selectedId!=-1)
        {
            ImGuiIO& io=ImGui::GetIO();
            ImVec2 size(361,200);
            ImVec2 pos(io.DisplaySize.x-size.x-10,40);
            ImGui::SetNextWindowPos(pos,ImGuiCond_Always);
            ImGui::SetNextWindowSize(size,ImGuiCond_Always);

            ImGui::Begin("inspector",nullptr,ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove);

            ImGui::Text("mode");
            ImGui::SameLine();//不换行
            if(ImGui::RadioButton("TR",gizmoOp==ImGuizmo::TRANSLATE))gizmoOp=ImGuizmo::TRANSLATE;
            ImGui::SameLine();
            if(ImGui::RadioButton("RT",gizmoOp==ImGuizmo::ROTATE))gizmoOp=ImGuizmo::ROTATE;
            ImGui::SameLine();
            if(ImGui::RadioButton("SC",gizmoOp==ImGuizmo::SCALE))gizmoOp=ImGuizmo::SCALE;

            if(ImGui::RadioButton("local",gizmoMode==ImGuizmo::LOCAL))gizmoMode=ImGuizmo::LOCAL;
            ImGui::SameLine();
            if(ImGui::RadioButton("world",gizmoMode==ImGuizmo::WORLD))gizmoMode=ImGuizmo::WORLD;

            SceneObject* selected=nullptr;
            for(auto& o:build.objects)
            {
                if(o.id==build.selectedId){selected=&o;break;}
            }
            if(selected)
            {
                float t[3],r[3],s[3];
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(selected->model), t, r, s);
                ImGui::InputFloat3("tr",t);
                ImGui::InputFloat3("rt",r);
                ImGui::InputFloat3("sc",s);
                ImGuizmo::RecomposeMatrixFromComponents(t,r,s,glm::value_ptr(selected->model));
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