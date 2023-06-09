#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "stb_image.h"

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
#include <string>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string> faces);

void setLights(Shader shaderName);

unsigned int loadTexture(char const * path, bool gammaCorrection);

void renderQuad();

bool priblizi(float& s, float k); //priblizava s ka k za +-0.01
bool pribliziFast(float& s,float k); //priblizava s ka k za +-0.1

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

//spotlight & blinn
bool spotlightOn = false;
bool blinn = true;

//hdr
bool hdr = false;
bool hdrKeyPressed = false;
float exposure = 1.0f;
bool bloom = false;
bool bloomKeyPressed = false;

// lightPos
glm::vec3 lightPos(0.0f, -4.5f, 0.0f);


bool active_p1=false;
bool active_p2=false;
bool active_p3=false;
bool pojedi_p1=false;
bool pojedi_p2=false;
bool pojedi_p3=false;
bool efekat1=false;
bool efekat2=false;
bool efekat3=false;
bool lokalna1=false;
bool lokalna2=false;
bool lokalna3=false;

float p1_scale = 2.0;
float p2_scale = 1.0;
float p3_scale = 1.0;
float p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;      //kordinate svakog padobrana
float pointLightconstant = 0.09f;
float pointLightquadratic = 0.039f;


//------------------------------------------------------

