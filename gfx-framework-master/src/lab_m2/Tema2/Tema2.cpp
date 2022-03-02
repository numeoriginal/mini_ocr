#include "lab_m2/Tema2/Tema2.h"

#include <vector>
#include <iostream>
#include <queue>
#include <list>
#include "pfd/portable-file-dialogs.h"
#include <cmath>

using namespace std;
using namespace m2;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema2::Tema2()
{
    outputMode = 0;
    gpuProcessing = false;
    saveScreenToImage = false;
    window->SetSize(600, 600);
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{
    // Load default texture fore imagine processing
    originalImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube", "imagine.png"), nullptr, "image", true, true);
    processedImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube", "imagine.png"), nullptr, "newImage", true, true);

    {
        Mesh* mesh = new Mesh("quad");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema2", "shaders");

    // Create a shader program for particle system
    {
        Shader *shader = new Shader("ImageProcessing");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "FragmentShader.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    window->SetSize(originalImage->GetWidth(), originalImage->GetHeight());
    outputMode = 1;
}


void Tema2::FrameStart()
{
    
}


void Tema2::Update(float deltaTimeSeconds)
{
   
    ClearScreen();
    auto shader = shaders["ImageProcessing"];
    shader->Use();

    if (saveScreenToImage)
    {
        window->SetSize(originalImage->GetWidth(), originalImage->GetHeight());
    }

    int flip_loc = shader->GetUniformLocation("flipVertical");
    glUniform1i(flip_loc, saveScreenToImage ? 0 : 1);

    int screenSize_loc = shader->GetUniformLocation("screenSize");
    glm::ivec2 resolution = window->GetResolution();
    glUniform2i(screenSize_loc, originalImage->GetWidth(), originalImage->GetHeight());

    int outputMode_loc = shader->GetUniformLocation("outputMode");
    glUniform1i(outputMode_loc, outputMode);

    int locTexture = shader->GetUniformLocation("textureImage");
    glUniform1i(locTexture, 0);


    auto textureImage = (gpuProcessing == true) ? originalImage : processedImage;
    textureImage->BindToTextureUnit(GL_TEXTURE0);

    RenderMesh(meshes["quad"], shader, glm::mat4(1));

    if (saveScreenToImage)
    {
        saveScreenToImage = false;

        GLenum format = GL_RGB;
        if (originalImage->GetNrChannels() == 4)
        {
            format = GL_RGBA;
        }

        glReadPixels(0, 0, originalImage->GetWidth(), originalImage->GetHeight(), format, GL_UNSIGNED_BYTE, processedImage->GetImageData());
        processedImage->UploadNewData(processedImage->GetImageData());
        SaveImage(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube", "shader_processing_"));

        float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
        //window->SetSize(static_cast<int>(600 * aspectRatio), 600);
        originalImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube", "shader_processing_.png"), nullptr, "newImage", true, true);
        processedImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube", "shader_processing_.png"), nullptr, "newImage", true, true);
    }


}



void Tema2::FrameEnd()
{
    DrawCoordinateSystem();
}


void Tema2::OnFileSelected(const std::string &fileName)
{
    if (fileName.size())
    {
        std::cout << fileName << endl;
        originalImage = TextureManager::LoadTexture(fileName, nullptr, "image", true, true);
        processedImage = TextureManager::LoadTexture(fileName, nullptr, "newImage", true, true);

        float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
        window->SetSize(originalImage->GetWidth(), originalImage->GetHeight());
    }
}

void Tema2::InitialImage() {
    unsigned char* data = originalImage->GetImageData();
    processedImage->UploadNewData(data);
}

queue<int> pxQue;
int id = 0;
int imj[1080][1920] = { 0 };

void Tema2::BFS() {

    //id++;
    unsigned int channels = originalImage->GetNrChannels();
    unsigned char* data = processedImage->GetImageData();
    
    glm::ivec2 image = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());
  
    while (!pxQue.empty()) {

        int offset = pxQue.front();
        //cout << offset << endl;
        
        int y = (offset / channels) / image.x;
        int x = (offset / channels) % image.x;

        pxQue.pop();

        if (imj[y][x - 1] == 0 && data[offset - channels] == 255 && data[offset + 1 - channels] == 255 && data[offset + 2 - channels] == 255) { // stanga
            imj[y][x - 1] = id;
            pxQue.push(offset - channels);
        }

        if (imj[y + 1][x] == 0 &&  data[offset + image.x * channels] == 255 &&
            data[offset + 1 + image.x * channels] == 255 && data[offset + 2 + image.x * channels] == 255) { // jos
            imj[y + 1][x] = id;
            pxQue.push(offset + image.x * channels);
       
        }
        if (imj[y][x + 1] == 0 && data[offset + channels] == 255 && data[offset + channels + 1] == 255 && data[offset + channels + 2] == 255 ) { // dreapta
            imj[y][x + 1] = id;
            pxQue.push(offset + channels);
        
        }
        
        if (imj[y - 1][x] == 0 &&  data[offset - image.x * channels] == 255 && data[offset + 1 - image.x * channels] == 255 &&
            data[offset + 2 - image.x * channels] == 255) { // sus
            imj[y - 1][x] = id;
            pxQue.push(offset - image.x * channels);
          
        }
        
       
            memset(&data[offset + 1], 0, 2);
      
    }
    processedImage->UploadNewData(data);
}

void Tema2::Find_letters() {
    unsigned int channels = originalImage->GetNrChannels();
    unsigned char* data = processedImage->GetImageData();
    int i = 0;
    int j = 0;
    int contor = 0;
    if (channels < 3)
        return;

    glm::ivec2 image = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

    for (i = 0; i < image.y; ++i)
    {
        for (j = 0; j < image.x; ++j)
        {
            int offset = channels * (i * image.x + j);

            if (imj[i][j] == 0 && data[offset] == 255 && data[offset + 1] == 255 && data[offset + 2] == 255)
            {
                id++;
                contor++;
                //cout << " Find" << i << " " << j << endl;
                pxQue.push(offset);
                imj[i][j] = id;
                BFS();
            }
        }
    }
    cout << contor;
    
}
glm::vec3 Tema2::rgb2hsv(float& fR, float& fG, float& fB) {
    float fCMax = max(max(fR, fG), fB);
    float fCMin = min(min(fR, fG), fB);
    float fDelta = fCMax - fCMin;
    glm::vec3 hsv;

    if (fDelta > 0) {
        if (fCMax == fR) {
            hsv.x = 60 * (fmod(((fG - fB) / fDelta), 6));
        }
        else if (fCMax == fG) {
            hsv.x = 60 * (((fB - fR) / fDelta) + 2);
        }
        else if (fCMax == fB) {
            hsv.x = 60 * (((fR - fG) / fDelta) + 4);
        }

        if (fCMax > 0) {
            hsv.y = fDelta / fCMax;
        }
        else {
            hsv.y = 0;
        }

        hsv.z = fCMax;
    }
    else {
        hsv.x = 0;
        hsv.y = 0;
        hsv.z = fCMax;
    }

    if (hsv.x < 0) {
        hsv.x += 360;
    }

    return hsv;
}

void Tema2::HSV()
{
    unsigned int channels = originalImage->GetNrChannels();
    unsigned char* data = processedImage->GetImageData();
    unsigned char* newData = processedImage->GetImageData();

    if (channels < 3)
        return;
   
    glm::ivec2 image = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

    glm::vec3 aux;

    for (int i = 0; i < image.y ; i++)
    {
        for (int j = 0; j < image.x ; j++)
        {
            int offset = channels * (i * image.x + j);
      
            // Reset save image data
            float r, g, b;
            r = data[offset];
            g = data[offset + 1];
            b = data[offset + 2];
            aux = rgb2hsv(r, g, b);
            
            
            if (aux.x > 80 && aux.x < 139) {
                memset(&newData[offset], 255, 3);
                //cout << aux.x << ' ' << endl;
            }
            else {
                memset(&newData[offset], 0, 3);
            }
            
        }
    }
    processedImage->UploadNewData(newData);
}



void Tema2::Median() {

    unsigned int channels = originalImage->GetNrChannels();
    unsigned char* data = originalImage->GetImageData();
    unsigned char* newData = processedImage->GetImageData();

    if (channels < 3)
        return;
    glm::ivec2 image = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

    vector<char> aux;

    for (int i = 1; i < image.y - 1; i++)
    {
        for (int j = 1; j < image.x - 1; j++)
        {
            int offset = channels * (i * image.x + j);
            char result;
            // Reset save image data


            for (int X = 0; X < 3; X++) {

                aux.clear();
                aux.push_back(data[offset]); // img[i][j]
                aux.push_back(data[offset + channels]);
                aux.push_back(data[offset - channels]);
                aux.push_back(data[offset + channels * image.x]);
                aux.push_back(data[offset + channels * image.x + channels]);
                aux.push_back(data[offset + channels * image.x - channels]);
                aux.push_back(data[offset - channels * image.x]);
                aux.push_back(data[offset - channels * image.x + channels]);
                aux.push_back(data[offset - channels * image.x - channels]);
                sort(aux.begin(), aux.end());
                result = aux[5];
                memset(&newData[offset], result, 1);
                offset++;
            }

        }
    }
    processedImage->UploadNewData(newData);

}


void Tema2::SaveImage(const std::string &fileName)
{
    cout << "Saving image! ";
    processedImage->SaveToFile((fileName + ".png").c_str());
    cout << "[Done]" << endl;
}


void Tema2::OpenDialog()
{
    std::vector<std::string> filters =
    {
        "Image Files", "*.png *.jpg *.jpeg *.bmp",
        "All Files", "*"
    };

    auto selection = pfd::open_file("Select a file", ".", filters).result();
    if (!selection.empty())
    {
        std::cout << "User selected file " << selection[0] << "\n";
        OnFileSelected(selection[0]);
    }
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input
}


void Tema2::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
    {
        OpenDialog();
    }

    if (key == GLFW_KEY_E)
    {
        gpuProcessing = !gpuProcessing;
        if (gpuProcessing == false)
        {
            outputMode = 0;
        }
        cout << "Processing on GPU: " << (gpuProcessing ? "true" : "false") << endl;
    }

    if (key - GLFW_KEY_1 == 0)
    {
        InitialImage();
    }
    if (key - GLFW_KEY_2 == 0)
    {
        cout << "Median\n";
        Median();
    }
    if (key - GLFW_KEY_3 == 0)
    {
        cout << "Find letters\n";
        Find_letters();
    }
    if (key - GLFW_KEY_4 == 0)
    {
        cout << "HSV\n";
        HSV();
    }

    if (key - GLFW_KEY_5 == 0)
    {
        cout << "Median & HSB GPU \n";
        outputMode = 5;
       
    }

    if (key == GLFW_KEY_S && mods & GLFW_MOD_CONTROL)
    {
        if (!gpuProcessing)
        {
            SaveImage("processCPU_" + std::to_string(outputMode));
        } else {
            saveScreenToImage = true;
        }
    }
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Tema2::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
