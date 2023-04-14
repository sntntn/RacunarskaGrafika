#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <cmath>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string> faces);

bool priblizi(float& s, float k); //priblizava s ka k za +-0.01
void pribliziFast(float& s,float k); //priblizava s ka k za +-0.1

// settings
const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 900;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int br=0;
int pom=0;
bool active_p1=false;
bool active_p2=false;
bool active_p3=false;
bool pojedi_p1=false;
bool pojedi_p2=false;
bool pojedi_p3=false;
bool efekat1=false;
bool efekat2=false;
bool efekat3=false;

//glm::vec3 krajnjaTacka = glm::vec3(9.80, -0.11, -6.04);
glm::vec3 krajnjaTacka = glm::vec3(0,0,0);
glm::vec3 padobran1 =glm::vec3(0, 0, 0);
glm::vec3 padobran2 =glm::vec3(0, 0, 0);
glm::vec3 padobran3 =glm::vec3(0, 0, 0);
float p1_scale = 2.0;
float p2_scale = 1.0;
float p3_scale = 1.0;
float p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;      //kordinate svakog padobrana


//------------------------------------------------------

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 ribaPosition = glm::vec3(0.0f);
    float ribaScale = 5.0f;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    //-------------------------------------------------------------------------------------------------------------------Ovde je zavrsena inicijalizacija
    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader obicanPadobranShader("resources/shaders/2.model_lighting.vs", "resources/shaders/obicanPadobran.fs");


    float skyboxVertices[] = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // load cubemap textures
    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/skybox/right.jpg"),
                    FileSystem::getPath("resources/textures/skybox/left.jpg"),
                    FileSystem::getPath("resources/textures/skybox/top.jpg"),
                    FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
                    FileSystem::getPath("resources/textures/skybox/front.jpg"),
                    FileSystem::getPath("resources/textures/skybox/back.jpg")
            };

    stbi_set_flip_vertically_on_load(false);
    unsigned int cubemapTexture = loadCubemap(faces);
    stbi_set_flip_vertically_on_load(true);                         //vracamo na true -> zelim da mi to bude default
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);


    // load models
    // -----------
    Model ourModel("resources/objects/riba/riba.obj");
    ourModel.SetShaderTextureNamePrefix("material.");
    Model riba1Model("resources/objects/riba1/riba1.obj");
    riba1Model.SetShaderTextureNamePrefix("material.");
    Model riba2Model("resources/objects/riba2/riba2.obj");
    riba1Model.SetShaderTextureNamePrefix("material.");
    Model padobranModel("resources/objects/padobran/padobran.obj");
    padobranModel.SetShaderTextureNamePrefix("material.");
    Model zvezdaModel("resources/objects/zvezda/zvezda.obj");
    zvezdaModel.SetShaderTextureNamePrefix("material.");

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;



    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();
        pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        br++;
        if(br%500==0 || pom==1){        //ovo je za sada nakon 500 iteracija, a zapravo nam treba na odredjeno osvetljenje
            pom=1;br=0;
            ourShader.setFloat("material.shininess", 32.0f);
        }


        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);


        // render the loaded model                                          -> GLAVNA ZLATNA RIBICA
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,programState->ribaPosition);
        model = glm::scale(model, glm::vec3(programState->ribaScale));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);


        //render zvezda1 rotirajuca
        if(!pojedi_p1){                                     //ako nije pojeden odgovarajuci padobranac nema jos punu boju
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sin(glfwGetTime() / 2) * 17, -3.0f, cos(glfwGetTime() / 2) * 17));
            model = glm::rotate(model, (float)glfwGetTime()/2-82, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(2.0f));
            ourShader.setMat4("model", model);
            zvezdaModel.Draw(ourShader);
        }


        //render zvezda2 rotirajuca
        if(!pojedi_p2){
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sin(glfwGetTime() / 4) * 14.5, -9.0f, cos(glfwGetTime() / 2) * 14.5));
            model = glm::rotate(model, (float)glfwGetTime()/2-82, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f));
            ourShader.setMat4("model", model);
            zvezdaModel.Draw(ourShader);
        }

        //render zvezda3 rotirajuca
        if(!pojedi_p3){
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-sin(glfwGetTime() / 4) * 4, -9.0f, -cos(glfwGetTime() / 2) * 4));
            model = glm::rotate(model, (float)glfwGetTime()*3, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f));
            ourShader.setMat4("model", model);
            zvezdaModel.Draw(ourShader);
        }


        obicanPadobranShader.use();                                                             //za nas drugi shader