struct PointLight {             //prosledjujemo vrednosti iz shadera ovde u main
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

void ProgramState::SaveToFile(std::string filename) {                   //funkcija koja nam pamti prethodno stanje programa
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

void ProgramState::LoadFromFile(std::string filename) {                  //funkcija koja mi zapisuje ostavljeno stanje programa
    std::ifstream in(filename);     //posto koristimo nestruktuisan tekstualni fajl (koji ne zna sta je svaka od ovih vrednost)
    if (in) {                          //moramo da ucitavamo iz program state fajla istim redosledom, kojim smo upisivali neke vrednosti u taj fajl
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

ProgramState *programState;             //pravimo pokazivac (alociramo ga na hipu, a u data segmentu cuvamo pokazivac na to)

void DrawImGui(ProgramState *programState);

int main() {    //--------------------------------------------------------------------------------------------------------------------------------------------MAIN
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);          //pozivamo svaki put kad se velicina prozora promeni
    glfwSetCursorPosCallback(window, mouse_callback);                                               //GLFW nam pomaze ko IO operacija
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
    programState->LoadFromFile("resources/program_state.txt");                                              //ovde mi je zapamceno prethodno stanje programa
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

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //Blending
    glEnable(GL_BLEND);                                                           //dodajemo blending/providnost to ide nakon fragment shadera
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);                  //podesavamo faktor(ALPHA) za source i faktor(1-ALPHA) za destination



    //-------------------------------------------------------------------------------------------------------------------Ovde je zavrsena inicijalizacija
    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/sjaj.vs", "resources/shaders/sjaj.fs");
    Shader obicanPadobranShader("resources/shaders/sjaj.vs", "resources/shaders/sjaj.fs");
    Shader sjajShader("resources/shaders/sjaj.vs", "resources/shaders/sjaj.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader boxShader("resources/shaders/box.vs", "resources/shaders/box.fs");
    Shader cloudShader("resources/shaders/cloud.vs", "resources/shaders/cloud.fs");
    Shader hdrShader("resources/shaders/hdr.vs","resources/shaders/hdr.fs");
    Shader bloomShader("resources/shaders/blur.vs","resources/shaders/blur.fs");

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

    float transparentVertices[] = {
            // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    float boxVertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
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

    //transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // box VAO
    unsigned int boxVBO, boxVAO;
    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &boxVBO);

    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);

    glBindVertexArray(boxVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    vector<glm::vec3> clouds
            {
                    glm::vec3(-6.5f +5, 2.0f+3, -0.48f -2),
                    glm::vec3( 3.5f +5, 4.0f, 1.51f -2),
                    glm::vec3( 2.0f +5, 1.5f, 0.7f -2),
                    glm::vec3(-4.3f +5, 3.5f, -2.3f -2),
                    glm::vec3( 6.0f +5, 4.5f, -1.6f -2),
                    glm::vec3( 0.0f +5, 4.0f, -1.6f -2)

            };                      ///TODO podesiti kordinate

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


    unsigned int transparentTexture = loadTexture(FileSystem::getPath("resources/textures/cloud.png").c_str(), true);

    unsigned int boxDiffuse = loadTexture(FileSystem::getPath("resources/textures/red.jpg").c_str(), true);
    unsigned int boxSpecular = loadTexture(FileSystem::getPath("resources/textures/specular.jpg").c_str(), true);
    unsigned int boxAmbient = loadTexture(FileSystem::getPath("resources/textures/ambient.jpg").c_str(), true);

    boxShader.use();
    boxShader.setInt("material.ambient", 0);
    boxShader.setInt("material.diffuse", 1);
    boxShader.setInt("material.specular", 2);


    // load models
    // -----------                                                                          //dodato true za gama fju
    Model ourModel("resources/objects/riba/riba.obj", true);
    ourModel.SetShaderTextureNamePrefix("material.");
    Model padobranModel("resources/objects/padobran/padobran.obj",true);
    padobranModel.SetShaderTextureNamePrefix("material.");
    Model zvezdaModel("resources/objects/zvezda/zvezda.obj",true);
    zvezdaModel.SetShaderTextureNamePrefix("material.");
    Model coinModel("resources/objects/coin/coin.obj",true);
    coinModel.SetShaderTextureNamePrefix("material.");

    PointLight& pointLight = programState->pointLight;                      //ovde pravimo osvetljenje i namestamo im vrednosti
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = pointLightconstant;
    pointLight.linear = 0.09f;
    pointLight.quadratic = pointLightquadratic;

    bloomShader.use();
    bloomShader.setInt("image",0);
    ///--- hdr
    hdrShader.use();
    hdrShader.setInt("hdrBuffer",0);
    hdrShader.setInt("bloomBlur",1);


    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // color attachments we'll use for rendering
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }
    ///---


    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------

    while (!glfwWindowShouldClose(window)) {///--------------------------------------------------------------------------------------------------------Pocetak render petlje
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        std::sort(clouds.begin(), clouds.end(),
                  [cameraPosition = programState->camera.Position](const glm::vec3& a, const glm::vec3& b) {
                      float d1 = glm::distance(a, cameraPosition);
                      float d2 = glm::distance(b, cameraPosition);
                      return d1 > d2;
                  });

        // render---------------------------------------------------------------------------------------------------------------------------------------- pocetak crtanja modela
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                  //moramo da cistimo i depth buffer ako je enable testiranje dubine (osim ako zelimo neki efekat)

        ///---
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ///---
        // don't forget to enable shader before setting uniforms
        ourShader.use();
        setLights(ourShader);
        pointLight.position = glm::vec3(4.0 * cos(currentFrame), 3.0f, 4.0 * sin(currentFrame));
        ourShader.setVec3("light.position", pointLight.position);
        ourShader.setVec3("pointLights[0].position", pointLight.position);
        ourShader.setVec3("pointLights[0].ambient", pointLight.ambient);
        ourShader.setVec3("pointLights[0].diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLights[0].specular", pointLight.specular);
        if( (efekat1 && !lokalna1) || (efekat2 && !lokalna2) || (efekat3 && !lokalna3) ){
            if(efekat1 && !lokalna1)lokalna1=true;
            if(efekat2 && !lokalna2)lokalna2=true;
            if(efekat3 && !lokalna3)lokalna3= true;
            pointLightquadratic=pointLightquadratic-0.013f;
            pointLightconstant=pointLightconstant-0.03f;
        }
        ourShader.setFloat("pointLights[0].constant", pointLightconstant);
        ourShader.setFloat("pointLights[0].linear", pointLight.linear);
        ourShader.setFloat("pointLights[0].quadratic", pointLightquadratic);
        ourShader.setFloat("material.shininess", 100.0f);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        sjajShader.setInt("blinn", blinn);

        // render the main model                                          -> GLAVNA ZLATNA RIBICA
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,programState->ribaPosition);
        model = glm::scale(model, glm::vec3(programState->ribaScale));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        //render zvezda sunce
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.0 * cos(currentFrame), 3.0f, 4.0 * sin(currentFrame)));
        model = glm::rotate(model, (float)glfwGetTime()/2-82, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.8f));
        ourShader.setMat4("model", model);
        zvezdaModel.Draw(ourShader);




        obicanPadobranShader.use();      ///-----------------------------------------------------------------------------------obicanPadobranshader
        setLights(sjajShader);
        obicanPadobranShader.setFloat("material.shininess", 100.0f);
        // view/projection transformations
        projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                      (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();
        obicanPadobranShader.setMat4("projection", projection);
        obicanPadobranShader.setMat4("view", view);
        obicanPadobranShader.setInt("blinn", blinn);


/*        //render padobran u ribi
        model = glm::mat4(1.0f);
        model = glm::translate(model, krajnjaTacka);
        model = glm::rotate(model, glm::radians(-75.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f));
        obicanPadobranShader.setMat4("model", model);
        padobranModel.Draw(obicanPadobranShader);
*/
        //render padobran1 ka ribi
        model = glm::mat4(1.0f);
        if(!pojedi_p1){
            p1x=sin(glfwGetTime() / 2) * 17;
            p1y=sin(glfwGetTime() / 4) * 5 +10.0f;
            p1z=cos(glfwGetTime() / 2) * 17;
            model = glm::translate(model, glm::vec3(p1x, p1y, p1z));
        }
        else{
            bool pom1,pom2,pom3;
            pom1=pribliziFast(p1x,0);
            pom2=pribliziFast(p1y,0);
            pom3=pribliziFast(p1z,0);
            efekat1=pom1 && pom2 && pom3;
            model = glm::translate(model, glm::vec3(p1x, p1y, p1z));
        }
        model = glm::rotate(model, (float)glfwGetTime()/2-82+180, glm::vec3(0.0f, 1.0f, 0.0f));
        if(active_p1){
            obicanPadobranShader.setVec3("dirLight.ambient", 1.0f, 1.0f, 1.0f);        //aktivirali smo padobran, vise nije skriven
            pojedi_p1=priblizi(p1_scale,0.2);
            model = glm::scale(model, glm::vec3(p1_scale));
        }
        else {
            model = glm::scale(model, glm::vec3(p1_scale));
        }
        obicanPadobranShader.setMat4("model", model);
        padobranModel.Draw(obicanPadobranShader);
        obicanPadobranShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);        //vracamo ambijentalnu da ne bi uticalo na druge padobrane
        //----------------------------------------------------------------------------



        //render padobran2 ka ribi
        model = glm::mat4(1.0f);

        if(!pojedi_p2){
            p2x=sin(glfwGetTime() / 4) * 14.5;
            p2y=-sin(glfwGetTime() / 4) * 9 +9.0f;
            p2z=cos(glfwGetTime() / 2) * 14.5;
            model = glm::translate(model, glm::vec3(p2x, p2y, p2z));
        }
        else{
            bool pom1,pom2,pom3;
            pom1=pribliziFast(p2x,0);
            pom2=pribliziFast(p2y,0);
            pom3=pribliziFast(p2z,0);
            efekat2=pom1 && pom2 && pom3;
            model = glm::translate(model, glm::vec3(p2x, p2y, p2z));
        }
        model = glm::rotate(model, (float)glfwGetTime()/2-82, glm::vec3(0.0f, 1.0f, 0.0f));
        if(active_p2){
            obicanPadobranShader.setVec3("dirLight.ambient", 1.0f, 1.0f, 1.0f);        //aktivirali smo padobran, vise nije skriven
            pojedi_p2=priblizi(p2_scale,0.2);
            model = glm::scale(model, glm::vec3(p2_scale));
        }
        else {
            model = glm::scale(model, glm::vec3(p2_scale));
        }
        obicanPadobranShader.setMat4("model", model);
        padobranModel.Draw(obicanPadobranShader);
        obicanPadobranShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);        //vracamo ambijentalnu da ne bi uticalo na druge padobrane

        //------------------------------------------------------------------------------


        //render padobran3 ka ribi
        model = glm::mat4(1.0f);
        if(!pojedi_p3){
            p3x=-sin(glfwGetTime() / 4) * 4;
            p3y=-sin(glfwGetTime() / 4) * 6 +9.0f;
            p3z=-cos(glfwGetTime() / 2) * 4;
            model = glm::translate(model, glm::vec3(p3x, p3y, p3z));
        }
        else{
            bool pom1,pom2,pom3;
            pom1=pribliziFast(p3x,0);
            pom2=pribliziFast(p3y,0);
            pom3=pribliziFast(p3z,0);
            efekat3=pom1 && pom2 && pom3;
            model = glm::translate(model, glm::vec3(p3x, p3y, p3z));
        }
        model = glm::rotate(model, (float)glfwGetTime()*3, glm::vec3(0.0f, 1.0f, 0.0f));
        if(active_p3){
            obicanPadobranShader.setVec3("dirLight.ambient", 1.0f, 1.0f, 1.0f);        //aktivirali smo padobran, vise nije skriven
            pojedi_p3=priblizi(p3_scale,0.2);
            model = glm::scale(model, glm::vec3(p3_scale));
        }
        else {
            model = glm::scale(model, glm::vec3(p3_scale));    // it's a bit too big for our scene, so scale it down
        }
        obicanPadobranShader.setMat4("model", model);
        padobranModel.Draw(obicanPadobranShader);
        obicanPadobranShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);        //vracamo ambijentalnu da ne bi uticalo na druge padobrane





        sjajShader.use();   ///-----------------------------------------------------------------------------------------------sjajShader
        setLights(sjajShader);
        sjajShader.setFloat("material.shininess", 100.0f);
        // view/projection transformations
        projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();
        sjajShader.setMat4("projection", projection);
        sjajShader.setMat4("view", view);
        sjajShader.setInt("blinn", blinn);



        //render zvezda1 rotirajuca
        if(!pojedi_p1){                                     //ako nije pojeden odgovarajuci padobranac nema jos punu boju
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sin(glfwGetTime() / 2) * 17, -3.0f, cos(glfwGetTime() / 2) * 17));
            //model = glm::translate(model, glm::vec3(4.0 * cos(currentFrame), 3.0f, 4.0 * sin(currentFrame)));
            model = glm::rotate(model, (float)glfwGetTime()/2-82, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(2.0f));
            sjajShader.setMat4("model", model);
            zvezdaModel.Draw(sjajShader);
        }
        else{                          //zvezda1 sa punom bojom -> sjaj shaderom kada je zavrseno skaliranje padobrana
            sjajShader.setVec3("dirLight.ambient", 1.0f, 1.0f, 1.0f);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sin(glfwGetTime() / 2) * 17, -3.0f, cos(glfwGetTime() / 2) * 17));
            //model = glm::translate(model, glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame)));
            model = glm::rotate(model, (float)glfwGetTime()*4, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(2.0f));
            sjajShader.setMat4("model", model);
            zvezdaModel.Draw(sjajShader);
            sjajShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        }//-----------------------------------------------------------------------------

        //render zvezda2 rotirajuca
        if(!pojedi_p2){
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sin(glfwGetTime() / 4) * 14.5, -9.0f, cos(glfwGetTime() / 2) * 14.5));
            model = glm::rotate(model, (float)glfwGetTime()/2-82, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f));
            sjajShader.setMat4("model", model);
            zvezdaModel.Draw(sjajShader);
        }
        else{                  //zvezda2 sa punom bojom -> sjaj shaderom kada je zavrseno skaliranje padobrana
            sjajShader.setVec3("dirLight.ambient", 1.0f, 1.0f, 1.0f);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sin(glfwGetTime() / 4) * 14.5, -9.0f, cos(glfwGetTime() / 2) * 14.5));
            model = glm::rotate(model, (float)glfwGetTime()*4, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f));
            sjajShader.setMat4("model", model);
            zvezdaModel.Draw(sjajShader);
            sjajShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        }//-----------------------------------------------------------------------------

        //render zvezda3 rotirajuca
        if(!pojedi_p3){
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-sin(glfwGetTime() / 4) * 4, -9.0f, -cos(glfwGetTime() / 2) * 4));
            model = glm::rotate(model, (float)glfwGetTime()*3, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f));
            sjajShader.setMat4("model", model);
            zvezdaModel.Draw(ourShader);
        }
        else{                      //zvezda3 sa punom bojom -> sjaj shaderom kada je zavrseno skaliranje padobrana
            sjajShader.setVec3("dirLight.ambient", 1.0f, 1.0f, 1.0f);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-sin(glfwGetTime() / 4) * 4, -9.0f, -cos(glfwGetTime() / 2) * 4));
            model = glm::rotate(model, (float)glfwGetTime()*4, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f));
            sjajShader.setMat4("model", model);
            zvezdaModel.Draw(sjajShader);
            sjajShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        }//-----------------------------------------------------------------------------
        //                                           -> coin
        if(efekat1 && efekat2 && efekat3){
            sjajShader.setVec3("dirLight.ambient", 1.0f, 1.0f, 1.0f);
            model = glm::mat4(1.0f);
            model = glm::translate(model,glm::vec3(0.0f,-4.0f,0.0f));
            model = glm::rotate(model, (float)glfwGetTime()*4, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5));
            sjajShader.setMat4("model", model);
            coinModel.Draw(sjajShader);
            sjajShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        }


        //--------------------------------------------------------------------------------------------------------------------------------------------iscrtani modeli

        glDisable(GL_CULL_FACE);

        // box
        // bind diffuse map
        if(!(efekat1 && efekat2 && efekat3)){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, boxDiffuse);

            // bind specular map
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, boxSpecular);

            // bind emission map
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, boxAmbient);
            boxShader.use();
            setLights(boxShader);
            boxShader.setFloat("material.shininess", 64.0f);
            boxShader.setMat4("projection", projection);
            boxShader.setMat4("view", view);
            glBindVertexArray(boxVAO);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -4.0f, 0.0f));
            model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, -1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f));
            boxShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        cloudShader.use();                                                          //transparent cloud
        cloudShader.setMat4("projection",projection);
        cloudShader.setMat4("view",view);
        glBindVertexArray(transparentVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);

        for (const glm::vec3& c : clouds)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, c);
           // model = glm::rotate(model, glm::radians(-90.0f),glm::vec3(0.0f,1.0f,0.0f));
            model = glm::scale(model,glm::vec3(15.0f));
            cloudShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);

        }
        glEnable(GL_CULL_FACE);

        //--------------------------------------------


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
          // set depth function back to default

        //load ping-pong
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        bloomShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            bloomShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
            renderQuad();

            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }

        // load hdr and bloom
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        hdrShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        hdrShader.setBool("hdr", hdr);
        hdrShader.setBool("bloom",bloom);
        hdrShader.setFloat("exposure", exposure);
        renderQuad();
        ///----

        if (programState->ImGuiEnabled)
            DrawImGui(programState);                                                //crtamo nas ImGui



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);                //jedan baffer koristi za pisanje drugi za citanje
        glfwPollEvents();
    }///-----------------------------------------------------------------------------------------------------------------------------------------------------Kraj render petlje


    glDeleteVertexArrays(1, &boxVAO);
    glDeleteBuffers(1, &boxVBO);
    glDeleteVertexArrays(1, &transparentVAO);
    glDeleteBuffers(1, &transparentVBO);                                        //da ne bi bilo curenja memorije
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);

    programState->SaveToFile("resources/program_state.txt");                        //pamtimo stanje programa u kom smo ga ostavili prethodni put
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();                                                           //gasimo nas ImGui
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
        ImGui::Begin("Enabled effect");
        ImGui::Text("B = %s\n",(active_p1)?"TRUE":"false");
        ImGui::Text("N = %s\n",(active_p2)?"TRUE":"false");
        ImGui::Text("M = %s\n",(active_p3)?"TRUE":"false");
        ImGui::Text("za RESTART -> %s\n",(efekat1 && efekat2 && efekat3)?"klikni R":"ISKORISTI PRVO SVE OPCIJE 'B','N','M'");
        ImGui::DragFloat("pointLight.constant", &pointLightconstant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &pointLightquadratic, 0.05, 0.0, 1.0);
        ImGui::Text("Press 'O' for SPOTLIGHT    |   'P' for BLINN\n");
        ImGui::Text("Press '1' for HDR          |   'P' for Bloom\n");
        ImGui::Text("Press 'Q' for exposure (-) |  'E' for exposure (+) \n");
        ImGui::DragFloat("exposure", &exposure, 0.05, 0.0, 1.0);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_I && action == GLFW_PRESS) {                                                        // klik na I za ImGui meni
        programState->ImGuiEnabled = !programState->ImGuiEnabled;                                           //dodeljujemo suprotnu vrednost u odnosu na proslu
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);                        //ako je ImGui iskljucen -> nemamo prikazan mis
        }
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {                          //za ispisivanje trenutnih kordinata (koristio sam kao pomoc pri postavljanju modela na scenu)
        cout << programState->camera.Position.x << " "
             << programState->camera.Position.y << " "
             << programState->camera.Position.z << '\n';
    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        active_p1=true;
    }
    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        active_p2=true;
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        active_p3=true;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS && efekat1 && efekat2 && efekat3) {                                    //restartuje /igricu/ animaciju
        active_p1=false;
        active_p2=false;
        active_p3=false;
        pojedi_p1=false;
        pojedi_p2=false;
        pojedi_p3=false;
        efekat1=false;
        efekat2=false;
        efekat3=false;
        lokalna1=false;
        lokalna2=false;
        lokalna3=false;
        p1_scale = 2.0;
        p2_scale = 1.0;
        p3_scale = 1.0;
        pointLightconstant = 0.09f;
        pointLightquadratic = 0.039f;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        blinn = !blinn;

    }
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        spotlightOn = !spotlightOn;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !hdrKeyPressed)
    {
        hdr = !hdr;
        hdrKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE)
    {
        hdrKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (exposure > 0.0f)
            exposure -= 0.005f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        exposure += 0.005f;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !bloomKeyPressed)
    {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
    {
        bloomKeyPressed = false;
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

bool pribliziFast(float& s,float k){
    if(s >= k-0.1 && s <= k+0.1)
        return true;
    if(s < k)
        s += 0.1;
    else
        s -= 0.1;
    return false;
}

void setLights(Shader shaderName){
    shaderName.setVec3("light.position", lightPos);
    shaderName.setVec3("viewPos", programState->camera.Position);

    // directional light
    shaderName.setVec3("dirLight.direction", 0.0f, -1.0, 0.0f);
    shaderName.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    shaderName.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shaderName.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    //pointlight properties
    shaderName.setVec3("pointLights[0].position", lightPos);
    shaderName.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    shaderName.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    shaderName.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    shaderName.setFloat("pointLights[0].constant", 1.0f);
    shaderName.setFloat("pointLights[0].linear", 0.09f);
    shaderName.setFloat("pointLights[0].quadratic", 0.032f);
    // spotLight
    shaderName.setVec3("spotLight.position", programState->camera.Position);
    shaderName.setVec3("spotLight.direction", programState->camera.Front);
    shaderName.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    if(spotlightOn){
        shaderName.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        shaderName.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    }
    else{
        shaderName.setVec3("spotLight.diffuse", 0.0f, 0.0f, 0.0f);
        shaderName.setVec3("spotLight.specular", 0.0f, 0.0f, 0.0f);
    }

    shaderName.setFloat("spotLight.constant", 0.0f);
    shaderName.setFloat("spotLight.linear", 0.09f);
    shaderName.setFloat("spotLight.quadratic", 0.0f);
    shaderName.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shaderName.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}


unsigned int loadTexture(char const * path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}



unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}