#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include<iostream>
#include<cmath>
#include<stb_image.h>

#include<ui.h>
#include<grid.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <camera.h>
#include<shader.h>
#include<model.h>
#include<build.h>
#include<raycaster.h>
#include "ImGuizmo.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
unsigned int LoadTexture2D(const char* path);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

float deltaTime = 0.0f;
float lastFrame = 0.0f; 
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
UI ui;
Grid grid;
BuildSystem build;
Raycaster raycaster;

ImGuizmo::OPERATION gizmoOp=ImGuizmo::TRANSLATE;
ImGuizmo::MODE gizmoMode=ImGuizmo::WORLD;

unsigned int indices[]=
{
    0,1,3,
    1,2,3
};

//世界坐标
glm::vec3 cubePositions[] = 
{
    glm::vec3( 0.0f,  0.0f,  0.0f), 
    glm::vec3( 2.0f,  5.0f, -15.0f), 
    glm::vec3(-1.5f, -2.2f, -2.5f),  
    glm::vec3(-3.8f, -2.0f, -12.3f),  
    glm::vec3( 2.4f, -0.4f, -3.5f),  
    glm::vec3(-1.7f,  3.0f, -7.5f),  
    glm::vec3( 1.3f, -2.0f, -2.5f),  
    glm::vec3( 1.5f,  2.0f, -2.5f), 
    glm::vec3( 1.5f,  0.2f, -1.5f), 
    glm::vec3(-1.3f,  1.0f, -1.5f)  
};