//        obicanPadobranShader.setVec3("pointLight.position", pointLight.position);
//        obicanPadobranShader.setVec3("pointLight.ambient", pointLight.ambient);
//        obicanPadobranShader.setVec3("pointLight.diffuse", pointLight.diffuse);
//        obicanPadobranShader.setVec3("pointLight.specular", pointLight.specular);
//        obicanPadobranShader.setFloat("pointLight.constant", pointLight.constant);
//        obicanPadobranShader.setFloat("pointLight.linear", pointLight.linear);
//        obicanPadobranShader.setFloat("pointLight.quadratic", pointLight.quadratic);
//        obicanPadobranShader.setVec3("viewPosition", programState->camera.Position);
//        obicanPadobranShader.setFloat("material.shininess", 32.0f);

        obicanPadobranShader.setMat4("projection", projection);
        obicanPadobranShader.setMat4("view", view);


        //render padobran u ribi
        model = glm::mat4(1.0f);
        model = glm::translate(model, krajnjaTacka);
        model = glm::rotate(model, glm::radians(-75.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f));
        obicanPadobranShader.setMat4("model", model);
        padobranModel.Draw(obicanPadobranShader);

        //render padobran1 ka ribi
        model = glm::mat4(1.0f);
        if(!pojedi_p1){
            p1x=sin(glfwGetTime() / 2) * 17;
            p1y=sin(glfwGetTime() / 4) * 5 +10.0f;
            p1z=cos(glfwGetTime() / 2) * 17;
            model = glm::translate(model, glm::vec3(p1x, p1y, p1z));
        }
        else{
            pribliziFast(p1x,0);
            pribliziFast(p1y,0);
            pribliziFast(p1z,0);
            model = glm::translate(model, glm::vec3(p1x, p1y, p1z));
        }
        model = glm::rotate(model, (float)glfwGetTime()/2-82+180, glm::vec3(0.0f, 1.0f, 0.0f));
        if(active_p1){
            pojedi_p1=priblizi(p1_scale,0.2);
            model = glm::scale(model, glm::vec3(p1_scale));
        }
        else {
            model = glm::scale(model, glm::vec3(p1_scale));
        }
        obicanPadobranShader.setMat4("model", model);
        padobranModel.Draw(obicanPadobranShader);
        //----------------------------------------------------------------------------
        if(pojedi_p1){                          //zvezda sa punom bojom -> drugim shaderom kada je zavrseno skaliranje padobrana
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sin(glfwGetTime() / 2) * 17, -3.0f, cos(glfwGetTime() / 2) * 17));
            model = glm::rotate(model, (float)glfwGetTime()*4, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(2.0f));
            obicanPadobranShader.setMat4("model", model);
            zvezdaModel.Draw(ourShader);
        }//----------------------------------------------------------------------------


        //render padobran2 ka ribi
        model = glm::mat4(1.0f);

        if(!pojedi_p2){
            p2x=sin(glfwGetTime() / 4) * 14.5;
            p2y=-sin(glfwGetTime() / 4) * 9 +9.0f;
            p2z=cos(glfwGetTime() / 2) * 14.5;
            model = glm::translate(model, glm::vec3(p2x, p2y, p2z));
        }
        else{
            pribliziFast(p2x,0);
            pribliziFast(p2y,0);
            pribliziFast(p2z,0);
            model = glm::translate(model, glm::vec3(p2x, p2y, p2z));
        }
        model = glm::rotate(model, (float)glfwGetTime()/2-82, glm::vec3(0.0f, 1.0f, 0.0f));
        if(active_p2){
            pojedi_p2=priblizi(p2_scale,0.2);
            model = glm::scale(model, glm::vec3(p2_scale));
        }
        else {
            model = glm::scale(model, glm::vec3(p2_scale));
        }
        obicanPadobranShader.setMat4("model", model);
        padobranModel.Draw(obicanPadobranShader);
        //------------------------------------------------------------------------------
        if(pojedi_p2){                  //zvezda sa punom bojom -> drugim shaderom kada je zavrseno skaliranje padobrana
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sin(glfwGetTime() / 4) * 14.5, -9.0f, cos(glfwGetTime() / 2) * 14.5));
            model = glm::rotate(model, (float)glfwGetTime()*4, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f));
            obicanPadobranShader.setMat4("model", model);
            zvezdaModel.Draw(ourShader);
        }//-----------------------------------------------------------------------------

        //render padobran3 ka ribi
        model = glm::mat4(1.0f);
        if(!pojedi_p3){
            p3x=-sin(glfwGetTime() / 4) * 4;
            p3y=-sin(glfwGetTime() / 4) * 6 +9.0f;
            p3z=-cos(glfwGetTime() / 2) * 4;
            model = glm::translate(model, glm::vec3(p3x, p3y, p3z));
        }
        else{
            pribliziFast(p3x,0);
            pribliziFast(p3y,0);
            pribliziFast(p3z,0);
            model = glm::translate(model, glm::vec3(p3x, p3y, p3z));
        }
        model = glm::rotate(model, (float)glfwGetTime()*3, glm::vec3(0.0f, 1.0f, 0.0f));
        if(active_p3){
            pojedi_p3=priblizi(p3_scale,0.2);
            model = glm::scale(model, glm::vec3(p3_scale));
        }
        else {
            model = glm::scale(model, glm::vec3(p3_scale));    // it's a bit too big for our scene, so scale it down
        }
        obicanPadobranShader.setMat4("model", model);
        padobranModel.Draw(obicanPadobranShader);
        //------------------------------------------------------------------------------
        if(pojedi_p3){                      //zvezda sa punom bojom -> drugim shaderom kada je zavrseno skaliranje padobrana
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-sin(glfwGetTime() / 4) * 4, -9.0f, -cos(glfwGetTime() / 2) * 4));
            model = glm::rotate(model, (float)glfwGetTime()*4, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f));
            obicanPadobranShader.setMat4("model", model);
            zvezdaModel.Draw(ourShader);
        }//------------------------------------------------------------------------------



        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        if (programState->ImGuiEnabled)
            DrawImGui(programState);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        ImGui::DragFloat3("Backpack position", (float*)&programState->ribaPosition);
        ImGui::DragFloat("Backpack scale", &programState->ribaScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {                                            //za lociranje kordinata
        cout << programState->camera.Position.x << " "
             << programState->camera.Position.y << " "
             << programState->camera.Position.z << '\n';
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        active_p1=true;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        active_p2=true;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        active_p3=true;
    }
}

bool priblizi(float& s, float k){                       //funkcija ce da se koristi za priblizavanje vrednosti s ka vrednosti k
    if(s >= k-0.01 && s <= k+0.01)
        return true;                    //stigli smo do zeljenog opsega
    if(s < k)
        s += 0.01;
    else
        s -= 0.01;
    return false;
}

void pribliziFast(float& s,float k){
    if(s >= k-0.1 && s <= k+0.1)
        return;
    if(s < k)
        s += 0.1;
    else
        s -= 0.1;
}