glm::vec3 pointLightPositions[] = {
    glm::vec3( 0.7f,  0.2f,  2.0f),
    glm::vec3( 2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3( 0.0f,  0.0f, -3.0f)
};

//------------TRANSFORM--------------
static bool wireframe = false;
float x=1.0f;
float zrotation=0.0f;
float xrotation=0.0f;
float yrotation=0.0f;
glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 pos =glm::vec3(0.0f,0.0f,0.0f);

//鼠标初始位置
float lastX = 400, lastY = 300;
bool firstMouse=true;

float cameraControl=false;

//灯的位置
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

bool usingFlashlight=false;




int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    stbi_set_flip_vertically_on_load(true);

    Model ourModel("backpack/backpack.obj");

    //初始化默认立方体
    build.cube.Init();


    //设置灯的数据
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // 只需要绑定VBO不用再次设置VBO的数据，因为箱子的VBO数据中已经包含了正确的立方体顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, build.cube.VBO);
    // 设置灯立方体的顶点属性（对我们的灯来说仅仅只有位置数据）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Shader ourShader("../../src/shader/shader.vs", "../../src/shader/shader.fs");
    Shader lampShader("../../src/shader/lightShader.vs","../../src/shader/lightShader.fs");
    Shader gridShader("../../src/gridshader/grid.vs",
                  "../../src/gridshader/grid.fs");

    ui.UIinit(window);

    Grid grid;
    grid.init(gridShader, 50.0f);

    //启用深度缓冲
    glEnable(GL_DEPTH_TEST);
    
    

    glm::mat4 view;
    view=glm::lookAt(glm::vec3(0.0f,0.0f,3.0f),
                     glm::vec3(0.0f,0.0f,0.0f),
                     glm::vec3(0.0f,1.0f,0.0f));

    float radius = 10.0f;
    
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //------------创建生成纹理对象---------
    unsigned int texture1 = LoadTexture2D("container2.png");
    unsigned int specularMap = LoadTexture2D("container2_specular.png");
    
    while(!glfwWindowShouldClose(window))
    {
        //检查是否按下右键
        bool rmbDown = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
        if (rmbDown && !cameraControl)
        {
            cameraControl = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; // 关键：防止第一次进入时镜头“猛跳”
        }
        else if (!rmbDown && cameraControl)
        {
            cameraControl = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        
        ourShader.use();
        ourShader.setFloat("u_Tiling", x);
        
        
        
        
        //变换
        glm::mat4 trans;
        trans = glm::rotate(trans, glm::radians(zrotation), glm::vec3(0.0, 0.0, 1.0));
        trans =glm::rotate(trans,glm::radians(xrotation),glm::vec3(1.0,0.0,0.0));
        trans =glm::rotate(trans,glm::radians(yrotation),glm::vec3(0.0,1.0,0.0));
        trans = glm::scale(trans, scale);
        //位移
        trans = glm::translate(trans, pos);
        //旋转
        
        float camX = sin(glfwGetTime()) * radius;
        float camZ = cos(glfwGetTime()) * radius;
        
        
        
        ourShader.setMat4("transform",trans);
        
        glm::mat4 view = camera.GetViewMatrix(); 
        ourShader.setMat4("view",view);
        
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)800.0f / (float)600.0f, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        
        ourShader.setVec3("viewPos",camera.Position);
        

        ourShader.setFloat("material.shininess",32.0f);
        //材质
        
        ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        ourShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[0].constant", 1.0f);
        ourShader.setFloat("pointLights[0].linear", 0.09f);
        ourShader.setFloat("pointLights[0].quadratic", 0.032f);
        // point light 2
        ourShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        ourShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[1].constant", 1.0f);
        ourShader.setFloat("pointLights[1].linear", 0.09f);
        ourShader.setFloat("pointLights[1].quadratic", 0.032f);
        // point light 3
        ourShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        ourShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[2].constant", 1.0f);
        ourShader.setFloat("pointLights[2].linear", 0.09f);
        ourShader.setFloat("pointLights[2].quadratic", 0.032f);
        // point light 4
        ourShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        ourShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("pointLights[3].constant", 1.0f);
        ourShader.setFloat("pointLights[3].linear", 0.09f);
        ourShader.setFloat("pointLights[3].quadratic", 0.032f);
        // spotLight
        ourShader.setVec3("spotLight.position", camera.Position);
        ourShader.setVec3("spotLight.direction", camera.Front);
        ourShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        ourShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("spotLight.constant", 1.0f);
        ourShader.setFloat("spotLight.linear", 0.09f);
        ourShader.setFloat("spotLight.quadratic", 0.032f);
        ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));   

        //灯的模型
        glm::mat4 model=glm::mat4();
        model=glm::translate(model,lightPos);
        model=glm::scale(model,glm::vec3(0.2f));

        lampShader.use();

        lampShader.setMat4("view",view);
        lampShader.setMat4("projection",projection);
        lampShader.setMat4("model",model);
        
        // 用 UI 控制渲染状态/参数（示例）
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //清除深度缓冲和清屏
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        
        ourShader.use();

        ourShader.setInt("material.texture_diffuse1", 0);
        ourShader.setInt("material.texture_specular1", 1);

        //ourModel.Draw(ourShader);

        for (auto& obj : build.objects)
        {
            ourShader.setMat4("transform", obj.model);
            build.cube.Draw();
            //选中时显示线框
            if (obj.selected)
            {
                glDisable(GL_DEPTH_TEST);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                lampShader.use();
                lampShader.setMat4("view", view);
                lampShader.setMat4("projection", projection);

                glm::mat4 outline = obj.model;
                outline = glm::scale(outline, glm::vec3(1.03f)); // 稍微放大
                lampShader.setMat4("model", outline);

                build.cube.Draw();

                // 恢复状态（如果你有 wireframe 开关，恢复它）
                glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
                glEnable(GL_DEPTH_TEST);
            }
        }


        //绘制光源
        lampShader.use();
        lampShader.setMat4("view", view);
        lampShader.setMat4("projection", projection);
        glBindVertexArray(lightVAO);
        for (unsigned int i = 0; i < 4; i++)
         {
             model = glm::mat4(1.0f);
             model = glm::translate(model, pointLightPositions[i]);
             model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
             lampShader.setMat4("model", model);
             glDrawArrays(GL_TRIANGLES, 0, 36);
         }

        // ===== ImGui draw =====
        ui.BeginUI();
        ImGuizmo::BeginFrame();
        ui.DrawUI(build,gizmoOp,gizmoMode);
        
        //调用imguizmo
        if (build.selectedId != -1)
        {
            // 找到选中的对象
            for (auto& obj : build.objects)
            {
                if (obj.id == build.selectedId)
                {
                    ImGuizmo::SetOrthographic(false);
                    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

                    // ImGuizmo 需要屏幕尺寸
                    ImGuiIO& io = ImGui::GetIO();
                    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

                    // 使用相机矩阵
                    ImGuizmo::Manipulate(
                        glm::value_ptr(view),
                        glm::value_ptr(projection),
                        gizmoOp,
                        gizmoMode,
                        glm::value_ptr(obj.model)
                    );

                    break;
                }
            }
        }
        ui.EndUI();

        lampShader.setMat4("model", glm::mat4(1.0f));
        grid.draw(view, projection, camera.Position);
        
        glfwSwapBuffers(window);//显示窗口
        glfwPollEvents(); 

        
    }

    ui.ReleaseUI();

    glfwTerminate();


    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime;
    if(!cameraControl) return;


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD,deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD,deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT,deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT,deltaTime);
}

//鼠标输入
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    if(!cameraControl) return;

    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset,yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // 如果 ImGui 正在使用鼠标，就不选
        if (ImGui::GetIO().WantCaptureMouse) return;

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)width / (float)height, 0.1f, 100.0f);

        raycaster.PickObject(mx, my, width, height, view, projection, build);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

//加载纹理对象
unsigned int LoadTexture2D(const char* path)
{
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int w, h, c;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &c, 0);
    if (data)
    {
        GLenum format = (c == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture: " << path
                  << " reason: " << stbi_failure_reason() << std::endl;
    }
    stbi_image_free(data);

    return tex;
}
